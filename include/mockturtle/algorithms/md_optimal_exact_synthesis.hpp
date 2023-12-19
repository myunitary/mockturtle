#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <bill/sat/interface/glucose.hpp>
#include <kitty/constructors.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cnf_view.hpp>

namespace mockturtle
{

struct md_optimal_exact_synthesis_params
{
	uint32_t conflict_limit{ 500000u };
	//uint32_t conflict_limit{ 0u };
	bool verbose{ false };
	bool verify_solution{ false };
};

struct md_optimal_exact_synthesis_stats
{
	stopwatch<>::duration time_total{};
	stopwatch<>::duration time_solving{};
	uint32_t num_vars{};
	uint32_t num_clauses{};

	void report() const
	{
		fmt::print( "[i] total time    = {:>5.2f} secs\n", to_seconds( time_total ) );
		fmt::print( "[i] solving time  = {:>5.2f} secs\n", to_seconds( time_solving ) );
    fmt::print( "[i] total vars    = {}\n", num_vars );
    fmt::print( "[i] total clauses = {}\n", num_clauses );
	}
};

namespace detail
{

typedef struct AND_ladder
{
	std::vector<uint32_t> and_ladder;
	uint8_t mc;
	uint8_t md;
} AND_ladder;

typedef std::vector<uint8_t> schedule;

static const std::vector<AND_ladder> and_ladders_lib = {
	{ { 1u, 1u, 1u }, 3u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 1u, 2u }, 4u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 2u, 1u }, 4u, 3u }, /* { 2, 0, 0 } */
	{ { 2u, 1u, 1u }, 4u, 3u }, /* { 3, 0, 0 } */
	{ { 1u, 1u, 3u }, 5u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 2u, 2u }, 5u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 3u, 1u }, 5u, 3u }, /* { 2, 0, 0 } */
	{ { 2u, 1u, 2u }, 5u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 2u, 1u }, 5u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 1u, 1u }, 5u, 3u }, /* { 3, 0, 0 } */
	{ { 1u, 1u, 4u }, 6u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 2u, 3u }, 6u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 3u, 2u }, 6u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 4u, 1u }, 6u, 3u }, /* { 2, 0, 0 } */
	{ { 2u, 1u, 3u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 2u, 2u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 3u, 1u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 1u, 2u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 2u, 1u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 4u, 1u, 1u }, 6u, 3u }, /* { 3, 0, 0 } */
	{ { 1u, 1u, 5u }, 7u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 2u, 4u }, 7u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 3u, 3u }, 7u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 4u, 2u }, 7u, 3u }, /* { 2, 0, 0 } */
	{ { 1u, 5u, 1u }, 7u, 3u }, /* { 2, 0, 0 } */
	{ { 2u, 1u, 4u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 2u, 3u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 3u, 2u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 2u, 4u, 1u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 1u, 3u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 2u, 2u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 3u, 3u, 1u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 4u, 1u, 2u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 4u, 2u, 1u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 5u, 1u, 1u }, 7u, 3u }, /* { 3, 0, 0 } */
	{ { 1u, 1u }, 2u, 2u }, /* { 2, 1 } */
	{ { 1u, 2u }, 3u, 2u }, /* { 2, 1 } */
	{ { 2u, 1u }, 3u, 2u }, /* { 3, 0 } */
	{ { 1u, 3u }, 4u, 2u }, /* { 2, 1 } */
	{ { 2u, 2u }, 4u, 2u }, /* { 3, 0 } */
	{ { 3u, 1u }, 4u, 2u }, /* { 3, 0 } */
	{ { 1u, 4u }, 5u, 2u }, /* { 2, 1 } */
	{ { 2u, 3u }, 5u, 2u }, /* { 3, 0 } */
	{ { 3u, 2u }, 5u, 2u }, /* { 3, 0 } */
	{ { 4u, 1u }, 5u, 2u }, /* { 3, 0 } */
	{ { 1u, 5u }, 6u, 2u }, /* { 2, 1 } */
	{ { 2u, 4u }, 6u, 2u }, /* { 3, 0 } */
	{ { 3u, 3u }, 6u, 2u }, /* { 3, 0 } */
	{ { 4u, 2u }, 6u, 2u }, /* { 3, 0 } */
	{ { 5u, 1u }, 6u, 2u }, /* { 3, 0 } */
	{ { 1u, 6u }, 7u, 2u }, /* { 2, 1 } */
	{ { 2u, 5u }, 7u, 2u }, /* { 3, 0 } */
	{ { 3u, 4u }, 7u, 2u }, /* { 3, 0 } */
	{ { 4u, 3u }, 7u, 2u }, /* { 3, 0 } */
	{ { 5u, 2u }, 7u, 2u }, /* { 3, 0 } */
	{ { 6u, 1u }, 7u, 2u }, /* { 3, 0 } */
	{ { 1u, 7u }, 8u, 2u }, /* { 2, 1 } */
	{ { 2u, 6u }, 8u, 2u }, /* { 3, 0 } */
	{ { 3u, 5u }, 8u, 2u }, /* { 3, 0 } */
	{ { 4u, 4u }, 8u, 2u }, /* { 3, 0 } */
	{ { 5u, 3u }, 8u, 2u }, /* { 3, 0 } */
	{ { 6u, 2u }, 8u, 2u }, /* { 3, 0 } */
	{ { 7u, 1u }, 8u, 2u }, /* { 4, 0 } */
	{ { 1u }, 1u, 1u }, /* { 2 } */
	{ { 2u }, 2u, 1u }, /* { 3 } */
	{ { 3u }, 3u, 1u }, /* { 3 } */
	{ { 4u }, 4u, 1u }, /* { 3 } */
	{ { 5u }, 5u, 1u }, /* { 3 } */
	{ { 6u }, 6u, 1u }, /* { 3 } */
	{ { 7u }, 7u, 1u }, /* { 4 } */
	{ { 8u }, 8u, 1u }, /* { 4 } */
	{ { 9u }, 9u, 1u }, /* { 4 } */
	{ { 10u }, 10u, 1u }, /* { 4 } */
	{ { 11u }, 11u, 1u }, /* { 4 } */
	{ { 12u }, 12u, 1u }, /* { 4 } */
	{ { 13u }, 13u, 1u }, /* { 4 } */
	{ { 14u }, 14u, 1u }, /* { 4 } */
	{ { 15u }, 15u, 1u } /* { 4 } */
};

