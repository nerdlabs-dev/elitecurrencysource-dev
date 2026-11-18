#!/usr/bin/env python3
"""Make Elite block and stake modifier checkpoints from a fully synced node.

Run it on a machine whose Elite wallet or daemon is fully synced and has
RPC turned on (server=1, rpcuser and rpcpassword in 1337.conf):

    python3 contrib/checkpoints/make-checkpoints.py

It reads the RPC login from 1337.conf and asks the node for the block hash
and stake modifier checksum every 25,000 blocks. The new lists replace all
the existing checkpoints. Run from the source tree, it first checks that
every existing checkpoint in src/checkpoints.cpp and src/kernel.cpp matches
what the node has. Then it writes checkpoints.txt with both lists ready to
paste.

It also works on its own, without the source tree (for example downloaded
next to a wallet): then it can't compare against the existing checkpoints,
so have them checked before the new ones go in.

With --apply it also writes the new lists into src/checkpoints.cpp and
src/kernel.cpp. It refuses if any existing checkpoint doesn't match: that
would mean the node is on a different chain from the one the wallet was
built for, and the result must not be used.

Only Python 3 is needed; it talks to the node over its RPC port.

Options:
  --datadir DIR    Elite data folder (default: ~/.1337, %APPDATA%\\1337 on
                   Windows, ~/Library/Application Support/1337 on a Mac)
  --conf FILE      config file (default: 1337.conf in the data folder)
  --rpcconnect IP  node address (default: from the config, else 127.0.0.1)
  --rpcport N      RPC port (default: from the config, else 13372)
  --interval N     blocks between checkpoints (default: 25000)
  --depth N        leave at least this many blocks above the last
                   checkpoint (default: 5000), so a recent reorganisation
                   can't end up in a checkpoint
  --repo DIR       source tree to check and update (default: the one this
                   script is in)
  --apply          write the new lists into the source files
"""

import argparse
import base64
import http.client
import json
import os
import re
import sys
import time


# ---------------------------------------------------------------- RPC

class RPC:
    def __init__(self, host, port, user, password):
        self.host, self.port = host, port
        self.auth = "Basic " + base64.b64encode(("%s:%s" % (user, password)).encode()).decode()
        self.conn = None
        self.nId = 0

    def _post(self, body):
        for attempt in range(3):
            try:
                if self.conn is None:
                    self.conn = http.client.HTTPConnection(self.host, self.port, timeout=120)
                self.conn.request("POST", "/", body, {"Authorization": self.auth,
                                                      "Content-Type": "application/json"})
                resp = self.conn.getresponse()
                data = resp.read()
                if resp.status == 401:
                    sys.exit("The node refused the RPC login: check rpcuser and rpcpassword in the config file.")
                return json.loads(data)
            except (OSError, http.client.HTTPException) as e:
                self.conn = None
                if attempt == 2:
                    sys.exit("Can't reach the node's RPC at %s:%s (%s). Is it running with server=1?"
                             % (self.host, self.port, e))
                time.sleep(2)

    def call(self, method, *params):
        self.nId += 1
        reply = self._post(json.dumps({"method": method, "params": list(params), "id": self.nId}))
        if reply.get("error"):
            sys.exit("RPC %s failed: %s" % (method, reply["error"]))
        return reply["result"]

    def batch(self, method, paramsList, nChunk=200):
        """Call method once per params, in chunks; results in the same order."""
        results = []
        for i in range(0, len(paramsList), nChunk):
            chunk = paramsList[i:i + nChunk]
            reqs = []
            for p in chunk:
                self.nId += 1
                reqs.append({"method": method, "params": list(p), "id": self.nId})
            replies = self._post(json.dumps(reqs))
            if not isinstance(replies, list):
                # an old node without batch support: one at a time
                results.extend(self.call(method, *p) for p in chunk)
                continue
            byId = {r["id"]: r for r in replies}
            for req in reqs:
                r = byId.get(req["id"])
                if r is None or r.get("error"):
                    sys.exit("RPC %s%s failed: %s" % (method, tuple(req["params"]), r and r.get("error")))
                results.append(r["result"])
            print("  %s: %d of %d" % (method, min(i + nChunk, len(paramsList)), len(paramsList)), end="\r", flush=True)
        print()
        return results


# ---------------------------------------------------------------- config

