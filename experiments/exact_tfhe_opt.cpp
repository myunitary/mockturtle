#include <cstdint>
#include <ctime>
#include <iostream>

#include <bill/sat/interface/glucose.hpp>
#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/balancing/esop_balancing.hpp>
#include <mockturtle/algorithms/exact_tfhe_opt.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/utils/cost_functions.hpp>
#include <mockturtle/views/depth_view.hpp>

#include <experiments.hpp>

static const uint32_t sota_md_epfl[] = {
	12u, 4u, 523u, 104u, 26u, 65u, 62u, 951u, 20u, 
	10u, 7u, 4u, 3u, 7u, 6u, 14u, 10u, 8u, 17u};

static const uint32_t sota_mc_epfl[] = {
	2761u, 1303u, 120327u, 34133u, 7666u, 15138u, 6822u, 71587u, 10777u, 
	5183u, 447u, 54u, 292u, 816u, 104u, 9983u, 522u, 116u, 3255u};

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

std::string benchmark_path( std::string const& benchmark_name )
{
	return fmt::format( "../experiments/epfl_tcad22/{}.v", benchmark_name );
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

uint32_t fhe_cost( uint32_t mc, uint32_t md )
{
	return mc * md * md;
}

int main()
{
	std::string json_name = "exact_tfhe_opt";
	// std::string json_name = "fhe_opt_with_exact_syn_rand";
	// std::string json_name = "fhe_opt_only_esop";
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, uint32_t, float, bool> exp_res( json_name, "benchmark", "MC before", "MC after", "MC best", "MD before", "MD after", "MD best", "improvement % (local)", "improvement % (global)", "iterations", "runtime [s]", "equivalent" );
	// experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, uint32_t, uint32_t, uint32_t, float, bool> exp_res( json_name, "benchmark", "MC before", "MC after", "MC best", "MD before", "MD after", "MD best", "improvement % (local)", "improvement % (global)", "iterations (R1)", "iterations (R2)", "iterations (R3)", "runtime [s]", "equivalent" );
	// experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, uint32_t, uint32_t, float, bool> exp_res( json_name, "benchmark", "MC before", "MC after", "MC best", "MD before", "MD after", "MD best", "improvement % (local)", "improvement % (global)", "iterations (R1)", "iterations (R2)", "runtime [s]", "equivalent" );
	auto const benchmarks = epfl_benchmarks();
	std::vector<uint32_t> const best_md = md_sota_epfl();
	std::vector<uint32_t> const best_mc = mc_sota_epfl();
	float time_opt{ 0.0f };

	mockturtle::exact_tfhe_opt_agent_params agent_ps;
	agent_ps.conflict_limit = 0u;
	agent_ps.verbose = true;
	agent_ps.use_advanced_constraints = false;
	agent_ps.use_exisiting_cache = false;
	mockturtle::exact_tfhe_opt_agent_stats agent_st;
	mockturtle::exact_tfhe_opt_agent<bill::solvers::glucose_41> agent{ agent_ps, &agent_st };

	for ( auto i{ 0u }; i < benchmarks.size(); ++i )
	// uint8_t target = 17u;
	// for ( auto i{ target }; i <= target; ++i )
	{
		auto const benchmark = benchmarks[i];
		fmt::print( "[i] processing {}\n", benchmark );

		mockturtle::xag_network xag;
		auto const read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );
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

		mockturtle::exact_tfhe_opt_params ps;
		ps.only_on_critical_path = true;
		ps.always_commit_impl = false;
		ps.randomly_commit_impl = false;
		ps.cut_enum_ps.cut_size = 6u;
		ps.cut_enum_ps.verbose = false;
		ps.cut_enum_ps.very_verbose = false;
		ps.progress = true;
		ps.verbose = false;
		mockturtle::exact_tfhe_opt_stats st;

		// first round : logic optimization
		while ( true )
		{
			if ( ite_ctr > 0 ) 
			{
				mc_before = mc_after;
				md_before = md_after;
			}
			++ite_ctr;

			clock_t opt_time_begin = clock();
			mockturtle::xag_network xag_new = mockturtle::exact_tfhe_opt( xag, agent, ps, &st );
			time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
			mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
			mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
			md_after = xag_new_md.depth();
			fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr, md_after );
			if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
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

		// second round : esop-based relaxation
		// uint32_t ite_ctr_2{ 0u };
		// if ( true )
		// {
		// 	mockturtle::balancing_params balance_ps;
		// 	balance_ps.cut_enumeration_ps = ps.cut_enum_ps;
		// 	balance_ps.only_on_critical_path = true;
		// 	balance_ps.progress = true;

		// 	while ( true )
		// 	{
		// 		if ( ite_ctr_2 > 0 )
		// 		{
		// 			md_before = md_after;
		// 			mc_before = mc_after;
		// 		}
		// 		++ite_ctr_2;

		// 		clock_t opt_time_begin = clock();
		// 		mockturtle::xag_network xag_new = mockturtle::balancing<mockturtle::xag_network, mockturtle::detail::num_and>( xag, mockturtle::esop_rebalancing<mockturtle::xag_network>{}, balance_ps );
		// 		time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
		// 		mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
		// 		mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
		// 		md_after = xag_new_md.depth();
		// 		// fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr + ite_ctr_2, md_after );
		// 		fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr_2, md_after );

		// 		if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
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

		// third round : logic optimization
		// uint32_t ite_ctr_3{ 0u };
		// while ( true )
		// {
		// 	if ( ite_ctr_3 > 0 ) 
		// 	{
		// 		mc_before = mc_after;
		// 		md_before = md_after;
		// 	}
		// 	++ite_ctr_3;

		// 	clock_t opt_time_begin = clock();
		// 	// mockturtle::xag_network xag_new = mockturtle::fhe_optimization( xag, g_local_opt_fn, ps, &st );
		// 	mockturtle::xag_network xag_new = mockturtle::fhe_optimization( xag, g_local_opt_fn, cm_local_opt_fn, ps, &st );
		// 	time_opt += float( clock() - opt_time_begin ) / CLOCKS_PER_SEC;
		// 	mockturtle::depth_view<mockturtle::xag_network, mockturtle::detail::num_and, false> xag_new_md{ xag_new };
		// 	mc_after = mockturtle::costs<mockturtle::xag_network, mockturtle::detail::num_and>( xag_new );
		// 	md_after = xag_new_md.depth();
		// 	// fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr + ite_ctr_2 + ite_ctr_3, md_after );
		// 	fmt::print( "[i] after {}th run, the md becomes {}\n", ite_ctr_2 + ite_ctr_3, md_after );
		// 	if ( fhe_cost( mc_before, md_before ) <= fhe_cost( mc_after, md_after ) )
		// 	{
		// 		mc_after = mc_before;
		// 		md_after = md_before;
		// 		break;
		// 	}
		// 	else
		// 	{
		// 		xag = xag_new;
		// 	}
		// }

		// const bool is_eq = true;
		const bool is_eq = abc_cec( xag, benchmark );
		if ( !is_eq )
		{
			std::cerr << "[e] not equivalent after optimization\n";
			abort();
		}

		float improve_local = ( ( static_cast<float>( fhe_cost( mc_init, md_init ) ) - static_cast<float>( fhe_cost( mc_after, md_after ) ) ) ) / static_cast<float>( fhe_cost( mc_init, md_init ) ) * 100;
		float improve_global = ( ( static_cast<float>( fhe_cost( mc_best, md_best ) ) - static_cast<float>( fhe_cost( mc_after, md_after ) ) ) ) / static_cast<float>( fhe_cost( mc_best, md_best ) ) * 100;
		exp_res( benchmark, mc_init, mc_after, mc_best, md_init, md_after, md_best, improve_local, improve_global, ite_ctr, time_opt, is_eq );
		// exp_res( benchmark, mc_init, mc_after, mc_best, md_init, md_after, md_best, improve_local, improve_global, ite_ctr, ite_ctr_2, ite_ctr_3, time_opt, is_eq );
		// exp_res( benchmark, mc_init, mc_after, mc_best, md_init, md_after, md_best, improve_local, improve_global, ite_ctr_2, ite_ctr_3, time_opt, is_eq );
		mockturtle::write_verilog( xag, ( "../experiments/benchmarks/exact_tfhe_opt/" + benchmark + ".v" ) );
	}

	exp_res.save();
	exp_res.table();
}