static const std::vector<std::vector<uint8_t>> schedule_constraints = {
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 2u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 3u, 0u, 0u }, 
	{ 2u, 1u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 2u, 1u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 3u, 0u }, 
	{ 4u, 0u }, 
	{ 2u }, 
	{ 3u }, 
	{ 3u }, 
	{ 3u }, 
	{ 3u }, 
	{ 3u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }, 
	{ 4u }
};

template<class Ntk, bill::solvers Solver>
struct md_optimal_exact_synthesis_impl
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;

public:
	md_optimal_exact_synthesis_impl( kitty::dynamic_truth_table const& func, std::vector<uint32_t> const& and_ladder, 
																	 md_optimal_exact_synthesis_params const& ps, md_optimal_exact_synthesis_stats& st )
		: num_vars_( func.num_vars() ), 
			func_( kitty::get_bit( func, 0 ) ? ~func : func ), 
			invert_( kitty::get_bit( func, 0 ) ), 
			and_ladder_( and_ladder ), 
			ps_( ps ), 
			st_( st )
	{
	}

	std::optional<Ntk> run()
	{
		stopwatch<> time_total( st_.time_total );

		Ntk ntk;
		cnf_view_params cvps;
		problem_network_t pntk( cvps );
		reset( pntk );
		encode( pntk );

		if ( const auto sat = solve( pntk ); sat )
		{
			const auto sol = extract_network( pntk );
			return sol;
		}
		return std::nullopt;
	}

