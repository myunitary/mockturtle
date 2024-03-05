#include <cstdint>
#include <ctime>
#include <iostream>

#include <bill/sat/interface/glucose.hpp>
#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/balancing/esop_balancing.hpp>
#include <mockturtle/algorithms/fhe_optimization.hpp>
#include <mockturtle/algorithms/node_resynthesis/fhe_optimization_exact_synthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/fhe_optimization_database.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_aiger.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/utils/cost_functions.hpp>
#include <mockturtle/views/depth_view.hpp>

#include <experiments.hpp>

static const uint32_t sota_md_epfl[] = {
	12u, 4u, 523u, 104u, 26u, 65u, 62u, 951u, 20u, 
	10u, 7u, 4u, 3u, 7u, 6u, 14u, 10u, 8u, 17u };

static const uint32_t sota_mc_epfl[] = {
	2761u, 1303u, 120327u, 34133u, 7666u, 15138u, 6822u, 71587u, 10777u, 
	5183u, 447u, 54u, 292u, 816u, 104u, 9983u, 522u, 116u, 3255u };

std::vector<uint32_t> md_sota_epfl()
{
	std::vector<uint32_t> best_md;
	for( auto i = 0u; i < 19u; ++i )
	{
		best_md.emplace_back( sota_md_epfl[i] );
	}
	return best_md;
}

std::vector<uint32_t> mc_sota_epfl()
{
	std::vector<uint32_t> best_mc;
	for( auto i = 0u; i < 19u; ++i )
	{
		best_mc.emplace_back( sota_mc_epfl[i] );
	}
	return best_mc;
}

static const std::string EPFL_benchmarks[] = {
	"adder", "bar", "div", "log2", "max", "multiplier", "sin", "sqrt", "square", 
	"arbiter", "cavlc", "ctrl" , "dec", "i2c", "int2float" , "mem_ctrl", "priority", "router", "voter"};

std::vector<std::string> epfl_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 19u; ++i )
	{
		result.emplace_back( EPFL_benchmarks[i] );
	}

	return result;
}

static const uint32_t sota_md_pldi[] = {
	8u, 8u, 36u, 36u, 36u, 20u, 6u, 6u, 5u,
	8u, 7u, 7u, 3u, 5u, 10u, 5u, 15u, 15u,
	11u, 10u, 5u, 3u, 8u, 8u, 10u
};

static const uint32_t sota_mc_pldi[] = {
	116u, 793u, 1450u, 1482u, 1482u, 1404u, 87u, 76u, 27u,
	78u, 121u, 121u, 13u, 18u, 177u, 36u, 391u, 116u,
	3015u, 668u, 120u, 304u, 1215u, 234u, 190u
};

std::vector<uint32_t> md_sota_pldi()
{
	std::vector<uint32_t> best_md;
	for( auto i = 0u; i < 25u; ++i )
	{
		best_md.emplace_back( sota_md_pldi[i] );
	}
	return best_md;
}

std::vector<uint32_t> mc_sota_pldi()
{
	std::vector<uint32_t> best_mc;
	for( auto i = 0u; i < 25u; ++i )
	{
		best_mc.emplace_back( sota_mc_pldi[i] );
	}
	return best_mc;
}

static const std::string PLDI_benchmarks[] = {
	"cardio", "dsort", "msort", "isort", "bsort", "osort", "hd01", "hd02", "hd03", 
	"hd04", "hd05", "hd06", "hd07", "hd08", "hd09", "hd10", "hd11", "hd12",
	"bar", "cavlc", "ctrl", "dec", "i2c", "int2float", "router"
};

std::vector<std::string> pldi_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 25u; ++i )
	{
		result.emplace_back( PLDI_benchmarks[i] );
	}

	return result;
}

std::string benchmark_path( std::string const& benchmark_name )
{
	// return fmt::format( "../experiments/epfl_tcad22/{}.v", benchmark_name );
	return fmt::format( "../experiments/pldi_benchmarks/{}.v", benchmark_name );
}