def default_datadir():
    if sys.platform.startswith("win"):
        return os.path.join(os.environ.get("APPDATA", ""), "1337")
    if sys.platform == "darwin":
        return os.path.expanduser("~/Library/Application Support/1337")
    return os.path.expanduser("~/.1337")


def read_conf(path):
    conf = {}
    try:
        with open(path) as f:
            for line in f:
                line = line.split("#", 1)[0].strip()
                if "=" in line:
                    k, v = line.split("=", 1)
                    conf[k.strip()] = v.strip()
    except OSError as e:
        sys.exit("Can't read %s (%s). Use --datadir or --conf to point at your 1337.conf." % (path, e))
    return conf


# ---------------------------------------------------------------- source files

# The mainnet maps are the first map_list_of after these names; the
# testnet maps have their own names and are left alone.
CHECKPOINT_MAP = ("src/checkpoints.cpp", "mapCheckpoints =")
MODIFIER_MAP = ("src/kernel.cpp", "mapStakeModifierCheckpoints =")


def read_text(path):
    with open(path, "rb") as f:
        raw = f.read().decode("utf-8")
    newline = "\r\n" if "\r\n" in raw else "\n"
    return raw.replace("\r\n", "\n"), newline


def map_span(text, name, path):
    """(start, end) of the entries between map_list_of and the closing ';'."""
    i = text.find(name)
    if i < 0:
        sys.exit("Can't find %s in %s" % (name, path))
    j = text.find("map_list_of", i)
    k = text.find(";", j)
    if j < 0 or k < 0:
        sys.exit("Can't find the list after %s in %s" % (name, path))
    return j + len("map_list_of"), k


def existing_checkpoints(repo):
    path = os.path.join(repo, CHECKPOINT_MAP[0])
    text, _ = read_text(path)
    a, b = map_span(text, CHECKPOINT_MAP[1], path)
    found = {}
    for m in re.finditer(r'\(\s*(\d+)\s*,\s*(?:uint256\("0x([0-9a-fA-F]{64})"\)|hashGenesisBlock)\s*\)', text[a:b]):
        found[int(m.group(1))] = m.group(2).lower() if m.group(2) else None   # None = genesis
    return found


def existing_modifiers(repo):
    path = os.path.join(repo, MODIFIER_MAP[0])
    text, _ = read_text(path)
    a, b = map_span(text, MODIFIER_MAP[1], path)
    return {int(m.group(1)): int(m.group(2), 16)
            for m in re.finditer(r'\(\s*(\d+)\s*,\s*0x([0-9a-fA-F]{1,8})\s*\)', text[a:b])}


def checkpoint_lines(points):
    out = []
    for h in sorted(points):
        if h == 0:
            out.append("        ( %7d, hashGenesisBlock )" % h)
        else:
            out.append('        ( %7d, uint256("0x%s"))' % (h, points[h]))
    return out


def modifier_lines(mods):
    return ["        ( %7d, 0x%08x )" % (h, mods[h]) for h in sorted(mods)]


def write_map(repo, where, lines):
    path = os.path.join(repo, where[0])
    text, newline = read_text(path)
    a, b = map_span(text, where[1], path)
    text = text[:a] + "\n" + "\n".join(lines) + "\n    " + text[b:]
    with open(path, "wb") as f:
        f.write(text.replace("\n", newline).encode("utf-8"))


# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser(description="Make Elite checkpoints from a fully synced node.")
    ap.add_argument("--datadir", default=default_datadir())
    ap.add_argument("--conf")
    ap.add_argument("--rpcconnect")
    ap.add_argument("--rpcport", type=int)
    ap.add_argument("--interval", type=int, default=25000)
    ap.add_argument("--depth", type=int, default=5000)
    ap.add_argument("--repo", default=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()

    conf = read_conf(args.conf or os.path.join(args.datadir, "1337.conf"))
    if "rpcuser" not in conf or "rpcpassword" not in conf:
        sys.exit("1337.conf has no rpcuser/rpcpassword. Add them (and server=1), restart the node, and run this again.")
    if conf.get("testnet", "0") not in ("", "0"):
        sys.exit("This config is for testnet; checkpoints have to come from mainnet.")
    rpc = RPC(args.rpcconnect or conf.get("rpcconnect", "127.0.0.1"),
              args.rpcport or int(conf.get("rpcport", 13372)),
              conf["rpcuser"], conf["rpcpassword"])

    fHaveSource = os.path.exists(os.path.join(args.repo, CHECKPOINT_MAP[0])) and \
                  os.path.exists(os.path.join(args.repo, MODIFIER_MAP[0]))
    if fHaveSource:
        old_points = existing_checkpoints(args.repo)
        old_mods = existing_modifiers(args.repo)
    else:
        if args.apply:
            sys.exit("--apply needs the source tree: run this from the repo, or pass --repo.")
        print("No source tree here, so the existing checkpoints can't be compared; have them checked before use.")
        old_points, old_mods = {}, {}

    tip = rpc.call("getblockcount")
    last = (tip - args.depth) // args.interval * args.interval
    if last <= 0:
        sys.exit("The node is only at block %d; let it sync first." % tip)
    print("Node is at block %d; checkpoints every %d blocks up to %d." % (tip, args.interval, last))

    # A node still syncing would give checkpoints for whatever it has so far
    info = rpc.call("getinfo")
    if info.get("connections", 0) == 0:
        print("Warning: the node has no connections, so it can't tell whether it's fully synced.")

    heights = sorted(set(range(0, last + 1, args.interval)) |
                     {h for h in old_points if h <= last} | {h for h in old_mods if h <= last})
    hashes = rpc.batch("getblockhash", [(h,) for h in heights])
    blocks = rpc.batch("getblock", [(x,) for x in hashes], nChunk=100)

    # The new lists hold only the every-interval heights; the old heights are
    # fetched just to check them
    points, mods, mismatches = {}, {}, []
    for h, x, blk in zip(heights, hashes, blocks):
        if blk.get("height") != h or blk.get("hash") != x:
            sys.exit("The node returned block %s for height %d; it may have reorganised mid-run. Run again." % (blk.get("hash"), h))
        x = x.lower()
        mod = int(blk["modifierchecksum"], 16)
        if h in old_points and old_points[h] is not None and old_points[h] != x:
            mismatches.append("block %d: wallet has %s, node has %s" % (h, old_points[h], x))
        if h in old_mods and old_mods[h] != mod:
            mismatches.append("stake modifier %d: wallet has 0x%08x, node has 0x%08x" % (h, old_mods[h], mod))
        if h % args.interval == 0:
            points[h], mods[h] = x, mod
    nAbove = sum(1 for h in old_points if h > last) + sum(1 for h in old_mods if h > last)
    if nAbove:
        print("Note: %d existing checkpoint(s) are above block %d; the new lists replace them too." % (nAbove, last))

    nOldPoints = sum(1 for h in old_points if h <= last)
    nOldMods = sum(1 for h in old_mods if h <= last)
    out = "checkpoints.txt"
    with open(out, "w") as f:
        f.write("// Elite checkpoints to block %d, every %d blocks\n" % (last, args.interval))
        f.write("// Read from a node at block %d on %s\n" % (tip, time.strftime("%Y-%m-%d %H:%M UTC", time.gmtime())))
        f.write("// %d of %d existing block checkpoints and %d of %d existing stake modifier checkpoints matched\n"
                % (nOldPoints - sum("block" in s for s in mismatches), nOldPoints,
                   nOldMods - sum("modifier" in s for s in mismatches), nOldMods))
        for s in mismatches:
            f.write("// MISMATCH %s\n" % s)
        f.write("\n// src/checkpoints.cpp, mapCheckpoints\n")
        f.write("\n".join(checkpoint_lines(points)) + "\n")
        f.write("\n// src/kernel.cpp, mapStakeModifierCheckpoints\n")
        f.write("\n".join(modifier_lines(mods)) + "\n")

    print("%d block checkpoints and %d stake modifier checkpoints written to %s" % (len(points), len(mods), out))
    if not fHaveSource:
        print("Existing checkpoints weren't compared (no source tree here).")
        return
    print("Existing checkpoints checked: %d block, %d stake modifier" % (nOldPoints, nOldMods))
    if mismatches:
        print("\nThese existing checkpoints DON'T match the node:")
        for s in mismatches:
            print("  " + s)
        sys.exit("\nNot safe to use: the node is on a different chain from the wallet's checkpoints.")
    print("All existing checkpoints match.")

    if args.apply:
        write_map(args.repo, CHECKPOINT_MAP, checkpoint_lines(points))
        write_map(args.repo, MODIFIER_MAP, modifier_lines(mods))
        print("Written into %s and %s." % (CHECKPOINT_MAP[0], MODIFIER_MAP[0]))


if __name__ == "__main__":
    main()