private:
	void reset( problem_network_t const& pntk )
	{
		ltfi_vars_.clear();
		truth_vars_.clear();
		truth_vars_.resize( func_.num_bits() );

		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			const auto var_tt = kitty::nth_var<kitty::dynamic_truth_table> ( num_vars_, i );
			for ( auto b{ 0u }; b < func_.num_bits(); ++b )
			{
				truth_vars_[b].emplace_back( pntk.get_constant( kitty::get_bit( var_tt, b ) ) );
			}
		}
	}

	void encode( problem_network_t& pntk )
	{
		/* add essential constraints */
		add_gate( pntk );
		add_output( pntk );
		add_func_constraint( pntk );
		add_struct_constraint( pntk );

		/* add extra constraints to prune searching space */

		st_.num_vars += pntk.num_vars();
		st_.num_clauses += pntk.num_clauses();
	}

	void add_gate( problem_network_t& pntk )
	{
		uint32_t i_ext{ 0u };
		for ( auto ands_each_depth: and_ladder_ )
		{
			for ( auto i_int{ 0u }; i_int < ands_each_depth; ++i_int )
			{
				for ( auto j{ 0u }; j < 2u; ++j )
				{
					ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + i_ext ) );
					/* create single selection variables */
					std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
				}
			}
			i_ext += ands_each_depth;
		}
	}

	void add_output( problem_network_t& pntk )
	{
		ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + ltfi_vars_.size() / 2 ) );
		std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
	}

	void add_func_constraint( problem_network_t& pntk )
	{
		const auto create_xor_clause = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars, uint32_t bit ) -> signal<problem_network_t> {
      std::vector<signal<problem_network_t>> ltfi( ltfi_vars.size() );
      for ( auto j{ 0u }; j < ltfi.size(); ++j )
      {
        ltfi[j] = pntk.create_and( ltfi_vars[j], truth_vars_[bit][j] );
      }
      return pntk.create_nary_xor( ltfi );
    };

    for ( auto b{ 1u }; b < func_.num_bits(); ++b )
    {
    	for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
	    {
	      truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
	    }

	    const auto po_signal = create_xor_clause( ltfi_vars_.back(), b );
      pntk.create_po( kitty::get_bit( func_, b ) ? po_signal : pntk.create_not( po_signal ) );
    }
	}

	void add_struct_constraint( problem_network_t& pntk )
	{
		/* (1) for any AND2 with a certain MD d, at least one of its fanins is an AND2 */
		/* with the MD of d-1                                                          */
		uint32_t range_begin = num_vars_;
		uint32_t range_end = num_vars_ + and_ladder_[0];
		uint32_t i_ext{ and_ladder_[0] };
		for ( auto d{ 1u }; d < and_ladder_.size(); ++d )
		{
			for ( auto i_int{ 0u }; i_int < and_ladder_[d]; ++i_int )
			{
				std::vector<signal<problem_network_t>> ltfi1;
				std::vector<signal<problem_network_t>> ltfi2;
				for ( auto j{ range_begin }; j < range_end; ++j )
				{
					ltfi1.emplace_back( ltfi_vars_[2 * i_ext][j] );
					ltfi2.emplace_back( ltfi_vars_[2 * i_ext + 1][j] );
				}
				pntk.create_or( pntk.create_nary_or( ltfi1 ), pntk.create_nary_or( ltfi2 ) );
				++i_ext;
			}
		}

		/* for any AND2 with a certain MD d, none of the AND2 with the same MD can be  */
		/* its fanin, which is naturally avoided by removing those selection variables */
	}

	bool solve( problem_network_t& pntk )
	{
		stopwatch<> time_solving( st_.time_solving );
    bill::result::clause_type assumptions;

    pntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( pntk.lit( f ) );
    } );

    const auto res = pntk.solve( assumptions, ps_.conflict_limit );

    if ( res && *res )
    {
      return true;
    }

    return false;
	}

	Ntk extract_network( problem_network_t& pntk )
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( num_vars_ );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );

    const auto extract_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) ->signal<Ntk> {
      std::vector<signal<Ntk>> ltfi;
      for ( auto j{ 0u }; j < ltfi_vars.size(); ++j )
      {
        if ( pntk.model_value( ltfi_vars[j] ) )
        {
          ltfi.emplace_back( nodes[j] );
          //std::cout << ( nodes[j].complement ? "!" : "" ) << nodes[j].index << " ";
        }
      }
      //std::cout << "\n";
      return ntk.create_nary_xor( ltfi );
    };

    for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
    {
      nodes.emplace_back( ntk.create_and( extract_ltfi( ltfi_vars_[2 * i] ), extract_ltfi( ltfi_vars_[2 * i + 1] ) ) );
    }

    const auto po = extract_ltfi( ltfi_vars_.back() );
    ntk.create_po( invert_ ? ntk.create_not( po ) : po );

    if ( ps_.verify_solution )
    {
      auto result = simulate<kitty::static_truth_table<5u>>( ntk )[0];
      const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );
      const std::string func_str = kitty::to_hex( invert_ ? ~func_ : func_ );
      if ( result_str != func_str )
      {
        std::cerr << "[e] incorrect implementation for " << func_str << "; "
                  << "whose function is " << result_str << std::endl;
        abort();
      }
      else
      {
        std::cout << "[m] the solution is verified to be correct\n";
      }
    }

    return ntk;
  }

