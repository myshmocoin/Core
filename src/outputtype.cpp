// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2025 The ShmoCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <outputtype.h>

#include <pubkey.h>
#include <script/script.h>
#include <script/sign.h>
#include <script/signingprovider.h>
#include <script/standard.h>
#include <util/vector.h>

#include <assert.h>
#include <string>

static const std::string OUTPUT_TYPE_STRING_LEGACY = "legacy";
static const std::string OUTPUT_TYPE_STRING_P2SH_SEGWIT = "p2sh-segwit";
static const std::string OUTPUT_TYPE_STRING_BECH32 = "bech32";
static const std::string OUTPUT_TYPE_STRING_MWEB = "mweb";

const std::array<OutputType, 4> OUTPUT_TYPES = {OutputType::LEGACY, OutputType::P2SH_SEGWIT, OutputType::BECH32, OutputType::MWEB};

bool ParseOutputType(const std::string& type, OutputType& output_type)
{
    if (type == OUTPUT_TYPE_STRING_LEGACY) {
        output_type = OutputType::LEGACY;
        return true;
    } else if (type == OUTPUT_TYPE_STRING_P2SH_SEGWIT) {
        output_type = OutputType::P2SH_SEGWIT;
        return true;
    } else if (type == OUTPUT_TYPE_STRING_BECH32) {
        output_type = OutputType::BECH32;
        return true;
    } else if (type == OUTPUT_TYPE_STRING_MWEB) {
        output_type = OutputType::MWEB;
        return true;
    }
    return false;
}

const std::string& FormatOutputType(OutputType type)
{
    switch (type) {
    case OutputType::LEGACY: return OUTPUT_TYPE_STRING_LEGACY;
    case OutputType::P2SH_SEGWIT: return OUTPUT_TYPE_STRING_P2SH_SEGWIT;
    case OutputType::BECH32: return OUTPUT_TYPE_STRING_BECH32;
    case OutputType::MWEB: return OUTPUT_TYPE_STRING_MWEB;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

CTxDestination GetDestinationForKey(const CPubKey& key, OutputType type, const SecretKey& scan_secret)
{
    assert(type != OutputType::MWEB || !scan_secret.IsNull());

    switch (type) {
    case OutputType::LEGACY: return PKHash(key);
    case OutputType::P2SH_SEGWIT:
    case OutputType::BECH32: {
        if (!key.IsCompressed()) return PKHash(key);
        CTxDestination witdest = WitnessV0KeyHash(key);
        CScript witprog = GetScriptForDestination(witdest);
        if (type == OutputType::P2SH_SEGWIT) {
            return ScriptHash(witprog);
        } else {
            return witdest;
        }
    }
    case OutputType::MWEB: {
        PublicKey spend_pubkey(key.data());
        return StealthAddress(spend_pubkey.Mul(scan_secret), spend_pubkey);
    }
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

std::vector<CTxDestination> GetAllDestinationsForKey(const CPubKey& key, const SecretKey& scan_secret)
{
    PKHash keyid(key);
    CTxDestination p2pkh{keyid};
    if (key.IsCompressed()) {
        CTxDestination segwit = WitnessV0KeyHash(keyid);
        CTxDestination p2sh = ScriptHash(GetScriptForDestination(segwit));

        if (!scan_secret.IsNull()) {
            CTxDestination stealth = GetDestinationForKey(key, OutputType::MWEB, scan_secret);
            return Vector(std::move(p2pkh), std::move(p2sh), std::move(segwit), std::move(stealth));
        } else {
            return Vector(std::move(p2pkh), std::move(p2sh), std::move(segwit));
        }
    } else {
        return Vector(std::move(p2pkh));
    }
}

CTxDestination AddAndGetDestinationForScript(FillableSigningProvider& keystore, const CScript& script, OutputType type)
{
    // Add script to keystore
    keystore.AddCScript(script);
    // Note that scripts over 520 bytes are not yet supported.
    switch (type) {
    case OutputType::LEGACY:
        return ScriptHash(script);
    case OutputType::P2SH_SEGWIT:
    case OutputType::BECH32: {
        CTxDestination witdest = WitnessV0ScriptHash(script);
        CScript witprog = GetScriptForDestination(witdest);
        // Check if the resulting program is solvable (i.e. doesn't use an uncompressed key)
        if (!IsSolvable(keystore, witprog)) return ScriptHash(script);
        // Add the redeemscript, so that P2WSH and P2SH-P2WSH outputs are recognized as ours.
        keystore.AddCScript(witprog);
        if (type == OutputType::BECH32) {
            return witdest;
        } else {
            return ScriptHash(witprog);
        }
    }
    case OutputType::MWEB:
        assert(false);
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}
