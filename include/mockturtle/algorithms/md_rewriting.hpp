#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_enumeration.hpp>
#include <mockturtle/algorithms/cut_enumeration/rewrite_cut.hpp>
#include <mockturtle/algorithms/md_optimal_exact_synthesis.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/cost_functions.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/utils/progress_bar.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cut_view.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <mockturtle/views/depth_view.hpp>

namespace mockturtle
{
	
struct md_rewriting_params
{
	md_rewriting_params()
	{
		cut_enumeration_ps.cut_size = 5;
    cut_enumeration_ps.cut_limit = 12;
    cut_enumeration_ps.minimize_truth_table = true;
	}

	cut_enumeration_params cut_enumeration_ps{};
	bool allow_zero_gain{ false };
	bool progress{ false };
	bool verbose{ false };
	uint32_t min_cand_cut_size{ 3u };
	bool verify_database{ false };
};

struct md_rewriting_stats
{
	stopwatch<>::duration time_total{ 0 };
	stopwatch<>::duration time_parse_db{ 0 };
	stopwatch<>::duration time_classify{ 0 };
	stopwatch<>::duration time_construct{ 0 };
	uint32_t cache_hit{ 0u };
	uint32_t cache_miss{ 0u };
	uint32_t classify_abort{ 0u };
	uint32_t unknown_func_abort{ 0u };
	//uint32_t dont_cares{ 0u };

	void report() const
	{
		std::cout << fmt::format( "[i] total time     = {:>5.2f} secs\n", to_seconds( time_total ) );
		std::cout << fmt::format( "[i] parse db time  = {:>5.2f} secs\n", to_seconds( time_parse_db ) );
		std::cout << fmt::format( "[i] classify time  = {:>5.2f} secs\n", to_seconds( time_classify ) );
		std::cout << fmt::format( "[i] - aborts       = {:>5}\n", classify_abort );
		std::cout << fmt::format( "[i] construct time = {:>5.2f} secs\n", to_seconds( time_construct ) );
		std::cout << fmt::format( "[i] cache hits     = {:>5}\n", cache_hit );
		std::cout << fmt::format( "[i] cache misses   = {:>5}\n", cache_miss );
		std::cout << fmt::format( "[i] unknown func.  = {:>5}\n", unknown_func_abort );
		//std::cout << fmt::format( "[i] don't cares    = {:>5}\n", dont_cares );
	}
};

namespace detail
{

struct md_rewriting_impl
{
	static constexpr uint32_t num_vars = 5u;
  using network_cuts_t = dynamic_network_cuts<xag_network, num_vars, true, cut_enumeration_rewrite_cut>;
  using cut_manager_t = detail::dynamic_cut_enumeration_impl<xag_network, num_vars, true, cut_enumeration_rewrite_cut>;
  using cut_t = typename network_cuts_t::cut_t;

public:
	md_rewriting_impl( std::string const& dbname, xag_network const& ntk, md_rewriting_params const& ps = {}, md_rewriting_stats* pst = nullptr )
    : dbname_( dbname ),
    	pfunc_md_( std::make_shared<decltype(pfunc_md_)::element_type>() ),
    	pclassify_cache_( std::make_shared<decltype(pclassify_cache_)::element_type>() ),
    	p_exact_impl_( std::make_shared<decltype(p_exact_impl_)::element_type>() ),
    	p_exact_impl_blacklist_( std::make_shared<decltype( p_exact_impl_blacklist_ )::element_type>() ),
    	ntk_( ntk ),
      ps_( ps ),
      pst_( pst ), 
      allow_no_gain_move_( ps_.allow_zero_gain )

  {
  	stopwatch<> time_total( pst_->time_total );
  	build_db();
  }

  ~md_rewriting_impl()
  {
  	if ( ps_.verbose )
  	{
  		pst_->report();
  	}
  }

  xag_network run()
  {
  	stopwatch<> time_total( pst_->time_total );
  	rewrite_critical_path();
  	return ntk_;
  }

  xag_network run2()
  {
  	stopwatch<> time_total( pst_->time_total );
  	uint32_t ctr{ 0u };
  	depth_view<xag_network, mc_cost<xag_network>, false> ntk_md_{ ntk_ };
  	uint32_t md_before = ntk_md_.depth();
  	uint32_t md_after{ 0u };
  	uint32_t num_no_gain_move{ 0u };
  	/* specify number of no gain moves allowed */
  	uint32_t no_gain_move_upper{ 10u };
  	allow_no_gain_move_ = true;

  	while ( md_after < md_before || allow_no_gain_move_ )
  	{
  		if ( ctr > 0u )
  		{
  			md_before = md_after;
  		}
  		rewrite_one_critical_node_a_time();
  		ntk_md_.update_levels();
  		md_after = ntk_md_.depth();
  		//std::cout << "[m] after " << ( ctr + 1 ) << "th run, MD is optimized from " << md_before << " to " << md_after << "\n";
  		if ( md_after >= md_before )
  		{
  			++num_no_gain_move;
  			if ( num_no_gain_move >= no_gain_move_upper )
  			{
  				allow_no_gain_move_ = false;
  			}
  		}
  		++ctr; 
  	}
  	return ntk_;
  }