private:
	uint32_t num_vars_;
	kitty::dynamic_truth_table func_;
	std::vector<uint32_t> const& and_ladder_;
	bool invert_{ false };
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	md_optimal_exact_synthesis_params const& ps_;
	md_optimal_exact_synthesis_stats& st_;
};

enum state { stop_with_solution, stop_without_solution, keep_searching };

uint32_t reschedule( schedule& s , uint32_t const& md_old, uint8_t const& ladder_md, 
										 std::vector<uint32_t> const& input_arrive_ordered )
{
	/* notice that 's' is guaranteed to be not full of zeros */
	uint32_t md_new{ 0u };
	bool updated{ false };
	for ( auto i{ 0u }; i < s.size(); ++i )
	{
		if ( s[i] == 0u )
		{
			continue;
		}

		if ( ( input_arrive_ordered[i] + ladder_md - s[i] ) < md_old )
		{
			s[i] -= 1u;
			updated = true;
			md_new = md_old;
			break;
		}
	}
	if ( !updated )
	{
		/* all signals not scheduled to level 0 are bounding MD */
		for ( auto i{ 0u }; i < s.size(); ++i )
		{
			if ( s[i] == 0u )
			{
				continue;
			}
			s[i] -= 1u;
		}
		md_new = md_old + 1u;
	}
	return md_new;
}

template<class Ntk, bill::solvers Solver>
struct md_optimum_with_input_arrival_time_impl
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;

public:
	md_optimum_with_input_arrival_time_impl( kitty::dynamic_truth_table const& func, schedule const& s,
																					 std::vector<uint32_t> const& and_ladder,
																					 md_optimal_exact_synthesis_params const& ps,
																					 md_optimal_exact_synthesis_stats& st )
		: num_vars_( func.num_vars() ), 
			func_( kitty::get_bit( func, 0 ) ? ~func : func ), 
			invert_( kitty::get_bit( func, 0 ) ), 
			schedule_( s ), 
			and_ladder_( and_ladder ), 
			ps_( ps ), 
			st_( st )
	{
	}

	std::optional<Ntk> run()
	{
		stopwatch<> time_total( st_.time_total );

		Ntk ntk;
		cnf_view_params cvps;
		problem_network_t pntk( cvps );
		reset( pntk );
		encode( pntk );

		if ( const auto sat = solve( pntk ); sat )
		{
			const auto sol = extract_network( pntk );
			return sol;
		}
		return std::nullopt;
	}

