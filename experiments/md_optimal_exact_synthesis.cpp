#include <fstream>
#include <iostream>

#include <kitty/constructors.hpp>
#include <kitty/operators.hpp>
#include <mockturtle/algorithms/md_optimal_exact_synthesis.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/topo_view.hpp>

#include <experiments.hpp>

namespace details
{

template<class Ntk>
struct num_and
{
	uint32_t operator()( Ntk const& ntk, typename Ntk::node const& n ) const
	{
		return ( ntk.is_and( n ) ? 1u : 0u );
	}
};

} /* namespace details */

typedef struct representatives
{
	std::string func;
	uint32_t mc;
	uint32_t md;
} representatives;

static std::vector<representatives> affine_representatives_5 = {
	{"00000000", 0u, 0u},
	{"80000000", 4u, 3u},
	{"80008000", 3u, 2u},
	{"00808080", 4u, 4u},
	{"80808080", 2u, 2u},
	{"08888000", 3u, 2u},
	{"aa2a2a80", 4u, 4u},
	{"88080808", 4u, 4u},
	{"2888a000", 3u, 2u},
	{"f7788000", 3u, 2u},
	{"a8202020", 3u, 3u},
	{"08880888", 3u, 3u},
	{"bd686868", 4u, 3u},
	{"aa808080", 4u, 3u},
	{"7e686868", 4u, 3u},
	{"2208a208", 4u, 4u},
	{"08888888", 4u, 4u},
	{"88888888", 1u, 1u},
	{"ea404040", 3u, 2u},
	{"2a802a80", 2u, 2u},
	{"73d28c88", 3u, 3u},
	{"ea808080", 3u, 2u},
	{"a28280a0", 3u, 3u},
	{"13284c88", 3u, 3u},
	{"a2220888", 3u, 3u},
	{"aae6da80", 4u, 4u},
	{"58d87888", 4u, 3u},
	{"8c88ac28", 4u, 3u},
	{"8880f880", 4u, 4u},
	{"9ee8e888", 4u, 3u},
	{"4268c268", 4u, 4u},
	{"16704c80", 4u, 2u},
	{"78888888", 3u, 2u},
	{"4966bac0", 4u, 2u},
	{"372840a0", 4u, 3u},
	{"5208d288", 3u, 3u},
	{"7ca00428", 4u, 3u},
	{"f8880888", 3u, 3u},
	{"2ec0ae40", 4u, 4u},
	{"f888f888", 3u, 2u},
	{"58362ec0", 4u, 4u},
	{"0eb8f6c0", 4u, 3u},
	{"567cea40", 4u, 4u},
	{"f8887888", 4u, 3u},
	{"78887888", 2u, 1u},
	{"e72890a0", 4u, 2u},
	{"268cea40", 3u, 2u},
	{"6248eac0", 4u, 4u}
};

/* Among the 48 implementations in 'affine_representatives_5', 22 better implementations are found */
static std::vector<representatives> affine_representatives_5_updated = {
	{"00808080", 4u, 3u},
	{"aa2a2a80", 4u, 3u},
	{"88080808", 4u, 3u},
	{"a8202020", 4u, 2u}, /* smc larger than fmc */
	{"08880888", 3u, 2u},
	{"2208a208", 4u, 3u},
	{"08888888", 4u, 3u},
	{"73d28c88", 4u, 2u}, /* smc larger than fmc */
	{"a28280a0", 4u, 2u}, /* smc larger than fmc */
	{"13284c88", 3u, 2u},
	{"a2220888", 4u, 2u}, /* smc larger than fmc */
	{"aae6da80", 4u, 3u},
	{"8880f880", 4u, 3u},
	{"4268c268", 4u, 3u},
	{"372840a0", 4u, 2u},
	{"5208d288", 4u, 2u}, /* smc larger than fmc */
	{"7ca00428", 4u, 2u},
	{"f8880888", 3u, 2u},
	{"2ec0ae40", 4u, 3u},
	{"58362ec0", 4u, 3u},
	{"567cea40", 4u, 3u},
	{"6248eac0", 4u, 3u}
};

