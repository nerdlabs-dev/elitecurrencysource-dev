#!/usr/bin/env node
// Checks that a running 1337d answers the RPC calls the eIquidus block
// explorer makes, with the parameter types it uses and the fields it reads.
//
//   node contrib/ci/explorer-compat-test.js <host> <port> <user> <password>
//
// Plain Node.js, no packages. Works on a fresh chain (only the genesis block);
// with more blocks it also checks transactions, addresses and descriptors.
// Exits non-zero on the first failure.
'use strict';
const http = require('http');

const [host, port, user, pass] = process.argv.slice(2);
if (!pass) {
  console.error('usage: explorer-compat-test.js <host> <port> <user> <password>');
  process.exit(2);
}

let failures = 0;
let id = 0;

// JSON-RPC call; resolves with { result, raw } or rejects with the RPC error
function rpc(method, params = []) {
  const body = JSON.stringify({ jsonrpc: '1.0', id: ++id, method, params });
  return new Promise((resolve, reject) => {
    const req = http.request({
      host, port, method: 'POST', path: '/',
      auth: `${user}:${pass}`,
      headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body) },
    }, (res) => {
      let data = '';
      res.on('data', (c) => { data += c; });
      res.on('end', () => {
        let reply;
        try { reply = JSON.parse(data); } catch (e) { return reject(new Error(`bad JSON from ${method}: ${data.slice(0, 200)}`)); }
        if (reply.error) return reject(new Error(`${method}: ${reply.error.message} (${reply.error.code})`));
        resolve({ result: reply.result, raw: data });
      });
    });
    req.on('error', reject);
    req.end(body);
  });
}

// Finds keys that appear twice in the same JSON object (JSON.parse silently
// keeps the last one, so they're easy to miss)
function duplicateKeys(text) {
  const dupes = [];
  let i = 0;
  const ws = () => { while (/\s/.test(text[i])) i++; };
  const str = () => {
    let s = ''; i++;
    while (text[i] !== '"') { if (text[i] === '\\') { s += text[i]; i++; } s += text[i]; i++; }
    i++; return s;
  };
  const value = () => {
    ws();
    if (text[i] === '{') {
      i++; ws(); const seen = new Set();
      if (text[i] === '}') { i++; return; }
      for (;;) {
        ws(); const k = str(); if (seen.has(k)) dupes.push(k); seen.add(k);
        ws(); i++; value(); ws();
        if (text[i] === ',') { i++; continue; }
        i++; return;
      }
    } else if (text[i] === '[') {
      i++; ws(); if (text[i] === ']') { i++; return; }
      for (;;) { value(); ws(); if (text[i] === ',') { i++; continue; } i++; return; }
    } else if (text[i] === '"') { str(); }
    else { while (i < text.length && !/[,\]}\s]/.test(text[i])) i++; }
  };
  value();
  return dupes;
}

function check(name, cond, detail) {
  if (cond) {
    console.log(`ok   - ${name}`);
  } else {
    console.log(`FAIL - ${name}${detail !== undefined ? ': ' + detail : ''}`);
    failures++;
  }
}

async function call(name, method, params, test) {
  try {
    const { result, raw } = await rpc(method, params);
    const dupes = duplicateKeys(raw);
    check(`${name}: no duplicate JSON keys`, dupes.length === 0, dupes.join(', '));
    await test(result);
    return result;
  } catch (e) {
    check(name, false, e.message);
    return undefined;
  }
}

const isNum = (v) => typeof v === 'number' && Number.isFinite(v);

