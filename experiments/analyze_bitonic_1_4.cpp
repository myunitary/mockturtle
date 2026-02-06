#include <kitty/print.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>
// #include <mockturtle/views/ownership_view.hpp>
#include <mockturtle/views/topo_view.hpp>

static const std::string benchmark_to_visualize[] = {
  "bitonic_sort_inc_1_4"
  // "bitonic_sort_inc_1_4_ideal"
  // "bitonic_sort_inc_1_4_0_opt"
  // "bitonic_sort_inc_1_4_0"
};

// using mpc_network = mockturtle::owner_view<mockturtle::xag_network>;

void print_sim_res( mockturtle::node_map<kitty::static_truth_table<4u>, mockturtle::xag_network> const& sim_res,
                    mockturtle::xag_network const& ntk )
{
  // mockturtle::topo_view{ ntk }.foreach_node( [&]( auto const& n ) {
  ntk.foreach_node( [&]( auto const& n ) {
    if ( ntk.is_constant( n ) )
    {
      return;
    }

    std::string prefix = "Node";
    // uint32_t index = n - ntk.num_pis();
    uint32_t index = n;
    if ( ntk.is_pi( n ) )
    {
      prefix = "PI";
      index = n;
    }
    else
    {
      fmt::print( "({}) ", ( ntk.is_and( n ) ? "AND" : "XOR" ) );
    }
    fmt::print( "[i] {} {}: ", prefix, index );
    kitty::print_hex( sim_res[n] );
    fmt::print( "\n" );
  } );

  fmt::print( "[i] POs:\n" );
  ntk.foreach_po( [&]( auto const& f ) {
    std::string prefix = "Node";
    // uint32_t index = f.index - ntk.num_pis();
    uint32_t index = f.index;
    if ( ntk.is_pi( ntk.get_node( f ) ) )
    {
      prefix = "PI";
      index = f.index;
    }
    fmt::print( "[i] {} {}: ", prefix, index );
    if ( ntk.is_complemented( f ) )
    {
      kitty::print_hex( ~sim_res[f] );
    }
    else
    {
      kitty::print_hex( sim_res[f] );
    }
    fmt::print( "; " );
  } );
  fmt::print( "\n" );
}

int main()
{
  for ( auto i = 0u; i < std::size( benchmark_to_visualize ); ++i )
  {
    std::string benchmark = benchmark_to_visualize[i];
    fmt::print( "[i] Processing {}...\n", benchmark );
    mockturtle::xag_network ntk;
    const std::string path = "../experiments/sort/" + benchmark + ".v";
    const auto read_stats = lorina::read_verilog( ( path ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] Cannot access {}... Failed to read benchmark...\n", path );
      abort();
    }
    fmt::print( "[i] Read benchmark successfully.\n" );

    const mockturtle::default_simulator<kitty::static_truth_table<4u>> sim{};
    auto const sim_res = mockturtle::simulate_nodes<kitty::static_truth_table<4u>>( ntk, sim );
    print_sim_res( sim_res, ntk );
    fmt::print( "[i] Simulate benchmark successfully.\n" );

    /* duplicate the partial network that implements each PO */
    // for ( auto j{ 1u }; j < 4u; ++j )
    // {
    //   ntk.clear_values();
    //   ntk.set_value( ntk.get_node( ntk.po_at( j ) ), 1u );
    //   mockturtle::topo_view<mockturtle::xag_network> topo{ ntk };
    //   fmt::print( "[m] (Reverse run) Mark node {}\n", topo.node_to_index( topo.get_node( topo.po_at( 0 ) ) ) );
    //   topo.foreach_node_reverse( [&]( auto const& n ) {
    //     if ( topo.is_constant( n ) || topo.is_pi( n ) )
    //     {
    //       return;
    //     }

    //     if ( topo.value( n ) )
    //     {
    //       topo.foreach_fanin( n, [&]( auto const& fi ) {
    //         topo.set_value( topo.get_node( fi ), 1u );
    //         fmt::print( "[m] (Reverse run) Mark node {}\n", ntk.node_to_index( ntk.get_node( fi ) ) );
    //       } );
    //     }

    //     return;
    //   } );
    //   fmt::print( "\n" );

    //   mockturtle::xag_network dest;
    //   mockturtle::node_map<mockturtle::xag_network::signal, mockturtle::xag_network> old_to_new{ ntk };
    //   old_to_new[ntk.get_constant( false )] = dest.get_constant( false );
    //   ntk.foreach_pi( [&]( auto const& n ) {
    //     old_to_new[n] = dest.create_pi();
    //   } );
    //   ntk.foreach_gate( [&]( auto const& n ) {
    //     if ( ntk.value( n ) )
    //     {
    //       fmt::print( "[m] Encountered marked node {}\n", ntk.node_to_index( n ) );
    //       std::vector<mockturtle::xag_network::signal> children( 2u );
    //       ntk.foreach_fanin( n, [&]( auto const& fi, auto index ) {
    //         assert( index < 2 );
    //         auto child = ntk.is_complemented( fi ) ? dest.create_not( old_to_new[fi] ) : old_to_new[fi];
    //         children[index] = child;
    //       } );
    //       old_to_new[n] = dest.clone_node( ntk, n, children );
    //       fmt::print( "[m] Node {} in dest corresponds to node {} in ntk.\n", dest.node_to_index( dest.get_node( old_to_new[n]) ), ntk.node_to_index( n ) );
    //     }
    //   } );
    //   fmt::print( "[m] Create dest successfully.\n" );
    //   mockturtle::xag_network::signal po = ntk.po_at( j );
    //   mockturtle::xag_network::signal f = old_to_new[po];
    //   dest.create_po( ntk.is_complemented( po ) ? dest.create_not( f ) : f );
    //   mockturtle::write_verilog( dest, fmt::format( "../experiments/sort/{}_{}.v", benchmark, j ) );
    //   fmt::print( "[i] Extract the partial circuit implementing po{} successfully.\n", j );
    // }
  }
}