/* Among the 22 implementations in 'affine_representatives_5_update', 2 better implementations are found */
static std::vector<representatives> affine_representatives_5_updated_updated = {
	{"2ec0ae40", 4u, 2u},
	{"6248eac0", 4u, 2u}
};

static std::vector<representatives> affine_representatives_5_eventual = {
	{"00000000", 0u, 0u},
	{"80000000", 4u, 3u},
	{"80008000", 3u, 2u},
	{"00808080", 4u, 3u},
	{"80808080", 2u, 2u},
	{"08888000", 3u, 2u},
	{"aa2a2a80", 4u, 3u},
	{"88080808", 4u, 3u},
	{"2888a000", 3u, 2u},
	{"f7788000", 3u, 2u},
	{"a8202020", 4u, 2u},
	{"08880888", 3u, 2u},
	{"bd686868", 4u, 3u},
	{"aa808080", 4u, 3u},
	{"7e686868", 4u, 3u},
	{"2208a208", 4u, 3u},
	{"08888888", 4u, 3u},
	{"88888888", 1u, 1u},
	{"ea404040", 3u, 2u},
	{"2a802a80", 2u, 2u},
	{"73d28c88", 4u, 2u},
	{"ea808080", 3u, 2u},
	{"a28280a0", 4u, 2u},
	{"13284c88", 3u, 2u},
	{"a2220888", 4u, 2u},
	{"aae6da80", 4u, 3u},
	{"58d87888", 4u, 3u},
	{"8c88ac28", 4u, 3u},
	{"8880f880", 4u, 3u},
	{"9ee8e888", 4u, 3u},
	{"4268c268", 4u, 3u},
	{"16704c80", 4u, 2u},
	{"78888888", 3u, 2u},
	{"4966bac0", 4u, 2u},
	{"372840a0", 4u, 2u},
	{"5208d288", 4u, 2u},
	{"7ca00428", 4u, 2u},
	{"f8880888", 3u, 2u},
	{"2ec0ae40", 4u, 2u},
	{"f888f888", 3u, 2u},
	{"58362ec0", 4u, 3u},
	{"0eb8f6c0", 4u, 3u},
	{"567cea40", 4u, 3u},
	{"f8887888", 4u, 3u},
	{"78887888", 2u, 1u},
	{"e72890a0", 4u, 2u},
	{"268cea40", 3u, 2u},
	{"6248eac0", 4u, 2u}
};

typedef struct AND_ladder
{
	std::vector<uint32_t> and_ladder;
	uint32_t mc;
	uint32_t md;
} AND_ladder;

