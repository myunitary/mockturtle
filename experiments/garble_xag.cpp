#include <string>
#include <fmt/format.h>

#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mapper.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <mockturtle/views/fanout_view.hpp>

#include <experiments.hpp>

using merge_view = mockturtle::fanout_view<mockturtle::topo_view<mockturtle::xag_network>>;

static const std::string EPFL_benchmarks[] = {
	"adder", "bar", "div", "log2", "max", "multiplier", "sin", "sqrt", "square", "arbiter", 
	"cavlc", "ctrl" , "dec", "i2c", "int2float" , "mem_ctrl", "priority", "router", "voter", 
	"hyp"};

static const std::string CRYPTO_benchmarks[] = {
  "adder_32bit_untilsat", "adder_64bit_untilsat", "AES-expanded_untilsat", "AES-non-expanded_unstilsat", 
  "comparator_32bit_signed_lt_untilsat", "comparator_32bit_signed_lteq_untilsat", "comparator_32bit_unsigned_lt_untilsat", "comparator_32bit_unsigned_lteq_untilsat", 
  "DES-expanded_untilsat", "DES-non-expanded_untilsat", "md5_untilsat", "mult_32x32_untilsat", "sha-1_untilsat", 
  "sha-256_untilsat"};

static const std::string MPC_benchmarks[] = {
  "auction_N_2_W_16", "auction_N_2_W_32", "auction_N_3_W_16", "auction_N_3_W_32", "auction_N_4_W_16", "auction_N_4_W_32", 
  "knn_comb_K_1_N_8", "knn_comb_K_1_N_16", "knn_comb_K_2_N_8", "knn_comb_K_2_N_16", "knn_comb_K_3_N_8", "knn_comb_K_3_N_16", 
  "voting_N_1_M_3", "voting_N_1_M_4", "voting_N_2_M_2", "voting_N_2_M_3", "voting_N_2_M_4", "voting_N_3_M_4", 
  "stable_matching_comb_Ks_4_S_8", "stable_matching_comb_Ks_8_S_8"};

std::vector<std::string> crypto_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 14u; ++i )
	{
		result.emplace_back( CRYPTO_benchmarks[i] );
	}

	return result;
}

std::vector<std::string> epfl_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 20u; ++i )
	{
		result.emplace_back( EPFL_benchmarks[i] );
	}

	return result;
}

std::vector<std::string> mpc_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 20u; ++i )
	{
		result.emplace_back( MPC_benchmarks[i] );
	}

	return result;
}

std::string benchmark_path( uint32_t benchmark_type, std::string const& benchmark_name )
{
	switch( benchmark_type )
	{
	case 0u:
		return fmt::format( "../experiments/epfl_benchmarks/{}.v", benchmark_name );
	case 1u:
		return fmt::format( "../experiments/crypto_benchmarks/{}.v", benchmark_name );
	case 2u:
		return fmt::format( "../experiments/mpc_benchmarks/{}.aig", benchmark_name );
	default:
		std::cout << "Unspecified type of benchmark. \n";
		abort();
	}
}

void count_and_size_rec( merge_view & xag, mockturtle::xag_network::node const& f, mockturtle::xag_network::node const& root )
{
	if ( xag.value( f ) == 0u ) 
	{	
		xag.incr_value( f );

		if ( xag.fanout( f ).size() == 1u )
		{
			// f is a candidate to be merged
			if ( f != root ) 
			{
				xag.incr_value( root );
			}

			xag.foreach_fanin( f, [&]( auto const fi ) {
				auto const child = xag.get_node( fi );

				if ( xag.is_and( child ) )
				{
					count_and_size_rec( xag, child, root );
				}
			} );
		}
	}
}

int main()
{
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, uint32_t, float> exp_res( "garble_xag", "benchmark", "num_and_before", "num_and_after", "garble_cost_before", "garble_cost_after", "improvement %", "maximum fanin size", "avg. runtime [s]" );
	uint32_t benchmark_type = 2u; /* 0u - epfl benchmark; 1u - crypto benchmark; 2u - mpc benchmark */
	std::vector<std::string> benchmarks;
	switch( benchmark_type )
	{
	case 0u:
		benchmarks = epfl_benchmarks();
		break;
	case 1u:
		benchmarks = crypto_benchmarks();
		break;
	case 2u:
		benchmarks = mpc_benchmarks();
		break;
	default:
		std::cout << "Unspecified type of benchmark. \n";
		abort();
	}

	for ( auto const& benchmark: benchmarks )
	{
		std::cout << "[i] processing " << benchmark << std::endl;

		mockturtle::xag_network xag;
		if ( benchmark_type == 2u )
		{
			auto const read_result = lorina::read_aiger( benchmark_path( benchmark_type, benchmark ), mockturtle::aiger_reader( xag ) );
			assert( read_result == lorina::return_code::success );
		}
		else
		{
			auto const read_result = lorina::read_verilog( benchmark_path( benchmark_type, benchmark ), mockturtle::verilog_reader( xag ) );
			assert( read_result == lorina::return_code::success );
		}
		
		mockturtle::xag_npn_resynthesis<mockturtle::xag_network> xag_resyn;
		mockturtle::exact_library<mockturtle::xag_network, decltype( xag_resyn )> lib( xag_resyn );
		xag = mockturtle::map( xag, lib );

		uint32_t num_and_bfr = 0u;
		uint32_t num_and_aft = 0u;
		uint32_t garble_cost_bfr = 0u;
		uint32_t garble_cost_aft = 0u;
		uint32_t max_fan_in = 0u;
		//std::vector<uint32_t> and_size;

		xag.foreach_gate( [&]( auto f ) {
			if ( xag.is_and( f ) )
			{
				++num_and_bfr;
			}
		} );
		garble_cost_bfr = num_and_bfr * 2;

		const clock_t begin_time = clock();
		merge_view xag_topo{xag};
		xag_topo.clear_values();

		xag_topo.foreach_gate( [&]( auto f ) {
			if ( xag_topo.is_and( f ) && xag_topo.value( f ) == 0u )
			{
				// this is the root node of a potentially larger AND
				++num_and_aft;
				uint32_t garble_cost_aft_each = 2u;
				count_and_size_rec( xag_topo, f, f );
				garble_cost_aft_each += xag_topo.value( f );
				garble_cost_aft += garble_cost_aft_each - 1u;

				//and_size.emplace_back( garble_cost_aft_each );
				if ( garble_cost_aft_each > max_fan_in )
				{
					max_fan_in = garble_cost_aft_each - 1u;
				}
			}
		} );

		float improve = ( ( static_cast<float> ( garble_cost_bfr ) - static_cast<float> ( garble_cost_aft ) ) / static_cast<float> ( garble_cost_bfr ) ) * 100;

		exp_res( benchmark, num_and_bfr, num_and_aft, garble_cost_bfr, garble_cost_aft, improve, max_fan_in, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) );
	}

	exp_res.save();
	exp_res.table();
	
	return 0;
}