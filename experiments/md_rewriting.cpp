#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/balancing/esop_balancing.hpp>
#include <mockturtle/algorithms/md_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_low_md_rewrite.hpp>


#include <mockturtle/algorithms/rewriting.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>

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

struct num_and
{
	uint32_t operator()( mockturtle::xag_network const& xag, mockturtle::xag_network::node n ) const
	{
		return ( xag.is_and( n ) ? 1u : 0u );
	}
};

int main()
{
	std::string json_name = "xag_low_md_esop_old_saturation";
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, uint32_t, float, bool> exp_res( json_name, "benchmark", "MC before", "MC after", "MD before", "MD after", "MD best", "improvement % (local)", "improvement % (global)", "iterations", "runtime [s]", "equivalent" );
	auto const benchmarks = epfl_benchmarks();
	std::vector<uint32_t> const best_md = md_sota_epfl();
	std::vector<uint32_t> const best_mc = mc_sota_epfl();

	// mockturtle::md_rewriting_params ps;
	// ps.cut_enumeration_ps.cut_size = 5u;
	// ps.cut_enumeration_ps.cut_limit = 12u;
	// ps.allow_zero_gain = false;
	// ps.progress = true;
	// ps.verbose = false;
	// ps.min_cand_cut_size = 2u;
	// ps.verify_database = false;

	// mockturtle::rewriting_params ps_rw;
	// ps_rw.cut_enumeration_ps.cut_size = 5u;
	// ps_rw.cut_enumeration_ps.cut_limit = 12u;
	// ps_rw.allow_zero_gain = false;
	// ps_rw.progress = true;
	// ps_rw.verbose = false;
	// ps_rw.min_cand_cut_size = 2u;

	for ( auto i{ 0u }; i < benchmarks.size(); ++i )
	//for ( auto i{ 11u }; i < 12u; ++i )
	{
		auto const benchmark = benchmarks[i];
		std::cout << "[i] processing " << benchmark << "\n";

		mockturtle::xag_network xag;
		auto const read_result = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );
		if ( read_result != lorina::return_code::success )
		{
			std::cout << "[e] failed to read benchmark\n";
			abort();
		}
		std::cout << "[m] read benchmark successfully\n";

		uint32_t md_best = best_md[i];
		uint32_t mc_best = best_mc[i];
		mockturtle::depth_view<mockturtle::xag_network, num_and, false> xag_md{ xag };
		uint32_t md_before = xag_md.depth();
		std::cout << "[m] before optimization, the md is " << md_before << "\n";
		uint32_t md_init = md_before;
		uint32_t md_after = md_before - 1;
		uint32_t ite_ctr{ 0u };

		// mockturtle::md_rewriting_stats st;
		// mockturtle::md_rewriting_stats* pst = &st;
		// mockturtle::detail::md_rewriting_impl md_rewriting( "db_md_5", xag, ps, pst );

		uint32_t mc_init{ 0u };
		xag.foreach_gate( [&]( auto const& n ) {
			if ( xag.is_and( n ) )
			{
				++mc_init;
			}
		} );
		uint32_t mc_before{ mc_init };
		uint32_t mc_after{ 0u };
		//std::cout << "[m] before optimization, the mc is " << mc_init << "\n";
		clock_t begin_time = clock();

		// if ( false ) {
		// //auto ctr_upper{ 3u };
		// //while ( ite_ctr <= ctr_upper )
		// while ( md_before > md_after )
		// {
		// 	if ( ite_ctr > 0 )
		// 	{
		// 		md_before = md_after;
		// 	}
		// 	//std::cout << "[m] round " << ite_ctr << ": \n";
		// 	++ite_ctr;

		// 	xag = md_rewriting.run();
		// 	xag_md.update_levels();
		// 	md_after = xag_md.depth();
		// }}

		// if ( false ) {
		// 	xag = md_rewriting.run3();
		// 	xag_md.update_levels();
		// 	md_after = xag_md.depth();
		// }

		// if ( false )
		// {
		// 	mockturtle::xag_low_md_rewrite_params ps;
		// 	ps.verbose = true;
		// 	mockturtle::xag_low_md_rewrite_stats st;
		// 	mockturtle::xag_low_md_rewrite_stats* pst = &st;
		// 	mockturtle::xag_low_md_rewrite xag_rewrite( "db_md_5", ps, pst );

		// 	while ( true )
		// 	{
		// 		if ( ite_ctr > 0 )
		// 		{
		// 			md_before = md_after;
		// 		}
		// 		++ite_ctr;

		// 		mockturtle::xag_network xag_new = mockturtle::rewriting<mockturtle::xag_network, decltype( xag_rewrite )>( xag, xag_rewrite, ps_rw );
		// 		mockturtle::depth_view<mockturtle::xag_network, num_and, false> xag_new_md{ xag_new };
		// 		md_after = xag_new_md.depth();
		// 		if ( md_after >= md_before )
		// 		{
		// 			md_after = md_before;
		// 			break;
		// 		}
		// 		else
		// 		{
		// 			xag = xag_new;
		// 		}
		// 	}
		// }

		if ( true )
		{
			mockturtle::balancing_params ps;
			ps.only_on_critical_path = true;
			ps.progress = true;

			while ( true )
			{
				if ( ite_ctr > 0 )
				{
					md_before = md_after;
					mc_before = mc_after;
				}
				++ite_ctr;

				mockturtle::xag_network xag_new = mockturtle::balancing<mockturtle::xag_network, num_and>( xag, mockturtle::esop_rebalancing<mockturtle::xag_network>{}, ps );
				//mockturtle::xag_network xag_new = mockturtle::esop_balancing( xag );

				mockturtle::depth_view<mockturtle::xag_network, num_and, false> xag_new_md{ xag_new };
				md_after = xag_new_md.depth();
				mc_after = 0u;
				xag_new.foreach_gate( [&]( auto const& n ) {
					if ( xag_new.is_and( n ) )
					{
						++mc_after;
					}
				} );

				if ( mc_after * md_after * md_after >= mc_before * md_before * md_before )
				{
					md_after = md_before;
					mc_after = mc_before;
					break;
				}
				else
				{
					xag = xag_new;
				}
			}
		}

		float time_opt = float( clock() - begin_time ) / CLOCKS_PER_SEC;
		const auto cec = abc_cec( xag, benchmark );
		if ( !cec )
		{
			std::cout << "[e] not equivalent\n";
			abort();
		}
		
		float improve_local = ( ( static_cast<float>( md_init * md_init * mc_init ) - static_cast<float>( md_after * md_after * mc_after ) ) / static_cast<float>( md_init * md_init * mc_init ) ) * 100;
		float improve_global = ( ( static_cast<float>( md_best * md_best * mc_best ) - static_cast<float>( md_after * md_after * mc_after ) ) / static_cast<float>( md_best * md_best * mc_best ) ) * 100;
		exp_res( benchmark, mc_init, mc_after, md_init, md_after, md_best, improve_local, improve_global, ite_ctr, time_opt, cec );
	}

	exp_res.save();
	exp_res.table();
}