static const std::vector<AND_ladder> and_ladders_lib = {
	{ { 1u, 1u, 1u }, 3u, 3u }, 
	{ { 1u, 1u, 2u }, 4u, 3u }, 
	{ { 1u, 2u, 1u }, 4u, 3u }, 
	{ { 2u, 1u, 1u }, 4u, 3u }, 
	{ { 1u, 1u, 3u }, 5u, 3u }, 
	{ { 1u, 2u, 2u }, 5u, 3u }, 
	{ { 1u, 3u, 1u }, 5u, 3u }, 
	{ { 2u, 1u, 2u }, 5u, 3u }, 
	{ { 2u, 2u, 1u }, 5u, 3u }, 
	{ { 3u, 1u, 1u }, 5u, 3u }, 
	{ { 1u, 1u, 4u }, 6u, 3u }, 
	{ { 1u, 2u, 3u }, 6u, 3u }, 
	{ { 1u, 3u, 2u }, 6u, 3u }, 
	{ { 1u, 4u, 1u }, 6u, 3u }, 
	{ { 2u, 1u, 3u }, 6u, 3u }, 
	{ { 2u, 2u, 2u }, 6u, 3u }, 
	{ { 2u, 3u, 1u }, 6u, 3u }, 
	{ { 3u, 1u, 2u }, 6u, 3u }, 
	{ { 3u, 2u, 1u }, 6u, 3u }, 
	{ { 4u, 1u, 1u }, 6u, 3u }, 
	{ { 1u, 1u, 5u }, 7u, 3u }, 
	{ { 1u, 2u, 4u }, 7u, 3u }, 
	{ { 1u, 3u, 3u }, 7u, 3u }, 
	{ { 1u, 4u, 2u }, 7u, 3u }, 
	{ { 1u, 5u, 1u }, 7u, 3u }, 
	{ { 2u, 1u, 4u }, 7u, 3u }, 
	{ { 2u, 2u, 3u }, 7u, 3u }, 
	{ { 2u, 3u, 2u }, 7u, 3u }, 
	{ { 2u, 4u, 1u }, 7u, 3u }, 
	{ { 3u, 1u, 3u }, 7u, 3u }, 
	{ { 3u, 2u, 2u }, 7u, 3u }, 
	{ { 3u, 3u, 1u }, 7u, 3u }, 
	{ { 4u, 1u, 2u }, 7u, 3u }, 
	{ { 4u, 2u, 1u }, 7u, 3u }, 
	{ { 5u, 1u, 1u }, 7u, 3u }, 
	{ { 1u, 1u }, 2u, 2u }, 
	{ { 1u, 2u }, 3u, 2u }, 
	{ { 2u, 1u }, 3u, 2u }, 
	{ { 1u, 3u }, 4u, 2u }, 
	{ { 2u, 2u }, 4u, 2u }, 
	{ { 3u, 1u }, 4u, 2u }, 
	{ { 1u, 4u }, 5u, 2u }, 
	{ { 2u, 3u }, 5u, 2u }, 
	{ { 3u, 2u }, 5u, 2u }, 
	{ { 4u, 1u }, 5u, 2u }, 
	{ { 1u, 5u }, 6u, 2u }, 
	{ { 2u, 4u }, 6u, 2u }, 
	{ { 3u, 3u }, 6u, 2u }, 
	{ { 4u, 2u }, 6u, 2u }, 
	{ { 5u, 1u }, 6u, 2u }, 
	{ { 1u, 6u }, 7u, 2u }, 
	{ { 2u, 5u }, 7u, 2u }, 
	{ { 3u, 4u }, 7u, 2u }, 
	{ { 4u, 3u }, 7u, 2u }, 
	{ { 5u, 2u }, 7u, 2u }, 
	{ { 6u, 1u }, 7u, 2u }, 
	{ { 1u, 7u }, 8u, 2u }, 
	{ { 2u, 6u }, 8u, 2u }, 
	{ { 3u, 5u }, 8u, 2u }, 
	{ { 4u, 4u }, 8u, 2u }, 
	{ { 5u, 3u }, 8u, 2u }, 
	{ { 6u, 2u }, 8u, 2u }, 
	{ { 7u, 1u }, 8u, 2u }, 
	{ { 1u }, 1u, 1u }, 
	{ { 2u }, 2u, 1u }, 
	{ { 3u }, 3u, 1u }, 
	{ { 4u }, 4u, 1u }, 
	{ { 5u }, 5u, 1u }, 
	{ { 6u }, 6u, 1u }, 
	{ { 7u }, 7u, 1u }, 
	{ { 8u }, 8u, 1u }, 
	{ { 9u }, 9u, 1u }, 
	{ { 10u }, 10u, 1u }, 
	{ { 11u }, 11u, 1u }, 
	{ { 12u }, 12u, 1u }, 
	{ { 13u }, 13u, 1u }, 
	{ { 14u }, 14u, 1u }, 
	{ { 15u }, 15u, 1u }
};

