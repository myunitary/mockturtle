#include <string>
#include <fmt/format.h>

#include <mockturtle/networks/xag.hpp>
#include <mockturtle/networks/xmg.hpp>
#include <mockturtle/networks/x1g.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
#include <mockturtle/algorithms/node_resynthesis/xohg_minmc.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/io/verilog_reader.hpp>

#include <experiments.hpp>

static const std::string crypto_epfl_benchmarks[] = {
	"adder_untilsat", "bar_untilsat", "div_untilsat", "log2_untilsat", "max_untilsat", "multiplier_untilsat", "sin_untilsat", "sqrt_untilsat", "square_untilsat",
  "arbiter_untilsat", "cavlc_untilsat", "ctrl_untilsat" , "dec_untilsat", "i2c_untilsat", "int2float_untilsat" , "mem_ctrl_untilsat", "priority_untilsat", "router_untilsat", "voter_untilsat", 
  "adder_32bit_untilsat", "adder_64bit_untilsat", "AES-expanded_untilsat", "AES-non-expanded_unstilsat", 
  "comparator_32bit_signed_lt_untilsat", "comparator_32bit_signed_lteq_untilsat", "comparator_32bit_unsigned_lt_untilsat", "comparator_32bit_unsigned_lteq_untilsat", 
  "DES-expanded_untilsat", "DES-non-expanded_untilsat", "md5_untilsat", "mult_32x32_untilsat", "sha-1_untilsat", 
  "sha-256_untilsat"};

static const std::string epfl_benchmarks_2022[] = {
	"adder", "bar", "div", "log2", "max", "multiplier", "sin", "sqrt", "square", "arbiter", 
	"cavlc", "ctrl" , "dec", "i2c", "int2float" , "mem_ctrl", "priority", "router", "voter", 
	"hyp"};

std::vector<std::string> crypto_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 33u; ++i )
	{
		result.emplace_back( crypto_epfl_benchmarks[i] );
	}

	return result;
}

std::vector<std::string> epfl_benchmarks_2022()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 20u; ++i )
	{
		result.emplace_back( epfl_benchmarks_2022[i] );
	}

	return result;
}

std::string benchmark_path( uint32_t benchmark_type = 0u, std::string const& benchmark_name )
{
	switch( benchmark_type )
	{
	case 0u:
		return fmt::format( "../experiments/benchmarks-2022.1/{}.v", benchmark_name );
	case 1u:
		return fmt::format( "../experiments/crypto_benchmarks/{}.v", benchmark_name );
	default:
		std::cout << "Unspecified type of benchmark. \n";
		abort();
	}
}

