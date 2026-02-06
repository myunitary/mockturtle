#include <mockturtle/algorithms/xag_decomp_max_local.hpp>
#include <mockturtle/properties/mccost.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/io/write_bench.hpp>

#include <chrono>

#include "experiments.hpp"

static const std::string toy_mpc_benchmark[] = {
  "and_ary_2", "and_ary_4", "and_ary_8", "and_ary_16"
};

static const std::string point_and_tree_benchmark[] = {
  "point_and_ary_2", "point_and_ary_4", "point_and_ary_8", "point_and_ary_16"
};

static const uint32_t toy_bench_num_paries[] = {
  2u, 2u, 2u, 2u
};

static const std::string mpc_benchmark[] = {
  "auction_N_2_W_16_opt", "auction_N_4_W_32_opt",
  "knn_comb_K_1_N_8_opt", "knn_comb_K_3_N_16_opt",
  "stable_matching_comb_Ks_4_S_8_opt", "stable_matching_comb_Ks_8_S_8_opt",
  "voting_N_1_M_3_opt", "voting_N_2_M_4_opt"
};

static const std::string epfl_benchmark[] = {
  "adder_opt",
  "div_opt",
  "max_opt",
  "multiplier_opt",
  "hyp_opt"
};

static const std::string sort_benchmark[] = {
  "bitonic_sort_inc_1_4_1"
  // "bitonic_sort_inc_1_4",
  // "bitonic_sort_inc_1_8",
  // "bitonic_sort_inc_2_8",
  // "bitonic_sort_inc_4_8"
};

static const std::string hash_benchmark[] = {
  "md5", "sha1", "sha256"
};

static const std::string tiny_garble_benchmark[] = {
  "aes_1cc_firstframe",
  "hamming_32bit_1cc_firstframe",
  "Lite_MIPS_firstframe",
  "comparator_32bit_unsigned_lt",
  "encoder_32bit_1cc_firstframe",
  "sha3_24cc_firstframe"
};

static const std::string float_arith_benchmark[] = {
  "FP-add_opt",
  "FP-div_opt",
  "FP-eq_opt",
  "FP-lt_opt",
  "FP-mul_opt"
};

static const uint32_t mpc_num_paries[] = {
  4u, 16u,
  9u, 17u,
  16u, 16u,
  8u, 16u
};

static const uint32_t epfl_num_paries[] = {
  2u, 2u,
  4u, 2u,
  2u
};

static const uint32_t hash_num_paries[] = {
  2u, 2u, 2u
};

static const uint32_t tiny_garble_num_parties[] = {
  2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u
};

static const uint32_t float_arith_num_parties[] = {
  2u, 2u, 2u, 2u, 2u
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
  // return fmt::format( "../experiments/mpc_benchmarks/{}.v", benchmark_name );
  // return fmt::format( "../experiments/EPFL_benchmarks/{}.v", benchmark_name );
  return fmt::format( "../experiments/and_ary_n/{}.v", benchmark_name );
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

bool abc_cec_spe_path( mockturtle::xag_network const& ntk, std::string const& path )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"cec -n {} /tmp/test.bench\"", path );

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

void profile_xag( mockturtle::xag_network const& ntk )
{
  fmt::print( "[m] Printing out the XAG network:\n" );
  fmt::print( "[m] PIs : " );
  ntk.foreach_pi( [&ntk]( auto const& pi ) {
    fmt::print( "pi{} ", ntk.node_to_index( pi ) );
  } );
  fmt::print( "\n" );
  fmt::print( "[m] Nodes : " );
  ntk.foreach_gate( [&ntk]( auto const& n ) {
    fmt::print( "n{} = {}( ", ntk.node_to_index( n ), ( ntk.is_and( n ) ? "AND" : "XOR" ) );
    ntk.foreach_fanin( n, [&ntk, &n]( auto const& ni, auto index ) {
      auto ni_n = ntk.get_node( ni );
      fmt::print( "{}{}{}{}", ( ntk.is_complemented( ni ) ? "~" : "" ), ( ntk.is_pi( ni_n ) ? "pi" : "n" ), ( ntk.node_to_index( ni_n ) ), ( index == 0 ? ", " : " ); " ) );
    } );
  } );
  fmt::print( "\n" );
  fmt::print("[m] POs : ");
  ntk.foreach_po( [&ntk]( auto const& po ) {
  fmt::print( "{}{}{} ", ( ntk.is_complemented( po ) ? "~" : "" ), ( ntk.is_pi( ntk.get_node( po ) ) ? "pi" : "n" ), ( ntk.node_to_index( ntk.get_node( po ) ) ) );
  } );
  fmt::print( "\n" );
}

