#pragma once

#include "../networks/xag.hpp"
#include "../utils/node_map.hpp"

#include <kitty/algorithm.hpp>
#include <kitty/bit_operations.hpp>
#include <kitty/esop.hpp>
#include <kitty/print.hpp>

#include <fstream>
#include <fmt/format.h>
#include <iostream>
#include <sstream>

namespace mockturtle
{

class fhe_rule_gen
{
public:
	void run( std::string const& dbname, std::string const& filename )
	{
		build_db( dbname );
		gen_rules( filename );
	}

private:
	std::string get_esop( std::array<uint32_t, 2> var_tt ) const
	{
		const uint32_t num_vars{ var_tt[0] };
		kitty::dynamic_truth_table tt( num_vars );
		std::stringstream ss;
		ss << std::hex << var_tt[1];
		std::string ss_str = ss.str();
		auto ss_str_size = ss_str.size();
		for ( auto i{ 0u }; i < ( ( 1 << ( num_vars - 2 ) ) - ss_str_size ); ++i )
		{
			ss_str = "0" + ss_str;
		}
		kitty::create_from_hex_string( tt, ss_str );

		auto tt_anf = kitty::detail::algebraic_normal_form( tt );
		const auto terms = kitty::count_ones( tt_anf );
		if ( terms == 0u )
		{
			return "0";
		}

		std::string expr{};
		uint32_t num_terms{ 0u };
		kitty::for_each_one_bit( tt_anf, [&]( auto b ) {
			if ( b != 0u )
			{
				++num_terms;
			}
		} );
		uint32_t term_cnt{ 1u };
		kitty::for_each_one_bit( tt_anf, [&]( auto b ) {
			if ( b == 0u )
			{
				expr += "1";
				return true;
			}

			uint32_t weight = __builtin_popcount( static_cast<uint32_t>( b ) );
			if ( weight != 1u )
			{
				expr += "( ";
			}
			uint32_t var_cnt{ 1u };
			for ( uint32_t i{ 0u }; i < tt_anf.num_vars(); ++i )
			{
				if ( ( b >> i ) & 1 )
				{
					expr += std::string( 1, 'a' + i );
					if ( var_cnt++ < weight )
					{
						expr += " & ";
					}
				}
			}
			if ( weight != 1u )
			{
				expr += " )";
			}

			if ( term_cnt++ < num_terms )
			{
				expr += " ^ ";
			}
		} );

		return expr;
	}