private:
	void reset( problem_network_t const& pntk )
	{
		ltfi_vars_.clear();
		truth_vars_.clear();
		truth_vars_.resize( func_.num_bits() );

		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			const auto var_tt = kitty::nth_var<kitty::dynamic_truth_table> ( num_vars_, i );
			for ( auto b{ 0u }; b < func_.num_bits(); ++b )
			{
				truth_vars_[b].emplace_back( pntk.get_constant( kitty::get_bit( var_tt, b ) ) );
			}
		}
	}

	void encode( problem_network_t& pntk )
	{
		/* add essential constraints */
		add_gate( pntk );
		add_output( pntk );
		add_func_constraint( pntk );
		add_struct_constraint( pntk );

		/* add extra constraints to prune searching space */

		st_.num_vars += pntk.num_vars();
		st_.num_clauses += pntk.num_clauses();
	}

	void add_gate( problem_network_t& pntk )
	{
		uint32_t i_ext{ 0u };
		for ( auto ands_each_depth: and_ladder_ )
		{
			for ( auto i_int{ 0u }; i_int < ands_each_depth; ++i_int )
			{
				for ( auto j{ 0u }; j < 2u; ++j )
				{
					ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + i_ext ) );
					/* create single selection variables */
					std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
				}
			}
			i_ext += ands_each_depth;
		}
	}

	void add_output( problem_network_t& pntk )
	{
		ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + ltfi_vars_.size() / 2 ) );
		std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
	}

	void add_func_constraint( problem_network_t& pntk )
	{
		const auto create_xor_clause = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars, uint32_t bit ) -> signal<problem_network_t> {
      std::vector<signal<problem_network_t>> ltfi( ltfi_vars.size() );
      for ( auto j{ 0u }; j < ltfi.size(); ++j )
      {
        ltfi[j] = pntk.create_and( ltfi_vars[j], truth_vars_[bit][j] );
      }
      return pntk.create_nary_xor( ltfi );
    };

    for ( auto b{ 1u }; b < func_.num_bits(); ++b )
    {
    	for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
	    {
	      truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
	    }

	    const auto po_signal = create_xor_clause( ltfi_vars_.back(), b );
      pntk.create_po( kitty::get_bit( func_, b ) ? po_signal : pntk.create_not( po_signal ) );
    }
	}

	void add_struct_constraint( problem_network_t& pntk )
	{
		/* (1) for any AND2 with a certain MD d, at least one of its fanins is an AND2     */
		/* with the MD of d-1                                                              */
		uint32_t range_begin = num_vars_;
		uint32_t range_end = num_vars_ + and_ladder_[0];
		uint32_t i_ext{ and_ladder_[0] };
		for ( auto d{ 1u }; d < and_ladder_.size() - 1u; ++d )
		{
			for ( auto i_int{ 0u }; i_int < and_ladder_[d]; ++i_int )
			{
				std::vector<signal<problem_network_t>> ltfi1;
				std::vector<signal<problem_network_t>> ltfi2;
				for ( auto j{ range_begin }; j < range_end; ++j )
				{
					ltfi1.emplace_back( ltfi_vars_[2 * i_ext][j] );
					ltfi2.emplace_back( ltfi_vars_[2 * i_ext + 1][j] );
				}
				pntk.create_or( pntk.create_nary_or( ltfi1 ), pntk.create_nary_or( ltfi2 ) );
				++i_ext;
			}
		}

		/* (2) for any AND2 with a certain MD d, none of the AND2 with the same MD can be  */
		/* its fanin, which is naturally avoided by excluding those selection variables    */
	}

	void add_schedule_constraint( problem_network_t& pntk )
	{
		/* a variable is unvailable until the step it is scheduled to */
		assert( num_vars_ == schedule_.size() );
		for ( auto vi{ 0u }; vi < num_vars_; ++vi )
		{
			uint32_t i_ext{ 0u };
			std::vector<signal<problem_network_t>> ltfi;
			for ( auto d{ 0u }; d < schedule_[vi]; ++d )
			{
				for ( auto i_int{ 0u }; i_int < and_ladder_[d]; ++i_int )
				{
					ltfi.emplace_back( !ltfi_vars_[2 * i_ext][vi] );
					ltfi.emplace_back( !ltfi_vars_[2 * i_ext + 1][vi] );
					++i_ext;
				}
			}
			pntk.create_nary_and( ltfi );
		}
	}

	bool solve( problem_network_t& pntk )
	{
		stopwatch<> time_solving( st_.time_solving );
    bill::result::clause_type assumptions;

    pntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( pntk.lit( f ) );
    } );

    const auto res = pntk.solve( assumptions, ps_.conflict_limit );

    if ( res && *res )
    {
      return true;
    }

    return false;
	}

	Ntk extract_network( problem_network_t& pntk )
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( num_vars_ );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );

    const auto extract_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) ->signal<Ntk> {
      std::vector<signal<Ntk>> ltfi;
      for ( auto j{ 0u }; j < ltfi_vars.size(); ++j )
      {
        if ( pntk.model_value( ltfi_vars[j] ) )
        {
          ltfi.emplace_back( nodes[j] );
          //std::cout << ( nodes[j].complement ? "!" : "" ) << nodes[j].index << " ";
        }
      }
      //std::cout << "\n";
      return ntk.create_nary_xor( ltfi );
    };

    for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
    {
      nodes.emplace_back( ntk.create_and( extract_ltfi( ltfi_vars_[2 * i] ), extract_ltfi( ltfi_vars_[2 * i + 1] ) ) );
    }

    const auto po = extract_ltfi( ltfi_vars_.back() );
    ntk.create_po( invert_ ? ntk.create_not( po ) : po );

    if ( ps_.verify_solution )
    {
      auto result = simulate<kitty::static_truth_table<5u>>( ntk )[0];
      const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );
      const std::string func_str = kitty::to_hex( invert_ ? ~func_ : func_ );
      if ( result_str != func_str )
      {
        std::cerr << "[e] incorrect implementation for " << func_str << "; "
                  << "whose function is " << result_str << std::endl;
        abort();
      }
      else
      {
        std::cout << "[m] the solution is verified to be correct\n";
      }
    }

    return ntk;
  }