  xag_network run3()
  {
  	stopwatch<> time_total( pst_->time_total );
  	//uint32_t ctr{ 0u };
  	depth_view<xag_network, mc_cost<xag_network>, false> ntk_md_{ ntk_ };
  	uint32_t md_before = ntk_md_.depth();
  	uint32_t md_after{ 0u };
  	//uint32_t num_no_gain_move{ 0u };
  	/* specify number of no gain moves allowed */
  	//uint32_t no_gain_move_upper{ 10u };
  	//allow_no_gain_move_ = false;


  	on_the_fly_rewrite_one_critical_node_a_time();
  	

  	// while ( md_after < md_before || allow_no_gain_move_ )
  	// {
  	// 	if ( ctr > 0u )
  	// 	{
  	// 		md_before = md_after;
  	// 	}
  	// 	//rewrite_one_critical_node_a_time();
  	// 	on_the_fly_rewrite_one_critical_node_a_time();
  	// 	ntk_md_.update_levels();
  	// 	md_after = ntk_md_.depth();
  	// 	//std::cout << "[m] after " << ( ctr + 1 ) << "th run, MD is optimized from " << md_before << " to " << md_after << "\n";
  	// 	if ( md_after >= md_before )
  	// 	{
  	// 		++num_no_gain_move;
  	// 		if ( num_no_gain_move >= no_gain_move_upper )
  	// 		{
  	// 			allow_no_gain_move_ = false;
  	// 		}
  	// 	}
  	// 	++ctr; 
  	// }
  	return ntk_;
  }

private:
	void build_db()
	{
		stopwatch t_parse_db( pst_->time_parse_db );

		db_pis_.resize( num_vars );
		std::generate( db_pis_.begin(), db_pis_.end(), [&]() { return db_.create_pi(); } );

		std::ifstream db;
		db.open( dbname_, std::ios::in );
		std::string line;
		uint32_t pos{ 0u };

		while ( std::getline( db, line ) )
		{
			pos = static_cast<uint32_t>( line.find( 'x' ) );
			const std::string repr_real_str = line.substr( ++pos, 8u );
			//std::cout << "[m] repr_real_str = " << repr_real_str << ", ";
			pos += 9u;
			line.erase( 0, pos );
			pos = line.find( 'x' );
			std::string repr_db_str = line.substr( ++pos, 8u );
			//std::cout << "repr_db_str = " << repr_db_str << ", ";
			pos += 9u;
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint32_t vars_num = std::stoul( line.substr( 0, pos++ ) );
			//std::cout << "vars_num = " << vars_num << ", ";
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint32_t mc = std::stoul( line.substr( 0, pos++ ) );
			line.erase( 0, pos );
			std::vector<uint32_t> delay;
			//std::vector<std::pair<uint32_t, uint32_t>> delay_asc;
			for ( auto i{ 0u }; i < vars_num; ++i )
			{
				pos = line.find( ' ' );
				delay.emplace_back( std::stoul( line.substr( 0, pos++ ) ) );
				//delay_asc.emplace_back( std::make_pair( delay.back(), i ) );
				line.erase( 0, pos );
			}
			//std::sort( delay_asc.begin(), delay_asc.end() );
			//std::sort( delay.begin(), delay.end() );
			//std::vector<uint32_t> delay_perm;
			//for ( auto const& delay_asc_each: delay_asc )
			//{
			//	delay_perm.emplace_back( delay_asc_each.second );
			//}
			// pos = line.find( ' ' );
			// const uint32_t md = std::stoul( line.substr( 0, pos++ ) );
			// std::cout << "md = " << md << "\n";
			// line.erase( 0, pos );

			std::vector<xag_network::signal> signals_md_opt( db_pis_.begin(), db_pis_.begin() + vars_num );

			while ( line.size() > 3 )
			{
				uint32_t signal_1, signal_2;
				pos = line.find( ' ' );
				signal_1 = std::stoul( line.substr( 0, pos++ ) );
				//std::cout << "[m] signal 1 is " << ( ( signal_1 % 2 ) ? "!" : "" )  << signal_1 / 2 << ", ";
				line.erase( 0, pos );
				pos = line.find( ' ' );
				signal_2 = std::stoul( line.substr( 0, pos++ ) );
				//std::cout << "[m] signal 2 is " << ( ( signal_2 % 2 ) ? "!" : "" )  << signal_2 / 2 << ".\n";
				line.erase( 0, pos );

				xag_network::signal signal1, signal2;
	    	if ( signal_1 == 0u )
	    	{
					signal1 = db_.get_constant( false );
	    	}
	    	else if ( signal_1 == 1u )
	    	{
					signal1 = db_.get_constant( true );
	    	}
	    	else
	    	{
	    		signal1 = signals_md_opt[signal_1 / 2 - 1] ^ ( signal_1 % 2 != 0 );
	    	}
	    	if ( signal_2 == 0u )
	    	{
					signal2 = db_.get_constant( false );
	    	}
	    	else if ( signal_2 == 1u )
	    	{
					signal2 = db_.get_constant( true );
	    	}
	    	else
	    	{
	    		signal2 = signals_md_opt[signal_2 / 2 - 1] ^ ( signal_2 % 2 != 0 );
	    	}

	    	if ( signal_1 > signal_2 )
	    	{
	    		signals_md_opt.emplace_back( db_.create_xor( signal1, signal2 ) );
	    	}
	    	else
	    	{
	    		signals_md_opt.emplace_back( db_.create_and( signal1, signal2 ) );
	    	}
	    }

	    const uint32_t signal_po = std::stoul( line );
	    //std::cout << "[m] po is " << ( ( signal_po % 2 ) ? "!" : "" )  << signal_po / 2 << ".\n";
	    const xag_network::signal po = signals_md_opt[signal_po / 2 - 1] ^ ( signal_po % 2 != 0 );
	    db_.create_po( po );

	    if ( ps_.verify_database )
	    {
	    	cut_view<xag_network> db_partial{ db_, db_pis_, po };
	    	kitty::static_truth_table<num_vars> repr_real, repr_db;
	    	kitty::create_from_hex_string( repr_real, repr_real_str );
	    	kitty::create_from_hex_string( repr_db, repr_db_str );

	    	auto result = simulate<kitty::static_truth_table<num_vars>>( db_partial )[0];
	    	if ( result != repr_db )
	    	{
	    		std::cerr << "[w] incorrect implementation for " << repr_db_str << "; "
	    							<< "whose function is " << kitty::to_hex( result ) << std::endl;
					repr_db_str = kitty::to_hex( result );

					const auto repr_correct = kitty::exact_spectral_canonization( result );
					if ( repr_correct != repr_real )
					{
						std::cerr << "[e] representative does not match\n";
						abort();
					}
				}
			}

			pfunc_md_->insert( std::make_pair( repr_real_str, std::make_tuple( repr_db_str, mc, delay, po ) ) );
		}

		std::cout << "[i] load db finished\n";
		std::cout << "[i] " << ( db_._storage )->nodes.size() << " nodes in the database XAG\n";

		on_the_fly_db_pis_.resize( num_vars );
		std::generate( on_the_fly_db_pis_.begin(), on_the_fly_db_pis_.end(), [&]() { return on_the_fly_db_.create_pi(); } );
	}

