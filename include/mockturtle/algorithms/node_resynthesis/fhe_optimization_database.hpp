#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/spectral.hpp>
#include <kitty/operations.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cut_view.hpp>

namespace mockturtle
{

struct fhe_optimization_database_params
{
	bool verbose{ false };
};

struct fhe_optimization_database_stats
{
	stopwatch<>::duration time_parse_db { 0 };
	stopwatch<>::duration time_classify { 0 };
	stopwatch<>::duration time_construct{ 0 };
	uint32_t cache_hit   { 0u };
	uint32_t cache_miss  { 0u };
	uint32_t unknown_func{ 0u };

	void report() const
	{
		std::cout << fmt::format( "[i] parse database time          = {:>5.2f} secs\n", to_seconds( time_parse_db ) );
		std::cout << fmt::format( "[i] function classification time = {:>5.2f} secs\n", to_seconds( time_classify ) );
		std::cout << fmt::format( "[i] construct network time       = {:>5.2f} secs\n", to_seconds( time_construct ) );
		std::cout << fmt::format( "[i] # cache hit  = {:>5}\n", cache_hit );
		std::cout << fmt::format( "[i] # cache miss = {:>5}\n", cache_miss );
		std::cout << fmt::format( "[i] # unclassifiable func. = {:>5}\n", unknown_func );
	}	
};

class fhe_optimization_database
{
public:
	fhe_optimization_database( std::string const& dbname, fhe_optimization_database_params const& ps = {}, fhe_optimization_database_stats* pst = nullptr )
		: ps_( ps ), pst_( pst )
	{
		build_db( dbname );
	}

	~fhe_optimization_database()
	{
		if ( ps_.verbose )
		{
			pst_->report();
		}
	}

