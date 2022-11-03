#include <string>
#include <fmt/format.h>

#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <mockturtle/views/fanout_view.hpp>

#include <experiments.hpp>

using merge_view = mockturtle::fanout_view<mockturtle::topo_view<mockturtle::xag_network>>;

static const std::string crypto_epfl_benchmarks[] = {
	"adder_untilsat", "bar_untilsat", "div_untilsat", "log2_untilsat", "max_untilsat", "multiplier_untilsat", "sin_untilsat", "sqrt_untilsat", "square_untilsat",
  "arbiter_untilsat", "cavlc_untilsat", "ctrl_untilsat" , "dec_untilsat", "i2c_untilsat", "int2float_untilsat" , "mem_ctrl_untilsat", "priority_untilsat", "router_untilsat", "voter_untilsat", 
  "adder_32bit_untilsat", "adder_64bit_untilsat", "AES-expanded_untilsat", "AES-non-expanded_unstilsat", 
  "comparator_32bit_signed_lt_untilsat", "comparator_32bit_signed_lteq_untilsat", "comparator_32bit_unsigned_lt_untilsat", "comparator_32bit_unsigned_lteq_untilsat", 
  "DES-expanded_untilsat", "DES-non-expanded_untilsat", "md5_untilsat", "mult_32x32_untilsat", "sha-1_untilsat", 
  "sha-256_untilsat"};

std::vector<std::string> crypto_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 33u; ++i )
	{
		result.emplace_back( crypto_epfl_benchmarks[i] );
	}

	return result;
}

std::string crypto_benchmark_path( std::string const& benchmark_name )
{
  return fmt::format( "../experiments/crypto_benchmarks/{}.v", benchmark_name );
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
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, float> exp_res( "xohg_minmc", "benchmark", "num_and_before", "num_and_after", "garble_cost_before", "garble_cost_after", "improvement %", "avg. runtime [s]" );

	for ( auto const& benchmark: crypto_benchmarks() )
	{
		std::cout << "[i] processing " << benchmark << std::endl;

		mockturtle::xag_network xag;
		auto const read_result = lorina::read_verilog( crypto_benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );
		assert( read_result == lorina::return_code::success );

		uint32_t num_and_bfr = 0u;
		uint32_t num_and_aft = 0u;
		uint32_t garble_cost_bfr = 0u;
		uint32_t garble_cost_aft = 0u;
		std::vector<uint32_t> and_size;

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

				and_size.emplace_back( garble_cost_aft_each );
			}
		} );

		float improve = ( ( garble_cost_bfr - garble_cost_aft ) / garble_cost_bfr ) * 100;

		exp_res( benchmark, num_and_bfr, num_and_aft, garble_cost_bfr, garble_cost_aft, improve, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) );
	}

	exp_res.save();
	exp_res.table();
	
	return 0;
}