	void build_db( std::string const& dbname )
	{
		std::ifstream db;
		db.open( dbname, std::ios::in );
		if ( !db.is_open() )
		{
			std::cout << fmt::format( "[e] Unable to open database {}...\n", dbname );
			abort();
		}

		std::cout << fmt::format( "[m] Start loading database {}...\n", dbname );
		std::generate( _pis.begin(), _pis.end(), [&]() { return _ntk.create_pi(); } );
		
		std::string line;
		uint32_t pos{ 0u };

		while ( std::getline( db, line ) )
		{
			pos = static_cast<uint32_t>( line.find( 'x' ) );
			const std::string repr_str = line.substr( ++pos, 8u );
			pos += 9u;
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint32_t num_vars = std::stoul( line.substr( 0, pos++ ) );
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint8_t mc = std::stoul( line.substr( 0, pos++ ) );
			line.erase( 0, pos );

			std::vector<uint8_t> delay;
			for ( auto i{ 0u }; i < num_vars; ++i )
			{
				pos = line.find( ' ' );
				delay.emplace_back( std::stoul( line.substr( 0, pos++ ) ) );
				line.erase( 0, pos );
			}

			std::vector<xag_network::signal> db_signals( _pis.begin(), _pis.begin() + num_vars );
			while ( line.size() > 3 )
			{
				uint32_t signal1_ul{}, signal2_ul{};
				pos = line.find( ' ' );
				signal1_ul = std::stoul( line.substr( 0, pos++ ) );
				line.erase( 0, pos );
				pos = line.find( ' ' );
				signal2_ul = std::stoul( line.substr( 0, pos++ ) );
				line.erase( 0, pos );

				xag_network::signal signal1{}, signal2{};
				if ( signal1_ul == 0u )
				{
					signal1 = _ntk.get_constant( false );
				}
				else if ( signal1_ul == 1u )
				{
					signal1 = _ntk.get_constant( true );
				}
				else
				{
					signal1 = db_signals[signal1_ul / 2 - 1] ^ ( signal1_ul % 2 );
				}

				if ( signal2_ul == 0u )
				{
					signal2 = _ntk.get_constant( false );
				}
				else if ( signal2_ul == 1u )
				{
					signal2 = _ntk.get_constant( true );
				}
				else
				{
					signal2 = db_signals[signal2_ul / 2 - 1] ^ ( signal2_ul % 2 );
				}

				if ( signal1_ul > signal2_ul )
				{
					db_signals.emplace_back( _ntk.create_xor( signal1, signal2 ) );
				}
				else
				{
					db_signals.emplace_back( _ntk.create_and( signal1, signal2 ) );
				}
			}

			const uint32_t po_ul = std::stoul( line );
			const xag_network::signal po = db_signals[po_ul / 2 - 1] ^ ( po_ul % 2 );
			const uint32_t po_index = _ntk.create_po( po );
			std::array<uint32_t, 2> var_tt{};
			var_tt[0] = num_vars;
			var_tt[1] = std::stoul( ( "0x" + repr_str.substr( 0, 1 << ( num_vars - 2 ) ) ),  nullptr, 16 );
			assert( _pos_var_tt.size() == po_index );
			_pos_var_tt.push_back( var_tt );
		}

		db.close();
		std::cout << fmt::format( "[m] Finish loading database...\n" );
	}

	void gen_rules( std::string const& filename )
	{
		node_map<std::string, xag_network> _expr{ _ntk };
		_ntk.clear_visited();

		uint8_t cnt{ 0u };
		for ( auto const& pi : _pis )
		{
			_expr[_ntk.get_node( pi )] = 'a' + cnt;
			cnt++;
		}

		_ntk.foreach_node( [&]( auto const& n ) {
			if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
			{
				return true;
			}

			if ( _ntk.visited( n ) )
			{
				return true;
			}

			_ntk.set_visited( n, 1u );

			std::array<std::string, 2u> operands{};
			uint8_t ind{ 0u }; 
			_ntk.foreach_fanin( n, [&]( auto const& f ) {
				const auto ni = _ntk.get_node( f );
				assert( !_ntk.is_complemented( f ) );
				assert( _expr[ni] != "" );

				operands[ind] = _expr[ni];
				if ( !_ntk.is_pi( ni ) )
				{
					operands[ind] = "( " + operands[ind] + " )";
				}
				++ind;
			} );

			if ( _ntk.is_and( n ) )
			{
				_expr[n] = operands[0] + " & " + operands[1];
			}
			else
			{
				_expr[n] = operands[1] + " ^ " + operands[0];
			}
			return true;
		} );


		std::ofstream rules;
		rules.open( filename, std::ios::out );
		if ( rules.is_open() )
		{
			std::cout << fmt::format( "[m] Start generating rules ...\n" );
			for ( auto i{ 0u }; i < _ntk.num_pos(); ++i )
			{
				const auto po_n = _ntk.get_node( _ntk.po_at( i ) );
				rules << get_esop( _pos_var_tt[i] );
				rules << " = ( ";
				rules << _expr[po_n];
				rules << " )\n";
			}

			rules.close();
			std::cout << fmt::format( "[m] Finish generating rules into {}...\n", filename );
		}
		else
		{
			std::cout << fmt::format( "[e] Unable to open file {}...\n", filename );
		}
	}

private:
	xag_network _ntk;
	std::vector<xag_network::signal> _pis{ 5u };
	std::vector<std::array<uint32_t, 2>> _pos_var_tt{};
};
} /* mockturtle */