	template<typename Fn>
	uint32_t run( xag_network& res, kitty::dynamic_truth_table const& func, std::vector<arrival_time_pair> const& inputs, Fn&& on_signal ) const
	{
		const auto num_vars = func.num_vars();
		assert( num_vars == inputs.size() );
		uint64_t repr{};
		std::vector<kitty::detail::spectral_operation> trans;

		const auto it_cache = cache_[num_vars].find( *( func.cbegin() ) );
		if ( it_cache != cache_[num_vars].end() )
		{
			++pst_->cache_hit;
			repr = std::get<0>( it_cache->second );
			trans = std::get<1>( it_cache->second );
		}
		else
		{
			++pst_->cache_miss;
			stopwatch time_classify( pst_->time_classify );
			repr = *( kitty::hybrid_exact_spectral_canonization( func, [&trans]( auto const& ops ) { trans = ops; } ).cbegin() );
			cache_[num_vars][*( func.cbegin() )] = std::make_tuple( repr, trans );
		}

		const auto it_db = db_[num_vars].find( repr );
		if ( it_db == db_[num_vars].end() )
		{
			++pst_->unknown_func;
			std::cerr << "[e] cannot find represent function " << repr << " (" << num_vars << "-variable) in database\n";
			abort();
		}
		std::vector<uint8_t> pin_delay = std::get<0>( it_db->second );
		pin_delay.resize( num_vars );
		xag_network::signal po_db = std::get<1>( it_db->second );

		/* construct local implementation on top of database */
		bool po_neg{ false };
		std::vector<xag_network::signal> xors;
		std::vector<xag_network::signal> pis( num_vars, res.get_constant( false ) );
		std::vector<uint8_t> delays( num_vars, 0u );
		for ( auto i{ 0u }; i < num_vars; ++i )
		{
			pis[i] = inputs[i].f;
			delays[i] = inputs[i].m_depth;
		}
		stopwatch t_construct( pst_->time_construct );
		for ( auto const& op: trans )
		{
			switch ( op._kind )
			{
			case kitty::detail::spectral_operation::kind::permutation:
			{
				const auto v1 = kitty::detail::log2[op._var1];
				const auto v2 = kitty::detail::log2[op._var2];
				std::swap( pis[v1], pis[v2] );
				std::swap( delays[v1], delays[v2] );
				break;
			}
			case kitty::detail::spectral_operation::kind::input_negation:
			{
				const auto v1 = kitty::detail::log2[op._var1];
				pis[v1] = res.create_not( pis[v1] );
				break;
			}
			case kitty::detail::spectral_operation::kind::output_negation:
			{
				po_neg = !po_neg;
				break;
			}
			case kitty::detail::spectral_operation::kind::spectral_translation:
			{
				const auto v1 = kitty::detail::log2[op._var1];
				const auto v2 = kitty::detail::log2[op._var2];
				pis[v1] = res.create_xor( pis[v1], pis[v2] );
				delays[v1] = std::max( delays[v1], delays[v2] );
				break;
			}
			case kitty::detail::spectral_operation::kind::disjoint_translation:
			{
				const auto v1 = kitty::detail::log2[op._var1];
				xors.emplace_back( pis[v1] );
				break;
			}
			default:
				abort();
			}
		}

		xag_network::signal po;
		if ( xag_db_.is_constant( xag_db_.get_node( po_db ) ) )
		{
			po = res.get_constant( xag_db_.is_complemented( po_db ) );
		}
		else
		{
			cut_view<xag_network> db_partial{ xag_db_, db_pis_, po_db };
			po = cleanup_dangling( db_partial, res, pis.begin(), pis.end() ).front();
		}
		for ( auto const& xor_op: xors )
		{
			po = res.create_xor( xor_op, po );
		}
		if ( po_neg )
		{
			po = res.create_not( po );
		}

		/* calculate delay */
		uint32_t delay{ pin_delay.front() + delays.front() };
		for ( auto i{ 1u }; i < num_vars; ++i )
		{
			delay = std::max( delay, static_cast<uint32_t>( pin_delay[i] + delays[i] ) );
		}

		/* post-process */
		on_signal( { po, delay } );


		/* for debugging */
		// fmt::print( "[m] databased-based new implementation is : Node {}{}, with the delay of {}\n", ( res.is_complemented( po ) ? "!" : "" ), res.get_node( po ), delay );


		return delay;
	}

private:
	void build_db( std::string const& dbname )
	{
		if ( ps_.verbose )
		{
			fmt::print( "[i] start loading database\n" );
		}

		stopwatch t_parse_db( pst_->time_parse_db );

		std::generate( db_pis_.begin(), db_pis_.end(), [&]() { return xag_db_.create_pi(); } );

		std::ifstream db;
		db.open( dbname, std::ios::in );
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

			std::vector<xag_network::signal> xag_db_signals( db_pis_.begin(), db_pis_.begin() + num_vars );
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
					signal1 = xag_db_.get_constant( false );
				}
				else if ( signal1_ul == 1u )
				{
					signal1 = xag_db_.get_constant( true );
				}
				else
				{
					signal1 = xag_db_signals[signal1_ul / 2 - 1] ^ ( signal1_ul % 2 );
				}
				if ( signal2_ul == 0u )
				{
					signal2 = xag_db_.get_constant( false );
				}
				else if ( signal2_ul == 1u )
				{
					signal2 = xag_db_.get_constant( true );
				}
				else
				{
					signal2 = xag_db_signals[signal2_ul / 2 - 1] ^ ( signal2_ul % 2 );
				}

				if ( signal1_ul > signal2_ul )
				{
					xag_db_signals.emplace_back( xag_db_.create_xor( signal1, signal2 ) );
				}
				else
				{
					xag_db_signals.emplace_back( xag_db_.create_and( signal1, signal2 ) );
				}
			}
			const uint32_t po_ul = std::stoul( line );
			const xag_network::signal po = xag_db_signals[po_ul / 2 - 1] ^ ( po_ul % 2 );
			xag_db_.create_po( po );

			for ( auto i{ num_vars }; i <= 5u; ++i )
			{
				const uint64_t repr = std::stoul( ( "0x" + repr_str.substr( 0, 1 << ( i - 2 ) ) ),  nullptr, 16 );
				db_[i][repr] = std::make_tuple( delay, po );
			}
		}

		for ( auto i{ 0u }; i <= 5u; ++i )
		{
			db_[i][0] = std::make_tuple( std::vector<uint8_t>{ { 0u } }, xag_db_.get_constant( false ) );
		}

		if ( ps_.verbose )
		{
			fmt::print( "[i] finish loading database\n" );
		}
	}

private:
	const fhe_optimization_database_params ps_;
	fhe_optimization_database_stats* pst_;

	xag_network xag_db_;
	std::vector<xag_network::signal> db_pis_{ 5u };
	std::vector<std::unordered_map<uint64_t, std::tuple<std::vector<uint8_t>, xag_network::signal>>> db_{ 6u };
	mutable std::vector<std::unordered_map<uint64_t, std::tuple<uint64_t, std::vector<kitty::detail::spectral_operation>>>> cache_{ 6u };
};

} /* namespace mockturtle */