	template<class TT>
	int32_t estimate_cut_cost( TT const& func, std::vector<uint32_t> const& input_arrive ) const
	{
		//std::vector<uint32_t> perm;
		const auto func_ext = kitty::extend_to<num_vars>( func );
		kitty::static_truth_table<num_vars> real_repr;
		std::vector<kitty::detail::spectral_operation> trans;

		const auto cache_it = pclassify_cache_->find( func_ext );
		if ( cache_it != pclassify_cache_->end() )
		{
			++pst_->cache_hit;
			if ( !std::get<0>( cache_it->second ) )
			{
				return -1;
			}
			real_repr = std::get<1>( cache_it->second );
		}
		else
		{
			++pst_->cache_miss;
			const auto spectral = call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization_limit( func_ext, 100000, [&trans]( auto const& ops ) {
					std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
				} );
			} );

			pclassify_cache_->insert( std::make_pair( func_ext, std::make_tuple( spectral.second, spectral.first, trans ) ) );
			real_repr = spectral.first;

			if ( !spectral.second )
			{
				++pst_->classify_abort;
				return -1;
			}
		}

		auto search = pfunc_md_->find( kitty::to_hex( real_repr ) );
		if ( search != pfunc_md_->end() )
		{
			std::vector<uint32_t> delay = std::get<2>( search->second );
			if ( delay.size() != input_arrive.size() )
			{
				//kitty::print_hex( func );
				//std::cout << "\n";
				//std::cout << "[e] sizes of 'input_arrive'(" << input_arrive.size() << ") and 'delay'(" << delay.size() << ") don't match\n";
				if ( input_arrive.size() < delay.size() )
				{
					abort();
				}
				auto length_dif{ input_arrive.size() - delay.size() };
				for ( auto i{ 0 }; i < length_dif; ++i )
				{
					delay.emplace_back( 0u );
				}
				//abort();
			}

			std::string db_repr_str = std::get<0>( search->second );
			kitty::static_truth_table<num_vars> db_repr;
			kitty::create_from_hex_string( db_repr, db_repr_str );
			call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
					std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
				} );
			} );

			for ( auto const& op: trans )
			{
				switch ( op._kind )
				{
				case kitty::detail::spectral_operation::kind::permutation:
				{	
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					std::swap( delay[v1], delay[v2] );
					break;
				}
				case kitty::detail::spectral_operation::kind::spectral_translation:
				{
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					delay[v1] = std::max( delay[v1], delay[v2] );
					break;
				}
				}
			}

			//std::vector<std::pair<uint32_t, uint32_t>> input_arrive_dsc;
			//or ( auto i{ 0u }; i < input_arrive.size(); ++i )
			//{
			//	input_arrive_dsc.emplace_back( std::make_pair( input_arrive[i], i ) );
			//}
			//std::reverse( input_arrive_dsc.begin(), input_arrive_dsc.end() );

			//perm.resize( input_arrive.size() );
			int32_t cost{ 0 };
			//std::cout << "[m] ";
			for ( auto i{ 0u }; i < delay.size(); ++i )
			{
				//perm[input_arrive_dsc[i].second] = delay_perm[i];
				//cost = std::max( cost, input_arrive_dsc[i].first + delay[i] );
				cost = std::max( cost , static_cast<int32_t>( input_arrive[i] + delay[i] ) );
				//std::cout << "(" << ( i + 1 ) << ") input_arrive is " << input_arrive[i] << "\tdelay is " << delay[i] << "; ";
			}
			//std::cout << "\n";
			return cost;
		}

		++pst_->unknown_func_abort;
		return -1;
	}

	template<class TT, typename LeavesIterator>
	xag_network::signal rewrite_cut( TT const& func, LeavesIterator begin, LeavesIterator end )
	{
		const auto func_ext = kitty::extend_to<num_vars>( func );
		std::vector<kitty::detail::spectral_operation> trans;
		kitty::static_truth_table<num_vars> real_repr;

		const auto cache_it = pclassify_cache_->find( func_ext );
		if ( cache_it == pclassify_cache_->end() )
		{
			std::cout << "[e] illegal cut: unable to classify the local function\n";
			abort();
		}

		real_repr = std::get<1>( cache_it->second );
		trans = std::get<2>( cache_it->second );
		
		auto search = pfunc_md_->find( kitty::to_hex( real_repr ) );
		if ( search == pfunc_md_->end() )
		{
			std::cout << "[e] illegal cut: the XAG implementation of current cut is unavailable\n";
			abort();
		}

		std::string db_repr_str = std::get<0>( search->second );
		xag_network::signal po_db_repr = std::get<3>( search->second );
		kitty::static_truth_table<num_vars> db_repr;
		kitty::create_from_hex_string( db_repr, db_repr_str );

		call_with_stopwatch( pst_->time_classify, [&]() {
			return kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
				std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
			} );
		} );

		bool po_inv{ false };
		std::vector<xag_network::signal> po_xors;
		std::vector<xag_network::signal> pis( num_vars, ntk_.get_constant( false ) );
		std::copy( begin, end, pis.begin() );
		stopwatch t_construct( pst_->time_construct );

		for ( auto const& op: trans )
		{
			switch ( op._kind )
			{
			case kitty::detail::spectral_operation::kind::permutation:
			{	
				const auto v1 = log2( op._var1 );
				const auto v2 = log2( op._var2 );
				std::swap( pis[v1], pis[v2] );
				break;
			}
			case kitty::detail::spectral_operation::kind::input_negation:
			{	
				const auto v1 = log2( op._var1 );
				pis[v1] = !pis[v1];
				break;
			}
			case kitty::detail::spectral_operation::kind::output_negation:
			{	
				po_inv = !po_inv;
				break;
			}
			case kitty::detail::spectral_operation::kind::spectral_translation:
			{
				const auto v1 = log2( op._var1 );
				const auto v2 = log2( op._var2 );
				pis[v1] = ntk_.create_xor( pis[v1], pis[v2] );
				break;
			}
			case kitty::detail::spectral_operation::kind::disjoint_translation:
			{
				const auto v1 = log2( op._var1 );
				po_xors.emplace_back( pis[v1] );
				break;
			}
			default:
				abort();
			}
		}

		xag_network::signal po;
		if ( db_.is_constant( db_.get_node( po_db_repr ) ) )
		{
			po = ntk_.get_constant( db_.is_complemented( po_db_repr ) );
		}
		else
		{
			cut_view<xag_network> db_partial{ db_, db_pis_, po_db_repr };
			po = cleanup_dangling( db_partial, ntk_, pis.begin(), pis.end() ).front();
		}

		for ( auto const& po_xor: po_xors )
		{
			po = ntk_.create_xor( po, po_xor );
		}

		return ( po_inv ? !po : po );
	}

	void clear_cuts_fanout_rec( fanout_view<xag_network>& ntk_fanout_, network_cuts_t& cuts, cut_manager_t& cut_manager, xag_network::node const& n )
	{
		ntk_fanout_.foreach_fanout( n, [&]( auto const& no ) {
			auto const ind = ntk_.node_to_index( no );
			if ( cuts.cuts( ind ).size() > 0 )
			{
				cut_manager.clear_cuts( no );
				clear_cuts_fanout_rec( ntk_fanout_, cuts, cut_manager, no );
			}
		} );
	}

	void rewrite_critical_path()
	{
		depth_view<xag_network, mc_cost<xag_network>, false> ntk_md_{ ntk_ };
		fanout_view<xag_network> ntk_fanout_{ ntk_ };

		std::vector<xag_network::node> critical_path;
		ntk_.foreach_gate( [&]( auto const& n ) {
			if ( ntk_md_.is_on_critical_path( n ) )
			{
				critical_path.emplace_back( n );
			}
		} );

		cut_enumeration_stats cut_enum_st;
    network_cuts_t cuts( ntk_.size() + ( ntk_.size() >> 1 ) );
    cut_manager_t cut_manager( ntk_, ps_.cut_enumeration_ps, cut_enum_st, cuts );
    cut_manager.init_cuts();

    const auto md_before = ntk_md_.depth();
    bool is_first_critical_node{ true };
    //std::cout << "\n\n[m] there are " << critical_path.size() << " nodes on the critical path\n";
    progress_bar pbar{ critical_path.size(), "rewriting |{0}| node = {1:>4} / " + std::to_string( critical_path.size() ) + "   original md = " + std::to_string( md_before ), ps_.progress };

    for ( auto i{ 1u }; i <= critical_path.size(); ++i )
    {
    	pbar( i, i );
    	//std::cout << "\n\n[m] the " << i << "th node on the critical path:\n";

    	auto const n = critical_path[i - 1];
    	if ( ntk_fanout_.fanout_size( n ) == 0u )
  		{
  			continue;
  		}

    	if ( !is_first_critical_node )
    	{
    		ntk_md_.update_levels();
    	}
    	is_first_critical_node = false;

  		cut_manager.clear_cuts( n );
  		cut_manager.compute_cuts( n );

  		uint32_t cut_index{ 0u };
  		int32_t best_gain{ -1 };
  		uint32_t best_cut_index{ 0u };
  		//std::vector<uint32_t> best_perm;

  		uint32_t root_level_before = ntk_md_.level( n );
    	if ( root_level_before == 0u )
    	{
    		continue;
    	}

    	for ( auto& cut: cuts.cuts( ntk_.node_to_index( n ) ) )
    	{
    		++cut_index;

    		if ( cut->size() < ps_.min_cand_cut_size )
    		{
    			continue;
    		}

    		const auto tt = cuts.truth_table( *cut );
    		std::vector<uint32_t> input_arrive;
    		//std::vector<uint32_t> perm;
    		for ( auto l: *cut )
    		{
    			input_arrive.emplace_back( ntk_md_.level( ntk_.index_to_node( l ) ) );
    		}
    		int32_t root_level_after = estimate_cut_cost( tt, input_arrive );
    		if ( root_level_after < 0 )
    		{
    			continue;
    		}

    		//std::cout << "[m] the " << cut_index << "th cut of current node - after / before rewriting, MD is " << root_level_after << " / " << root_level_before << "; ";
    		int32_t current_gain = root_level_before - root_level_after;
    		//std::cout << " current gain / best gain is " << current_gain << " / " << best_gain << "\n";
    		if ( current_gain > best_gain )
    		{
    			//std::cout << "[m] best gain is updated\n";
    			best_gain = current_gain;
    			best_cut_index = cut_index - 1;
    			//best_perm = perm;
    		}
    	}

    	if ( best_gain > 0 || ( ps_.allow_zero_gain && best_gain == 0 ) )
    	{
    		auto cut = ( cuts.cuts( ntk_.node_to_index( n ) ) )[best_cut_index];
    		const auto tt = cuts.truth_table( cut );
    		std::array<xag_network::signal, num_vars> leaves;
    		auto ctr = 0u;
    		for ( auto l : cut )
    		{
    			leaves[ctr++] = ntk_.make_signal( ntk_.index_to_node( l ) );
    		}
    		//if ( leaves.size() != perm.size() )
    		//{
    		//	std::cout << "[e] number of leaves and number of entries of perm information are not consistent\n";
    		//	abort();
    		//}

    		/* rewrite current cut */
    		auto const signal_new = rewrite_cut( tt, leaves.begin(), leaves.end() );
    		//if ( n == ntk_.get_node( signal_new ) )
    		//{
    		//	std::cout << "[e] new implementation of current cut is the same as before\n";
    		//	abort();
    		//}
    		if ( n != ntk_.get_node( signal_new ) )
    		{
    			ntk_.substitute_node( n, signal_new );
    		}
    		
    		clear_cuts_fanout_rec( ntk_fanout_, cuts, cut_manager, n );
    		/* update depth information */
	    }
    }

    cleanup_dangling( ntk_ );

    ntk_md_.update_levels();
    std::cout << "\toptimized md = " << ntk_md_.depth() << "\n";
	}

	void rewrite_one_critical_node_a_time()
	{
		depth_view<xag_network, mc_cost<xag_network>, false> ntk_md_{ ntk_ };

		//std::vector<xag_network::node> critical_path;
		//ntk_.foreach_gate( [&]( auto const& n ) {
		//	if ( ntk_md_.is_on_critical_path( n ) )
		//	{
		//		critical_path.emplace_back( n );
		//	}
		//} );
		//std::cout << "[m] there are " << critical_path.size() << " nodes on the critical path\n";

		cut_enumeration_stats cut_enum_st;
    network_cuts_t cuts( ntk_.size() + ( ntk_.size() >> 1 ) );
    cut_manager_t cut_manager( ntk_, ps_.cut_enumeration_ps, cut_enum_st, cuts );
		cut_manager.init_cuts();

		ntk_.foreach_gate( [&]( auto const& n ) {
			if ( ntk_md_.is_on_critical_path( n ) )
			{
				//std::cout << "[i] processing node " << n << "\n";
		    uint32_t cut_index{ 0u };
	  		int32_t best_gain{ -1 };
	  		uint32_t best_cut_index{ 0u };

	  		cut_manager.clear_cuts( n );
	  		cut_manager.compute_cuts( n );

	  		uint32_t root_level_before = ntk_md_.level( n );
	    	if ( root_level_before == 0u )
	    	{
	    		return true;
	    	}

	    	for ( auto& cut: cuts.cuts( ntk_.node_to_index( n ) ) )
	    	{
	    		++cut_index;

	    		if ( cut->size() < ps_.min_cand_cut_size )
	    		{
	    			continue;
	    		}

	    		const auto tt = cuts.truth_table( *cut );
	    		std::vector<uint32_t> input_arrive;
	    		for ( auto l: *cut )
	    		{
	    			input_arrive.emplace_back( ntk_md_.level( ntk_.index_to_node( l ) ) );
	    		}
	    		int32_t root_level_after = estimate_cut_cost( tt, input_arrive );
	    		if ( root_level_after < 0 )
	    		{
	    			continue;
	    		}

	    		//std::cout << "[m] the " << cut_index << "th cut of current node - after / before rewriting, MD is " << root_level_after << " / " << root_level_before << "; ";
	    		int32_t current_gain = root_level_before - root_level_after;
	    		//std::cout << " current gain / best gain is " << current_gain << " / " << best_gain << "\n";
	    		if ( current_gain > best_gain )
	    		{
	    			//std::cout << "[m] best gain is updated\n";
	    			best_gain = current_gain;
	    			best_cut_index = cut_index - 1;
	    		}
	    	}

	    	if ( best_gain > 0 || ( allow_no_gain_move_ && best_gain == 0 ) )
	    	{
	    		auto cut = ( cuts.cuts( ntk_.node_to_index( n ) ) )[best_cut_index];
	    		const auto tt = cuts.truth_table( cut );
	    		std::array<xag_network::signal, num_vars> leaves;
	    		auto ctr = 0u;
	    		for ( auto l : cut )
	    		{
	    			leaves[ctr++] = ntk_.make_signal( ntk_.index_to_node( l ) );
	    		}

	    		/* rewrite current cut */
	    		auto const signal_new = rewrite_cut( tt, leaves.begin(), leaves.end() );
	    		//if ( n == ntk_.get_node( signal_new ) )
	    		//{
	    		//	std::cout << "[e] new implementation of current cut is the same as before\n";
	    		//	abort();
	    		//}
	    		if ( n != ntk_.get_node( signal_new ) )
	    		{
	    			//std::cout << "[m] ----------rewiting operated----------\n";
	    			ntk_.substitute_node( n, signal_new );
	    			return false;
	    		}
	    	}
	    }
	    return true;
		} );

		cleanup_dangling( ntk_ );
	}

	template<class TT>
	int32_t compute_cut_cost( TT const& func, std::vector<uint32_t> const& input_arrive ) const
	{
		const auto func_ext = kitty::extend_to<num_vars>( func );
		kitty::static_truth_table<num_vars> real_repr;
		std::vector<kitty::detail::spectral_operation> trans;

		const auto cache_it = pclassify_cache_->find( func_ext );
		if ( cache_it != pclassify_cache_->end() )
		{
			++pst_->cache_hit;
			if ( !std::get<0>( cache_it->second ) )
			{
				return -1;
			}
			real_repr = std::get<1>( cache_it->second );
		}
		else
		{
			++pst_->cache_miss;
			const auto spectral = call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization_limit( func_ext, 100000, [&trans]( auto const& ops ) {
					std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
				} );
			} );

			pclassify_cache_->insert( std::make_pair( func_ext, std::make_tuple( spectral.second, spectral.first, trans ) ) );
			real_repr = spectral.first;

			if ( !spectral.second )
			{
				++pst_->classify_abort;
				return -1;
			}
		}

		auto search = pfunc_md_->find( kitty::to_hex( real_repr ) );
		if ( search != pfunc_md_->end() )
		{
			std::vector<uint32_t> delay = std::get<2>( search->second );
			if ( delay.size() != input_arrive.size() )
			{
				//kitty::print_hex( func );
				//std::cout << "\n";
				//std::cout << "[e] sizes of 'input_arrive'(" << input_arrive.size() << ") and 'delay'(" << delay.size() << ") don't match\n";
				if ( input_arrive.size() < delay.size() )
				{
					abort();
				}
				auto length_dif{ input_arrive.size() - delay.size() };
				for ( auto i{ 0 }; i < length_dif; ++i )
				{
					delay.emplace_back( 0u );
				}
				//abort();
			}

			std::string db_repr_str = std::get<0>( search->second );
			kitty::static_truth_table<num_vars> db_repr;
			kitty::create_from_hex_string( db_repr, db_repr_str );
			call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
					std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
				} );
			} );

			for ( auto const& op: trans )
			{
				switch ( op._kind )
				{
				case kitty::detail::spectral_operation::kind::permutation:
				{	
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					std::swap( delay[v1], delay[v2] );
					break;
				}
				case kitty::detail::spectral_operation::kind::spectral_translation:
				{
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					delay[v1] = std::max( delay[v1], delay[v2] );
					break;
				}
				}
			}

			//std::vector<std::pair<uint32_t, uint32_t>> input_arrive_dsc;
			//or ( auto i{ 0u }; i < input_arrive.size(); ++i )
			//{
			//	input_arrive_dsc.emplace_back( std::make_pair( input_arrive[i], i ) );
			//}
			//std::reverse( input_arrive_dsc.begin(), input_arrive_dsc.end() );

			//perm.resize( input_arrive.size() );
			int32_t cost{ 0 };
			//std::cout << "[m] ";
			for ( auto i{ 0u }; i < delay.size(); ++i )
			{
				//perm[input_arrive_dsc[i].second] = delay_perm[i];
				//cost = std::max( cost, input_arrive_dsc[i].first + delay[i] );
				cost = std::max( cost , static_cast<int32_t>( input_arrive[i] + delay[i] ) );
				//std::cout << "(" << ( i + 1 ) << ") input_arrive is " << input_arrive[i] << "\tdelay is " << delay[i] << "; ";
			}
			//std::cout << "\n";
			return cost;
		}

		++pst_->unknown_func_abort;
		return -1;
	}

	template<class TT>
	int32_t on_the_fly_compute_cut_cost( TT const& func,
																			 uint8_t const& fmc, uint32_t const& md_max,
																			 std::vector<uint32_t> const& input_arrive )
	{
		/* TODO: seems like 'func's are originally 'func_ext's,  */
		/* i.e., static_truth_table<num_vars>                    */
		const auto func_ext = kitty::extend_to<num_vars>( func );
		auto func_changeable = kitty::extend_to<num_vars>( func );
		const auto blacklist_it = p_exact_impl_blacklist_->find( func_ext );
		if ( blacklist_it != p_exact_impl_blacklist_->end() )
		{
			if ( std::find( blacklist_it->second.begin(), blacklist_it->second.end(), input_arrive ) != blacklist_it->second.end() )
			{
				return -1;
			}
		}

		const auto cache_it = p_exact_impl_->find( func_ext );
		if ( cache_it != p_exact_impl_->end() )
		{
			for ( auto const& solution: cache_it->second )
			{
				if ( std::get<0>( solution ) == input_arrive )
				{
					return std::get<1>( solution );
				}
			}
		}

		std::optional<xag_network> pres;
		uint32_t md_min{ 0u };
		auto num_var = ( kitty::min_base_inplace( func_changeable ) ).size();
		kitty::dynamic_truth_table tt( num_var );
		std::string tt_str = kitty::to_hex( func_ext );
		kitty::create_from_hex_string( tt, tt_str.substr( 0, 1 << ( num_var - 2 ) ) );

		std::tie( md_min, pres ) = md_optimum_with_input_arrival_time( tt, fmc, md_max, input_arrive );
		if ( md_min == 0u )
		{
			auto const blacklist_it = p_exact_impl_blacklist_->find( func_ext );
			if ( blacklist_it != p_exact_impl_blacklist_->end() )
			{
				( blacklist_it->second ).emplace_back( input_arrive );
			}
			else
			{
				std::vector<std::vector<uint32_t>> input_arrives{ input_arrive };
				p_exact_impl_blacklist_->insert( std::make_pair( func_ext, input_arrives ) );
			}


			std::cout << "[m] no solution found by on-the-fly exact synthesis\n";
			

			return -1;
		}

		/* store the solution into the on-the-fly database */
		node_map<signal<xag_network>, xag_network> old2new( *pres );
		/* constants */
		old2new[pres->get_constant( false )] = on_the_fly_db_.get_constant( false );
		if ( pres->get_node( pres->get_constant( true ) ) != pres->get_node( pres->get_constant( false ) ) )
		{
			old2new[pres->get_constant( true )] = on_the_fly_db_.get_constant( true );
		}
		/* pis */
		pres->foreach_pi( [&]( auto const& pi, auto idx ) {
			old2new[pi] = on_the_fly_db_pis_[idx];
		} );
		/* gates */
		pres->foreach_gate( [&]( auto const& n ) {
			std::vector<signal<xag_network>> children( pres->fanin_size( n ) );
			pres->foreach_fanin( n, [&]( auto const& fi, auto i ) {
				children[i] = pres->is_complemented( fi ) ? on_the_fly_db_.create_not( old2new[fi] ) : old2new[fi];
			} );
			old2new[n] = on_the_fly_db_.clone_node( *pres, n, children );
		} );
		/* pos */
		xag_network::signal po_res{ on_the_fly_db_.get_constant( false ) };
		pres->foreach_po( [&]( auto const& po ) {
			auto const po_res_copy = pres->is_complemented( po ) ? on_the_fly_db_.create_not( old2new[po] ) : old2new[po];
			on_the_fly_db_.create_po( po_res_copy );
			po_res = po_res_copy;
		} );

		/* record this solution into 'p_exact_impl_' */
		if ( cache_it != p_exact_impl_->end() )
		{
			( cache_it->second ).emplace_back( std::make_tuple( input_arrive, md_min, po_res ) );


			std::cout << "[m] adding 'input_arrive' { ";
			for ( auto input_arrive_each: input_arrive )
			{
				std::cout << input_arrive_each << ", ";
			}
			std::cout << "} under truth table ";
			kitty::print_hex( func );
			std::cout << "\n";
		}
		else
		{
			std::vector<std::tuple<std::vector<uint32_t>, uint32_t, xag_network::signal>> solutions{ std::make_tuple( input_arrive, md_min, po_res ) };
			p_exact_impl_->insert( std::make_pair( func_ext, solutions ) );

			std::cout << "[m] inserting entry of truth table ";
			kitty::print_hex( func );
			std::cout << " with 'input_arrive' { ";
			for ( auto input_arrive_each: input_arrive )
			{
				std::cout << input_arrive_each << ", ";
			}
			std::cout << "}\n";
		}

		return md_min;
	}

	template <class TT>
	std::pair<int32_t, uint32_t> compute_cut_cost_return_fmc( TT const& func, std::vector<uint32_t> const& input_arrive ) const
	{
		const auto func_ext = kitty::extend_to<num_vars>( func );
		kitty::static_truth_table<num_vars> real_repr;
		std::vector<kitty::detail::spectral_operation> trans;
		uint32_t fmc{ 0u };

		const auto cache_it = pclassify_cache_->find( func_ext );
		if ( cache_it != pclassify_cache_->end() )
		{
			++pst_->cache_hit;
			if ( !std::get<0>( cache_it->second ) )
			{
				return { -1, 0u };
			}
			real_repr = std::get<1>( cache_it->second );
		}
		else
		{
			++pst_->cache_miss;
			const auto spectral = call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization_limit( func_ext, 100000, [&trans]( auto const& ops ) {
					std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
				} );
			} );

			pclassify_cache_->insert( std::make_pair( func_ext, std::make_tuple( spectral.second, spectral.first, trans ) ) );
			real_repr = spectral.first;

			if ( !spectral.second )
			{
				++pst_->classify_abort;
				return { -1, 0u };
			}
		}

		auto search = pfunc_md_->find( kitty::to_hex( real_repr ) );
		if ( search != pfunc_md_->end() )
		{
			fmc = std::get<1>( search->second );
			std::vector<uint32_t> delay = std::get<2>( search->second );
			if ( delay.size() != input_arrive.size() )
			{
				//kitty::print_hex( func );
				//std::cout << "\n";
				//std::cout << "[e] sizes of 'input_arrive'(" << input_arrive.size() << ") and 'delay'(" << delay.size() << ") don't match\n";
				if ( input_arrive.size() < delay.size() )
				{
					abort();
				}
				auto length_dif{ input_arrive.size() - delay.size() };
				for ( auto i{ 0 }; i < length_dif; ++i )
				{
					delay.emplace_back( 0u );
				}
				//abort();
			}

			std::string db_repr_str = std::get<0>( search->second );
			kitty::static_truth_table<num_vars> db_repr;
			kitty::create_from_hex_string( db_repr, db_repr_str );
			call_with_stopwatch( pst_->time_classify, [&]() {
				return kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
					std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
				} );
			} );

			for ( auto const& op: trans )
			{
				switch ( op._kind )
				{
				case kitty::detail::spectral_operation::kind::permutation:
				{	
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					std::swap( delay[v1], delay[v2] );
					break;
				}
				case kitty::detail::spectral_operation::kind::spectral_translation:
				{
					const auto v1 = log2( op._var1 );
					const auto v2 = log2( op._var2 );
					delay[v1] = std::max( delay[v1], delay[v2] );
					break;
				}
				}
			}

			int32_t cost{ 0 };
			//std::cout << "[m] ";
			for ( auto i{ 0u }; i < delay.size(); ++i )
			{
				cost = std::max( cost , static_cast<int32_t>( input_arrive[i] + delay[i] ) );
				//std::cout << "(" << ( i + 1 ) << ") input_arrive is " << input_arrive[i] << "\tdelay is " << delay[i] << "; ";
			}
			//std::cout << "\n";
			//return std::make_pair( cost, fmc );
			return { cost, fmc };
		}

		++pst_->unknown_func_abort;
		return { -1, 0u };
	}

	template<class TT, typename LeavesIterator>
	xag_network::signal on_the_fly_rewrite_cut( TT const& func, std::vector<uint32_t> const& input_arrive,
																							LeavesIterator begin, LeavesIterator end )
	{
		/* it is assured that the optimal implementation is in the on-the-fly database */
		const auto func_ext = kitty::extend_to<num_vars>( func );
		const auto cache_it = p_exact_impl_->find( func_ext );
		if ( cache_it == p_exact_impl_->end() )
		{
			std::cout << "[e] illegal cut: optimum implementation not found\n";
			abort();
		}
		xag_network::signal po_db_repr{ on_the_fly_db_.get_constant( false ) };
		for ( auto const& solution: cache_it->second )
		{
			if ( std::get<0>( solution ) == input_arrive )
			{
				po_db_repr = std::get<2>( solution );
				break;
			}
		}
		if ( po_db_repr == on_the_fly_db_.get_constant( false ) )
		{
			std::cout << "[e] illegal cut: optimum implementation not found\n";
			abort();
		}

		std::vector<xag_network::signal> pis( num_vars, ntk_.get_constant( false ) );
		std::copy( begin, end, pis.begin() );
		xag_network::signal po;
		if ( on_the_fly_db_.is_constant( on_the_fly_db_.get_node( po_db_repr ) ) )
		{
			po = on_the_fly_db_.get_constant( on_the_fly_db_.is_complemented( po_db_repr ) );
		}
		else
		{
			cut_view<xag_network> db_partial{ on_the_fly_db_, on_the_fly_db_pis_, po_db_repr };
			po = cleanup_dangling( db_partial, ntk_, pis.begin(), pis.end() ).front();
		}

		return po;
	}

	void on_the_fly_rewrite_one_critical_node_a_time()
	{
		depth_view<xag_network, mc_cost<xag_network>, false> ntk_md_{ ntk_ };

		cut_enumeration_stats cut_enum_st;
    network_cuts_t cuts( ntk_.size() + ( ntk_.size() >> 1 ) );
    cut_manager_t cut_manager( ntk_, ps_.cut_enumeration_ps, cut_enum_st, cuts );
		cut_manager.init_cuts();

		uint32_t rewriting_cntr{ 0u };

		ntk_.foreach_gate( [&]( auto const& n ) {
			if ( ntk_md_.is_on_critical_path( n ) )
			{
				std::cout << "\n[i] processing node " << n << "\n";
		    

		    uint32_t cut_index{ 0u };
	  		int32_t best_gain{ -1 };
	  		uint32_t best_cut_index{ 0u };
	  		std::vector<uint32_t> best_input_arrive;
	  		bool spectral_lib{ true };

	  		cut_manager.clear_cuts( n );
	  		cut_manager.compute_cuts( n );

	  		uint32_t root_level_before = ntk_md_.level( n );
	    	if ( root_level_before == 0u )
	    	{
	    		return true;
	    	}

	    	for ( auto& cut: cuts.cuts( ntk_.node_to_index( n ) ) )
	    	{
	    		++cut_index;

	    		if ( cut->size() < ps_.min_cand_cut_size )
	    		{
	    			continue;
	    		}

	    		/* data type of 'tt' is static truth table */
	    		const auto tt = cuts.truth_table( *cut );
	    		std::vector<uint32_t> input_arrive;
	    		bool all_the_same{ true };
	    		uint32_t offset{ 0u };
	    		offset = ~offset;
	    		for ( auto l: *cut )
	    		{
	    			uint32_t input_arrive_each{ ntk_md_.level( ntk_.index_to_node( l ) ) };
	    			if ( all_the_same && !input_arrive.empty() )
	    			{
	    				all_the_same = ( input_arrive_each == input_arrive.back() );
	    			}
	    			input_arrive.emplace_back( input_arrive_each );
	    			offset = ( input_arrive_each < offset ) ? input_arrive_each : offset;
	    		}
	    		std::vector<uint32_t> input_arrive_norm;
	    		for ( auto input_arrive_each: input_arrive )
	    		{
	    			input_arrive_norm.emplace_back( input_arrive_each - offset );
	    		}

	    		int32_t md_max{ -1 };
	    		uint32_t fmc{ 0u };
	    		std::tie( md_max, fmc ) = compute_cut_cost_return_fmc( tt, input_arrive_norm );
	    		if ( md_max < 0 )
	    		{
	    			continue;
	    		}

	    		//int32_t md_min = all_the_same ? md_max : on_the_fly_compute_cut_cost( tt, fmc, static_cast<uint32_t>( md_max ), input_arrive_norm );
	    		int32_t md_min{ -1 };
	    		if ( all_the_same )
	    		{
	    			md_min = md_max;
	    		}
	    		else
	    		{
	    			md_min = on_the_fly_compute_cut_cost( tt, fmc, static_cast<uint32_t>( md_max ), input_arrive_norm );
	    		}
	    		if ( md_min < 0 )
	    		{
	    			md_min = md_max;
	    		}

	    		std::cout << "[m] the " << cut_index << "th cut of current node: ";
	    		int32_t current_gain = root_level_before - ( md_min + offset );
	    		std::cout << " current gain / best gain is " << current_gain << " / " << best_gain << "\n";
	    		if ( current_gain > best_gain )
	    		{
	    			//std::cout << "[m] best gain is updated\n";
	    			best_gain = current_gain;
	    			best_cut_index = cut_index - 1;
	    			spectral_lib = ( md_min == md_max );
	    			best_input_arrive = input_arrive;
	    		}
	    	}

	    	if ( best_gain > 0 || ( allow_no_gain_move_ && best_gain == 0 ) )
	    	{
	    		std::cout << "[m] rewrite current cut on root " << n << " saves md by " << best_gain << "\n";


	    		auto cut = ( cuts.cuts( ntk_.node_to_index( n ) ) )[best_cut_index];
	    		const auto tt = cuts.truth_table( cut );
	    		std::array<xag_network::signal, num_vars> leaves;
	    		auto ctr = 0u;
	    		for ( auto l : cut )
	    		{
	    			leaves[ctr++] = ntk_.make_signal( ntk_.index_to_node( l ) );
	    		}

	    		/* rewrite current cut */
	    		auto signal_new = ntk_.get_constant( false );
	    		if ( spectral_lib )
	    		{
	    			signal_new = rewrite_cut( tt, leaves.begin(), leaves.end() );
	    		}
	    		else
	    		{
	    			signal_new = on_the_fly_rewrite_cut( tt, best_input_arrive, leaves.begin(), leaves.end() );
	    		}
	    		if ( n != ntk_.get_node( signal_new ) )
	    		{
	    			/* TODO: this does work as expected... */
	    			/* even if a substitution is operated, it won't jump out from the loop */
	    			ntk_.substitute_node( n, signal_new );


	    			++rewriting_cntr;


	    			//return false;
	    			return true;
	    		}
	    	}
	    }
	    return true;
		} );

		cleanup_dangling( ntk_ );


		std::cout << "[m] " << rewriting_cntr << " rewritings are operated eventually.\n"; 
	}

