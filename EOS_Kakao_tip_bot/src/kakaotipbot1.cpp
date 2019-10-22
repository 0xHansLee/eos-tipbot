// test 해야할 것 : eosio.token이 아닌 계정에서 eosio.token smart contract를 deploy하여 똑같이 EOS, 0 token을 보냈을때? 혹은 EOS, 4를 보냈을때?
// jungle net test 후 audit 하기(받기?)

// 다른 토큰 생성컨트랙트 이용해서 EOS, 0 만들어서 보냈을 때 -> deposit 작동 안함. on_notify 작동 확인
#include <kakaotipbot1.hpp>

ACTION kakaotipbot1::tip(name token_contract, 
                         uint64_t from_id, 
                         string from_name, 
                         uint64_t to_id,
                         string to_name,
                         asset quantity,
                         string memo) 
{
  // require authority of kakaotipbot1
  require_auth(get_self());
  
  // symbol check. need to check contract name and token symbol!
  auto sym = quantity.symbol;
  eosio::check(sym.is_valid(), "Invalid symbol name");
  eosio::check(quantity.symbol == symbol_eos, "Wrong with token symbol"); // only EOS(0)
  account_balance account_table(get_self(), get_self().value);
  
  //----------------------------- creating from ID table for testing  -----------------------------
  // auto iterator = account_table.find(from_id);
  // if(iterator == account_table.end())
  // {
  //   account_table.emplace(get_self(), [&](auto& newRow){
  //     newRow.user_id = from_id;
  //     newRow.balance = 100*quantity;
  //   });
  // }
  //----------------------------------------------------------------------------------
  
  // check whether the from_id exist 
  const auto& from_row = account_table.get(from_id, "No ID");                        // OK
  
  // check balance of from_id
  print("from_row.balance = ", from_row.balance);
  eosio::check(from_row.balance.amount >= quantity.amount, "Not enough balance");   // OK
  
  // check maximum memo size
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes");                 // OK
  
  // subtract the quantity from the from_id balance
  auto itr_from = account_table.find(from_id);
  account_table.modify(itr_from, get_self(), [&](auto& fromRow){
    fromRow.balance -= quantity;
  });
  
  // to_id check in table and add the quantity
  auto itr_to = account_table.find(to_id);
  if(itr_to == account_table.end())
  {
    account_table.emplace(get_self(), [&](auto& toRow){
      toRow.user_id = to_id;
      toRow.balance = quantity;
    });
  }
  else
  {
    account_table.modify(itr_to, get_self(), [&](auto& toRow){
      toRow.balance += quantity;
    });
  }
}

[[eosio::on_notify("eosio.token::transfer")]]
void kakaotipbot1::deposit(name from,
                           name to,
                           asset quantity,
                           string memo)
{
  // print("deposited!");
  if(to != get_self() || from == get_self())
  {
    print("Wrong Tx");
  }
  
  eosio::check(quantity.amount > 0, "negative amount of token transferred");
  eosio::check(quantity.symbol == symbol_eos, "Wrong with token symbol");
  
  account_balance account_table(get_self(), get_self().value);
  
  uint64_t newId = stoull(memo);
  // print("\n\n memo : ", memo,"\n");           // need to check if there is overflow
  // print("\n\n newId: ", newId,"\n");
  auto itr_deposit = account_table.find(newId);
  if(itr_deposit == account_table.end())
  {
    account_table.emplace(get_self(), [&](auto& depositRow){
      depositRow.user_id = newId;
      depositRow.balance = quantity;
    });
  }
  else
  {
    account_table.modify(itr_deposit, get_self(), [&](auto& depositRow){
      depositRow.balance += quantity;
    });
  }
}

ACTION kakaotipbot1::withdraw(name token_contract,
                              uint64_t from_id,
                              string from_name,
                              name to_account,
                              asset quantity,
                              string memo)
{
  require_auth(get_self());
  account_balance account_table(get_self(), get_self().value);
  const auto& from_row = account_table.get(from_id, "No ID");
  eosio::check(from_row.balance.amount >= quantity.amount, "Not enough balance");
  eosio::check(quantity.symbol == symbol_eos, "Wrong with token symbol");
  eosio::check(memo.size() <= 256, "memo has more than 256 bytes"); 
  
  action{
    permission_level{get_self(), "active"_n},
    "eosio.token"_n,
    "transfer"_n,
    std::make_tuple(get_self(), to_account, quantity, memo)
  }.send();
  
  auto itr_from = account_table.find(from_id);
  account_table.modify(itr_from, get_self(), [&](auto& fromRow){
    fromRow.balance -= quantity;
  });
}