void save_verilog( mockturtle::xag_network const& ntk, std::string const& benchmark, std::string const& dir )
{
  mockturtle::write_bench( ntk, "/tmp/tmp.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"read /tmp/tmp.bench; write_verilog {}.v\"", ( dir + benchmark ) );
  std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

  if ( !pipe )
  {
    throw std::runtime_error( "popen() failed" );
  }
}

// int main()
// {
//   std::string db_name = "xag_mc_min_6_db";
//   experiments::experiment<std::string, uint32_t, uint32_t, uint32_t> exp( "point_and_tree_max_local", "benchmark", "AND count", "joint AND count", "opt. joint AND count" );

//   // for ( auto i{ 0u }; i < std::size( mpc_benchmark ); ++i )
//   for ( auto i{ 0u }; i < std::size( point_and_tree_benchmark ); ++i )
//   {
//     // const std::string benchmark = mpc_benchmark[i];
//     const std::string benchmark = point_and_tree_benchmark[i];
//     fmt::print( "[i] Processing {}...\n", benchmark );

//     mockturtle::xag_network ntk;
//     const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Failed to read benchmark...\n" );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

//     // const uint32_t num_parties = bench_num_paries[i];
//     const uint32_t num_parties = toy_bench_num_paries[i];
//     mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk, num_parties );
//     const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//     // uint32_t cost_cur = cost_orig;
//     uint32_t cost_opt{ 0u };
//     mockturtle::xag_decomp_max_local_params ps;
//     mockturtle::xag_decomp_max_local_stats st;
//     mockturtle::xag_decomp_max_local decomp( num_parties, ntk, db_name, ps, st );
//     mockturtle::xag_network ntk_opt = decomp.run();
//     ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk_opt, num_parties );
//     cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );

//     const bool eq = abc_cec( ntk_opt, benchmark );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: Non-equivalence!\n" );
//     }
//     else
//     {
//       fmt::print( "[i] Passed equivelence checking\n" );
//     }

//     fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] MC = {}, original cost = {}, optimized cost = {}\n\n",
//                 st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, *multiplicative_complexity( ntk ), st.cost_original, st.cost_optimal );

//     exp( benchmark, num_ands, cost_orig, cost_opt );
//     // profile_xag( ntk_opt );
//   }

//   exp.save();
//   exp.table();

//   return 0;
// }

// int main()
// {
//   std::string db_name = "xag_mc_min_6_db";
//   experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t> exp( "epfl_max_local_non_in_place", "benchmark", "AND count", "joint AND count", "opt. joint AND count", "# itr." );
  
//   // for ( auto i{ 0u }; i < std::size( mpc_benchmark ); ++i )
//   for ( auto i{ 0u }; i < std::size( epfl_benchmark ); ++i )
//   // for ( auto i{ 0u }; i < std::size( toy_mpc_benchmark ); ++i )
//   // for ( auto i{ 1u }; i < 2u; ++i )
//   {
//     // auto benchmark = mpc_benchmark[i];
//     auto benchmark = epfl_benchmark[i];
//     // auto benchmark = toy_mpc_benchmark[i];
//     fmt::print( "[i] Processing {}...\n", benchmark );
//     mockturtle::xag_network ntk;
//     const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Failed to read benchmark...\n" );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

//     // const uint32_t num_parties = mpc_num_paries[i];
//     // const uint32_t num_parties = toy_bench_num_paries[i];
//     const uint32_t num_parties = epfl_num_paries[i];
//     bool until_saturation{ true };
//     const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( mockturtle::owner_view<mockturtle::xag_network>{ ntk, num_parties } );
//     uint32_t cost_cur = cost_orig;
//     uint32_t cost_opt = cost_orig;
//     uint32_t iter_cnt{ 1u };
//     // bool first{ true };

