#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <optional>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <kitty/constructors.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cnf_view.hpp>
#include <mockturtle/views/cut_view.hpp>

namespace mockturtle
{

struct fhe_optimization_exact_syn_params
{
	uint32_t conflict_limit{ 0u };
	bool use_advanced_constraints{ false };
	bool verbose{ false };
	bool verify_solution{ false };
};

struct fhe_optimization_exact_syn_stats
{
	stopwatch<>::duration time_solving{};
	// uint32_t num_vars{};
	// uint32_t num_clauses{};
	uint32_t num_instances{};

	void report() const
	{
		fmt::print( "[i] solving time  = {:>5.2f} secs\n", to_seconds( time_solving ) );
		// fmt::print( "[i] total vars    = {}\n", num_vars );
		// fmt::print( "[i] total clauses = {}\n", num_clauses );
		fmt::print( "[i] num instances = {}\n", num_instances );
	}
};

namespace detail
{

// A schedule indicates the depths, starting from which a variable is available
typedef std::vector<uint8_t> schedule;

typedef struct AND_fence
{
	std::vector<uint8_t> and_fence;
	uint8_t mc;
	uint8_t md;
	std::vector<uint8_t> baseline_scheduling_req;
} AND_fence;

static const std::vector<AND_fence> and_fences_lib = {
	{ { 1u }, 1u, 1u, { 2 } },
	{ { 2u }, 2u, 1u, { 3 } },
	{ { 3u }, 3u, 1u, { 3 } },
	{ { 4u }, 4u, 1u, { 3 } },
	{ { 5u }, 5u, 1u, { 3 } },
	{ { 6u }, 6u, 1u, { 3 } },
	{ { 7u }, 7u, 1u, { 3 } },
	{ { 8u }, 8u, 1u, { 3 } },
	{ { 9u }, 9u, 1u, { 3 } },
	{ { 10u }, 10u, 1u, { 4 } },
	{ { 11u }, 11u, 1u, { 4 } },
	{ { 12u }, 12u, 1u, { 4 } },
	{ { 13u }, 13u, 1u, { 4 } },
	{ { 14u }, 14u, 1u, { 4 } },
	{ { 15u }, 15u, 1u, { 4 } },
	{ { 1u, 1u }, 2u, 2u, { 2, 0 } },
	{ { 1u, 2u }, 3u, 2u, { 2, 0 } },
	{ { 2u, 1u }, 3u, 2u, { 3, 0 } },
	{ { 1u, 3u }, 4u, 2u, { 2, 0 } },
	{ { 2u, 2u }, 4u, 2u, { 3, 0 } },
	{ { 3u, 1u }, 4u, 2u, { 3, 0 } },
	{ { 1u, 4u }, 5u, 2u, { 2, 0 } },
	{ { 2u, 3u }, 5u, 2u, { 3, 0 } },
	{ { 3u, 2u }, 5u, 2u, { 3, 0 } },
	{ { 4u, 1u }, 5u, 2u, { 3, 0 } },
	{ { 1u, 5u }, 6u, 2u, { 2, 0 } },
	{ { 2u, 4u }, 6u, 2u, { 3, 0 } },
	{ { 3u, 3u }, 6u, 2u, { 3, 0 } },
	{ { 4u, 2u }, 6u, 2u, { 3, 0 } },
	{ { 5u, 1u }, 6u, 2u, { 3, 0 } },
	{ { 1u, 6u }, 7u, 2u, { 2, 0 } },
	{ { 2u, 5u }, 7u, 2u, { 3, 0 } },
	{ { 3u, 4u }, 7u, 2u, { 3, 0 } },
	{ { 4u, 3u }, 7u, 2u, { 3, 0 } },
	{ { 5u, 2u }, 7u, 2u, { 3, 0 } },
	{ { 6u, 1u }, 7u, 2u, { 3, 0 } },
	{ { 1u, 7u }, 8u, 2u, { 2, 0 } },
	{ { 2u, 6u }, 8u, 2u, { 3, 0 } },
	{ { 3u, 5u }, 8u, 2u, { 3, 0 } },
	{ { 4u, 4u }, 8u, 2u, { 3, 0 } },
	{ { 5u, 3u }, 8u, 2u, { 3, 0 } },
	{ { 6u, 2u }, 8u, 2u, { 3, 0 } },
	{ { 7u, 1u }, 8u, 2u, { 3, 0 } },
	{ { 1u, 1u, 1u }, 3u, 3u, { 2, 0, 0 } },
	{ { 1u, 1u, 2u }, 4u, 3u, { 2, 0, 0 } },
	{ { 1u, 2u, 1u }, 4u, 3u, { 2, 0, 0 } },
	{ { 2u, 1u, 1u }, 4u, 3u, { 3, 0, 0 } },
	{ { 1u, 1u, 3u }, 5u, 3u, { 2, 0, 0 } },
	{ { 1u, 2u, 2u }, 5u, 3u, { 2, 0, 0 } },
	{ { 1u, 3u, 1u }, 5u, 3u, { 2, 0, 0 } },
	{ { 2u, 1u, 2u }, 5u, 3u, { 3, 0, 0 } },
	{ { 2u, 2u, 1u }, 5u, 3u, { 3, 0, 0 } },
	{ { 3u, 1u, 1u }, 5u, 3u, { 3, 0, 0 } },
	{ { 1u, 1u, 4u }, 6u, 3u, { 2, 0, 0 } },
	{ { 1u, 2u, 3u }, 6u, 3u, { 2, 0, 0 } },
	{ { 1u, 3u, 2u }, 6u, 3u, { 2, 0, 0 } },
	{ { 1u, 4u, 1u }, 6u, 3u, { 2, 0, 0 } },
	{ { 2u, 1u, 3u }, 6u, 3u, { 3, 0, 0 } },
	{ { 2u, 2u, 2u }, 6u, 3u, { 3, 0, 0 } },
	{ { 2u, 3u, 1u }, 6u, 3u, { 3, 0, 0 } },
	{ { 3u, 1u, 2u }, 6u, 3u, { 3, 0, 0 } },
	{ { 3u, 2u, 1u }, 6u, 3u, { 3, 0, 0 } },
	{ { 4u, 1u, 1u }, 6u, 3u, { 3, 0, 0 } },
	{ { 1u, 1u, 5u }, 7u, 3u, { 2, 0, 0 } },
	{ { 1u, 2u, 4u }, 7u, 3u, { 2, 0, 0 } },
	{ { 1u, 3u, 3u }, 7u, 3u, { 2, 0, 0 } },
	{ { 1u, 4u, 2u }, 7u, 3u, { 2, 0, 0 } },
	{ { 1u, 5u, 1u }, 7u, 3u, { 2, 0, 0 } },
	{ { 2u, 1u, 4u }, 7u, 3u, { 3, 0, 0 } },
	{ { 2u, 2u, 3u }, 7u, 3u, { 3, 0, 0 } },
	{ { 2u, 3u, 2u }, 7u, 3u, { 3, 0, 0 } },
	{ { 2u, 4u, 1u }, 7u, 3u, { 3, 0, 0 } },
	{ { 3u, 1u, 3u }, 7u, 3u, { 3, 0, 0 } },
	{ { 3u, 2u, 2u }, 7u, 3u, { 3, 0, 0 } },
	{ { 3u, 3u, 1u }, 7u, 3u, { 3, 0, 0 } },
	{ { 4u, 1u, 2u }, 7u, 3u, { 3, 0, 0 } },
	{ { 4u, 2u, 1u }, 7u, 3u, { 3, 0, 0 } },
	{ { 5u, 1u, 1u }, 7u, 3u, { 3, 0, 0 } }
};

static const uint8_t num_and_fences = 78u;

std::vector<uint32_t> process_input_arrival( std::vector<uint32_t> const& input_arrival_times, std::vector<uint8_t>& idx, uint32_t& diff )
{
	diff = *std::min_element( input_arrival_times.begin(), input_arrival_times.end() );
	std::vector<uint32_t> input_arrival_ordered( input_arrival_times );
	std::for_each( input_arrival_ordered.begin(), input_arrival_ordered.end(), [diff]( uint32_t& n ) { n -= diff; } );
	std::sort( idx.begin(), idx.end(), [&input_arrival_ordered]( uint8_t idx1, uint8_t idx2 ) { return input_arrival_ordered[idx1] < input_arrival_ordered[idx2]; } );
	for ( auto i{ 0u }; i < idx.size(); ++i )
	{
		input_arrival_ordered[i] = input_arrival_times[idx[i]];
	}

	return input_arrival_ordered;
}

bool reschedule( uint32_t depth_target, std::vector<uint32_t>& depth, schedule& scheduling )
{
	// while keeping the target depth achievable, scheduling each variable to an as-low-as-possible level
	for ( auto i{ 0u }; i < depth.size(); ++i )
	{
		if ( depth[i] < depth_target )
		{
			if ( scheduling[i] == 0u )
			{
				// impossible to achieve the target depth using the current AND fence
				return false;
			}

			uint32_t depth_diff = depth_target - depth[i];
			if ( scheduling[i] < depth_diff )
			{
				depth[i] += scheduling[i];
				scheduling[i] = 0u;
			}
			else
			{
				depth[i] += depth_diff;
				scheduling[i] -= depth_diff;
			}
		}
	}

	return true;
}

template<bill::solvers Solver>
struct md_optimization_exact_syn
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;


public:
	md_optimization_exact_syn( kitty::dynamic_truth_table const& func, uint32_t const& g_m_depth,
	                           uint32_t& c_m_depth, std::vector<uint32_t> const& input_arrival_times, 
	                           fhe_optimization_exact_syn_params const& ps,
	                           fhe_optimization_exact_syn_stats* pst )
		: num_vars_( func.num_vars() ), func_( kitty::get_bit( func, 0 ) ? ~func : func ),
		  invert_( kitty::get_bit( func, 0 ) ), input_arrival_times_( input_arrival_times ),
		  md_( c_m_depth ), g_m_depth_( g_m_depth ), ps_( ps ), pst_( pst )
	{
		const auto tt = ( num_vars_ < 5u ) ? kitty::extend_to( func_, 5u ) : func_;
		mc_ = kitty::get_spectral_mc( tt );
	}

