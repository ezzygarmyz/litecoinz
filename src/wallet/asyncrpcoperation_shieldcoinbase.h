// Copyright (c) 2017 The Zcash developers
// Copyright (c) 2017-2020 The LitecoinZ Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef ASYNCRPCOPERATION_SHIELDCOINBASE_H
#define ASYNCRPCOPERATION_SHIELDCOINBASE_H

#include <amount.h>
#include <asyncrpcoperation.h>
#include <primitives/transaction.h>
#include <rpc/request.h>
#include <transaction_builder.h>
#include <wallet/paymentdisclosure.h>
#include <wallet/wallet.h>
#include <zcash/Address.hpp>
#include <zcash/JoinSplit.hpp>

#include <tuple>
#include <unordered_map>

#include <univalue.h>

// Default transaction fee if caller does not specify one.
#define SHIELD_COINBASE_DEFAULT_MINERS_FEE   10000

struct ShieldCoinbaseUTXO {
    uint256 txid;
    int vout;
    CScript scriptPubKey;
    CAmount amount;
};

// Package of info which is passed to perform_joinsplit methods.
struct ShieldCoinbaseJSInfo
{
    std::vector<libzcash::JSInput> vjsin;
    std::vector<libzcash::JSOutput> vjsout;
    CAmount vpub_old = 0;
    CAmount vpub_new = 0;
};

class AsyncRPCOperation_shieldcoinbase : public AsyncRPCOperation {
public:
    AsyncRPCOperation_shieldcoinbase(
        const JSONRPCRequest& request,
        TransactionBuilder builder,
        CMutableTransaction contextualTx,
        std::vector<ShieldCoinbaseUTXO> inputs,
        std::string toAddress,
        CAmount fee = SHIELD_COINBASE_DEFAULT_MINERS_FEE,
        UniValue contextInfo = NullUniValue);
    virtual ~AsyncRPCOperation_shieldcoinbase();

    // We don't want to be copied or moved around
    AsyncRPCOperation_shieldcoinbase(AsyncRPCOperation_shieldcoinbase const&) = delete;             // Copy construct
    AsyncRPCOperation_shieldcoinbase(AsyncRPCOperation_shieldcoinbase&&) = delete;                  // Move construct
    AsyncRPCOperation_shieldcoinbase& operator=(AsyncRPCOperation_shieldcoinbase const&) = delete;  // Copy assign
    AsyncRPCOperation_shieldcoinbase& operator=(AsyncRPCOperation_shieldcoinbase &&) = delete;      // Move assign

    virtual void main();

    virtual UniValue getStatus() const;

    bool testmode = false;  // Set to true to disable sending txs and generating proofs

    bool paymentDisclosureMode = false; // Set to true to save esk for encrypted notes in payment disclosure database.

private:
    JSONRPCRequest request_;
    TransactionBuilder builder_;
    CTransactionRef tx_;
    std::vector<ShieldCoinbaseUTXO> inputs_;
    libzcash::PaymentAddress tozaddr_;
    CAmount fee_;
    UniValue contextinfo_;     // optional data to include in return value from getStatus()

    friend class ShieldToAddress;

    uint256 joinSplitPubKey_;
    unsigned char joinSplitPrivKey_[crypto_sign_SECRETKEYBYTES];

    bool main_impl();

    // JoinSplit without any input notes to spend
    UniValue perform_joinsplit(ShieldCoinbaseJSInfo &);

    void lock_utxos(CWallet* const pwallet);

    void unlock_utxos(CWallet* const pwallet);

    // payment disclosure!
    std::vector<PaymentDisclosureKeyInfo> paymentDisclosureData_;
};

class ShieldToAddress : public boost::static_visitor<bool>
{
private:
    AsyncRPCOperation_shieldcoinbase *m_op;
    CAmount sendAmount;
    JSONRPCRequest request_;
public:
    ShieldToAddress(AsyncRPCOperation_shieldcoinbase *op, CAmount sendAmount, const JSONRPCRequest& request) :
        m_op(op), sendAmount(sendAmount), request_(request) {}

    bool operator()(const libzcash::SproutPaymentAddress &zaddr) const;
    bool operator()(const libzcash::SaplingPaymentAddress &zaddr) const;
    bool operator()(const libzcash::InvalidEncoding& no) const;
};

#endif /* ASYNCRPCOPERATION_SHIELDCOINBASE_H */
