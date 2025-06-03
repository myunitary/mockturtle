#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/properties/mccost.hpp>
#include <mockturtle/views/ownership_view.hpp>

static const std::string mpc_two_party_benchmark[] = {
  "voting_N_1_M_3", "voting_N_1_M_4"
  // "voting_N_1_M_3_opt", "voting_N_1_M_4_opt"
};

std::vector<std::string> mpc_two_party_benchmarks()
{
  std::vector<std::string> benchmarks{};
  for ( auto i{ 0u }; i < 2u; ++i )
  {
    benchmarks.push_back( mpc_two_party_benchmark[i] );
  }
  return benchmarks;
}

std::string benchmark_path( std::string const& benchmark_name )
{
  return fmt::format( "../experiments/mpc_benchmarks/{}.v", benchmark_name );
}

int main_ownership_view()
{
  auto const benchmarks = mpc_two_party_benchmarks();
  for ( auto const& benchmark : benchmarks )
  {
    fmt::print( "[i] processing {}...\n", benchmark );
    
    mockturtle::xag_network ntk;
    const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] failed to read benchmark...\n" );
      abort();
    }
    fmt::print( "[i] read benchmark successfully!\n" );

    /* profiling the circuit */
    const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );
    const uint32_t num_xors = ntk.num_gates() - num_ands;

    mockturtle::ownership_view<mockturtle::xag_network> ntk_os{ ntk };
    ntk_os.assign_ownership();
    uint32_t num_joint_ands{}, num_joint_xors{};
    ntk_os.foreach_gate( [&ntk_os, &num_joint_ands, &num_joint_xors]( auto const& n ) {
      if ( __builtin_popcount( ntk_os.get_ownership( n ) ) > 1 )
      {
        if ( ntk_os.is_and( n ) )
        {
          ++num_joint_ands;
        }
        else
        {
          ++num_joint_xors;
        }
      }
    } );

    fmt::print( "[i] # ANDs = {}, # XORs = {}\n", num_ands, num_xors );
    fmt::print( "[i] # joint AND = {} ( -{:.2f}% ), # joint XOR = {}\n", num_joint_ands, ( static_cast<double>( num_ands - num_joint_ands ) / num_ands ), num_joint_xors );
  }

  return 0;
}

int main()
{
  auto const benchmarks = mpc_two_party_benchmarks();
  for ( auto const& benchmark : benchmarks )
  {
    fmt::print( "[i] processing {}...\n", benchmark );
    
    mockturtle::xag_network ntk;
    const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] failed to read benchmark...\n" );
      abort();
    }
    fmt::print( "[i] read benchmark successfully!\n" );

    /* profiling the circuit */
    const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );
    const uint32_t num_xors = ntk.num_gates() - num_ands;

    mockturtle::owner_view ntk_os{ ntk };
    uint32_t num_joint_ands{}, num_joint_xors{};
    ntk_os.foreach_gate( [&ntk_os, &num_joint_ands, &num_joint_xors]( auto const& n ) {
      if ( ntk_os.is_local( n ) == 0u )
      {
        if ( ntk_os.is_and( n ) )
        {
          ++num_joint_ands;
        }
        else
        {
          ++num_joint_xors;
        }
      }
    } );

    fmt::print( "[i] # ANDs = {}, # XORs = {}\n", num_ands, num_xors );
    fmt::print( "[i] # joint AND = {} ( -{:.2f}% ), # joint XOR = {}\n", num_joint_ands, ( static_cast<double>( num_ands - num_joint_ands ) / num_ands ), num_joint_xors );
  }

  return 0;
}