	std::optional<xag_network> exact_syn_flow()
	{
		/* TO CONFIRM: is the case of 0-mc functions ruled out already? */
		if ( mc_ == 0u )
		{
			return std::nullopt;
		}

		// aligning and ordering input arrival times
		std::vector<uint8_t> idx( input_arrival_times_.size() );  // the indices of the ordered variables in 'input_arrival_times_'
		std::iota( idx.begin(), idx.end(), 0u );
		uint32_t diff{ 0u };
		std::vector<uint32_t> input_arrival_ordered = process_input_arrival( input_arrival_times_, idx, diff );
		g_m_depth_ -= diff;


		/* for debugging */
		fmt::print( "[m] start exact synthesis\n" );
		fmt::print( "[m] input arrival times are : " );
		for ( auto pin_delay : input_arrival_times_ )
		{
			fmt::print( "{}, ", pin_delay );
		}
		fmt::print( "\n" );
		fmt::print( "[m] aligned input arrival times are : " );
		for ( auto pin_delay : input_arrival_ordered )
		{
			fmt::print( "{}, ", pin_delay );
		}
		fmt::print( "\n" );
		fmt::print( "[m] indices are : " );
		for ( auto index : idx )
		{
			fmt::print( "{}, ", index );
		}
		fmt::print( "\n" );


		// initializing 'md_expected', 'schedules' and 'depths'
		std::array<uint32_t, num_and_fences> md_expected;
		std::array<schedule, num_and_fences> schedules;
		std::array<std::vector<uint32_t>, num_and_fences> depths;
		for ( auto i{ 0u }; i < num_and_fences; ++i )
		{
			if ( and_fences_lib[i].mc < mc_ )
			{
				md_expected[i] = 0u;
				schedules[i] = schedule{};
				depths[i].resize( 0 );
				continue;
			}

			auto const& schedule_req = and_fences_lib[i].baseline_scheduling_req;
			schedule s( input_arrival_ordered.size(), schedule_req.size() );
			std::vector<uint32_t> depth( input_arrival_ordered );
			uint8_t to_be_scheduled{ 0u };  // starting from scheduling variables with low input arrival time
			for ( auto d{ 0u }; d < schedule_req.size(); ++d )
			{
				if ( schedule_req[d] == 0u )
				{
					break;
				}
				for ( auto need_schedule{ 0u }; need_schedule < schedule_req[d]; ++need_schedule )
				{
					depth[to_be_scheduled] += ( schedule_req.size() - d );
					s[to_be_scheduled++] = d;
				}
			}

			md_expected[i] = *std::max_element( depth.begin(), depth.end() );
			if ( md_expected[i] < md_ )
			{
				md_ = md_expected[i];
			}
			bool reschedule_able = reschedule( md_expected[i], depth, s );
			// assert( reschedule_able );
			schedules[i] = s;
			depths[i] = depth;
		}

		if ( md_ >= g_m_depth_ )
		{


			/* for debugging */
			fmt::print( "[m] impossible to realize a smaller depth, skipping...\n", md_, g_m_depth_ );


			return std::nullopt;
		}


		/* for debugging */
		fmt::print( "[m] potential minimal depth is {} (v.s. {})\n", md_, g_m_depth_ );


		while ( true )
		{
			bool matched{ false };
			for ( auto i{ 0u }; i < num_and_fences; ++i )
			{
				if ( md_expected[i] == md_ )
				{
					matched = true;
					++pst_->num_instances;

					schedule s( schedules[i].size() );
					for ( auto j{ 0u }; j < s.size(); ++j )
					{
						s[idx[j]] = schedules[i][j];
					}


					/* for debugging */
					//fmt::print( "[m] trying to solve such an eaxt synthesis problem : " );



					auto p_sol = exact_syn_instance( and_fences_lib[i].and_fence, s );
					if ( p_sol )
					{
						md_ += diff;
						return p_sol;
					}
					
					bool reschedule_able = reschedule( ++md_expected[i], depths[i], schedules[i] );
					if ( !reschedule_able )
					{
						// abandoning this AND fence
						md_expected[i] = 0u;
						depths[i].resize( 0 );
						schedules[i].resize( 0 );
					}
				}
			}

			if ( !matched )
			{
				/* TODO: should not happen, add an assertion */
				return std::nullopt;
			}

			// relaxing the target depth
			if ( ++md_ >= g_m_depth_ )
			{
				return std::nullopt;
			}
		}
	}

private:
	/* initializing the instance */
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

