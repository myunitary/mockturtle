#include <bill/sat/interface/common.hpp>
#include <bill/sat/interface/abc_bsat2.hpp>
#include <fmt/format.h>
#include "../networks/xag.hpp"
#include "../utils/stopwatch.hpp"
#include "../views/cnf_view.hpp"
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace mockturtle
{

enum Solving_mode
{
  Normal, Qubit, Qubit_step, Step
};

struct and_forest_exact_syn_params
{
  uint32_t num_vars{ 3u };
  /* reserve for future extension */
  // std::vector<uint32_t> excluded_monos{};
  uint32_t conflict_limit{ 0u };
  std::optional<std::string> write_dimacs{};
  Solving_mode solving_mode{ Solving_mode::Normal };
  bool verbose{ false };
};

struct and_forest_exact_syn_stats
{
  stopwatch<>::duration time{};
  uint32_t num_vars{};
  uint32_t num_clauses{};
  
  void report()
  {
    fmt::print( "[i] Total time    = {:>5.2f} secs\n", to_seconds( time ) );
    fmt::print( "[i] Total vars    = {}\n", num_vars );
    fmt::print( "[i] Total clauses = {}\n", num_clauses );
  }
};

struct and_forest_heuristic_syn_params
{
  uint32_t num_vars{ 3u };
  uint32_t conflict_limit{ 0u };
  std::optional<std::string> write_dimacs{};
  Solving_mode solving_mode{ Solving_mode::Qubit };
  bool verbose{ false };
};

struct and_forest_heuristic_syn_stats
{
  stopwatch<>::duration time{};
  uint32_t num_vars{};
  uint32_t num_clauses{};
  
  void report()
  {
    fmt::print( "[i] Total time    = {:>5.2f} secs\n", to_seconds( time ) );
    fmt::print( "[i] Total vars    = {}\n", num_vars );
    fmt::print( "[i] Total clauses = {}\n", num_clauses );
  }
};

namespace detail
{

uint32_t factorial( const uint32_t op )
{
  uint32_t res{ 1u };
  for ( uint32_t i{ op }; i >= 2u; --i )
  {
    res *= i;
  }
  return res;
}

uint32_t count_mono( const uint32_t num_vars, const uint32_t num_vars_con )
{
  if ( num_vars == num_vars_con )
  {
    return 1u;
  }

  if ( num_vars_con == 1u || num_vars_con == ( num_vars - 1u ) )
  {
    return num_vars;
  }

  uint32_t res{ factorial( num_vars ) };
  res /= ( factorial( num_vars_con ) * factorial( num_vars - num_vars_con ) );
  return res;
}

template<class Ntk, bill::solvers Solver>
struct and_forest_heuristic_synthesis_impl
{
  using cnf_view_network = cnf_view<Ntk, false, Solver>;

  and_forest_heuristic_synthesis_impl( and_forest_heuristic_syn_params const& ps, and_forest_heuristic_syn_stats& st )
    : _num_vars( ps.num_vars ),
      _num_monos( ( 1 << _num_vars ) - 1u ),
      _ps( ps ),
      _st( st )
  {}

private:
  void init()
  {
    _num_monos_level.clear();
    _num_monos_level.resize( _num_vars );
    for ( uint32_t i{ 0u }; i < _num_vars; ++i )
    {
      _num_monos_level[i] = count_mono( _num_vars, i + 1 );
      // fmt::print( "[m] _num_monos_level[{}] = {}\n", i, _num_monos_level[i] );
    }

    _func_vars_sol.clear();
    _func_vars_sol.resize( _num_vars + _num_monos_level[1] );
    for ( auto& _func_vars_sol_each_node : _func_vars_sol )
    {
      _func_vars_sol_each_node.resize( _num_vars );
    }

    _conn_vars_sol.clear();
    _conn_vars_sol.resize( _num_monos_level[1] );

    /* assign value for function variables related to PIs */
    for ( uint8_t i{ 0u }; i < _num_vars; ++i )
    {
      auto& vars = _func_vars_sol[i];
      for ( uint8_t j{ 0u }; j < vars.size(); ++j )
      {
        vars[j] = ( j == i ) ? true : false;
      }
    }

    uint32_t cnt{ 0u };
    for ( uint8_t v1{ 0u }; v1 < _num_vars - 1u; ++v1 )
    {
      for ( uint8_t v2{ v1 + 1u }; v2 < _num_vars; ++v2, ++cnt )
      {
        auto& f_vars = _func_vars_sol[_num_vars + cnt];
        auto& c_vars = _conn_vars_sol[cnt];
        c_vars.resize( _num_vars );
        for ( uint32_t i{ 0u }; i < c_vars.size(); ++i )
        {
          c_vars[i] = false;
        }
        for ( uint8_t j{ 0u }; j < _num_vars; ++j )
        {
          if ( j == v1 || j == v2 )
          {
            f_vars[j] = true;
            c_vars[j] = true;
          }
        }
      }
    }
    _current_level = 2u;
    _current_monos = _num_vars + _num_monos_level[1];
  }

  void add_two_fan_in_constraint_odd( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& c_vars, uint32_t offset )
  {
    std::vector<signal<cnf_view_network>> products;
    /* one fan-in shall be a PI; the other fan-in is a node in the previous level */
    for ( uint32_t v1{ 0u }; v1 < _num_vars; ++v1 )
    {
      for ( uint32_t v2{ c_vars.size() - offset }; v2 < c_vars.size(); ++v2 )
      {
        std::vector<signal<cnf_view_network>> each_product{};
        for ( uint32_t i{ 0u }; i < c_vars.size(); ++i )
        {
          if ( i == v1 || i == v2 )
          {
            each_product.emplace_back( c_vars[i] );
          }
          else
          {
            each_product.emplace_back( cvntk.create_not( c_vars[i] ) );
          }
        }
        products.emplace_back( cvntk.create_nary_and( each_product ) );
      }
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_two_fan_in_constraint_even( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& c_vars )
  {
    assert( _current_level % 2 == 0 );
    std::vector<signal<cnf_view_network>> products;
    /* both fan-ins shall be nodes in the '_current_level / 2' level */
    uint32_t ind{ 0u };
    for ( uint8_t i{ 0u }; i < ( _current_level / 2 - 1 ); ++i )
    {
      ind += _num_monos_level[i];
    }
    uint32_t offset{ _num_monos_level[_current_level / 2 - 1] };
    for ( uint32_t v1{ ind }; v1 < ind + offset - 1; ++v1 )
    {
      for ( uint32_t v2{ v1 }; v2 < ind + offset; ++v2 )
      {
        std::vector<signal<cnf_view_network>> each_product{};
        for ( uint32_t i{ 0u }; i < c_vars.size(); ++i )
        {
          if ( i == v1 || i == v2 )
          {
            each_product.emplace_back( c_vars[i] );
          }
          else
          {
            each_product.emplace_back( cvntk.create_not( c_vars[i] ) );
          }
        }
        products.emplace_back( cvntk.create_nary_and( each_product ) );
      }
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }


  void add_ensure_reached_constraint( cnf_view_network& cvntk, uint32_t mono, uint32_t offset )
  {
    std::vector<bool> mono_bi( _num_vars );
    for ( uint8_t i{ 0u }; i < _num_vars; ++i, mono >>= 1 )
    {
      mono_bi[i] = mono & 1;
    }

    std::vector<signal<cnf_view_network>> products( _num_monos );
    for ( uint32_t j { _func_vars.size() - offset }; j < _func_vars.size(); ++j )
    {
      std::vector<signal<cnf_view_network>> each_product( _num_vars );
      for ( uint8_t i{ 0u }; i <_num_vars; ++i )
      {
        each_product[i] = mono_bi[i] ? _func_vars[j][i] : cvntk.create_not( _func_vars[j][i] );
      }
      products[j] = cvntk.create_nary_and( each_product );
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_at_most_k_constraint( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& vars, const uint32_t k )
  {
    /* implement the sequential-counter-based at-most-k encoding */
    std::vector<std::vector<signal<cnf_view_network>>> reg_vars( vars.size() - 1 );
    for ( auto& reg_vars_each_element : reg_vars )
    {
      reg_vars_each_element.resize( k );
      /* introduce additional variables */
      /* 'reg_vars[i][j] is true' -> 'j element among vars[0] to vars[i] are true' */
      std::generate( reg_vars_each_element.begin(), reg_vars_each_element.end(), [&]() { return cvntk.create_pi(); } );
    }

    /* Clause 1 - var[i] -> reg_vars[i][0] */
    /* Semantically: since vars[i] is true, the counter records at least one */
    {
      std::vector<signal<cnf_view_network>> sums( vars.size() - 1 );
      for ( uint32_t i{ 0u }; i < sums.size(); ++i )
      {
        sums[i] = cvntk.create_or( cvntk.create_not( vars[i] ), reg_vars[i][0] );
      }
      cvntk.create_po( cvntk.create_nary_and( sums ) );
    }

    /* Clause 2 - !reg_vars[0][1],..., !reg_vars[0][k-1] */
    /* Semantically: reg_vars[0] records at most one */
    {
      std::vector<signal<cnf_view_network>> terms( k - 1 );
      for ( uint32_t j{ 1u }; j < k; ++j )
      {
        terms[j - 1] = cvntk.create_not( reg_vars[0][j] );
      }
      cvntk.create_po( cvntk.create_nary_and( terms ) );
    }

    {
      /* Clause 3 - reg_vars[i][j] -> reg_vars[i+1][j] */
      /* Semantically: If there are j elements among vars[0] to vars[i] that are true, */
      /* there must be at least j elements that are true among vars[0] to vars[i+1] */
      std::vector<signal<cnf_view_network>> products( vars.size() - 2 );
      for ( uint32_t i{ 0u }; i < products.size(); ++i )
      {
        std::vector<signal<cnf_view_network>> sums( k );
        for ( uint32_t j{ 0u }; j < k; ++j )
        {
          sums[j] = cvntk.create_or( cvntk.create_not( reg_vars[i][j] ), reg_vars[i + 1][j] );
        }
        products[i] = cvntk.create_nary_and( sums );
      }
      cvntk.create_po( cvntk.create_nary_and( products ) );
    }

    {
      /* Clause 4 - (vars[i] and reg_vars[i-1][j]) -> reg_vars[i][j+1] */
      /* Semantically: If there are j elements among vars[0] to vars[i-1] that are true */
      /* and vars[i] is true, then there are j+1 elements among vars[0] to [i] that are true */
      std::vector<signal<cnf_view_network>> products( vars.size() - 2 );
      for ( uint32_t i{ 1u }; i < products.size() + 1; ++i )
      {
        std::vector<signal<cnf_view_network>> sums( k - 1 );
        for ( uint32_t j{ 0u }; j < k - 1; ++j )
        {
          std::vector<signal<cnf_view_network>> sums_ops( 3 );
          sums_ops[0] = cvntk.create_not( vars[i] );
          sums_ops[1] = cvntk.create_not( reg_vars[i - 1][j] );
          sums_ops[2] = reg_vars[i][j + 1];
          sums[j] = cvntk.create_nary_or( sums_ops );
        }
        products[i - 1] = cvntk.create_nary_and( sums );
      }
      cvntk.create_po( cvntk.create_nary_and( products ) );
    }

    {
      /* Clause 5 - vars[i] -> !reg_vars[i-1][k] */
      /* Semantically: If vars[i] is true, there cannot be k elements among vars[0] */
      /* to vars[i-1] that are true, otherwise there will be more than k elements */
      /* among vars[0] to vars[i] */
      std::vector<signal<cnf_view_network>> sums( vars.size() - 1 );
      for ( uint32_t i{ 1u }; i < sums.size() + 1; ++i )
      {
        sums[i - 1] = cvntk.create_or( cvntk.create_not( vars[i] ), cvntk.create_not( reg_vars[i - 1][k - 1] ) );
      }
      cvntk.create_po( cvntk.create_nary_and( sums ) );
    }
  }

  void dump_result( cnf_view_network const& cvntk )
  {
    uint32_t _func_vars_sol_size_old = _func_vars_sol.size();
    _func_vars_sol.resize( _func_vars_sol_size_old + _num_monos_level[_current_level - 1] );
    _conn_vars_sol.resize( _conn_vars_sol.size() + _num_monos_level[_current_level - 1] );
    for ( uint32_t i{ _func_vars_sol_size_old }; i < _func_vars_sol.size(); ++i )
    {
      std::vector<bool> _conn_vars_sol_each_node( _current_monos, false );
      std::vector<bool> _func_vars_sol_each_node( _num_vars, false );
      std::vector<signal<cnf_view_network>> _conn_vars_each_node = _conn_vars[i - _func_vars_sol_size_old];
      for ( uint32_t j{ 0u }; j < _conn_vars_sol_each_node.size(); ++j )
      {
        if ( cvntk.model_value( _conn_vars_each_node[j] ) )
        {
          _conn_vars_sol_each_node[j] = true;
          for ( uint8_t k{ 0u }; k < _num_vars; ++k )
          {
            _func_vars_sol_each_node[k] = static_cast<bool>( _func_vars_sol_each_node[k]) | static_cast<bool>( _func_vars_sol[j][k] );
          }
        }
        else
        {
          _conn_vars_sol_each_node[j] = false;
        }
      }
      _func_vars_sol[i] = _func_vars_sol_each_node;
      _conn_vars_sol[i - _num_vars] = _conn_vars_sol_each_node;
    }
  }

  bool next_level_odd( uint32_t num_non_po )
  {
    cnf_view_params cvps;
    cvps.write_dimacs = _ps.write_dimacs;
    cnf_view_network cvntk( cvps );

    /* declare function variables */
    /* if '_current_level' is odd */
    _func_vars.clear();
    _func_vars.resize( _current_monos + _num_monos_level[_current_level - 1] );
    for ( auto& _func_vars_each_node : _func_vars )
    {
      _func_vars_each_node.resize( _num_vars );
    }

    /* assign function variables */
    for ( uint32_t i{ 0u }; i < _current_monos; ++i )
    {
      for ( uint8_t j{ 0u }; j < _num_vars; ++j )
      {
        _func_vars[i][j] = _func_vars_sol[i][j] ? cvntk.get_constant( true ) : cvntk.get_constant( false );
      }
    }

    /* declare connectivity variables */
    _conn_vars.clear();
    _conn_vars.resize( _num_monos_level[_current_level - 1] );
    for ( auto& _conn_vars_each_node : _conn_vars )
    {
      _conn_vars_each_node.resize( _current_monos );
      std::generate( _conn_vars_each_node.begin(), _conn_vars_each_node.end(), [&]() { return cvntk.create_pi(); } );
    }

    /* constraints ensuring the correctness of function variables */
    uint32_t num_mono_last_level{ _num_monos_level[_current_level - 2] };
    for ( uint32_t cnt{ 0u }; cnt < _conn_vars.size(); ++cnt )
    {
      auto& f_vars = _func_vars[ _current_monos + cnt];
      auto& c_vars = _conn_vars[cnt];
      for ( uint32_t i{ 0u }; i < _num_vars; ++i )
      {
        std::vector<signal<cnf_view_network>> products;
        for ( uint32_t j{ 0u }; j < c_vars.size(); ++j )
        {
          products.emplace_back( cvntk.create_and( c_vars[j], _func_vars[j][i] ) );
        }
        f_vars[i] = cvntk.create_nary_or( products );
      }

      /* add constraints to guarantee that exactly two connectivity variables of a node are true */
      add_two_fan_in_constraint_odd( cvntk, c_vars, num_mono_last_level );
      // add_two_fan_in_constraint_even( cvntk, c_vars );
    }

    /* constraints ensuring all product terms containing '_current_level' input variables are implemented */
    for ( uint32_t i{ 1u }; i <= _num_monos; ++i )
    {
      if ( __builtin_popcount( i ) == _current_level )
      {
        add_ensure_reached_constraint( cvntk, i, _num_monos_level[_current_level - 1] );
      }
    }

    /* constraints minimizing nodes in level '_current_level - 1' contributing to nodes in level '_current_level' */
    std::vector<signal<cnf_view_network>> shared_nodes( num_mono_last_level );
    for ( uint32_t i{ _current_monos - num_mono_last_level }; i < _current_monos; ++i )
    {
      std::vector<signal<cnf_view_network>> node_contribution( _conn_vars.size() );
      for ( uint32_t j{ 0u }; j < _conn_vars.size(); ++j )
      {
        node_contribution[j] = _conn_vars[j][i];
      }
      shared_nodes[i + num_mono_last_level - _current_monos] = cvntk.create_nary_or( node_contribution );
    }
    add_at_most_k_constraint( cvntk, shared_nodes, num_non_po );

    /* solve the instance */
    _st.num_vars += cvntk.num_vars();
    _st.num_clauses += cvntk.num_clauses();
    bill::result::clause_type assumptions;
    cvntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( cvntk.lit( f ) );
    } );
    const auto res = cvntk.solve( assumptions, _ps.conflict_limit );
    if ( res && *res )
    {
      fmt::print( "[m] Solved!\n" );
      dump_result( cvntk );
      return true;
    }
    return false;
  }

  bool next_level_even( uint32_t num_non_po )
  {
    cnf_view_params cvps;
    cvps.write_dimacs = _ps.write_dimacs;
    cnf_view_network cvntk( cvps );

    /* declare function variables */
    /* if '_current_level' is odd */
    _func_vars.clear();
    _func_vars.resize( _current_monos + _num_monos_level[_current_level - 1] );
    for ( auto& _func_vars_each_node : _func_vars )
    {
      _func_vars_each_node.resize( _num_vars );
    }

    /* assign function variables */
    for ( uint32_t i{ 0u }; i < _current_monos; ++i )
    {
      for ( uint8_t j{ 0u }; j < _num_vars; ++j )
      {
        _func_vars[i][j] = _func_vars_sol[i][j] ? cvntk.get_constant( true ) : cvntk.get_constant( false );
      }
    }

    /* declare connectivity variables */
    _conn_vars.clear();
    _conn_vars.resize( _num_monos_level[_current_level - 1] );
    for ( auto& _conn_vars_each_node : _conn_vars )
    {
      _conn_vars_each_node.resize( _current_monos );
      std::generate( _conn_vars_each_node.begin(), _conn_vars_each_node.end(), [&]() { return cvntk.create_pi(); } );
    }

    /* constraints ensuring the correctness of function variables */
    uint32_t num_mono_last_level{ _num_monos_level[_current_level - 2] };
    for ( uint32_t cnt{ 0u }; cnt < _conn_vars.size(); ++cnt )
    {
      auto& f_vars = _func_vars[ _current_monos + cnt];
      auto& c_vars = _conn_vars[cnt];
      for ( uint32_t i{ 0u }; i < _num_vars; ++i )
      {
        std::vector<signal<cnf_view_network>> products;
        for ( uint32_t j{ 0u }; j < c_vars.size(); ++j )
        {
          products.emplace_back( cvntk.create_and( c_vars[j], _func_vars[j][i] ) );
        }
        f_vars[i] = cvntk.create_nary_or( products );
      }

      /* add constraints to guarantee that exactly two connectivity variables of a node are true */
      add_two_fan_in_constraint_even( cvntk, c_vars );
    }

    /* constraints ensuring all product terms containing '_current_level' input variables are implemented */
    for ( uint32_t i{ 1u }; i <= _num_monos; ++i )
    {
      if ( __builtin_popcount( i ) == _current_level )
      {
        add_ensure_reached_constraint( cvntk, i, _num_monos_level[_current_level - 1] );
      }
    }

    /* if '_current_level' is even */
    /* constraints minimizing nodes in level '_current_level / 2' contributing to nodes in level '_current_level' */
    /* first, count the number of nodes in level '_current_level / 2' that contributes to nodes in level '_current_level / 2 + 1' */
    uint32_t ind{ 0u };
    for ( uint8_t i{ 0u }; i < ( _current_level / 2 - 1 ); ++i )
    {
      ind += _num_monos_level[i];
    }
    uint32_t offset_int{ _num_monos_level[_current_level / 2 - 1] };
    std::vector<signal<cnf_view_network>> shared_nodes( offset_int );
    uint32_t num_current_not_non_po{ 0u };
    for ( uint32_t i{ ind }; i < ( ind + offset_int ); ++i )
    {
      bool non_po{ false };
      for ( uint32_t j{ ind - _num_vars + offset_int }; j < ( ind - _num_vars + offset_int + _num_monos_level[_current_level / 2] ); ++j )
      {
        if ( non_po )
        {
          break;
        }
        if ( _conn_vars_sol[j][i] )
        {
          non_po = true;
        }
      }
      if ( !non_po )
      {
        std::vector<signal<cnf_view_network>> node_contribution( _conn_vars.size() );
        for ( uint32_t k{ 0u }; k < _conn_vars.size(); ++k )
        {
          node_contribution[k] = _conn_vars[k][i];
        }
        shared_nodes[num_current_not_non_po++] = cvntk.create_nary_or( node_contribution );
      }
    }

    shared_nodes.resize( num_current_not_non_po );
    uint32_t num_allowed_to_be_non_po = num_non_po > ( offset_int - num_current_not_non_po ) ? ( num_non_po - ( offset_int - num_current_not_non_po ) ) : 0u;
    fmt::print( "[m] number of allowed to be non-po nodes: {}\n", num_allowed_to_be_non_po );
    if ( num_allowed_to_be_non_po == 0u )
    {
      for ( auto const& shared_node : shared_nodes )
      {
        cvntk.add_clause( cvntk.create_not( shared_node ) );
      }
    }
    else
    {
      add_at_most_k_constraint( cvntk, shared_nodes, num_allowed_to_be_non_po );
    }

    /* solve the instance */
    _st.num_vars += cvntk.num_vars();
    _st.num_clauses += cvntk.num_clauses();
    bill::result::clause_type assumptions;
    cvntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( cvntk.lit( f ) );
    } );
    const auto res = cvntk.solve( assumptions, _ps.conflict_limit );
    if ( res && *res )
    {
      fmt::print( "[m] Solved!\n" );
      dump_result( cvntk );
      return true;
    }
    return false;
  }

  Ntk extract_ntk()
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( _num_vars );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );
    for ( auto const& _conn_vars_sol_each_node : _conn_vars_sol )
    {
      std::vector<signal<Ntk>> fan_ins;
      for ( uint32_t i{ 0u }; i < _conn_vars_sol_each_node.size(); ++i )
      {
        if ( _conn_vars_sol_each_node[i] )
        {
          fan_ins.emplace_back( nodes[i] );
        }
      }

      assert( fan_ins.size() == 2u );
      signal<Ntk> new_node = ntk.create_nary_and( fan_ins );
      nodes.emplace_back( new_node );
      ntk.create_po( new_node );

      fmt::print( "[m] The fan-ins of Node {0} is Node {1} and Node {2}.\n", static_cast<uint32_t>( ntk.get_node( new_node ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[0] ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[1] ) ) );
    }

    // fmt::print( "[m] Results: \n" );
    // for ( uint8_t i{ 0u }; i < _func_vars_sol.size(); ++i )
    // {
    //   auto& func_vars = _func_vars_sol[i];
    //   for ( uint8_t j{ 0u }; j < func_vars.size(); ++j )
    //   {
    //     fmt::print( "_func_vars[{0}][{1}] = {2}; ", i, j, ( func_vars[j] ? "true" : "false" ) );
    //   }
    //   fmt::print( "\n" );
    // }
    // fmt::print( "\n" );
    // for ( uint8_t i{ 0u }; i < _conn_vars_sol.size(); ++i )
    // {
    //   auto& conn_vars = _conn_vars_sol[i];
    //   for ( uint32_t j{ 0u }; j < conn_vars.size(); ++j )
    //   {
    //     fmt::print( "_conn_vars[{0}][{1}] = {2}; ", i, j, ( conn_vars[j] ? "true" : "false" ) );
    //   }
    //   fmt::print( "\n" );
    // }

    return ntk;
  }

public:
  Ntk run_qubit()
  {
    stopwatch<> t( _st.time );
    init();
    for ( ++_current_level; _current_level <= _num_vars; ++_current_level )
    {
      fmt::print( "[m] Current level is {}\n", _current_level );
      bool proceed{ false };
      uint32_t shared_nodes_lb{ 0u };
      uint32_t shared_nodes_ub{ 0u };
      shared_nodes_lb = std::ceil( static_cast<float>( _num_monos_level[_current_level - 1] ) / _current_level );
      shared_nodes_ub = std::min( _num_monos_level[_current_level - 2], _num_monos_level[_current_level - 1] );
      for ( uint32_t d{ shared_nodes_lb }; d <= shared_nodes_ub; ++d )
      {
        fmt::print( "[m] Allowing {} shared nodes...\n", d );
        proceed = next_level_odd( d );
        if ( proceed )
        {
          _current_monos += _num_monos_level[_current_level - 1];
          break;
        }
      }
      
      if ( !proceed )
      {
        fmt::print( "[e] No solution found.\n" );
        abort();
      }
    }
    fmt::print( "[m] Dumping the network...\n" );
    return extract_ntk();
  }

  Ntk run_step()
  {
    stopwatch<> t( _st.time );
    init();
    for ( ++_current_level; _current_level <= _num_vars; ++_current_level )
    {
      fmt::print( "[m] Current level is {}\n", _current_level );
      bool proceed{ false };
      uint32_t shared_nodes_lb{ 0u };
      uint32_t shared_nodes_ub{ 0u };
      if ( _current_level % 2 != 0 )
      {
        shared_nodes_lb = std::ceil( static_cast<float>( _num_monos_level[_current_level - 1] ) / _current_level );
        shared_nodes_ub = std::min( _num_monos_level[_current_level - 2], _num_monos_level[_current_level - 1] );
        fmt::print( "[m] lower bound is {}\n", shared_nodes_lb );
        fmt::print( "[m] upper bound is {}\n", shared_nodes_ub );
      }
      else
      {
        shared_nodes_lb = std::ceil( std::log( _current_level ) / std::log( 2 ) );
        shared_nodes_ub = _num_monos_level[_current_level / 2 - 1];
        fmt::print( "[m] lower bound is {}\n", shared_nodes_lb );
        fmt::print( "[m] upper bound is {}\n", shared_nodes_ub );
      }
      
      for ( uint32_t d{ shared_nodes_lb }; d <= shared_nodes_ub; ++d )
      {
        fmt::print( "[m] Allowing {} shared nodes...\n", d );
        if ( _current_level % 2 != 0 )
        {
          proceed = next_level_odd( d );
        }
        else
        {
          proceed = next_level_even( d );
        }

        if ( proceed )
        {
          _current_monos += _num_monos_level[_current_level - 1];
          break;
        }
      }
      
      if ( !proceed )
      {
        fmt::print( "[e] No solution found.\n" );
        abort();
      }
    }
    fmt::print( "[m] Dumping the network...\n" );
    return extract_ntk();
  }

private:
  uint8_t _num_vars;
  uint32_t _num_monos;
  uint8_t _current_level;
  uint32_t _current_monos;
  std::vector<uint32_t> _num_monos_level;
  std::vector<std::vector<signal<cnf_view_network>>> _func_vars;
  std::vector<std::vector<signal<cnf_view_network>>> _conn_vars;
  std::vector<std::vector<bool>> _func_vars_sol;
  std::vector<std::vector<bool>> _conn_vars_sol;
  and_forest_heuristic_syn_params const& _ps;
  and_forest_heuristic_syn_stats& _st;
};

template<class Ntk, bill::solvers Solver>
struct and_forest_exact_synthesis_impl
{
  using cnf_view_network = cnf_view<Ntk, false, Solver>;

  and_forest_exact_synthesis_impl( and_forest_exact_syn_params const& ps, and_forest_exact_syn_stats& st )
    : _num_vars( ps.num_vars ),
      _num_monos( ( 1 << _num_vars ) - 1u ),
      // _excluded_monos( ps.excluded_monos ),
      _ps( ps ),
      _st( st )
  {}

private:
  void init( cnf_view_network& cvntk )
  {
    /* declare function variables */
    _func_vars.clear();
    _func_vars.resize( _num_monos );
    for ( auto& _func_vars_each_node : _func_vars )
    {
      _func_vars_each_node.resize( _num_vars );
    }

    /* assign value for function variables related to PIs */
    for ( uint8_t i{ 0u }; i <_num_vars; ++i )
    {
      auto& vars = _func_vars[i];
      for ( uint8_t j{ 0u }; j < vars.size(); ++j )
      {
        vars[j] = ( j == i ) ? cvntk.get_constant( true ) : cvntk.get_constant( false );
      }
    }

    /* declare connectivity variables */
    _conn_vars.clear();
    _conn_vars.resize( _num_monos - _num_vars );
    for ( auto& _conn_vars_each_node : _conn_vars )
    {
      _conn_vars_each_node.resize( _num_monos );
      std::generate( _conn_vars_each_node.begin(), _conn_vars_each_node.end(), [&]() { return cvntk.create_pi(); } );
    }
  }

  void add_two_fan_in_constraint( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& c_vars, uint32_t ind )
  {
    std::vector<signal<cnf_view_network>> products;
    for ( uint32_t v1{ 0u }; v1 < ind - 1u; ++v1 )
    {
      for ( uint32_t v2{ v1 + 1u }; v2 < ind; ++v2 )
      {
        // fmt::print( "[m] " );
        std::vector<signal<cnf_view_network>> each_product{};
        for ( uint32_t i{ 0u }; i < c_vars.size(); ++i )
        {
          if ( i == v1 || i == v2 )
          {
            // fmt::print( "_conn_vars[{0}][{1}] = true; ", ( ind - _num_vars ), i );
            each_product.emplace_back( c_vars[i] );
          }
          else
          {
            // fmt::print( "_conn_vars[{0}][{1}] = false; ", ( ind - _num_vars ), i );
            each_product.emplace_back( cvntk.create_not( c_vars[i] ) );
          }
        }
        products.emplace_back( cvntk.create_nary_and( each_product ) );
        // fmt::print( "\n" );
      }
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_ensure_reached_constraint( cnf_view_network& cvntk, uint32_t mono )
  {
    std::vector<bool> mono_bi( _num_vars );
    for ( uint8_t i{ 0u }; i < _num_vars; ++i, mono >>= 1 )
    {
      mono_bi[i] = mono & 1;
    }

    std::vector<signal<cnf_view_network>> products( _num_monos );
    // fmt::print( "[m] " );
    for ( uint32_t j { 0u }; j < _num_monos; ++j )
    {
      std::vector<signal<cnf_view_network>> each_product( _num_vars );
      for ( uint8_t i{ 0u }; i <_num_vars; ++i )
      {
        each_product[i] = mono_bi[i] ? _func_vars[j][i] : cvntk.create_not( _func_vars[j][i] );
        // if ( mono_bi[i] )
        // {
        //   fmt::print( "_func_vars[{0}][{1}] = true; ", j, i );
        // }
        // else
        // {
        //   fmt::print( "_func_vars[{0}][{1}] = false; ", j, i );
        // }
      }
      products[j] = cvntk.create_nary_and( each_product );
      // fmt::print( "\n" );
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_constraints( cnf_view_network& cvntk )
  {
    /* assign value for function and connectivity variables related to 2-dim monomials */
    uint32_t cnt{ 0u };
    for ( uint8_t v1{ 0u }; v1 < _num_vars - 1u; ++v1 )
    {
      for ( uint8_t v2{ v1 + 1u }; v2 < _num_vars; ++v2, ++cnt )
      {
        auto& f_vars = _func_vars[_num_vars + cnt];
        auto& c_vars = _conn_vars[cnt];
        for ( auto& c_vars_each_conn: c_vars )
        {
          c_vars_each_conn = cvntk.get_constant( false );
        }
        for ( uint8_t j{ 0u }; j < _num_vars; ++j )
        {
          if ( j == v1 || j == v2 )
          {
            // fmt::print( "[m] _func_vars[{0}][{1}] = true\n", _num_vars + cnt, j );
            f_vars[j] = cvntk.get_constant( true );
            // fmt::print( "[m] _conn_vars[{0}][{1}] = true\n", cnt, j );
            c_vars[j] = cvntk.get_constant( true );
          }
        }
      }
    }

    for ( ; ( _num_vars + cnt ) < _num_monos; ++cnt )
    {
      auto& f_vars = _func_vars[_num_vars + cnt];
      auto& c_vars = _conn_vars[cnt];
      for ( uint8_t i{ 0u }; i < _num_vars; ++i )
      {
        std::vector<signal<cnf_view_network>> products;
        for ( uint32_t j{ 0u }; j < _num_monos; ++j )
        {
          // if ( ( ( j + 1 ) >> i ) & 1 )
          // {
          //   products.emplace_back( c_vars[j] );
          // }
          products.emplace_back( cvntk.create_and( c_vars[j], _func_vars[j][i] ) );
        }
        /* determine value for the function variables of remaining nodes */
        f_vars[i] = cvntk.create_nary_or( products );
      }

      /* add constraints to guarantee that exactly two connectivity variables of a node are true */
      add_two_fan_in_constraint( cvntk, c_vars, ( _num_vars + cnt ) );
    }

    for ( uint32_t i{ 1u }; i <= _num_monos; ++i )
    {
      if ( __builtin_popcount( i ) > 2u )
      {
        add_ensure_reached_constraint( cvntk, i );
      }
    }
  }

  Ntk extract_ntk( cnf_view_network const& cvntk )
  // void extract_ntk( cnf_view_network const& cvntk )
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( _num_vars );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );
    for ( auto const& _conn_vars_each_node : _conn_vars )
    {
      std::vector<signal<Ntk>> fan_ins;
      for ( uint32_t i{ 0u }; i < _conn_vars_each_node.size(); ++i )
      {
        if ( cvntk.model_value( _conn_vars_each_node[i] ) )
        {
          fan_ins.emplace_back( nodes[i] );
        }
      }
      assert( fan_ins.size() == 2u );
      signal<Ntk> new_node = ntk.create_nary_and( fan_ins );
      nodes.emplace_back( new_node );
      ntk.create_po( new_node );

      fmt::print( "[m] The fan-ins of Node {0} is Node {1} and Node {2}.\n", static_cast<uint32_t>( ntk.get_node( new_node ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[0] ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[1] ) ) );
    }

    fmt::print( "[m] Results: \n" );
    for ( uint8_t i{ 0u }; i < _func_vars.size(); ++i )
    {
      auto& func_vars = _func_vars[i];
      for ( uint8_t j{ 0u }; j < func_vars.size(); ++j )
      {
        fmt::print( "_func_vars[{0}][{1}] = {2}; ", i, j, ( cvntk.model_value( func_vars[j] ) ? "true" : "false" ) );
      }
      fmt::print( "\n" );
    }
    fmt::print( "\n" );
    for ( uint8_t i{ 0u }; i < _conn_vars.size(); ++i )
    {
      auto& conn_vars = _conn_vars[i];
      for ( uint32_t j{ 0u }; j < conn_vars.size(); ++j )
      {
        fmt::print( "_conn_vars[{0}][{1}] = {2}; ", i, j, ( cvntk.model_value( conn_vars[j] ) ? "true" : "false" ) );
      }
      fmt::print( "\n" );
    }

    return ntk;
  }

  std::optional<Ntk> solve( cnf_view_network& cvntk )
  // void solve( cnf_view_network& cvntk )
  {
    _st.num_vars += cvntk.num_vars();
    _st.num_clauses += cvntk.num_clauses();

    bill::result::clause_type assumptions;
    cvntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( cvntk.lit( f ) );
    } );
    const auto res = cvntk.solve( assumptions, _ps.conflict_limit );
    if ( res && *res )
    {
      return extract_ntk( cvntk );
      // extract_ntk( cvntk );
    }
    else
    {
      return std::nullopt;
    }
  }

  void add_one_fan_in_pebble_constraint( cnf_view_network& cvntk )
  {
    /* at least one fan-in of each node must be a PI */
    for ( auto& c_vars_each_node : _conn_vars )
    {
      std::vector<signal<cnf_view_network>> pis( _num_vars );
      for ( uint8_t i{ 0u }; i < _num_vars; ++i )
      {
        pis[i] = c_vars_each_node[i];
      }
      cvntk.create_po( cvntk.create_nary_or( pis ) );
    }
  }

  void add_at_most_k_constraint( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& vars, const uint32_t k )
  {
    /* implement the sequential-counter-based at-most-k encoding */
    std::vector<std::vector<signal<cnf_view_network>>> reg_vars( vars.size() - 1 );
    for ( auto& reg_vars_each_element : reg_vars )
    {
      reg_vars_each_element.resize( k );
      /* introduce additional variables */
      /* 'reg_vars[i][j] is true' -> 'j element among vars[0] to vars[i] are true' */
      std::generate( reg_vars_each_element.begin(), reg_vars_each_element.end(), [&]() { return cvntk.create_pi(); } );
    }

    /* Clause 1 - var[i] -> reg_vars[i][0] */
    /* Semantically: since vars[i] is true, the counter records at least one */
    {
      std::vector<signal<cnf_view_network>> sums( vars.size() - 1 );
      for ( uint32_t i{ 0u }; i < sums.size(); ++i )
      {
        sums[i] = cvntk.create_or( cvntk.create_not( vars[i] ), reg_vars[i][0] );
      }
      cvntk.create_po( cvntk.create_nary_and( sums ) );
    }

    /* Clause 2 - !reg_vars[0][1],..., !reg_vars[0][k-1] */
    /* Semantically: reg_vars[0] records at most one */
    {
      std::vector<signal<cnf_view_network>> terms( k - 1 );
      for ( uint32_t j{ 1u }; j < k; ++j )
      {
        terms[j - 1] = cvntk.create_not( reg_vars[0][j] );
      }
      cvntk.create_po( cvntk.create_nary_and( terms ) );
    }

    {
      /* Clause 3 - reg_vars[i][j] -> reg_vars[i+1][j] */
      /* Semantically: If there are j elements among vars[0] to vars[i] that are true, */
      /* there must be at least j elements that are true among vars[0] to vars[i+1] */
      std::vector<signal<cnf_view_network>> products( vars.size() - 2 );
      for ( uint32_t i{ 0u }; i < products.size(); ++i )
      {
        std::vector<signal<cnf_view_network>> sums( k );
        for ( uint32_t j{ 0u }; j < k; ++j )
        {
          sums[j] = cvntk.create_or( cvntk.create_not( reg_vars[i][j] ), reg_vars[i + 1][j] );
        }
        products[i] = cvntk.create_nary_and( sums );
      }
      cvntk.create_po( cvntk.create_nary_and( products ) );
    }

    {
      /* Clause 4 - (vars[i] and reg_vars[i-1][j]) -> reg_vars[i][j+1] */
      /* Semantically: If there are j elements among vars[0] to vars[i-1] that are true */
      /* and vars[i] is true, then there are j+1 elements among vars[0] to [i] that are true */
      std::vector<signal<cnf_view_network>> products( vars.size() - 2 );
      for ( uint32_t i{ 1u }; i < products.size() + 1; ++i )
      {
        std::vector<signal<cnf_view_network>> sums( k - 1 );
        for ( uint32_t j{ 0u }; j < k - 1; ++j )
        {
          std::vector<signal<cnf_view_network>> sums_ops( 3 );
          sums_ops[0] = cvntk.create_not( vars[i] );
          sums_ops[1] = cvntk.create_not( reg_vars[i - 1][j] );
          sums_ops[2] = reg_vars[i][j + 1];
          sums[j] = cvntk.create_nary_or( sums_ops );
        }
        products[i - 1] = cvntk.create_nary_and( sums );
      }
      cvntk.create_po( cvntk.create_nary_and( products ) );
    }

    {
      /* Clause 5 - vars[i] -> !reg_vars[i-1][k] */
      /* Semantically: If vars[i] is true, there cannot be k elements among vars[0] */
      /* to vars[i-1] that are true, otherwise there will be more than k elements */
      /* among vars[0] to vars[i] */
      std::vector<signal<cnf_view_network>> sums( vars.size() - 1 );
      for ( uint32_t i{ 1u }; i < sums.size() + 1; ++i )
      {
        sums[i - 1] = cvntk.create_or( cvntk.create_not( vars[i] ), cvntk.create_not( reg_vars[i - 1][k - 1] ) );
      }
      cvntk.create_po( cvntk.create_nary_and( sums ) );
    }
  }

  void add_minimized_shared_nodes_constraint( cnf_view_network& cvntk, const uint32_t num_shared_nodes )
  {
    std::vector<signal<cnf_view_network>> shared_nodes( _num_monos - _num_vars );
    for ( uint32_t i{ _num_vars }; i < _num_monos; ++i )
    {
      std::vector<signal<cnf_view_network>> node_contribution( _num_monos - _num_vars );
      for ( uint32_t j{ 0u }; j < _conn_vars.size(); ++j )
      {
        node_contribution[j] = _conn_vars[j][i];
      }
      shared_nodes[i - _num_vars] = cvntk.create_nary_or( node_contribution );
    }

    /* at most 'num_shared_nodes' elements among the ones in 'shared_nodes' shall be true */
    add_at_most_k_constraint( cvntk, shared_nodes, num_shared_nodes );
  }

public:
  Ntk run()
  // void run()
  {
    stopwatch<> t( _st.time );
    Ntk ntk;
    cnf_view_params cvps;
    cvps.write_dimacs = _ps.write_dimacs;
    cnf_view_network cvntk( cvps );
    init( cvntk );
    add_constraints( cvntk );

    if ( _ps.solving_mode == Solving_mode::Qubit || _ps.solving_mode == Solving_mode::Qubit_step )
    {
      add_one_fan_in_pebble_constraint( cvntk );
    }
    if ( _ps.solving_mode == Solving_mode::Normal || _ps.solving_mode == Solving_mode::Qubit )
    {
      /* accept any circuit implementation */
      auto res = solve( cvntk );
      if ( res )
      {
        return *res;
      }
      else
      {
        fmt::print( "[e] No solution found.\n" );
        abort();
      }
    }
    else
    {
      uint32_t num_shared_nodes{ std::ceil( std::log( _num_vars ) / std::log( 2 ) ) };
      add_minimized_shared_nodes_constraint( cvntk, num_shared_nodes );
      fmt::print( "[m] Targeting exploiting {} shared nodes...\n", num_shared_nodes );
      auto res = solve( cvntk );
      if ( res )
      {
        fmt::print( "[m] Managed to synthesize a network!\n" );
        return *res;
      }
      else
      {
        while ( true )
        {
          ++num_shared_nodes;
          cnf_view_network cvntk_new( cvps );
          init( cvntk_new );
          add_constraints( cvntk_new );
          if ( _ps.solving_mode == Solving_mode::Qubit_step )
          {
            add_one_fan_in_pebble_constraint( cvntk_new );
          }
          add_minimized_shared_nodes_constraint( cvntk_new, num_shared_nodes );
          fmt::print( "[m] Targeting exploiting {} shared nodes...\n", num_shared_nodes );
          res = solve( cvntk_new );
          if ( res )
          {
            fmt::print( "[m] Managed to synthesize a network!\n" );
            return *res;
          }
          if ( num_shared_nodes >= ( _num_monos - _num_vars ) )
          {
            fmt::print( "[e] No solution found.\n" );
            abort();
          }
          /* TODO: come up with an upper bound for 'num_shared_nodes' */
        }
      }
    }
  }
private:
  uint32_t _num_vars;
  uint32_t _num_monos;
  // std::vector<uint32_t> const& _excluded_monos;
  std::vector<std::vector<signal<cnf_view_network>>> _func_vars;
  std::vector<std::vector<signal<cnf_view_network>>> _conn_vars;
  and_forest_exact_syn_params const& _ps;
  and_forest_exact_syn_stats& _st;
};

} /* namespace detail */

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::bsat2>
Ntk and_forest_exact_synthesis( and_forest_exact_syn_params const& ps = {}, and_forest_exact_syn_stats* pst = {} )
// void and_forest_exact_synthesis( and_forest_exact_syn_params const& ps = {}, and_forest_exact_syn_stats* pst = {} )
{
	and_forest_exact_syn_stats st;
  const Ntk ntk = detail::and_forest_exact_synthesis_impl<Ntk, Solver>{ ps, st }.run();
  // detail::and_forest_exact_synthesis_impl<Ntk, Solver>{ ps, st }.run();
  
  if ( ps.verbose )
  {
    st.report();
  }
	if ( pst )
	{
		*pst = st;
	}
  
  return ntk;
}

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::bsat2>
Ntk and_forest_heuristic_synthesis( and_forest_heuristic_syn_params const& ps = {}, and_forest_heuristic_syn_stats* pst = {} )
{
  and_forest_heuristic_syn_stats st;
  const Ntk ntk = ( ps.solving_mode == Solving_mode::Qubit ) ? detail::and_forest_heuristic_synthesis_impl<Ntk, Solver>{ ps, st }.run_qubit() : detail::and_forest_heuristic_synthesis_impl<Ntk, Solver>{ ps, st }.run_step();
  
  if ( ps.verbose )
  {
    st.report();
  }
  if ( pst )
  {
    *pst = st;
  }
  
  return ntk;
}

} /* namespace mockturtle */