private:
	std::string dbname_;
	xag_network db_;
	std::vector<xag_network::signal> db_pis_;
	std::shared_ptr<std::unordered_map<std::string, std::tuple<std::string, uint32_t, std::vector<uint32_t>, xag_network::signal>>> pfunc_md_;
	std::shared_ptr<std::unordered_map<kitty::static_truth_table<num_vars>, std::tuple<bool, kitty::static_truth_table<num_vars>, std::vector<kitty::detail::spectral_operation>>, kitty::hash<kitty::static_truth_table<num_vars>>>> pclassify_cache_;

	xag_network on_the_fly_db_;
	std::vector<xag_network::signal> on_the_fly_db_pis_;
	/* TODO: change data type of 'input_arrive' to array */
	std::shared_ptr<std::unordered_map<kitty::static_truth_table<num_vars>, std::vector<std::tuple<std::vector<uint32_t>, uint32_t, xag_network::signal>>, kitty::hash<kitty::static_truth_table<num_vars>>>> p_exact_impl_;
	std::shared_ptr<std::unordered_map<kitty::static_truth_table<num_vars>, std::vector<std::vector<uint32_t>>, kitty::hash<kitty::static_truth_table<num_vars>>>> p_exact_impl_blacklist_;

	xag_network ntk_;
	md_rewriting_params const& ps_;
	md_rewriting_stats* pst_;

	bool allow_no_gain_move_;
};

} /* namespace detail */

} /* namespace mockturtle */
