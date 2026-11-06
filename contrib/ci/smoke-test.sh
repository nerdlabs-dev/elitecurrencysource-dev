#!/usr/bin/env bash
# Quick check that a freshly built 1337d works, used by the GitHub Actions build.
#   contrib/ci/smoke-test.sh path/to/1337d[.exe]
#
# Starts the daemon on its own, with no network, in a throwaway data directory,
# then over RPC: creates addresses, signs and verifies a message (OpenSSL key
# code), exports and re-imports a private key, encrypts the wallet and signs
# with it unlocked (wallet encryption code), and checks that testnet starts
# (its genesis block). Fails on the first thing that doesn't work.
set -euo pipefail

BIN="$1"
WORK="$(mktemp -d 2>/dev/null || mktemp -d -t elite-smoke)"
# Windows binaries want a Windows path for -datadir
if command -v cygpath >/dev/null 2>&1; then DIR="$(cygpath -m "$WORK")"; else DIR="$WORK"; fi
RPCPORT=$((20000 + RANDOM % 20000))
PORT=$((RPCPORT + 1))
PIDS=()

conf() { # conf <datadir> <extra lines>
    mkdir -p "$1"
    printf 'rpcuser=smoke\nrpcpassword=smoke-%s\nrpcport=%s\nport=%s\nlisten=0\nconnect=127.0.0.1:1\ndnsseed=0\nupnp=0\nprinttoconsole=0\n%s' \
        "$RANDOM$RANDOM" "$RPCPORT" "$PORT" "$2" > "$1/1337.conf"
}
cli() { "$BIN" -datadir="$CUR" -conf="$CUR/1337.conf" "$@"; }
start() { # start <datadir>; runs in the background (-daemon isn't available on Windows)
    CUR="$1"
    "$BIN" -datadir="$CUR" -conf="$CUR/1337.conf" -server > "$WORK/stdout.log" 2>&1 &
    PIDS+=($!)
    local pid=$!
    for i in $(seq 1 120); do
        if cli getinfo > /dev/null 2>&1; then return 0; fi
        kill -0 "$pid" 2>/dev/null || { echo "daemon exited during startup"; break; }
        sleep 1
    done
    echo "daemon did not answer RPC"
    tail -n 50 "$CUR"/debug.log "$CUR"/db.log "$CUR"/testnet/debug.log "$CUR"/testnet/db.log "$WORK/stdout.log" 2>/dev/null || true
    return 1
}
stop() {
    cli stop > /dev/null 2>&1 || true
    for i in $(seq 1 60); do
        if ! cli getinfo > /dev/null 2>&1; then sleep 2; return 0; fi
        sleep 1
    done
    echo "daemon did not stop"; return 1
}
cleanup() {
    for p in "${PIDS[@]:-}"; do [ -n "$p" ] && kill "$p" 2>/dev/null || true; done
    rm -rf "$WORK"
}
trap cleanup EXIT
check() { # check <description> <command...>
    if "${@:2}"; then echo "ok   - $1"; else echo "FAIL - $1"; exit 1; fi
}

echo "== $("$BIN" -? 2>&1 | head -n 1)"

# --- main network, no peers
CUR="$DIR/main"; conf "$CUR" ""
start "$CUR"
cli getinfo | grep -E '"version"|"protocolversion"|"blocks"'
ADDR=$(cli getnewaddress | tr -d '\r')
SIG=$(cli signmessage "$ADDR" "smoke test" | tr -d '\r')
check "sign and verify a message" test "$(cli verifymessage "$ADDR" "$SIG" "smoke test" | tr -d '\r')" = "true"
check "tampered message is rejected" test "$(cli verifymessage "$ADDR" "$SIG" "smoke tesT" | tr -d '\r')" = "false"
KEY=$(cli dumpprivkey "$ADDR" | tr -d '\r')
stop

# --- second wallet: import that key, then encrypt the wallet
CUR="$DIR/import"; conf "$CUR" ""
start "$CUR"
cli importprivkey "$KEY" imported > /dev/null
check "imported key gives the same address" sh -c "\"$BIN\" -datadir=\"$CUR\" -conf=\"$CUR/1337.conf\" validateaddress $ADDR | grep -q '\"ismine\" : true'"
cli encryptwallet "smoke-passphrase" > /dev/null || true
for i in $(seq 1 60); do cli getinfo > /dev/null 2>&1 || break; sleep 1; done
sleep 2
start "$CUR"
check "wrong passphrase is refused" sh -c "! \"$BIN\" -datadir=\"$CUR\" -conf=\"$CUR/1337.conf\" walletpassphrase wrong 30 > /dev/null 2>&1"
cli walletpassphrase "smoke-passphrase" 30
SIG2=$(cli signmessage "$ADDR" "after encryption" | tr -d '\r')
check "sign with the decrypted key" test "$(cli verifymessage "$ADDR" "$SIG2" "after encryption" | tr -d '\r')" = "true"
stop

# --- testnet starts (genesis block)
CUR="$DIR/test"; conf "$CUR" "testnet=1
"
start "$CUR"
check "testnet starts at height 0" test "$(cli getblockcount | tr -d '\r')" = "0"
stop

echo "all smoke tests passed"