void profile_mc_optimal_xags( std::string const& filename )
{
	std::cout << "static std::vector<representatives> affine_representatives_5 = {\n";
	std::ifstream db;
	db.open( filename, std::ios::in );
	std::string line;
	uint32_t pos{ 0u };
	while ( std::getline( db, line ) )
	{
		pos = static_cast<uint32_t>( line.find( 'x' ) );
    auto tt_str = line.substr( ++pos, 8u );
    pos += 9u;
    line.erase( 0, pos );

    kitty::dynamic_truth_table tt( 5u );
    kitty::create_from_hex_string( tt, tt_str );
    auto num_vars = ( kitty::min_base_inplace( tt ) ).size();

    mockturtle::xag_network xag_mc_opt;
    std::vector<mockturtle::xag_network::signal> signals_mc_opt( num_vars );
    std::generate( signals_mc_opt.begin(), signals_mc_opt.end(), [&]() { return xag_mc_opt.create_pi(); } );

    while ( line.size() > 3 )
  	{
   		uint32_t signal_1, signal_2;
    	signal_1 = std::stoul( line.substr( 0, line.find( ',' ) ) );
    	line.erase( 0, line.find( ' ' ) + 1 );
    	signal_2 = std::stoul( line.substr( 0, line.find( ',' ) ) );
    	line.erase( 0, line.find( ' ' ) + 1 );

    	mockturtle::xag_network::signal signal1, signal2;
    	if ( signal_1 == 0u )
    	{
				signal1 = xag_mc_opt.get_constant( false );
    	}
    	else if ( signal_1 == 1u )
    	{
				signal1 = xag_mc_opt.get_constant( true );
    	}
    	else
    	{
    		signal1 = signals_mc_opt[signal_1 / 2 - 1] ^ ( signal_1 % 2 != 0 );
    	}
    	if ( signal_2 == 0u )
    	{
				signal2 = xag_mc_opt.get_constant( false );
    	}
    	else if ( signal_2 == 1u )
    	{
				signal2 = xag_mc_opt.get_constant( true );
    	}
    	else
    	{
    		signal2 = signals_mc_opt[signal_2 / 2 - 1] ^ ( signal_2 % 2 != 0 );
    	}
    	if ( signal_1 > signal_2 )
    	{
    		signals_mc_opt.emplace_back( xag_mc_opt.create_xor( signal1, signal2 ) );
    	}
    	else
    	{
    		signals_mc_opt.emplace_back( xag_mc_opt.create_and( signal1, signal2 ) );
    	}
  	}

  	const uint32_t signal_po = std::stoul( line );
  	xag_mc_opt.create_po( signals_mc_opt[signal_po / 2 - 1] ^ ( signal_po % 2 != 0 ) );
  	mockturtle::depth_view<mockturtle::xag_network, details::num_and<mockturtle::xag_network>, false> xag_mc_opt_md{ xag_mc_opt };

  	std::cout << "\t{\"";
  	std::cout << tt_str;
  	std::cout << "\", , " << xag_mc_opt_md.depth();
  	std::cout << "u},\n";
  }
  std::cout << "};\n";
}

void analyze_mc_optimal_xags()
{
	std::vector<int32_t> max_mc_of_interest( 3 );

	for ( auto const& repr: affine_representatives_5 )
	{
		if ( repr.mc > 0 && ( repr.md == 1 ) )
		{
			continue;
		}

		int32_t md_of_interest = repr.md - 1;
		int32_t mc_of_interest_up = repr.mc * repr.md * repr.md / ( md_of_interest * md_of_interest );
		if ( ( mc_of_interest_up * md_of_interest * md_of_interest ) == ( repr.mc * repr.md * repr.md ) )
		{
			--mc_of_interest_up;
		}

		max_mc_of_interest[md_of_interest - 1] = std::max( mc_of_interest_up, max_mc_of_interest[md_of_interest - 1] );

	}

	for ( auto d{ 1u }; d <= max_mc_of_interest.size(); ++d )
	{
		std::cout << "For MD = " << d << ", the largest MC of interest is " << max_mc_of_interest[d - 1] << "\n";
	}
}