	/* adding the variables */
	void add_gate( problem_network_t& pntk, std::vector<uint8_t> const& and_fence )
	{
		uint8_t i_ext{ 0u };
		for ( uint8_t ands_each_depth : and_fence )
		{
			for ( uint8_t i_int{ 0u }; i_int < ands_each_depth; ++i_int )
			{
				for ( auto j{ 0u }; j < 2u; ++j )
				{
					ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + i_ext ) );
					std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
				}
			}
			i_ext += ands_each_depth;
		}
	}

	void add_po( problem_network_t& pntk )
	{
		ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + ltfi_vars_.size() / 2 ) );
		std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
	}

	/* defining principle constraints */
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
			for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
			{
				truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
			}
			const auto po = create_xor_clause( ltfi_vars_.back(), b );
			pntk.create_po( kitty::get_bit( func_, b ) ? po : pntk.create_not( po ) );
		}
	}

	void add_struct_constraint( problem_network_t& pntk, std::vector<uint8_t> const& and_fence )
	{
		// (1) for any AND with a certain MD d, at least one of its fanins is an AND with the MD of d - 1
		uint32_t range_begin{ num_vars_ };
		uint32_t range_end{ num_vars_ + and_fence[0] };
		uint32_t i_ext{ and_fence[0] };
		for ( auto d{ 1u }; d < and_fence.size(); ++d )
		{
			for ( auto i_int{ 0u }; i_int < and_fence[d]; ++i_int )
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
			range_begin = range_end;
			range_end += and_fence[d];
		}
		// (2) for any AND with a certain MD d, none of the AND with the same MD can be its fanin;
		// This is already guaranteed by removing the corresponding selection variables
	}

	void add_schedule_constraint( problem_network_t& pntk, std::vector<uint8_t> const& and_fence, schedule const& s )
	{
		// ensuring a variable is unavailable until the depth it is scheduled to
		for ( auto j{ 0u }; j < num_vars_; ++j )
		{
			uint32_t i_ext{ 0u };
			for ( auto d{ 0u }; d < s[j]; ++d )
			{
				for ( auto i_int{ 0u }; i_int < and_fence[d]; ++i_int )
				{
					pntk.add_clause( !ltfi_vars_[2 * i_ext][j] );
					pntk.add_clause( !ltfi_vars_[2 * i_ext + 1][j] );
					++i_ext;
				}
			}
		}
	}

	// defining advanced constraints
	void add_no_useless_signal_constraint( problem_network_t& pntk )
	{
		for ( auto j{ 0u }; j < num_vars_ + ltfi_vars_.size() / 2; ++j )
		{
			if ( ( j < num_vars_ ) && !kitty::has_var( func_, j ) )
			{
				continue;
			}

			std::vector<signal<problem_network_t>> clause;
			for ( auto const& ltfi : ltfi_vars_ )
			{
				if ( j < ltfi.size() )
				{
					clause.emplace_back( ltfi[j] );
				}
			}

			pntk.create_nary_or( clause );
		}
	}

	void add_no_empty_ltfi_constraint( problem_network_t& pntk )
	{
		for ( auto const& ltfi : ltfi_vars_ )
		{
			pntk.create_nary_or( ltfi );
		}
	}

	void add_no_subset_ltfi_constraint( problem_network_t& pntk )
	{
		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			auto const& ltfi1 = ltfi_vars_[2 * i];
			auto const& ltfi2 = ltfi_vars_[2 * i + 1];
			std::vector<signal<problem_network_t>> interm1( ltfi1.size() );
			std::vector<signal<problem_network_t>> interm2( ltfi1.size() );

			for ( auto j{ 0u }; j < ltfi1.size(); ++j )
			{
				interm1[j] = pntk.create_and( ltfi1[j], pntk.create_not( ltfi2[j] ) );
				interm2[j] = pntk.create_and( pntk.create_not( ltfi1[j] ), ltfi2[j] );
			}
			pntk.create_nary_or( interm1 );
			pntk.create_nary_or( interm2 );
		}
	}

	void add_symmetry_breaking_constraint( problem_network_t& pntk )
	{
		// forcing the left LTFI always lexicographically smaller than the right one
		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			auto const& ltfi1 = ltfi_vars_[2 * i];
			auto const& ltfi2 = ltfi_vars_[2 * i + 1];
			std::vector<signal<problem_network_t>> helper( ltfi1.size() - 1u );
			std::generate( helper.begin(), helper.end(), [&]() { return pntk.create_pi(); } );

			// letting the LSB of an LTFI be the MSB in lexicographical ordering
			pntk.create_or( !ltfi1[0], ltfi2[0] );
			pntk.add_clause(  ltfi1[0],  ltfi2[0],  helper[0] );
			pntk.add_clause( !ltfi1[0], !ltfi2[0],  helper[0] );
			pntk.add_clause(  ltfi1[0], !ltfi2[0], !helper[0] );

			// introducing induction to less significant variables in the LTFIs
			for ( auto j{ 1u }; j < helper.size() - 1; ++j )
			{
				pntk.create_or( helper[j - 1], !helper[j] );
				pntk.add_clause( !ltfi1[j],  ltfi2[j], !helper[j - 1] );
				pntk.add_clause( !ltfi1[j], !helper[j - 1], helper[j] );
				pntk.add_clause(  ltfi2[j], !helper[j - 1], helper[j] );
				pntk.add_clause(  ltfi1[j], !ltfi2[j], !helper[j] );
			}

			pntk.create_or( !ltfi1.back(), !helper.back() );
			pntk.create_or(  ltfi2.back(), !helper.back() );
		}
	}

	// putting all stuffs together
	void encode( problem_network_t& pntk, std::vector<uint8_t> const& and_fence, schedule const& s )
	{
		// adding principle constraints
		add_gate( pntk, and_fence );
		add_po( pntk );
		add_func_constraint( pntk );
		add_struct_constraint( pntk, and_fence );
		add_schedule_constraint( pntk, and_fence, s );

		if ( ps_.use_advanced_constraints )
		{
			// adding advanced constraints to prune searching space
			add_no_useless_signal_constraint( pntk );
			add_no_empty_ltfi_constraint( pntk );
			add_no_subset_ltfi_constraint( pntk );
			add_symmetry_breaking_constraint( pntk );
		}
	}

	bool solve( problem_network_t& pntk )
	{
		stopwatch t_solving( pst_->time_solving );
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

	xag_network decompose_to_xag( problem_network_t& pntk )
	{
		xag_network xag;
		std::vector<signal<xag_network>> nodes( num_vars_ );
		std::generate( nodes.begin(), nodes.end(), [&]() { return xag.create_pi(); } );

		const auto decompose_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) ->signal<xag_network> {
			std::vector<signal<xag_network>> ltfi;
			for ( auto j{ 0u }; j < ltfi_vars.size(); ++j )
			{
				if ( pntk.model_value( ltfi_vars[j] ) )
				{
					ltfi.emplace_back( nodes[j] );
				}
			}
			return xag.create_nary_xor( ltfi );
		};

		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			nodes.emplace_back( xag.create_and( decompose_ltfi( ltfi_vars_[2 * i] ), decompose_ltfi( ltfi_vars_[2 * i + 1] ) ) );
		}

		const auto po = decompose_ltfi( ltfi_vars_.back() );
		xag.create_po( invert_ ? xag.create_not( po ) : po );

		// verifying the Boolean function implemented by the constructed XAG
		if ( ps_.verify_solution )
		{
			auto result = simulate<kitty::static_truth_table<5u>>( xag )[0];
			const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );
			const std::string func_str = kitty::to_hex( invert_ ? ~func_ : func_ );
			if ( result_str != func_str )
			{
				fmt::print( "[e] incorrect XAG implementation for {}, whose implemented function is {}...\n", func_str, result_str );
				abort();
			}
			else
			{
				fmt::print( "[m] the XAG implementation is verified to be correct.\n" );
			}
		}

		return xag;
	}

	std::optional<xag_network> exact_syn_instance( std::vector<uint8_t> const& and_fence, schedule const& s )
	{
		xag_network xag;
		cnf_view_params cvps;
		problem_network_t pntk( cvps );

		reset( pntk );
		encode( pntk, and_fence, s );
		if( const auto sat = solve( pntk ); sat )
		{


			/* for debugging */
			fmt::print( "[m] found an exact implementation\n" );


			const auto solution = decompose_to_xag( pntk );


			/* for debugging */
			fmt::print( "[m] decomposed the abstract XAG into an XAG\n" );


			return solution;
		}

		return std::nullopt;
	}

