#include <mockturtle/algorithms/xag_decomp_max_local.hpp>
#include <mockturtle/properties/mccost.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>

#include "experiments.hpp"

static const std::string mpc_benchmark[] = {
  "auction_N_2_W_16_opt", "auction_N_4_W_32_opt",
  "knn_comb_K_1_N_8_opt", "knn_comb_K_3_N_16_opt",
  "stable_matching_comb_Ks_4_S_8_opt", "stable_matching_comb_Ks_8_S_8_opt",
  "voting_N_1_M_3_opt", "voting_N_2_M_4_opt"
};

static const uint32_t bench_num_paries[] = {
  4u, 16u,
  9u, 17u,
  16u, 16u,
  8u, 16u
};

// std::vector<std::string> mpc_two_party_benchmarks()
// {
//   std::vector<std::string> benchmarks{};
//   for ( auto i{ 0u }; i < 3u; ++i )
//   {
//     benchmarks.push_back( mpc_two_party_benchmark[i] );
//   }
//   return benchmarks;
// }

std::string benchmark_path( std::string const& benchmark_name )
{
  return fmt::format( "../experiments/mpc_benchmarks/{}.v", benchmark_name );
}

bool abc_cec( mockturtle::xag_network const& ntk, std::string const& benchmark )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"cec -n {} /tmp/test.bench\"", benchmark_path( benchmark ) );

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

// int main()
// {
//   auto const benchmarks = mpc_two_party_benchmarks();
//   std::string db_name = "xag_mc_min_6_db";
//   for ( auto const& benchmark : benchmarks )
//   {
//     // if ( benchmark == "voting_N_1_M_3_opt" || benchmark == "voting_N_1_M_3" )
//     if ( benchmark != "comparetor_32bit_signed_opt" )
//     {
//       continue;
//     }

//     fmt::print( "[i] Processing {}...\n", benchmark );

//     mockturtle::xag_network ntk;
//     const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Failed to read benchmark...\n" );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_parties = 2u;
//     mockturtle::xag_decomp_max_local_params ps;
//     mockturtle::xag_decomp_max_local_stats st;
//     mockturtle::xag_decomp_max_local decomp( num_parties, ntk, db_name, ps, st );
//     mockturtle::xag_network ntk_opt = decomp.run();

//     const bool eq = abc_cec( ntk_opt, benchmark );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: Non-equivalence!\n" );
//     }
//     else
//     {
//       fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] MC = {}, original cost = {}, optimized cost = {}\n",
//                   st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, *multiplicative_complexity( ntk ), st.cost_original, st.cost_optimal );
//     }

//     fmt::print( "\n" );
//   }

//   return 0;
// }

int main()
{
  std::string db_name = "xag_mc_min_6_db";
  experiments::experiment<std::string, uint32_t, uint32_t, uint32_t> exp( "mpc_max_local", "benchmark", "AND count", "joint AND count", "opt. joint AND count" );
  
  for ( auto i{ 0u }; i < std::size( mpc_benchmark ); ++i )
  {
    auto benchmark = mpc_benchmark[i];
    fmt::print( "[i] Processing {}...\n", benchmark );
    mockturtle::xag_network ntk;
    const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] Failed to read benchmark...\n" );
      abort();
    }
    fmt::print( "[i] Read benchmark successfully.\n" );

    const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

    const uint32_t num_parties = bench_num_paries[i];
    bool until_saturation{ false };
    mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk );
    const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
    uint32_t cost_cur = cost_orig;
    uint32_t cost_opt{ 0u };
    uint32_t iter_cnt{ 1u };

    do
    {
      mockturtle::xag_decomp_max_local_params ps;
      mockturtle::xag_decomp_max_local_stats st;
      ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk );
      mockturtle::xag_decomp_max_local_in_place decomp( num_parties, ntk_mpc, db_name, ps, st );
      decomp.run();
      mockturtle::xag_network ntk_opt = ntk_mpc.toNtk();
      cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
      if ( cost_opt < cost_cur )
      {
        cost_cur = cost_opt;
        ntk = ntk_opt;
        fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
                    st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
        fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
      }
      else
      {
        fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
                    st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
        fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
        break;
      }
    } while ( until_saturation );

    const bool eq = abc_cec( ntk, benchmark );
    if ( !eq )
    {
      fmt::print( "[e] Fatal error: non-equivalence!\n" );
    }
    else
    {
      fmt::print( "[i] Passed equivelence checking\n" );
    }

    fmt::print( "\n" );

    exp( benchmark, num_ands, cost_orig, cost_opt );
  }

  exp.save();
  exp.table();

  return 0;
}