void md_optimal_exact_synthesis()
{
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, bool> exp_res( "md_optimal_xag_test", "function", "mc_before", "mc_after", "md_before", "md_after", "runtime[s]", "found" );

	uint32_t cases_impr{ 0u };

	for ( auto const& benchmark: affine_representatives_5 )
	//for ( auto const& benchmark: affine_representatives_5_updated )
	//for ( auto const& benchmark: affine_representatives_5_updated_updated )
	{
		std::cout << "[i] processing " << benchmark.func << "\n";
		if ( benchmark.func == "00000000" )
		{
			exp_res( benchmark.func, 0u, 0u, 0u, 0u, 0., true );
			continue;
		}
		if ( ( benchmark.md == 1u ) && ( benchmark.mc > 0u ) )
		{
			exp_res( benchmark.func, benchmark.mc, benchmark.mc, benchmark.md, benchmark.md, 0., true );
			continue;
		}
		kitty::dynamic_truth_table tt( 5u );
		kitty::create_from_hex_string( tt, benchmark.func );
		auto num_vars = ( kitty::min_base_inplace( tt ) ).size();
		uint32_t mc = benchmark.mc;
		uint32_t md = benchmark.md;
		kitty::dynamic_truth_table tt_min_base( num_vars );
		kitty::create_from_hex_string( tt_min_base, benchmark.func.substr( 0, 1 << ( num_vars - 2 ) ) );

		/* Look for the md-optimal XAG implementation */
		bool found{ false };
		const clock_t begin_time = clock();
		for ( auto i{ 0u }; i < and_ladders_lib.size(); ++i )
		{
			const auto ladder = and_ladders_lib[i];
			if ( ladder.md > ( md - 1u ) )
			{
				continue;
			}
			// if ( ladder.md < ( md - 1u ) )
			// {
			// 	/* if there is an XAG with an MD of md             */
			// 	/* there shall be also one with the MD of (md + 1) */
			// 	break;
			// }
			uint32_t mc_upper = mc * md * md / ( ladder.md * ladder.md );
			if ( ( mc_upper * ladder.md * ladder.md ) == ( mc * md * md ) )
			{
				--mc_upper;
			}
			if ( ( ladder.mc < mc ) || ( ladder.mc > mc_upper ) )
			{
				continue;
			}

			mockturtle::md_optimal_exact_synthesis_params ps;
			ps.verify_solution = true;

			mockturtle::md_optimal_exact_synthesis_stats st;

			const auto p_xag = md_optimal_exact_synthesis( tt_min_base, ladder.and_ladder, ps, &st );
			if ( p_xag )
			{
				float time_found = float( clock() - begin_time ) / CLOCKS_PER_SEC;
				exp_res( benchmark.func, benchmark.mc, ladder.mc, benchmark.md, ladder.md, time_found, true );
				++cases_impr;
				found = true;
				break;
			}
		}
		if ( !found )
		{
			float time_found = float( clock() - begin_time ) / CLOCKS_PER_SEC;
			exp_res( benchmark.func, benchmark.mc, benchmark.mc, benchmark.md, benchmark.md, time_found, false );
		}	
	}
	exp_res.save();
	exp_res.table();
	std::cout << "Among 48 functions, better implementations are found for " << cases_impr << " of them.\n";
	//std::cout << "Among 22 functions, better implementations are found for " << cases_impr << " of them.\n";
	//std::cout << "Among 2 functions, better implementations are found for " << cases_impr << " of them.\n";
}