private:
	uint8_t num_vars_;
	kitty::dynamic_truth_table func_;
	uint8_t mc_;
	uint32_t& md_;
	uint32_t g_m_depth_;
	const std::vector<uint32_t> input_arrival_times_;
	const bool invert_;
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	const fhe_optimization_exact_syn_params ps_;
	fhe_optimization_exact_syn_stats* pst_;
};

} /* detail */

template<bill::solvers Solver>
class fhe_optimization_exact_syn
{
public:
	fhe_optimization_exact_syn( fhe_optimization_exact_syn_params const& ps = {}, fhe_optimization_exact_syn_stats* pst = nullptr )
		: ps_( ps ), pst_( pst ) {}

	~fhe_optimization_exact_syn()
	{
		if ( ps_.verbose )
		{
			pst_->report();
		}
	}

	template<typename Fn>
	uint32_t run( xag_network& res, kitty::dynamic_truth_table const& func, std::vector<arrival_time_pair> const& inputs, uint32_t g_m_depth, Fn&& on_signal ) const
	{
		const auto num_vars = func.num_vars();
		assert( num_vars == inputs.size() );
		uint32_t c_m_depth{ g_m_depth };
		std::vector<xag_network::signal> pis( num_vars, res.get_constant( false ) );
		std::vector<uint32_t> delays( num_vars, 0u );
		for ( auto i{ 0u }; i < num_vars; ++i )
		{
			pis[i] = inputs[i].f;
			delays[i] = inputs[i].m_depth;
		}

		auto p_sol = detail::md_optimization_exact_syn<Solver>( func, g_m_depth, c_m_depth, delays, ps_, pst_ ).exact_syn_flow();
		if ( !p_sol )
		{
			return g_m_depth;
		}

		std::vector<xag_network::signal> sol_pis;
		p_sol->foreach_pi( [&sol_pis]( auto const& pi ) { sol_pis.emplace_back( pi ); } );
		assert( sol_pis.size() == pis.size() );
		xag_network::signal sol_po;
		p_sol->foreach_po( [&sol_po]( auto const& po ) { sol_po = po; } );
		
		xag_network::signal po;
		if ( p_sol->is_constant( p_sol->get_node( sol_po ) ) )
		{
			po = res.get_constant( p_sol->is_complemented( sol_po ) );
		}
		else
		{
			cut_view<xag_network> sol{ *p_sol, sol_pis, sol_po };
			po = cleanup_dangling( sol, res, pis.begin(), pis.end() ).front();
		}
		on_signal( { po, c_m_depth } );

		return c_m_depth;
	}

private:
	const fhe_optimization_exact_syn_params ps_;
	fhe_optimization_exact_syn_stats* pst_;
};

} /* mockturtle */