//     do
//     {
//       mockturtle::xag_decomp_max_local_params ps;
//       mockturtle::xag_decomp_max_local_stats st;
//       mockturtle::xag_decomp_max_local decomp( num_parties, ntk, db_name, ps, st );
//       mockturtle::xag_network ntk_opt = decomp.run();
//       // cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//       if ( st.cost_optimal < cost_cur )
//       // if ( st.modified && ( cost_opt <= cost_cur ) )
//       {
//         cost_cur = st.cost_optimal;
//         cost_opt = st.cost_optimal;
//         ntk = ntk_opt;
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
//       }
//       else
//       {
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
//         break;
//       }
//     } while ( until_saturation );

//     const bool eq = abc_cec( ntk, benchmark );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: non-equivalence!\n" );
//       abort();
//     }
//     else
//     {
//       fmt::print( "[i] Passed equivelence checking\n" );
//     }

//     fmt::print( "\n" );

//     exp( benchmark, num_ands, cost_orig, cost_opt, iter_cnt );

//     /* measure the number of input bits from each party */
//     fmt::print( "[i] Identifying the new input frontier...\n" );
//     std::vector<uint32_t> num_pis_new = mockturtle::identify_frontier( ntk, num_parties );
//     for ( auto j{ 0u }; j < num_pis_new.size(); ++j )
//     {
//       fmt::print( "[i] {} input bits required from the {}-th party\n", num_pis_new[j], ( j + 1 ) );
//     }
//     fmt::print( "\n" );
//   }

//   exp.save();
//   exp.table();

//   return 0;
// }

// int main()
// {
//   std::string db_name = "xag_mc_min_6_db";
//   experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t> exp( "epfl_max_local_in_place_rev_test", "benchmark", "AND count", "joint AND count", "opt. joint AND count", "# itr." );
  
//   // for ( auto i{ 0u }; i < std::size( mpc_benchmark ); ++i )
//   // for ( auto i{ 0u }; i < std::size( epfl_benchmark ); ++i )
//   // for ( auto i{ 0u }; i < std::size( toy_mpc_benchmark ); ++i )
//   for ( auto i{ 1u }; i < 2u; ++i )
//   {
//     // auto benchmark = mpc_benchmark[i];
//     auto benchmark = epfl_benchmark[i];
//     // auto benchmark = toy_mpc_benchmark[i];
//     fmt::print( "[i] Processing {}...\n", benchmark );
//     mockturtle::xag_network ntk;
//     const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Failed to read benchmark...\n" );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

//     // const uint32_t num_parties = mpc_num_paries[i];
//     // const uint32_t num_parties = toy_bench_num_paries[i];
//     const uint32_t num_parties = epfl_num_paries[i];
//     bool until_saturation{ true };
//     mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk, num_parties );
//     const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//     uint32_t cost_cur = cost_orig;
//     uint32_t cost_opt{ 0u };
//     uint32_t iter_cnt{ 1u };
//     bool first{ true };

//     do
//     {
//       mockturtle::xag_decomp_max_local_params ps;
//       mockturtle::xag_decomp_max_local_stats st;
//       if ( !first )
//       {
//         ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk, num_parties );
//       }
//       first = false;

//       mockturtle::xag_decomp_max_local_in_place decomp( num_parties, ntk_mpc, db_name, ps, st );
//       // decomp.run_reverse_topo();
//       decomp.run();
//       mockturtle::xag_network ntk_opt = ntk_mpc.toNtk();
//       cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//       // if ( cost_opt < cost_cur )s
//       if ( st.modified && ( cost_opt <= cost_cur ) )
//       {
//         cost_cur = cost_opt;
//         ntk = ntk_opt;
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
//       }
//       else
//       {
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
//         break;
//       }
//     } while ( until_saturation );

//     const bool eq = abc_cec( ntk, benchmark );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: non-equivalence!\n" );
//       abort();
//     }
//     else
//     {
//       fmt::print( "[i] Passed equivelence checking\n" );
//     }

//     fmt::print( "\n" );

//     exp( benchmark, num_ands, cost_orig, cost_opt, iter_cnt );

//     /* measure the number of input bits from each party */
//     fmt::print( "[i] Identifying the new input frontier...\n" );
//     std::vector<uint32_t> num_pis_new = mockturtle::identify_frontier( ntk, num_parties );
//     for ( auto j{ 0u }; j < num_pis_new.size(); ++j )
//     {
//       fmt::print( "[i] {} input bits required from the {}-th party\n", num_pis_new[j], ( j + 1 ) );
//     }
//     fmt::print( "\n" );
//   }