template<class Ntk>
bool abc_cec_crypto( Ntk const& ntk, std::string const& benchmark )
{
	mockturtle::write_bench( ntk, "/tmp/test.bench" );
	std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
	std::string command = fmt::format( "{}abc -q \"cec -n {} /tmp/test.bench\"", abc_path, crypto_benchmark_path( benchmark ) );

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

namespace detail
{
template<class Ntk = mockturtle::xag_network>
struct num_and
{
	uint32_t operator()( Ntk const& ntk, mockturtle::node<Ntk> const& n ) const
	{
		return ntk.is_and( n ) ? 1 : 0;
	}
};

template<class Ntk = mockturtle::x1g_network>
struct num_oh
{
	uint32_t operator()( Ntk const& ntk, mockturtle::node<Ntk> const& n ) const
	{
		return ntk.is_onehot( n ) ? 1 : 0;
	}
};

template<class Ntk = mockturtle::xmg_network>
struct num_maj
{
	uint32_t operator()( Ntk const& ntk, mockturtle::node<Ntk> const& n ) const
	{
		return ntk.is_maj( n ) ? 1 : 0;
	}
};
}

int main()
{
	experiments::experiment<std::string, uint32_t, uint32_t, float, uint32_t, float, bool> exp_res( "garble_xohg", "benchmark", "num_oh_before", "num_oh_after", "improvement %", "iterations", "avg. runtime [s]", "equivalent" );
	uint32_t benchmark_type = 0u; // 0u - epfl benchmark; 1u - crypto benchmark

	for ( auto const& benchmark: crypto_benchmarks() )
	{
		std::cout << "[i] processing " << benchmark << std::endl;

		mockturtle::cut_rewriting_params ps_cut_rew;
		ps_cut_rew.cut_enumeration_ps.cut_size = 6u;
		ps_cut_rew.cut_enumeration_ps.cut_limit = 12u;
		ps_cut_rew.verbose = false;
		ps_cut_rew.progress = false;
		ps_cut_rew.min_cand_cut_size = 2u;

		/* xag optimization */

		/*
		mockturtle::xag_network xag;
		lorina::read_verilog( crypto_benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );

		uint32_t num_and = 0u;
		uint32_t num_and_bfr = 0u;
		uint32_t num_and_aft = 0u;

		xag.foreach_gate( [&]( auto f ) {
			if ( xag.is_and( f ) )
			{
				++num_and;
			}
		} );
		num_and_bfr = num_and;

		mockturtle::xag_minmc_resynthesis xag_resyn( "../experiments/db" );

		uint32_t ite_cnt = 0u;
		const clock_t begin_time = clock();
		while ( num_and > num_and_aft )
		{
			num_and = num_and_aft;
			num_and_aft = 0u;

			mockturtle::cut_rewriting_with_compatibility_graph( xag, xag_resyn, ps_cut_rew, nullptr, ::detail::num_and<mockturtle::xag_network>() );
			xag = mockturtle::cleanup_dangling( xag );

			xag.foreach_gate( [&]( auto f ) {
				if ( xag.is_and( f ) )
				{
					++num_and_aft;
				}
			} );
		}

		const auto cec = abc_cec_crypto( xag, benchmark );

		float improve = ( ( num_and_bfr - num_and_aft ) / num_and_bfr ) * 100;

		exp_res( benchmark, num_and_bfr, num_and_aft, improve, ite_cnt, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) / ite_cnt, cec );

		*/	

		/* function converting xag to xohg */

		/* xohg optimization */
		//

		mockturtle::x1g_network x1g;
		auto const read_result = lorina::read_verilog( benchmark_path( benchmark_type, benchmark ), mockturtle::verilog_reader( x1g ) );
		assert( read_result == lorina::return_code::success );

		uint32_t num_oh = 0u;
		uint32_t num_oh_bfr = 0u;
		uint32_t num_oh_aft = 0u;

		x1g.foreach_gate( [&]( auto f ) {
			if ( x1g.is_onehot( f ) )
			{
				++num_oh;
			}
		} );
		num_oh_bfr = num_oh;

		/* make the cache static? */
		mockturtle::exact_xohg_resynthesis_minmc_params ps_xohg_resyn;
		ps_xohg_resyn.print_stats = true;
		ps_xohg_resyn.cache = std::make_shared<mockturtle::exact_xohg_resynthesis_minmc_params::cache_map_t>();
		ps_xohg_resyn.blacklist_cache = std::make_shared<mockturtle::exact_xohg_resynthesis_minmc_params::blacklist_cache_map_t>();

		mockturtle::exact_xohg_resynthesis_minmc_stats* pst_xohg_resyn = nullptr;
		bool use_db = ( ps_cut_rew.cut_enumeration_ps.cut_size == 5u ) ? false : true;

		mockturtle::exact_xohg_resynthesis_minmc xohg_resyn( "../experiments/db", ps_xohg_resyn, pst_xohg_resyn, use_db );

		uint32_t ite_cnt = 0u;
		const clock_t begin_time = clock();
		while ( num_oh > num_oh_aft )
		{
			++ite_cnt;
			num_oh = num_oh_aft;
			num_oh_aft = 0u;

			mockturtle::cut_rewriting_with_compatibility_graph( x1g, xohg_resyn, ps_cut_rew, nullptr, ::detail::num_oh<mockturtle::x1g_network>() );

			x1g = mockturtle::cleanup_dangling( x1g );

			x1g.foreach_gate( [&]( auto f ) {
				if ( x1g.is_onehot( f ) )
				{
					++num_oh_aft;
				}
			} );
		}

		const auto cec = abc_cec_crypto( x1g, benchmark );

		float improve = ( ( num_oh_bfr - num_oh_aft ) / num_oh_bfr ) * 100;

		exp_res( benchmark, num_oh_bfr, num_oh_aft, improve, ite_cnt, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) / ite_cnt, cec );

		//

		/* xmg optimization */
		/*

		mockturtle::xmg_network xmg;
		lorina::read_verilog( crypto_benchmark_path( benchmark ), mockturtle::verilog_reader( xmg ) );
		// or use mapper to get the initial xmg model

		uint32_t num_maj = 0u;
		uint32_t num_maj_bfr = 0u;
		uint32_t num_maj_aft = 0u;

		xmg.foreach_gate( [&]( auto f ) {
			if ( xmg.is_maj( f ) )
			{
				++num_maj;
			}
		} );
		num_maj_bfr = num_maj;

		mockturtle::exact_xohg_resynthesis_minmc_params ps_xmg_resyn;
		ps_xmg_resyn.print_stats = true;
		ps_xmg_resyn.cache = std::make_shared<mockturtle::exact_xohg_resynthesis_minmc_params::cache_map_t>();
		ps_xmg_resyn.blacklist_cache = std::make_shared<mockturtle::exact_xohg_resynthesis_minmc_params::blacklist_cache_map_t>();

		mockturtle::exact_xohg_resynthesis_minmc_stats* pst_xmg_resyn = nullptr;

		mockturtle::exact_xmg_resynthesis_minmc xmg_resyn( "../experiments/db", ps_xmg_resyn, pst_xmg_resyn );

		uint32_t ite_cnt = 0u;
		const clock_t begin_time = clock();
		while ( num_maj > num_maj_aft )
		{
			++ite_cnt;
			num_maj = num_maj_aft;
			num_maj_aft = 0u;

			mockturtle::cut_rewriting_with_compatibility_graph( xmg, xmg_resyn, ps_cut_rew, nullptr, ::detail::num_maj<mockturtle::xmg_network>() );
			xmg = mockturtle::cleanup_dangling( xmg );

			xmg.foreach_gate( [&]( auto f ) {
				if ( xmg.is_maj( f ) )
				{
					++num_maj_aft;
				}
			} );
		}

		const auto cec = abc_cec_crypto( xmg, benchmark );

		float improve = ( ( num_maj_bfr - num_maj_aft ) / num_maj_bfr ) * 100;

		exp_res( benchmark, num_maj_bfr, num_maj_aft, improve, ite_cnt, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) / ite_cnt, cec );

		*/
	}

	exp_res.save();
	exp_res.table();
	
	return 0;
}