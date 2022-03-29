#pragma once
#include <eosio/chain/types.hpp>
#include <eosio/chain/block_state.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/trace.hpp>
#include <functional>
#include <tuple>

namespace eosio::chain {

class signals_processor {
public:
   using trx_deque = eosio::chain::deque< std::tuple< chain::transaction_trace_ptr, packed_transaction_ptr > >;
   using applied_transaction_func = std::function< void ( const trx_deque&, const chain::block_state_ptr& ) >;
   using irreversible_block_func = std::function< void ( const chain::block_state_ptr& ) >;
   using block_start_func = std::function< void ( uint32_t block_num ) >;

   /**
    * Class for tracking transactions and which block they belong to.
    */
   signals_processor() {
   }

   void register_callbacks(applied_transaction_func at, irreversible_block_func ib, block_start_func bs) {
      _callbacks.emplace_back(at, ib, bs);
   }

   /// connect to chain controller applied_transaction signal
   void signal_applied_transaction( const chain::transaction_trace_ptr& trace, const chain::signed_transaction& strx ) {
#warning TODO need to store packed transaction after code merged
      _trxs.emplace_back(trace, packed_transaction_ptr{});
   }

   /// connect to chain controller accepted_block signal
   void signal_accepted_block( const chain::block_state_ptr& bsp ) {
      push_transactions(bsp);
      _block_started = false;
   }

   /// connect to chain controller irreversible_block signal
   void signal_irreversible_block( const chain::block_state_ptr& bsp ) {
      for(auto& callback : _callbacks) {
         std::get<1>(callback)(bsp);
      }
   }


   /// connect to chain controller block_start signal
   void signal_block_start( uint32_t block_num ) {
      if (_block_started) {
         push_transactions(chain::block_state_ptr{});
      }
      _block_started = true;
      for(auto& callback : _callbacks) {
         std::get<2>(callback)(block_num);
      }
   }

private:
   void push_transactions( const chain::block_state_ptr& bsp ) {
      for(auto& callback : _callbacks) {
         std::get<0>(callback)(_trxs, bsp);
      }
      _trxs.clear();
   }
   trx_deque _trxs;
   eosio::chain::vector< std::tuple< applied_transaction_func, irreversible_block_func, block_start_func > > _callbacks;
   bool _block_started = false;
};

} // eosio::chain