//   exp.save();
//   exp.table();

//   return 0;
// }

/* experiments on sorting algorithms */
// int main()
// {
//   std::string db_name = "xag_mc_min_6_db";
//   experiments::experiment<std::string, uint32_t, uint32_t, uint32_t> exp( "sort_max_local_toy", "benchmark", "AND count", "joint AND count", "opt. joint AND count" );

//   // for ( auto i = 0u; i < std::size( sort_benchmark ); ++i )
//   for ( auto i = 0u; i < 1u; ++i )
//   {
//     std::string benchmark = sort_benchmark[i];
//     uint8_t bitwidth = 1 << i;
//     uint32_t num_parties = 2u;
//     fmt::print( "[i] Processing {}...\n", benchmark );
//     mockturtle::xag_network ntk;
//     const std::string path = "../experiments/sort/" + benchmark + ".v";
//     const auto read_stats = lorina::read_verilog( ( path ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Cannot access {}... Failed to read benchmark...\n", path );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

//     bool until_saturation{ true };
//     // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 1u, 1u, 2u, 2u, 2u, 2u };
//     // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 1u, 2u, 1u, 2u, 2u, 2u };
//     // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 2u, 2u, 1u, 1u, 2u, 2u };
//     // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 2u, 1u, 2u, 1u, 2u, 1u, 2u };
//     // std::array<uint32_t, 4u> pis_ownership_per_word = { 1u, 1u, 2u, 2u };
//     std::array<uint32_t, 4u> pis_ownership_per_word = { 1u, 2u, 1u, 2u };
//     std::vector<uint32_t> pis_ownership_per_bit( pis_ownership_per_word.size() * bitwidth );
//     for ( auto j{ 0u }; j < pis_ownership_per_word.size(); ++j )
//     {
//       for ( auto k{ 0u }; k < bitwidth; ++k )
//       {
//         pis_ownership_per_bit[j * bitwidth + k] = pis_ownership_per_word[j];
//       }
//     }
//     mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk, num_parties, pis_ownership_per_bit, true );
//     const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//     uint32_t cost_cur = cost_orig;
//     uint32_t cost_opt{ 0u };
//     uint32_t iter_cnt{ 1u };
//     bool first{ true };

//     do
//     {
//       mockturtle::xag_decomp_max_local_params ps;
//       mockturtle::xag_decomp_max_local_stats st;
//       if ( !first )
//       {
//         ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk, num_parties, pis_ownership_per_bit, true );
//       }
//       first = false;

//       mockturtle::xag_decomp_max_local_in_place decomp( num_parties, ntk_mpc, db_name, ps, st );
//       // decomp.run();
//       /* run it! */
//       decomp.run_reverse_topo();
//       mockturtle::xag_network ntk_opt = ntk_mpc.toNtk();
//       cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//       if ( cost_opt < cost_cur )
//       // if ( st.modified && ( cost_opt <= cost_cur ) )
//       {
//         cost_cur = cost_opt;
//         ntk = ntk_opt;
//         mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
//       }
//       else
//       {
//         // mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
//         break;
//       }
//     } while ( until_saturation );

//     // ( void ) first;
//     // do
//     // {
//     //   mockturtle::xag_decomp_max_local_params ps;
//     //   mockturtle::xag_decomp_max_local_stats st;
//     //   mockturtle::xag_decomp_max_local decomp( num_parties, ntk, db_name, ps, st, pis_ownership_per_bit );
//     //   mockturtle::xag_network ntk_opt = decomp.run();
//     //   cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( mockturtle::owner_view<mockturtle::xag_network>{ ntk_opt, num_parties, pis_ownership_per_bit, true } );
//     //   if ( cost_opt < cost_cur )
//     //   {
//     //     cost_cur = cost_opt;
//     //     ntk = ntk_opt;
//     //     fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//     //                 st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//     //     fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
//     //   }
//     //   else
//     //   {
//     //     mockturtle::write_verilog( ntk_opt, "test.v" );
//     //     fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//     //                 st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//     //     fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
//     //     break;
//     //   }
//     // } while ( until_saturation );
    

//     const bool eq = abc_cec_spe_path( ntk, path );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: non-equivalence!\n" );
//       abort();
//     }
//     else
//     {
//       fmt::print( "[i] Passed equivelence checking\n" );
//     }

//     fmt::print( "\n" );

