#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>

#include <string>

using std::string;

using namespace eosio;

CONTRACT kakaotipbot1 : public contract {
  public:
  // constructor 수정필요?
    using contract::contract;
    kakaotipbot1(eosio::name receiver, eosio::name code, datastream<const char*> ds):contract(receiver, code, ds), symbol_eos("EOS", 0) {}

    ACTION tip(name token_contract, 
               uint64_t from_id, 
               string from_name, 
               uint64_t to_id,
               string to_name,
               asset quantity,
               string memo);
               
    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name from,
                   name to,
                   asset quantity,
                   string memo);
               
    ACTION withdraw(name token_contract,
                    uint64_t from_id,
                    string from_name,
                    name to_account,
                    asset quantity,
                    string memo);
  
  private:
    const symbol symbol_eos;
    TABLE account {
      uint64_t user_id;
      asset balance;
      
      auto primary_key() const {return user_id;}
    };
    
    typedef eosio::multi_index<"table"_n, account> account_balance;
    
    // account_balance user_table;
};

// EOSIO_DISPATCH(kakaotipbot1, (tip) (deposit) (withdraw))   // DISPATCH 없어도되나?