template<class Ntk, class NodeCostFn>
void compute_input_to_output_delay( Ntk const& ntk_md, std::vector<uint32_t>& delay )
{
	mockturtle::node_map<uint32_t, mockturtle::xag_network> required( ntk_md, UINT32_MAX );
	mockturtle::topo_view<Ntk> ntk_topo{ ntk_md };
	uint32_t md = ntk_topo.depth();

	ntk_topo.foreach_po( [&]( auto const po ) {
		required[po] = md;
	} );

	ntk_topo.foreach_node_reverse( [&]( auto const& n ) {
		uint32_t cost_n = NodeCostFn{}( ntk_topo, n );
		ntk_topo.foreach_fanin( n, [&]( auto const& fi ) {
			required[fi] = std::min( required[fi], required[n] - cost_n );
		} );
	} );

	uint32_t delay_max{ 0u };
	ntk_topo.foreach_pi( [&]( auto const& pi ) {
		delay.emplace_back( md - required[ntk_topo.make_signal( pi )] );
		delay_max = std::max( delay_max, delay.back() );
	} );

	if ( delay_max != md )
	{
		std::cout << "[e] max delay(" << delay_max << ") and MC(" << md << ") are non consistent\n";
		std::cout << "[e] function 'compute_input_to_output_delay' is functionally incorrect\n";
	  abort();
	}
}