(async () => {
  // Panel / sync calls (eIquidus api_cmds)
  const height = await call('getblockcount', 'getblockcount', [], (r) => check('getblockcount is a number', isNum(r), r));
  await call('getinfo', 'getinfo', [], (r) => {
    check('getinfo.moneysupply (supply mode GETINFO)', isNum(r.moneysupply), JSON.stringify(r.moneysupply));
    check('getinfo.difficulty', r.difficulty !== undefined);
    check('getinfo.connections', isNum(r.connections));
  });
  await call('getdifficulty', 'getdifficulty', [], (r) => {
    check('getdifficulty proof-of-work / proof-of-stake', isNum(r['proof-of-work']) && isNum(r['proof-of-stake']), JSON.stringify(r));
  });
  await call('getmininginfo', 'getmininginfo', [], (r) => {
    check('getmininginfo has netmhashps or networkhashps', 'netmhashps' in r || 'networkhashps' in r);
  });
  await call('getnetworkhashps', 'getnetworkhashps', [], (r) => check('getnetworkhashps is a number', isNum(r), r));
  await call('getnetworkhashps "120" (string, as eIquidus sends settings parameters)', 'getnetworkhashps', ['120'],
    (r) => check('getnetworkhashps with a string parameter', isNum(r), r));
  await call('getconnectioncount', 'getconnectioncount', [], (r) => check('getconnectioncount is a number', isNum(r)));
  await call('getpeerinfo', 'getpeerinfo', [], (r) => {
    check('getpeerinfo is an array', Array.isArray(r));
    for (const p of r) check(`peer ${p.addr} has addr, version, subver`, typeof p.addr === 'string' && isNum(p.version) && typeof p.subver === 'string');
  });
  await call('getblockchaininfo', 'getblockchaininfo', [], (r) => {
    check('getblockchaininfo.moneysupply (supply mode GETBLOCKCHAININFO)', isNum(r.moneysupply));
    check('getblockchaininfo.blocks matches getblockcount', r.blocks === height, `${r.blocks} vs ${height}`);
  });
  await call('gettxoutsetinfo', 'gettxoutsetinfo', [], (r) => {
    check('gettxoutsetinfo.total_amount (supply mode TXOUTSET)', isNum(r.total_amount));
  });
  await call('getmaxmoney', 'getmaxmoney', [], (r) => check('getmaxmoney is a number', isNum(r) && r > 0));
  await call('getsupply', 'getsupply', [], (r) => check('getsupply is a number', isNum(r)));

  // Blocks: eIquidus calls getblockhash <height> and getblock <hash>
  const tip = await call('getblockhash (number)', 'getblockhash', [height], (r) => check('getblockhash returns a hash', /^[0-9a-f]{64}$/.test(r)));
  await call('getblockhash (string height)', 'getblockhash', [String(height)], (r) => check('getblockhash with a string height', r === tip));
  const block = await call('getblock', 'getblock', [tip], (r) => {
    check('getblock has hash, height, time, tx[]', r.hash === tip && r.height === height && isNum(r.time) && Array.isArray(r.tx));
    check('getblock tx entries are txid strings', r.tx.every((t) => /^[0-9a-f]{64}$/.test(t)));
  });
  await call('getblock verbosity 0 (hex)', 'getblock', [tip, 0], (r) => check('getblock 0 returns hex', /^[0-9a-f]+$/.test(r)));
  await call('getblock verbosity 2', 'getblock', [tip, 2], (r) => check('getblock 2 returns decoded transactions', typeof r.tx[0] === 'object' && r.tx[0].txid !== undefined));
  await call('getblock true (old form)', 'getblock', [tip, true], (r) => check('getblock true still returns decoded transactions', typeof r.tx[0] === 'object'));

  // Transactions (needs a block after the genesis block)
  if (height > 0 && block) {
    const txid = block.tx[0];
    const tx = await call('getrawtransaction <txid> 1', 'getrawtransaction', [txid, 1], (r) => {
      check('tx has txid, vin[], vout[], blockhash, time, confirmations',
        r.txid === txid && Array.isArray(r.vin) && Array.isArray(r.vout) && r.blockhash === tip && isNum(r.time) && isNum(r.confirmations));
      check('vin[0] is coinbase or has txid/vout', r.vin[0].coinbase !== undefined || (r.vin[0].txid && isNum(r.vin[0].vout)));
      for (const o of r.vout) {
        check(`vout ${o.n} has value, n, scriptPubKey.type`, isNum(o.value) && isNum(o.n) && typeof o.scriptPubKey.type === 'string');
        if (o.scriptPubKey.type === 'pubkey' || o.scriptPubKey.type === 'pubkeyhash')
          check(`vout ${o.n} has address and addresses[]`, typeof o.scriptPubKey.address === 'string' && Array.isArray(o.scriptPubKey.addresses));
      }
    });
    await call('getrawtransaction <txid> true', 'getrawtransaction', [txid, true], (r) => check('verbose=true works', r.txid === txid));
    await call('getrawtransaction <txid> "1"', 'getrawtransaction', [txid, '1'], (r) => check('verbose="1" works', r.txid === txid));
    await call('getrawtransaction <txid> 0', 'getrawtransaction', [txid, 0], (r) => check('verbose=0 returns hex', /^[0-9a-f]+$/.test(r)));

    // Pay-to-pubkey outputs: eIquidus builds pkh(<pubkey>) from the asm and
    // asks getdescriptorinfo + deriveaddresses for the address
    const p2pk = tx && tx.vout.find((o) => o.scriptPubKey.type === 'pubkey');
    if (p2pk) {
      const descriptor = 'pkh(' + p2pk.scriptPubKey.asm.replace(' OP_CHECKSIG', '') + ')';
      const info = await call('getdescriptorinfo pkh(<pubkey>)', 'getdescriptorinfo', [descriptor],
        (r) => check('descriptor comes back with an 8 character checksum', /#[a-z0-9]{8}$/.test(r.descriptor), r.descriptor));
      if (info) {
        await call('deriveaddresses <descriptor>', 'deriveaddresses', [info.descriptor], (r) => {
          check('deriveaddresses gives the output\'s address', Array.isArray(r) && r[0] === p2pk.scriptPubKey.address, `${r} vs ${p2pk.scriptPubKey.address}`);
        });
      }
      try {
        await rpc('deriveaddresses', [descriptor + '#aaaaaaaa']);
        check('a wrong checksum is rejected', false);
      } catch (e) { check('a wrong checksum is rejected', true); }
    }
  } else {
    console.log('(only the genesis block: transaction checks skipped)');
  }

  // Descriptors from a wallet key (if the node has a wallet): pk() and pkh()
  // must both give the key's address, as staking rewards pay to a pubkey
  try {
    const { result: addr } = await rpc('getnewaddress', []);
    const { result: v } = await rpc('validateaddress', [addr]);
    for (const kind of ['pk', 'pkh']) {
      const info = await call(`getdescriptorinfo ${kind}(<wallet pubkey>)`, 'getdescriptorinfo', [`${kind}(${v.pubkey})`],
        (r) => check(`${kind}() descriptor gets a checksum`, /#[a-z0-9]{8}$/.test(r.descriptor), r.descriptor));
      if (info)
        await call(`deriveaddresses ${kind}(<wallet pubkey>)`, 'deriveaddresses', [info.descriptor],
          (r) => check(`deriveaddresses ${kind}() gives the key's address`, r[0] === addr, `${r} vs ${addr}`));
    }
  } catch (e) {
    console.log(`(no wallet: wallet key descriptor checks skipped: ${e.message})`);
  }

  await call('getdescriptorinfo raw()', 'getdescriptorinfo', ['raw(00)'], (r) => check('raw() descriptor gets a checksum', r.checksum.length === 8));

  console.log(failures ? `${failures} check(s) FAILED` : 'all explorer compatibility checks passed');
  process.exit(failures ? 1 : 0);
})();