private:
	uint32_t num_vars_;
	kitty::dynamic_truth_table func_;
	std::vector<uint32_t> const& and_ladder_;
	schedule const& schedule_;
	bool invert_{ false };
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	md_optimal_exact_synthesis_params const& ps_;
	md_optimal_exact_synthesis_stats& st_;
};

} /* detail */

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::glucose_41>
std::optional<Ntk> md_optimal_exact_synthesis( kitty::dynamic_truth_table const& func,
																							 std::vector<uint32_t> const& and_ladder,
																							 md_optimal_exact_synthesis_params const& ps = {}, 
																							 md_optimal_exact_synthesis_stats* pst = nullptr )
{
	md_optimal_exact_synthesis_stats st;
	const auto pres = detail::md_optimal_exact_synthesis_impl<Ntk, Solver>{ func, and_ladder, ps, st }.run();
	
	if ( ps.verbose )
	{
		st.report();
	}
	if ( pst )
	{
		*pst = st;
	}

	return pres;
}

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::glucose_41>
std::pair<uint32_t, std::optional<xag_network>> md_optimum_with_input_arrival_time( 
	kitty::dynamic_truth_table const& func,
	uint8_t const& fmc, uint32_t const& md_max,
	std::vector<uint32_t> const& input_arrive, 
	md_optimal_exact_synthesis_params const& ps = {},
	md_optimal_exact_synthesis_stats* pst = nullptr )
{
	/* 'input_arrive' is already normalized */
	md_optimal_exact_synthesis_stats st;
	uint32_t md_expected{ 0u };
	std::vector<uint8_t> idx( input_arrive.size() );
	std::iota( idx.begin(), idx.end(), 0u );
	std::vector<uint32_t> input_arrive_ordered( input_arrive );
	std::sort( idx.begin(), idx.end(), [&input_arrive_ordered]( uint8_t idx1, uint8_t idx2 ) { return input_arrive_ordered[idx1] < input_arrive_ordered[idx2]; } );
	std::vector<uint32_t> md_mins;
	//std::vector<uint32_t> md_maxs;
	std::vector<detail::schedule> ss;
	uint32_t md_min{ 0u };
	std::optional<xag_network> pres{ std::nullopt }; 
	for ( auto i{ 0u }; i < detail::and_ladders_lib.size(); ++i )
	{
		if ( fmc > detail::and_ladders_lib[i].mc )
		{
			md_mins.emplace_back( 0u );
			//md_maxs.emplace_back( 0u );
			ss.emplace_back( detail::schedule{} );
			continue;
		}

		detail::schedule s;
		auto schedule_constraint = detail::schedule_constraints[i];
		std::vector<uint32_t> delay( input_arrive_ordered );
		uint8_t inner_idx{ 0u };
		for ( auto each_depth{ 0u }; each_depth < schedule_constraint.size(); ++each_depth )
		{
			if ( schedule_constraint[each_depth] == 0u )
			{
				break;
			}
			for ( auto each_var{ 0u }; each_var < schedule_constraint[each_depth]; ++each_var )
			{
				delay[inner_idx] += ( schedule_constraint.size() - each_depth );
				++inner_idx;
				s.emplace_back( each_depth );
			}
		}

		uint32_t md_expected{ 0u };
		for ( auto const& delay_each: delay )
		{
			md_expected = delay_each > md_expected ? delay_each : md_expected;
		}

		for ( auto si{ 0u }; si < delay.size(); ++si )
		{
			if ( ( si < s.size() && s[si] != 0u ) )
			{
				/* check if the scheduled m. depth can be reduced */
				if ( delay[si] < md_expected )
				{
					s[si] -= std::min( static_cast<uint8_t>( md_expected - delay[si] ), s[si] );
				}
			}
			else
			{
				if ( delay[si] < md_expected )
				{
					/* 'md_expected' is not bounded by current signal */
					/* schedule it with a as-low-as-possible m. depth */
					s.emplace_back( schedule_constraint.size() - ( md_expected - delay[si] ) );
				}
				else
				{
					s.emplace_back( schedule_constraint.size() );
				}
			}
		}

		md_min = ( i == 0u ) ? md_expected : ( ( md_expected < md_min ) ? md_expected : md_min );
		md_mins.emplace_back( md_expected );
		//md_maxs.emplace_back( input_arrive_ordered.back() + and_ladders_lib[i].md );
		ss.emplace_back( s );
	}

	detail::state stop = ( md_min >= md_max ) ? detail::stop_without_solution : detail::keep_searching;

	while ( stop == detail::keep_searching )
	{
		bool no_ladder_of_interest{ true };

		for ( auto i{ 0u }; i < detail::and_ladders_lib.size(); ++i )
		{
			if ( md_mins[i] == 0u || md_mins[i] > md_min )
			{
				continue;
			}
			/* find a target ladder */
			no_ladder_of_interest = false;
			bool same_md{ true };
			while ( same_md )
			{
				/* SAT solving targeting this ladder and current scheduling */
				detail::schedule s( ss[i] );
				for ( auto si{ 0u }; si < ss[i].size(); ++si )
				{
					s[idx[si]] = ss[i][si];
				}
				/* TODO: inconsistency in returned datatypes: 'pxag' v.s. 'optional<xag_network>' */
				detail::md_optimum_with_input_arrival_time_impl<Ntk, Solver>{ func, s, detail::and_ladders_lib[i].and_ladder, ps, st }.run();

				if ( pres )
				{
					/* if SAT */
					stop = detail::stop_with_solution;
					break;
				}

				/* if UNSAT */
				bool all_zero{ true };
				for ( auto s_each: ss[i] )
				{
					if ( s_each != 0u )
					{
						all_zero = false;
						break;
					}
				}
				if ( all_zero )
				{
					/* rule out this ladder */
					md_mins[i] = 0u;
					same_md = false;
				}
				else
				{
					/* update scheduling solution and expected MD of this ladder */
					uint32_t md_new = detail::reschedule( ss[i], md_mins[i], detail::and_ladders_lib[i].md, input_arrive_ordered );
					if ( md_new > md_mins[i] )
					{
						md_mins[i] = md_new;
						same_md = false;
					}
				}
			}
			break;
		}

		if ( no_ladder_of_interest )
		{
			++md_min;
			if ( md_min >= md_max )
			{
				stop = detail::stop_without_solution;
			}
		}
	}
		
	if ( ps.verbose )
	{
		st.report();
	}
	if ( pst )
	{
		*pst = st;
	}

	return pres ? std::make_pair( md_min, pres ) : std::make_pair( 0u, std::nullopt );
}

} /* mockturtle */