//     exp( benchmark, num_ands, cost_orig, cost_opt );

//     /* measure the number of input bits from each party */
//     fmt::print( "[i] Identifying the new input frontier...\n" );
//     std::vector<uint32_t> num_pis_new = mockturtle::identify_frontier( ntk, num_parties, pis_ownership_per_bit, true );
//     for ( auto j{ 0u }; j < num_pis_new.size(); ++j )
//     {
//       fmt::print( "[i] {} input bits required from the {}-th party\n", num_pis_new[j], ( j + 1 ) );
//     }
//     fmt::print( "\n" );
//   }

//   exp.save();
//   exp.table();

//   return 0;
// }

/* experiments on hash functions */
// int main()
// {
//   std::string db_name = "xag_mc_min_6_db";
//   experiments::experiment<std::string, uint32_t, uint32_t, uint32_t> exp( "hash_max_local_two_party", "benchmark", "AND count", "joint AND count", "opt. joint AND count" );

//   for ( auto i = 0u; i < std::size( hash_benchmark ); ++i )
//   {
//     std::string benchmark = hash_benchmark[i];
//     uint32_t num_parties = hash_num_paries[i];
//     fmt::print( "[i] Processing {}...\n", benchmark );
//     mockturtle::xag_network ntk;
//     const std::string path = "../experiments/hash/" + benchmark + ".v";
//     const auto read_stats = lorina::read_verilog( ( path ), mockturtle::verilog_reader( ntk, benchmark ) );
//     if ( read_stats != lorina::return_code::success )
//     {
//       fmt::print( "[e] Cannot access {}... Failed to read benchmark...\n", path );
//       abort();
//     }
//     fmt::print( "[i] Read benchmark successfully.\n" );

//     const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

//     bool until_saturation{ true };
//     mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk, num_parties );
//     const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//     uint32_t cost_cur = cost_orig;
//     uint32_t cost_opt{ 0u };
//     uint32_t iter_cnt{ 1u };
//     bool first{ true };

//     do
//     {
//       mockturtle::xag_decomp_max_local_params ps;
//       mockturtle::xag_decomp_max_local_stats st;
//       if ( !first )
//       {
//         ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk, num_parties );
//       }
//       first = false;

//       mockturtle::xag_decomp_max_local_in_place decomp( num_parties, ntk_mpc, db_name, ps, st );
//       // decomp.run();
//       /* run it! */
//       decomp.run_reverse_topo();
//       mockturtle::xag_network ntk_opt = ntk_mpc.toNtk();
//       cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
//       if ( cost_opt < cost_cur )
//       // if ( st.modified && ( cost_opt <= cost_cur ) )
//       {
//         cost_cur = cost_opt;
//         ntk = ntk_opt;
//         // mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
//       }
//       else
//       {
//         // mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
//         fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
//                     st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
//         fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
//         break;
//       }
//     } while ( until_saturation );

//     const bool eq = abc_cec_spe_path( ntk, path );
//     if ( !eq )
//     {
//       fmt::print( "[e] Fatal error: non-equivalence!\n" );
//       abort();
//     }
//     else
//     {
//       fmt::print( "[i] Passed equivelence checking\n" );
//     }

//     fmt::print( "\n" );

//     exp( benchmark, num_ands, cost_orig, cost_opt );

//     /* measure the number of input bits from each party */
//     fmt::print( "[i] Identifying the new input frontier...\n" );
//     std::vector<uint32_t> num_pis_new = mockturtle::identify_frontier( ntk, num_parties );
//     for ( auto j{ 0u }; j < num_pis_new.size(); ++j )
//     {
//       fmt::print( "[i] {} input bits required from the {}-th party\n", num_pis_new[j], ( j + 1 ) );
//     }
//     fmt::print( "\n" );
//   }

//   exp.save();
//   exp.table();

//   return 0;
// }