template<class Ntk>
bool abc_cec( Ntk const& ntk, std::string const& benchmark )
{
	mockturtle::write_bench( ntk, "/tmp/test.bench" );
	std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
	std::string command = fmt::format( "{}abc -q \"cec -n {} /tmp/test.bench\"", abc_path, benchmark_path( benchmark ) );

	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

	if ( !pipe )
	{
		throw std::runtime_error( "popen() failed" );
	}

	while ( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
	{
		result += buffer.data();
	}

	return result.size() >= 23 && result.substr( 0u, 23u ) == "Networks are equivalent";
}

template<class Ntk>
void abc_write_verilog( Ntk const& ntk, std::string const& dir )
{
	mockturtle::write_bench( ntk, "/tmp/tmp.bench" );
	std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
	std::string command = fmt::format( "{}abc -q \"read /tmp/tmp.bench; write_verilog {}\"", abc_path, dir );
	std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

	if ( !pipe )
	{
		throw std::runtime_error( "popen() failed" );
	}
}

template<class Ntk>
void abc_write_aiger( Ntk const& ntk, std::string const& dir )
{
	mockturtle::write_bench( ntk, "/tmp/tmp.bench" );
	std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
	std::string command = fmt::format( "{}abc -q \"read /tmp/tmp.bench; strash; write {}\"", abc_path, dir );
	std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

	if ( !pipe )
	{
		throw std::runtime_error( "popen() failed" );
	}
}

uint32_t fhe_cost( uint32_t mc, uint32_t md )
{
	return mc * md * md;
}

int main()
{
	// std::string json_name = "fhe_optimization_pldi_exact";
	// std::string json_name = "fhe_optimization_pldi_esop_test";
	// std::string json_name = "fhe_optimization_pldi_hybrid_5_restart_md_target";
	// experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, uint32_t, float, bool> exp_res( json_name, "benchmark", "MD best", "MC best", "MD after", "MC after", "improvement %", "#ite_ctr", "runtime [s]", "equivalent" );
	// experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, uint32_t, uint32_t, std::vector<uint8_t>, float, bool> exp_res( json_name, "benchmark", "MD best", "MC best", "MD after", "MC after", "improvement %", "#ite_ctr_rewrite", "#ite_ctr_esop", "script", "runtime [s]", "equivalent" );
	// auto const benchmarks = epfl_benchmarks();
	// std::vector<uint32_t> const best_md = md_sota_epfl();
	// std::vector<uint32_t> const best_mc = mc_sota_epfl();
	auto const benchmarks = pldi_benchmarks();
	std::vector<uint32_t> const best_md = md_sota_pldi();
	std::vector<uint32_t> const best_mc = mc_sota_pldi();
	float time_opt{ 0.0f };

	mockturtle::fhe_optimization_database_params g_local_opt_fn_ps;
	g_local_opt_fn_ps.verbose = false;
	mockturtle::fhe_optimization_database_stats g_local_opt_fn_st;
	mockturtle::fhe_optimization_database g_local_opt_fn{ "db_fhe_5", g_local_opt_fn_ps, &g_local_opt_fn_st };

	mockturtle::fhe_optimization_exact_syn_params cm_local_opt_fn_ps;
	cm_local_opt_fn_ps.verbose = true;
	cm_local_opt_fn_ps.verify_solution = true;
	cm_local_opt_fn_ps.use_advanced_constraints = false;
	cm_local_opt_fn_ps.use_exisiting_cache = false;
	mockturtle::fhe_optimization_exact_syn_stats cm_local_opt_fn_st;
	mockturtle::fhe_optimization_exact_syn<bill::solvers::glucose_41> cm_local_opt_fn{ cm_local_opt_fn_ps, &cm_local_opt_fn_st };

	// for ( auto i{ 0u }; i < benchmarks.size(); ++i )
	uint8_t target = 2u;
	for ( auto i{ target }; i <= target; ++i )
	{
		auto const benchmark = benchmarks[i];
		fmt::print( "[i] processing {}\n", benchmark );

		mockturtle::xag_network xag;
		auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );
		// auto read_stats = lorina::read_aiger( benchmark_path( benchmark ), mockturtle::aiger_reader( xag ) );

		if ( read_stats != lorina::return_code::success )
		{
			fmt::print( "[e] failed to read benchmark\n" );
			abort();
		}
		fmt::print( "[m] read benchmark successfully\n" );

		uint32_t md_best{ best_md[i] };
		uint32_t mc_best{ best_mc[i] };
		mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_md{ xag };
		uint32_t md_init{ xag_md.depth() };
		fmt::print( "[m] before optimization, the md is {}\n", md_init );
		uint32_t md_before{ md_init };
		uint32_t md_after{ md_before - 1 };
		uint32_t ite_ctr{ 0u };
		uint32_t mc_init{ mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag ) };
		uint32_t mc_before{ mc_init };
		uint32_t mc_after{ 0u };
		time_opt = 0.;

		mockturtle::fhe_optimization_params ps;
		ps.only_on_critical_path = true;
		ps.always_accept_exact_impl = false;
		ps.randomly_take_equal_impl = false;
		ps.cut_enum_ps.cut_size = 5u;
		ps.cut_enum_ps.verbose = false;
		ps.cut_enum_ps.very_verbose = false;
		ps.progress = true;
		ps.verbose = false;
		mockturtle::fhe_optimization_stats st;

		// first round : logic rewriting until saturation
		while ( true )
		{
			if ( ite_ctr > 0 ) 
			{
				mc_before = mc_after;
				md_before = md_after;
			}
			++ite_ctr;

			clock_t opt_time_begin = clock();
			// mockturtle::xag_network xag_new = mockturtle::fhe_optimization( xag, g_local_opt_fn, ps, &st );
			mockturtle::xag_network xag_new = mockturtle::fhe_optimization( xag, g_local_opt_fn, cm_local_opt_fn, ps, &st );
			time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
			mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
			mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
			md_after = xag_new_md.depth();
			fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr, md_after );
			if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
			// if ( md_before < md_after || ( ( md_before == md_after ) && ( mc_before <= mc_after ) ) )
			{
				mc_after = mc_before;
				md_after = md_before;
				break;
			}
			else
			{
				xag = xag_new;
			}
		}

		// second round : esop-based optimization until saturation
		// if ( true )
		// {
		// 	mockturtle::balancing_params balance_ps;
		// 	balance_ps.cut_enumeration_ps = ps.cut_enum_ps;
		// 	balance_ps.only_on_critical_path = true;
		// 	balance_ps.progress = true;

		// 	while ( true )
		// 	{
		// 		if ( ite_ctr > 0 )
		// 		{
		// 			md_before = md_after;
		// 			mc_before = mc_after;
		// 		}
		// 		++ite_ctr;

		// 		clock_t opt_time_begin = clock();
		// 		mockturtle::xag_network xag_new = mockturtle::balancing<mockturtle::xag_network, mockturtle::detail::num_and>( xag, mockturtle::esop_rebalancing<mockturtle::xag_network>{}, balance_ps );
		// 		time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
		// 		mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
		// 		mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
		// 		md_after = xag_new_md.depth();
		// 		fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr, md_after );

		// 		// if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
		// 		if ( md_before < md_after || ( ( md_before == md_after ) && ( mc_before <= mc_after ) ) )
		// 		{
		// 			md_after = md_before;
		// 			mc_after = mc_before;
		// 			break;
		// 		}
		// 		else
		// 		{
		// 			xag = xag_new;
		// 		}
		// 	}
		// }

		// randomly apply optimization algorithms
		// std::random_device rd;    // seed source
		// std::mt19937 gen( rd() ); // Mersenne Twister algorithm-based random number engine, seeded with rd()
		// // std::mt19937 gen{}; // Mersenne Twister algorithm-based random number engine, seeded with rd()
		// std::uniform_int_distribution<unsigned int> random( 0, 1 );

		// mockturtle::balancing_params esop_ps;
		// esop_ps.cut_enumeration_ps = ps.cut_enum_ps;
		// esop_ps.only_on_critical_path = true;
		// esop_ps.progress = true;
		// uint32_t ite_ctr_rewrite{}, ite_ctr_esop{};
		// uint8_t num_relaxation{ 5u };
		// // bool next_algorithm{ true };
		// bool next_algorithm{ static_cast<bool>( random( gen ) ) };
		// bool already_tried_other{ false };
		// mockturtle::write_verilog( xag, "/tmp/current_best.v" ); /* record */
		// uint32_t mc_current_best{ mc_init };
		// uint32_t md_current_best{ md_init };
		// std::vector<uint8_t> script_current_best;
		// std::vector<uint8_t> script_tmp;

		// while ( true )
		// {
		// 	script_tmp.emplace_back( next_algorithm );
		// 	if ( next_algorithm )
		// 	{
		// 		clock_t opt_time_begin = clock();
		// 		mockturtle::xag_network xag_new = mockturtle::fhe_optimization( xag, g_local_opt_fn, cm_local_opt_fn, ps, &st );
		// 		time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
		// 		mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
		// 		mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
		// 		md_after = xag_new_md.depth();
		// 		++ite_ctr_rewrite;
		// 		fmt::print( "[i] after {}th run, the md becomes {}\n", ( ite_ctr_esop + ite_ctr_rewrite ), md_after );
		// 		// if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
		// 		if ( md_before < md_after || ( ( md_before == md_after ) && ( mc_before <= mc_after ) ) )
		// 		{
		// 			// mc_after = mc_before;
		// 			// md_after = md_before;
		// 			if ( already_tried_other )
		// 			{
		// 				/* decide where to restart */
		// 				// if ( fhe_cost( mc_before, md_before ) >= fhe_cost( mc_current_best, md_current_best ) )
		// 				if ( md_current_best < md_before || ( ( md_current_best == md_before ) && ( mc_current_best <= mc_before ) ) )
		// 				{
		// 					/* recover */
		// 					xag = mockturtle::xag_network{};
		// 					read_stats = lorina::read_verilog( "/tmp/current_best.v", mockturtle::verilog_reader( xag ) );
		// 					if ( read_stats != lorina::return_code::success )
		// 					{
		// 						std::cerr << "[e] recover network failed\n";
		// 						abort();
		// 					}
		// 					/* ensure functional equivalence */
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] recovered to NEQ!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] recovering... -> MC : {}, MD : {}\n", mc_current_best, md_current_best );
		// 					}

		// 					mc_before = mc_current_best;
		// 					md_before = md_current_best;
		// 					script_tmp = script_current_best;
		// 				}
		// 				else
		// 				{
		// 					/* ensure functional equivalence */
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] not equivalent already!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] proved to be EQ, updating record... <- MC : {}, MD : {}\n", mc_before, md_before );
		// 					}
		// 					/* update record */
		// 					mockturtle::write_verilog( xag, "/tmp/current_best.v" );
		// 					mc_current_best = mc_before;
		// 					md_current_best = md_before;
		// 					script_current_best = script_tmp;
		// 				}
						
		// 				if ( --num_relaxation )
		// 				{
		// 					/* relaxation */
		// 					fmt::print( "[i] applying relaxation...\n" );
		// 					abc_write_aiger( xag, "/tmp/tmp.aig" );
		// 					xag = mockturtle::xag_network{};
		// 					read_stats = lorina::read_aiger( benchmark_path( "/tmp/tmp.aig" ), mockturtle::aiger_reader( xag ) );
		// 					if ( read_stats != lorina::return_code::success )
		// 					{
		// 						std::cerr << "[e] relaxation failed\n";
		// 						abort();
		// 					}
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] NEQ after relaxation!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] relaxation succeeded!\n" );
		// 					}
		// 					mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_tmp_md{ xag };
		// 					mc_before = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag );
		// 					md_before = xag_tmp_md.depth();
		// 					script_tmp.emplace_back( 2u );
		// 					already_tried_other = false;
		// 				}
		// 				else
		// 				{
		// 					mc_after = mc_before;
		// 					md_after = md_before;
		// 					break;
		// 				}	
		// 			}
		// 			else
		// 			{
		// 				next_algorithm = false;
		// 				already_tried_other = true;
		// 				continue;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			xag = xag_new;
		// 			mc_before = mc_after;
		// 			md_before = md_after;
		// 			already_tried_other = false;
		// 		}
		// 	}
		// 	else
		// 	{
		// 		clock_t opt_time_begin = clock();
		// 		mockturtle::xag_network xag_new = mockturtle::balancing<mockturtle::xag_network, mockturtle::detail::num_and>( xag, mockturtle::esop_rebalancing<mockturtle::xag_network>{}, esop_ps );
		// 		time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
		// 		mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
		// 		mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
		// 		md_after = xag_new_md.depth();
		// 		++ite_ctr_esop;
		// 		fmt::print( "[i] after {}th run, the md becomes {}\n", ( ite_ctr_esop + ite_ctr_rewrite ), md_after );
		// 		// if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
		// 		if ( md_before < md_after || ( ( md_before == md_after ) && ( mc_before <= mc_after ) ) )
		// 		{
		// 			// md_after = md_before;
		// 			// mc_after = mc_before;
		// 			if ( already_tried_other )
		// 			{
		// 				/* decide where to restart */
		// 				// if ( fhe_cost( mc_before, md_before ) >= fhe_cost( mc_current_best, md_current_best ) )
		// 				if ( md_current_best < md_before || ( ( md_current_best == md_before ) && ( mc_current_best <= mc_before ) ) )
		// 				{
		// 					/* recover */
		// 					xag = mockturtle::xag_network{};			
		// 					read_stats = lorina::read_verilog( "/tmp/current_best.v", mockturtle::verilog_reader( xag ) );
		// 					if ( read_stats != lorina::return_code::success )
		// 					{
		// 						std::cerr << "[e] recover network failed\n";
		// 						abort();
		// 					}
		// 					/* ensure functional equivalence */
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] recovered to NEQ!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] recovering... -> MC : {}, MD : {}\n", mc_current_best, md_current_best );
		// 					}

		// 					mc_before = mc_current_best;
		// 					md_before = md_current_best;
		// 					script_tmp = script_current_best;
		// 				}
		// 				else
		// 				{
		// 					/* ensure functional equivalence */
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] not equivalent already!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] proved to be EQ, updating record... <- MC : {}, MD : {}\n", mc_before, md_before );
		// 					}
		// 					/* update record */
		// 					mockturtle::write_verilog( xag, "/tmp/current_best.v" );
		// 					mc_current_best = mc_before;
		// 					md_current_best = md_before;
		// 					script_current_best = script_tmp;
		// 				}

		// 				if ( --num_relaxation )
		// 				{
		// 					/* relaxation */
		// 					fmt::print( "[i] applying relaxation...\n" );
		// 					abc_write_aiger( xag, "/tmp/tmp.aig" );
		// 					xag = mockturtle::xag_network{};
		// 					read_stats = lorina::read_aiger( "/tmp/tmp.aig", mockturtle::aiger_reader( xag ) );
		// 					if ( read_stats != lorina::return_code::success )
		// 					{
		// 						std::cerr << "[e] relaxation failed\n";
		// 						abort();
		// 					}
		// 					if ( !abc_cec( xag, benchmark ) )
		// 					{
		// 						std::cerr << "[e] NEQ after relaxation!\n";
		// 						abort();
		// 					}
		// 					else
		// 					{
		// 						fmt::print( "[m] relaxation succeeded!\n" );
		// 					}
		// 					mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_tmp_md{ xag };
		// 					mc_before = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag );
		// 					md_before = xag_tmp_md.depth();
		// 					script_tmp.emplace_back( 2u );
		// 					already_tried_other = false;
		// 				}
		// 				else
		// 				{
		// 					mc_after = mc_before;
		// 					md_after = md_before;
		// 					break;
		// 				}	
		// 			}
		// 			else
		// 			{
		// 				next_algorithm = false;
		// 				already_tried_other = true;
		// 				continue;
		// 			}
		// 		}
		// 		else
		// 		{
		// 			xag = xag_new;
		// 			mc_before = mc_after;
		// 			md_before = md_after;
		// 			already_tried_other = false;
		// 		}
		// 	}

		// 	next_algorithm = random( gen );
		// }

		// const bool is_eq = true;
		const bool is_eq = abc_cec( xag, benchmark );
		if ( !is_eq )
		{
			std::cerr << "[e] not equivalent after optimization\n";
			abort();
		}

		// float improve_local = ( ( static_cast<float>( fhe_cost( mc_init, md_init ) ) - static_cast<float>( fhe_cost( mc_after, md_after ) ) ) ) / static_cast<float>( fhe_cost( mc_init, md_init ) ) * 100;
		float improve_global = ( ( static_cast<float>( fhe_cost( mc_best, md_best ) ) - static_cast<float>( fhe_cost( mc_after, md_after ) ) ) ) / static_cast<float>( fhe_cost( mc_best, md_best ) ) * 100;
		// exp_res( benchmark, md_best, mc_best, md_after, mc_after, improve_global, ite_ctr, time_opt, is_eq );
		// exp_res( benchmark, md_best, mc_best, md_after, mc_after, improve_global, ite_ctr_rewrite, ite_ctr_esop, script_tmp, time_opt, is_eq );
		// mockturtle::write_verilog( xag, ( "../experiments/benchmarks/fhe_opt_pldi_exact/" + benchmark + ".v" ) );
		// mockturtle::write_verilog( xag, ( "../experiments/benchmarks/fhe_opt_pldi_esop/" + benchmark + ".v" ) );
		// mockturtle::write_verilog( xag, ( "../experiments/benchmarks/fhe_opt_pldi_hybrid/restart_md/" + benchmark + ".v" ) );
	}

	// exp_res.save();
	// exp_res.table();
}