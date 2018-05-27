#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

class ping_contract : public eosio::contract {
    public:
        using eosio::contract::contract;
        void ping(account_name receiver) {
            eosio::print("Pong");
        }
};

EOSIO_ABI( ping_contract, (ping) )