/* experiments on TinyGarble benchmarks */
int main()
{
  std::string db_name = "xag_mc_min_6_db";
  // std::string benchmark_suite = "EPFL";
  // std::string benchmark_suite = "tiny_garble";
  std::string benchmark_suite = "float_arith";
  experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, float> exp( ( fmt::format( "hybrid_{}_max_local", benchmark_suite ) ), "benchmark", "AND count", "joint AND count", "opt. joint AND count", "runtime [s]" );

  // for ( auto i = 0u; i < std::size( epfl_benchmark ); ++i )
  // for ( auto i = 0u; i < std::size( tiny_garble_benchmark ); ++i )
  for ( auto i = 0u; i < std::size( float_arith_benchmark ); ++i )
  {
    // std::string benchmark = epfl_benchmark[i];
    // uint32_t num_parties = epfl_num_paries[i];
    // std::string benchmark = tiny_garble_benchmark[i];
    // uint32_t num_parties = tiny_garble_num_parties[i];
    std::string benchmark = float_arith_benchmark[i];
    uint32_t num_parties = float_arith_num_parties[i];
    fmt::print( "[i] Processing {}...\n", benchmark );
    mockturtle::xag_network ntk;
    // const std::string path = fmt::format("../experiments/EPFL_benchmarks/topo_jmc_opt/{}.v", benchmark );
    // const std::string path = "../experiments/tiny_garble_benchmarks/topo_jmc_opt/" + benchmark + ".v";
    const std::string path = "../experiments/float_arith_benchmarks/topo_jmc_opt/" + benchmark + ".v";
    const auto read_stats = lorina::read_verilog( ( path ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] Cannot access {}... Failed to read benchmark...\n", path );
      abort();
    }
    fmt::print( "[i] Read benchmark successfully.\n" );

    const uint32_t num_ands = *mockturtle::multiplicative_complexity( ntk );

    bool until_saturation{ true };
    mockturtle::owner_view<mockturtle::xag_network> ntk_mpc( ntk, num_parties );
    const uint32_t cost_orig = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
    uint32_t cost_cur = cost_orig;
    uint32_t cost_opt{ 0u };
    uint32_t iter_cnt{ 1u };
    bool first{ true };

    const auto begin_time = std::chrono::high_resolution_clock::now();
    do
    {
      mockturtle::xag_decomp_max_local_params ps;
      mockturtle::xag_decomp_max_local_stats st;
      if ( !first )
      {
        ntk_mpc = mockturtle::owner_view<mockturtle::xag_network>( ntk, num_parties );
      }
      first = false;

      mockturtle::xag_decomp_max_local_in_place decomp( num_parties, ntk_mpc, db_name, ps, st );
      // decomp.run();
      /* run it! */
      decomp.run_reverse_topo();
      mockturtle::xag_network ntk_opt = ntk_mpc.toNtk();
      cost_opt = mockturtle::costs<mockturtle::owner_view<mockturtle::xag_network>, mockturtle::mc_mpc_cost>( ntk_mpc );
      st.cost_optimal = cost_opt;
      if ( cost_opt < cost_cur )
      // if ( st.modified && ( cost_opt <= cost_cur ) )
      {
        cost_cur = cost_opt;
        ntk = ntk_opt;
        // mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
        fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
                    st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
        fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt++ );
      }
      else
      {
        // mockturtle::write_verilog( ntk, fmt::format( "test_rnd{}.v", iter_cnt ) );
        fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n[i] original cost = {}, optimized cost = {}\n",
                    st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl, st.cost_original, st.cost_optimal );
        fmt::print( "[m] Finished the {}-th round of optimization\n", iter_cnt );
        break;
      }
    } while ( until_saturation );

    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - begin_time;

    const bool eq = abc_cec_spe_path( ntk, path );
    if ( !eq )
    {
      fmt::print( "[e] Fatal error: non-equivalence!\n" );
      abort();
    }
    else
    {
      fmt::print( "[i] Passed equivelence checking\n" );
    }

    fmt::print( "\n" );

    exp( benchmark, num_ands, cost_orig, cost_opt, duration.count() );
    // exp( benchmark, num_ands, cost_orig, cost_orig );

    /* measure the number of input bits from each party */
    fmt::print( "[i] Identifying the new input frontier...\n" );
    std::vector<uint32_t> num_pis_new = mockturtle::identify_frontier( ntk, num_parties );
    for ( auto j{ 0u }; j < num_pis_new.size(); ++j )
    {
      fmt::print( "[i] {} input bits required from the {}-th party\n", num_pis_new[j], ( j + 1 ) );
    }
    fmt::print( "\n" );

    save_verilog( ntk, benchmark, fmt::format( "../experiments/{}_benchmarks/jmc_opt/", benchmark_suite ) );
  }

  exp.save();
  exp.table();

  return 0;
}