void md_optimal_exact_synthesis_gendb( std::string const filename )
{
	std::ifstream db_mc;
	db_mc.open( "../experiments/db_mc_5", std::ios::in );
	std::string line;
	uint32_t pos{ 0u };
	uint32_t benchmark_cnt{ 1u };

	while ( std::getline( db_mc, line ) )
	{
		representatives benchmark = affine_representatives_5_eventual[benchmark_cnt];
		pos = static_cast<uint32_t>( line.find( 'x' ) );
    auto tt_str = line.substr( ++pos, 8u );
    std::cout << "[i] processing " << benchmark.func << "(" << tt_str << ")\n";

    if ( tt_str != benchmark.func )
    {
    	std::cout << "[e] functions don't match\n";
    	abort();
    }
    pos += 9u;
    line.erase( 0, pos );
    kitty::static_truth_table<5u> tt_static;
    kitty::create_from_hex_string( tt_static, tt_str );
		auto repr = kitty::exact_spectral_canonization( tt_static );
    std::string repr_str = kitty::to_hex( repr );

    kitty::dynamic_truth_table tt( 5u );
    kitty::create_from_hex_string( tt, tt_str );
    auto num_vars = ( kitty::min_base_inplace( tt ) ).size();
    uint32_t mc_target = benchmark.mc;
    uint32_t md_target = benchmark.md;

    /* Reconstruct the mc-optimal XAG implementation */
    mockturtle::xag_network xag_mc_opt;
    std::vector<mockturtle::xag_network::signal> signals_mc_opt( num_vars );
    std::generate( signals_mc_opt.begin(), signals_mc_opt.end(), [&]() { return xag_mc_opt.create_pi(); } );

    while ( line.size() > 3 )
  	{
   		uint32_t signal_1, signal_2;
    	signal_1 = std::stoul( line.substr( 0, line.find( ',' ) ) );
    	line.erase( 0, line.find( ' ' ) + 1 );
    	signal_2 = std::stoul( line.substr( 0, line.find( ',' ) ) );
    	line.erase( 0, line.find( ' ' ) + 1 );

    	mockturtle::xag_network::signal signal1, signal2;
    	if ( signal_1 == 0u )
    	{
				signal1 = xag_mc_opt.get_constant( false );
    	}
    	else if ( signal_1 == 1u )
    	{
				signal1 = xag_mc_opt.get_constant( true );
    	}
    	else
    	{
    		signal1 = signals_mc_opt[signal_1 / 2 - 1] ^ ( signal_1 % 2 != 0 );
    	}
    	if ( signal_2 == 0u )
    	{
				signal2 = xag_mc_opt.get_constant( false );
    	}
    	else if ( signal_2 == 1u )
    	{
				signal2 = xag_mc_opt.get_constant( true );
    	}
    	else
    	{
    		signal2 = signals_mc_opt[signal_2 / 2 - 1] ^ ( signal_2 % 2 != 0 );
    	}

    	if ( signal_1 > signal_2 )
    	{
    		signals_mc_opt.emplace_back( xag_mc_opt.create_xor( signal1, signal2 ) );
    	}
    	else
    	{
    		signals_mc_opt.emplace_back( xag_mc_opt.create_and( signal1, signal2 ) );
    	}
  	}

  	const uint32_t signal_po = std::stoul( line );
  	xag_mc_opt.create_po( signals_mc_opt[signal_po / 2 - 1] ^ ( signal_po % 2 != 0 ) );
  	mockturtle::depth_view<mockturtle::xag_network, details::num_and<mockturtle::xag_network>, false> xag_mc_opt_md{ xag_mc_opt };
  	uint32_t md = xag_mc_opt_md.depth();
  	mockturtle::xag_network xag_md_opt{ xag_mc_opt };

  	if ( md != md_target )
  	{
  		kitty::dynamic_truth_table tt_min_base( num_vars );
	    kitty::create_from_hex_string( tt_min_base, tt_str.substr( 0, 1 << ( num_vars - 2 ) ) );
	    bool find{ false };

	    /* Look for the MD-optimal XAG implementation */
	    for ( auto i{ 0u }; i < and_ladders_lib.size(); ++i )
	    {
	    	const auto ladder = and_ladders_lib[i];
	    	if ( ladder.md < md_target )
	    	{
	    		continue;
	    	}
	    	if ( ladder.mc < mc_target )
	    	{
	    		continue;
	    	}
	    	if ( ladder.md >= md_target && ladder.mc > mc_target )
	    	{
	    		break;
	    	}

	    	mockturtle::md_optimal_exact_synthesis_params ps;
				mockturtle::md_optimal_exact_synthesis_stats st;

				const auto p_xag_md_optimal = mockturtle::md_optimal_exact_synthesis( tt_min_base, ladder.and_ladder, ps, &st );
				if ( p_xag_md_optimal )
				{
					xag_md_opt = *p_xag_md_optimal;
					md = md_target;
					find = true;
					break;
				}
	    }

	    if ( !find )
	    {
	    	std::cout << "[e] Failed to find MD-optimal implementation for current function\n";
	    	abort();
	    }
	  }

	  mockturtle::depth_view<mockturtle::xag_network, details::num_and<mockturtle::xag_network>, false> xag_md_opt_md{ xag_md_opt };
	  std::vector<uint32_t> delay;
	  compute_input_to_output_delay<decltype( xag_md_opt_md ), details::num_and<decltype( xag_md_opt_md )>>( xag_md_opt_md, delay );
	  if ( delay.size() != num_vars )
	  {
	  	std::cout << "[e] number of entries of delay information(" << delay.size() << ") and number of pis(" << num_vars << ") are non consistent\n";
	  	abort();
	  }

	  std::ofstream db_md;
	  db_md.open( "db_md_5", std::ios::app );
    db_md << "0x" << repr_str << " ";
    db_md << "0x" << tt_str << " ";
    db_md << num_vars << " ";
    for ( auto const& delay_each: delay )
    {
    	db_md << delay_each << " ";
    }
    //db_md << mc << " ";
    //db_md << md << " ";
    xag_md_opt.foreach_gate( [&]( auto const& f ) {
    	xag_md_opt.foreach_fanin( f, [&]( auto const& fi ) {
    		db_md << static_cast<uint32_t>( ( fi.index << 1 ) + fi.complement ) << " ";
    	} );
    } );
    mockturtle::xag_network::signal po = xag_md_opt.po_at( 0 );
    db_md << static_cast<uint32_t>( ( po.index << 1 ) + po.complement ) << "\n";
    db_md.close();
    ++benchmark_cnt;
  }
}

int main ()
{
	//profile_mc_optimal_xags( "../experiments/db_mc_5" );
	//analyze_mc_optimal_xags();
	md_optimal_exact_synthesis();
	//md_optimal_exact_synthesis_gendb( "../experiments/db_mc_5" );

	return 0;

}