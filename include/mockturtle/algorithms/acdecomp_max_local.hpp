#pragma once

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/operators.hpp>
#include <kitty/print.hpp>
#include <kitty/static_truth_table.hpp>

#include <limits>
#include <numeric>

namespace mockturtle
{

struct acdecomp_max_local_params
{
	uint32_t lut_size = 6u; /* `lut_size` has to be no larger than six, as required by the synthesis engine */
	uint32_t max_free_set_size = 4u;
};

struct decomp_result
{
	kitty::dynamic_truth_table tt;
	std::vector<uint32_t> support;
};

class acdecomp_max_local
{
private:
	static constexpr uint32_t max_num_vars = 10u;
	using TT = kitty::static_truth_table<max_num_vars>;
	struct encoding_column
	{
		uint64_t column[2];
		uint32_t index;
		uint32_t cost;
		float sort_cost;
	};
	
public:
	explicit acdecomp_max_local( kitty::dynamic_truth_table const& tt, acdecomp_max_local_params ps )
		: _tt_orig( tt ),
		  _num_vars( tt.num_vars() ),
		  _ps( ps )
	{}
	
	// bool investigate_decomp( uint64_t* const& tt_begin, std::vector<std::vector<uint32_t>> ownerships )
	bool investigate_decomp( std::vector<std::vector<uint32_t>> ownerships )
	{
		if ( _num_vars > _ps.max_free_set_size + _ps.lut_size )
		{
			_ps.max_free_set_size = _num_vars - _ps.lut_size;
		}

		for ( uint32_t i = 1u; i < ownerships.size(); ++i )
		{
			const uint32_t ownerships_i_size = ownerships[i].size();

      /* for debugging */
      fmt::print( "[m] Trying to put {} leaves in the free set (max. {})...\n", ownerships_i_size, _ps.max_free_set_size );

			if ( ownerships_i_size > _ps.max_free_set_size )
			{
				continue;
			}

      std::copy( _tt_orig.begin(), _tt_orig.end(), _tt_res.begin() );
      std::iota( _perm.begin(), _perm.end(), 0u );
			push_into_free_set( ownerships[i] );

      /* for debugging */
      fmt::print( "[m] The original truth table is " );
      kitty::print_hex( _tt_orig );
      fmt::print( ", the reordered one is " );
      kitty::print_hex( _tt_res );
      fmt::print( "\n" );

			if ( check_multiplicity( ownerships[i] ) )
			{
        /* for debugging */
        fmt::print( "[m] Multiplicity is {} (max. {}). A good deal...\n", _multiplicity, ( 1 << ( _ps.lut_size - _free_set_size ) ) );

				return true;
			}
      else
      {
         fmt::print( "[m] Multiplicity is {} (max. {}). Miu is too high!\n", _multiplicity, ( 1 << ( _ps.lut_size - _free_set_size ) ) );
      }
		}
	
		return false;
	}
	
	bool compute_decomp()
	{
		std::vector<TT> isets = compute_isets();
		compute_encodings();

		if ( _multiplicity <= 4u )
		{
			return solve_encoding_exact( isets );
		}
		return solve_encoding_heuristic( isets );
	}

	void get_decomp()
	{
    // /* for debugging */
    // fmt::print( "[m] Require {} bound set functions, ", _bound_set.size() );

    // _decomp_res.clear();
		// for ( uint32_t i = 0u; i < _bound_set.size(); ++i )
		// {
		// 	TT tt = _bound_set[i];
		// 	TT care = _care_set[i];
		// 	decomp_result decomp_res;

		// 	const uint32_t isets_support_size = _num_vars - _free_set_size;
		// 	uint32_t k = 0u;
		// 	for ( uint32_t j = 0u; j < isets_support_size; ++j )
		// 	{
		// 		if ( !kitty::has_var( tt, care, j ) )
		// 		{
		// 			adjust_tt_on_dc( tt, care, j );
		// 			continue;
		// 		}

		// 		if ( k < j )
		// 		{
		// 			kitty::swap_inplace( tt, k, j );
		// 			kitty::swap_inplace( care, k, j );
		// 		}

		// 		decomp_res.support.push_back( _perm[_free_set_size + j]);
		// 		++k;
		// 	}
		// 	decomp_res.tt = kitty::shrink_to( tt, decomp_res.support.size() );

    //   // assert( !decomp_res.support.empty() );
    //   if ( decomp_res.support.empty() )
    //   {
    //     continue;
    //   }
      
		// 	_decomp_res.push_back( decomp_res );
		// }

    // /* for debugging */
    // fmt::print( "{} bound set functions added\n", _decomp_res.size() );

		// compute_top_lut();

    /* for debugging */
    fmt::print( "[m] Require {} bound set functions.\n", _bound_set.size() );

    _decomp_res.clear();

    for ( uint32_t i = 0; i < _bound_set.size(); ++i )
    {
      decomp_result dec;
      auto tt = _bound_set[i];
      auto care = _care_set[i];

      /* compute and minimize support for bound set variables */
      uint32_t k = 0;
      for ( uint32_t j = 0; j < _num_vars - _free_set_size; ++j )
      {
        if ( !kitty::has_var( tt, care, j ) )
        {
          /* fix truth table */
          adjust_tt_on_dc( tt, care, j );
          continue;
        }

        if ( k < j )
        {
          kitty::swap_inplace( tt, k, j );
          kitty::swap_inplace( care, k, j );
        }
        dec.support.push_back( _perm[_free_set_size + j] );
        ++k;
      }

      dec.tt = kitty::shrink_to( tt, dec.support.size() );
      _decomp_res.push_back( dec );

      /* for debugging */
      if ( dec.support.size() == 0u )
      {
        fmt::print( "[e] Weird, a BS function is without support, the function is " );
        kitty::print_hex( _bound_set[i] );
        fmt::print( "\n" );
      }
      else
      {
        fmt::print( "[m] A BS function is added. Its support is: " );
        for ( auto const& ele : dec.support )
        {
          fmt::print( "{} ", ele );
        }
        fmt::print( "; its function is " );
        kitty::print_hex( dec.tt );
        fmt::print( "\n" );
      }
    }

    /* for debugging */
    fmt::print( "{} bound set functions added\n", _decomp_res.size() );

    /* compute the decomposition for the top-level LUT */
    compute_top_lut();
	}

private:
	inline void push_into_free_set( std::vector<uint32_t> const& indices )
	{
    // for ( uint32_t i = 0u; i < indices.size(); ++i )
    // {
    //   uint32_t ind_i = indices[i];
    //   /* `ind_i` should stay in `_perm[i]` */
    //   if ( _perm[i] == ind_i )
    //   {
    //     continue;
    //   }

    //   /* `ind_pos` records where `ind_i` is */
    //   uint32_t ind_pos = 0u;
    //   while ( _perm[ind_pos] != ind_i )
    //   {
    //     ++ind_pos;
    //   }
    //   std::swap( _perm[i], _perm[ind_pos] );
    //   kitty::swap_inplace( _tt_res, i, ind_pos );
    // }

    uint32_t k = 0u;
    for ( uint32_t i = 0u; i < indices.size(); ++i )
    {
      while ( k != indices[i] )
      {
        ++k;
      }

      if ( _perm[i] == k )
      {
        ++k;
        continue;
      }

      std::swap( _perm[i], _perm[k] );
      kitty::swap_inplace( _tt_res, i, k );
      ++k;
    }
	}

	bool check_multiplicity( std::vector<uint32_t> const& indices )
	{
		std::function<uint32_t( TT const& tt )> column_multiplicity_fn[5] = {
			[this]( TT const& tt ) { return column_multiplicity_le3<1u>( tt ); },
			[this]( TT const& tt ) { return column_multiplicity_le3<2u>( tt ); },
			[this]( TT const& tt ) { return column_multiplicity_le3<3u>( tt ); },
			[this]( TT const& tt ) { return column_multiplicity_le5<4u>( tt ); },
			[this]( TT const& tt ) { return column_multiplicity_le5<5u>( tt ); }
		};

		_free_set_size = indices.size();
		_multiplicity = column_multiplicity_fn[_free_set_size - 1]( _tt_res );

		/* cases where more than two-level decomposition is required is not considered for now  */
		/* when `_multiplicity` is larger than 16, `g` function cannot be realized via one LUT6 */
		if ( _multiplicity <= ( 1 << ( _num_vars - _free_set_size ) ) &&
         _multiplicity <= ( 1 << ( _ps.lut_size - _free_set_size ) ) && _multiplicity <= 16u )
		{
			return true;
		}

		return false;
	}

	template<uint32_t free_set_size>
	uint32_t column_multiplicity_le3( TT const& tt )
	{
		// uint64_t multiplicity_set[4] = { 0u, 0u, 0u, 0u };
		// uint32_t multiplicity = 0u;
		// const uint32_t num_blocks = ( _num_vars > 6u ) ? ( 1u << ( _num_vars - 6u ) ) : 1u ;
    // const uint32_t len = ( num_blocks == 1u ) ? ( 1 << ( _num_vars - _free_set_size ) ) : ( 64u >> _free_set_size ); 
		// constexpr uint64_t masks_bits[] = { 0x0, 0x3, 0xF, 0x3F };
		// constexpr uint64_t masks_indices[] = { 0x0, 0x0, 0x0, 0x3 };

		// for ( uint32_t i = 0u; i < num_blocks; ++i )
		// {
		// 	uint64_t coef = tt._bits[i];
		// 	for ( uint32_t j = 0u; j < len; ++j )
		// 	{
		// 		multiplicity_set[( coef >> 6u ) & masks_indices[free_set_size]] |= uint64_t( 1u ) << ( coef & masks_bits[free_set_size] );
		// 		coef >>= ( 1u << free_set_size );
		// 	}
		// }

		// /* TODO : think about optimizing this part */
		// multiplicity = static_cast<uint32_t>( __builtin_popcountl( multiplicity_set[0] ) );
		// if ( free_set_size == 3u )
		// {
		// 	multiplicity += static_cast<uint32_t>( __builtin_popcountl( multiplicity_set[1] ) );
		// 	multiplicity += static_cast<uint32_t>( __builtin_popcountl( multiplicity_set[2] ) );
		// 	multiplicity += static_cast<uint32_t>( __builtin_popcountl( multiplicity_set[3] ) );
		// }

		// return multiplicity;

    uint64_t multiplicity_set[4] = { 0u, 0u, 0u, 0u };
    uint32_t multiplicity = 0;
    uint32_t const num_blocks = ( _num_vars > 6 ) ? ( 1u << ( _num_vars - 6 ) ) : 1;
    uint64_t constexpr masks_bits[] = { 0x0, 0x3, 0xF, 0x3F };
    uint64_t constexpr masks_idx[] = { 0x0, 0x0, 0x0, 0x3 };

    /* extract iset functions */
    /* for debugging */
    fmt::print( "[m] Figuring out the multiplicity...\n" );
    const uint32_t len = ( num_blocks == 1u ) ? ( 1 << ( _num_vars - _free_set_size ) ) : ( 64u >> _free_set_size ); 
    for ( auto i = 0u; i < num_blocks; ++i )
    {
      uint64_t cof = tt._bits[i];
      for ( auto j = 0; j < len; ++j )
      {
        uint64_t& repr = multiplicity_set[( cof >> 6 ) & masks_idx[_free_set_size]];
        uint64_t temp = repr;
        repr |= ( uint64_t( 1 ) << ( cof & masks_bits[_free_set_size] ) );
        if ( repr != temp )
        {
          fmt::print( "[m] new pattern ({}) detected, increasing multiplicity...\n", ( cof & masks_bits[_free_set_size] ) );
        }
        cof >>= ( 1u << _free_set_size );
      }
    }

    multiplicity = __builtin_popcountl( multiplicity_set[0] );

    fmt::print( "[m] The multiplicity is calculated to be {}\n", multiplicity );

    if ( _free_set_size == 3 )
    {
      multiplicity += __builtin_popcountl( multiplicity_set[1] );
      multiplicity += __builtin_popcountl( multiplicity_set[2] );
      multiplicity += __builtin_popcountl( multiplicity_set[3] );
    }

    return multiplicity;
	}

	template<uint32_t free_set_size>
	uint32_t column_multiplicity_le5( TT const& tt )
	{
		std::array<uint64_t, 64u> multiplicity_set;
		const uint32_t num_blocks = ( _num_vars > 6u ) ? ( 1u << ( _num_vars - 6u ) ) : 1u;
		constexpr uint64_t masks[] = { 0x0, 0x3, 0xF, 0xFF, 0xFFFF, 0xFFFFFFFF };
		uint32_t multiplicity_set_size = 0u;

		uint64_t fs_func_prev = UINT64_MAX;
		for ( uint32_t i = 0u; i < num_blocks; ++i )
		{
			uint64_t coef = tt._bits[i];
			for ( uint32_t j = 0u; j < ( 64 >> free_set_size ); ++j )
			{
				uint64_t fs_func = coef & masks[free_set_size];
				if ( fs_func != fs_func_prev )
				{
					multiplicity_set[multiplicity_set_size++] = fs_func;
					fs_func_prev = fs_func;
				}
				coef >>= ( 1u << free_set_size );
			}
		}

		std::sort( multiplicity_set.begin(), multiplicity_set.begin() + multiplicity_set_size );

		uint32_t multiplicity = 1u;
		for ( uint8_t i = 1u; i < multiplicity_set_size; ++i )
		{
			if ( multiplicity_set[i] != multiplicity_set[i - 1] )
			{
				++multiplicity;
			}
		}

		return multiplicity;
	}

	std::vector<TT> compute_isets()
	{
		// const uint32_t isets_support_size = _num_vars - _free_set_size;
		// std::vector<TT> isets( _multiplicity );
		// std::unordered_map<uint64_t, uint32_t> column_to_iset;
		// TT tt = _tt_res;
		// uint32_t offset = 0u;
		// uint32_t num_blocks = ( _num_vars > 6u ) ? ( 1u << ( _num_vars - 6u) ) : 1u;
		// constexpr uint64_t masks[] = { 0x0, 0x3, 0xF, 0xFF, 0xFFFF, 0xFFFFFFFF };

		// auto it = std::begin( tt );
		// for ( uint32_t i = 0u; i < num_blocks; ++i )
		// {
		// 	for ( uint32_t j = 0u; j < ( 64u >> _free_set_size ); ++j )
		// 	{
		// 		uint64_t val = *it & masks[_free_set_size];
		// 		auto el = column_to_iset.find( val );
		// 		if ( el != column_to_iset.end() )
		// 		{
		// 			isets[el->second]._bits[i / ( 1u << _free_set_size )] != ( uint64_t( 1u ) << ( j + offset ) );
		// 		}
		// 		else
		// 		{
		// 			isets[column_to_iset.size()]._bits[i / ( 1u << _free_set_size )] |= ( uint64_t( 1u ) << ( j + offset ) );
		// 			column_to_iset[val] = column_to_iset.size();
		// 		}

		// 		*it >>= ( 1u << _free_set_size );
		// 	}

		// 	offset = ( offset + ( 64u >> _free_set_size ) ) & 0x3F;
		// 	++it;
		// }

		// for ( TT& iset : isets )
		// {
		// 	tt_expansion( iset, isets_support_size );
		// }

    // _free_set.clear();
    // _free_set.resize( _multiplicity );
		// for ( auto const& pair : column_to_iset )
		// {
		// 	_free_set[pair.second]._bits[0] = pair.first;
		// 	tt_expansion( _free_set[pair.second], _free_set_size );
		// }

    /* for debugging */
    // {
    //   fmt::print( "[m] isets:\n" );
    //   uint32_t i = 0u;
    //   for ( auto iset : isets )
    //   {
    //     kitty::print_hex( iset );
    //     fmt::print( " of func " );
    //     kitty::print_hex( _free_set[i++] );
    //     fmt::print( "\n" );
    //   }
    // }

    /* construct isets involved in multiplicity */
    uint32_t isets_support = _num_vars - _free_set_size;
    std::vector<TT> isets( _multiplicity );

    /* construct isets */
    std::unordered_map<uint64_t, uint32_t> column_to_iset;
    TT tt = _tt_res;
    uint32_t offset = 0;
    uint32_t num_blocks = ( _num_vars > 6 ) ? ( 1u << ( _num_vars - 6 ) ) : 1;
    uint64_t constexpr masks[] = { 0x0, 0x3, 0xF, 0xFF, 0xFFFF, 0xFFFFFFFF };

    auto it = std::begin( tt );
    for ( auto i = 0u; i < num_blocks; ++i )
    {
      for ( auto j = 0; j < ( 64 >> _free_set_size ); ++j )
      {
        uint64_t val = *it & masks[_free_set_size];

        auto el = column_to_iset.find( val );
        if ( el != column_to_iset.end() )
        {
          isets[el->second]._bits[i / ( 1u << _free_set_size )] |= UINT64_C( 1 ) << ( j + offset );
        }
        else
        {
          isets[column_to_iset.size()]._bits[i / ( 1u << _free_set_size )] |= UINT64_C( 1 ) << ( j + offset );
          column_to_iset[val] = column_to_iset.size();
        }

        *it >>= ( 1u << _free_set_size );
      }

      offset = ( offset + ( 64 >> _free_set_size ) ) & 0x3F;
      ++it;
    }

    /* extend isets to cover the whole truth table */
    for ( TT& iset : isets )
    {
      tt_expansion( iset, isets_support );
    }

    /* save free_set functions */
    std::vector<TT> free_set_tts( _multiplicity );

    for ( auto const& pair : column_to_iset )
    {
      free_set_tts[pair.second]._bits[0] = pair.first;
      tt_expansion( free_set_tts[pair.second], _free_set_size );
    }

    _free_set = std::move( free_set_tts );

    /* for debugging */
    fmt::print( "[m] isets: \n" );
    for ( auto i = 0u; i < isets.size(); ++i )
    {
      fmt::print( "[m] The {}-th iset is ", ( i + 1 ) );
      kitty::print_hex( isets[i] );
      fmt::print( "\n" );
    }

		return isets;
	}

	void tt_expansion( TT& tt, uint32_t num_vars_local )
	{
		if ( num_vars_local < 6u )
		{
			auto mask = *tt.begin();
			for ( uint32_t i = num_vars_local; i < _num_vars; ++i )
			{
				mask |= ( mask << ( 1 << i ) );
			}

			std::fill( tt.begin(), tt.end(), mask );
		}
		else
    {
			uint32_t num_blocks = ( 1u << ( num_vars_local - 6u ) );
			auto it = tt.begin();
			while ( it != tt.end() )
			{
				it = std::copy( tt.cbegin(), tt.cbegin() + num_blocks, it );
			}
		}
	}

	void compute_encodings()
	{
		// uint32_t count = 0, num_combs = 0u;
		// if ( __builtin_popcount( _multiplicity ) == 1 )
		// {
		// 	uint32_t num_combs_enum[4] = { 1u, 3u, 35u, 6435u };
		// 	for ( uint32_t i = 0u; i < 4u; ++i )
		// 	{
		// 		if ( ( _multiplicity >> i ) == 2u )
		// 		{
		// 			num_combs = num_combs_enum[i];
		// 		}
		// 	}
		// 	_min_support_encodings = std::vector<std::array<uint32_t, 2u>>( num_combs );
		// 	compute_encodings_rec<false, true>( 0u, 0u, 0u, count, _multiplicity >> 1u, true );
		// 	assert( count == num_combs );
		// 	return;
		// }

		// /* constrian the number of offset classes to guanrantee a strict encoding */
		// int32_t min_num_isets = 0;
		// if ( _multiplicity <= 4u )
		// {
		// 	min_num_isets = 2;
		// }
		// else if ( _multiplicity <= 8u )
		// {
		// 	min_num_isets = 4;
		// }
		// else
    // {
		// 	min_num_isets = 8;
		// }
		// min_num_isets = _multiplicity - min_num_isets;
		
		// /* exploit don't-cares when `_multiplicity` is not a power of two and no larger than eight */
		// if ( _multiplicity > 8u )
		// {
		// 	uint32_t class_sizes[13] = { 3, 3, 15, 25, 35, 35, 255, 501, 957, 1749, 3003, 4719, 6435 };
		// 	num_combs = class_sizes[_multiplicity - 3];
		// 	_min_support_encodings = std::vector<std::array<uint32_t, 2u>>( num_combs );
		// 	compute_encodings_rec<false, false>( 0u, 0u, 0u, count, min_num_isets, true );
		// }
		// else
		// {
		// 	uint32_t class_sizes[13] = { 6, 3, 90, 130, 105, 35, 9330, 23436, 48708, 78474, 91377, 70785, 32175 };
		// 	num_combs = class_sizes[_multiplicity - 3];
		// 	_min_support_encodings = std::vector<std::array<uint32_t, 2u>>( num_combs );
		// 	compute_encodings_rec<true, false>( 0u, 0u, 0u, count, min_num_isets, true );
		// }
		
		// assert( count == num_combs );

    uint32_t count = 0;

    /* enable don't cares only if not a power of 2 */
    uint32_t num_combs = 2;
    if ( __builtin_popcount( _multiplicity ) == 1 )
    {
      uint32_t num_combs_exact[4] = { 1, 3, 35, 6435 };
      for ( uint32_t i = 0; i < 4; ++i )
      {
        if ( ( _multiplicity >> i ) == 2u )
        {
          num_combs = num_combs_exact[i];
        }
      }
      _min_support_encodings = std::vector<std::array<uint32_t, 2>>( num_combs );
      compute_encodings_rec<false, true>( 0, 0, 0, count, _multiplicity >> 1, true );
      assert( count == num_combs );
      return;
    }

    /* constraint the number of offset classes for a strict encoding */
    int32_t min_set_size = 1;
    if ( _multiplicity <= 4 )
      min_set_size = 2;
    else if ( _multiplicity <= 8 )
      min_set_size = 4;
    else
      min_set_size = 8;
    min_set_size = _multiplicity - min_set_size;

    if ( _multiplicity > 8 )
    {
      /* distinct elements in 2 indistinct bins with at least `min_set_size` elements in the indistinct bins */
      uint32_t class_sizes[13] = { 3, 3, 15, 25, 35, 35, 255, 501, 957, 1749, 3003, 4719, 6435 };
      num_combs = class_sizes[_multiplicity - 3];
      _min_support_encodings = std::vector<std::array<uint32_t, 2>>( num_combs );
      compute_encodings_rec<false, false>( 0, 0, 0, count, min_set_size, true );
    }
    else
    {
      /* distinct elements in 3 bins, of which 2 are indistinct, and with at least `min_set_size` elements in the indistinct bins */
      uint32_t class_sizes[13] = { 6, 3, 90, 130, 105, 35, 9330, 23436, 48708, 78474, 91377, 70785, 32175 };
      num_combs = class_sizes[_multiplicity - 3];
      _min_support_encodings = std::vector<std::array<uint32_t, 2>>( num_combs );
      compute_encodings_rec<true, false>( 0, 0, 0, count, min_set_size, true );
    }

    assert( count == num_combs );
	}

	template<bool enable_dc, bool equal_size_partition>
	void compute_encodings_rec( uint32_t onset, uint32_t offset, uint32_t iset_index, uint32_t& count, int32_t min_num_isets, bool is_first )
	{
		// if ( iset_index == _multiplicity )
		// {
		// 	if ( equal_size_partition )
		// 	{
		// 		if ( __builtin_popcount( onset ) != __builtin_popcount( offset ) )
		// 		{
		// 			return;
		// 		}
		// 	}
		// 	else if ( __builtin_popcount( onset ) < min_num_isets || __builtin_popcount( offset ) < min_num_isets )
		// 	{
		// 		return;
		// 	}

		// 	_min_support_encodings[count][0] = onset;
		// 	_min_support_encodings[count][1] = offset;
		// 	++count;
		// 	return;
		// }

		// if ( enable_dc )
		// {
		// 	compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, is_first );
		// }

		// onset |= uint32_t( 1u ) << iset_index;
		// compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, false );
		// onset &= ~( uint32_t( 1u ) << iset_index );

		// if ( is_first )
		// {
		// 	return;
		// }

		// offset |= uint32_t( 1u ) << iset_index;
		// compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, false );
		// offset &= ~( uint32_t( 1u ) << iset_index );

    if ( iset_index == _multiplicity )
    {
      if ( equal_size_partition )
      {
        /* sets must be equally populated */
        if ( __builtin_popcount( onset ) != __builtin_popcount( offset ) )
        {
          return;
        }
      }
      else if ( __builtin_popcount( onset ) < min_num_isets || __builtin_popcount( offset ) < min_num_isets )
      {
        /* ON-set and OFF-set must be populated with at least min_set_size elements */
        return;
      }

      _min_support_encodings[count][0] = onset;
      _min_support_encodings[count][1] = offset;
      ++count;

      /* for debugging */
      fmt::print( "[m] Adding the {}th code: onset is {}, offset is {}.\n", count, onset, offset );

      return;
    }

    /* var in DCSET */
    if ( enable_dc )
    {
      compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, is_first );
    }

    /* move var in ONSET */
    onset |= 1 << iset_index;
    compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, false );
    onset &= ~( 1 << iset_index );

    /* remove symmetries */
    if ( is_first )
    {
      return;
    }

    /* move var in OFFSET */
    offset |= 1 << iset_index;
    compute_encodings_rec<enable_dc, equal_size_partition>( onset, offset, iset_index + 1, count, min_num_isets, false );
    offset &= ~( 1 << iset_index );
	}

	bool solve_encoding_exact( std::vector<TT> const& isets )
	{
		// std::vector<encoding_column> matrix;
		// matrix.reserve( _min_support_encodings.size() );
		// create_covering_matrix<false>( isets, matrix, false );

		// /* `solution[5]` stores the bound set size */
		// std::array<uint32_t, 6u> solution = solve_covering_exact( matrix );
		// if ( solution[0] == UINT32_MAX )
		// {
		// 	return false;
		// }

		// _bound_set.clear();
		// _care_set.clear();
		// _isets_onset.clear();
		// _isets_offset.clear();

		// for ( uint32_t i = 0u; i < solution[5]; ++i )
		// {
		// 	TT tt, care;
		// 	const uint32_t onset = _min_support_encodings[matrix[solution[i]].index][0];
		// 	const uint32_t offset = _min_support_encodings[matrix[solution[i]].index][1];
		// 	for ( uint32_t j = 0u; j < _multiplicity; ++j)
		// 	{
		// 		if ( ( onset >> j ) & 1 )
		// 		{
		// 			tt |= isets[j];
		// 		}
		// 		if ( ( offset >> j ) & 1 )
		// 		{
		// 			care |= isets[j];
		// 		}
		// 	}
		// 	care |= tt;

		// 	_bound_set.push_back( tt );
		// 	_care_set.push_back( care );
		// 	_isets_onset.push_back( onset );
		// 	_isets_offset.push_back( offset );
		// }

		// return true;

    std::vector<encoding_column> matrix;
    matrix.reserve( _min_support_encodings.size() );
    _bound_set.clear();

    /* create covering matrix */
    create_covering_matrix<false>( isets, matrix, false );

    /* solve the covering problem */
    std::array<uint32_t, 6> solution = solve_covering_exact( matrix );

    /* check for failed decomposition */
    if ( solution[0] == UINT32_MAX )
    {
      return false;
    }

    /* compute best bound sets */
    uint32_t isets_support = _num_vars - _free_set_size;
    _care_set.clear();
    _isets_onset.clear();
    _isets_offset.clear();
    for ( uint32_t i = 0; i < solution[5]; ++i )
    {
      TT tt;
      TT care;

      const uint32_t onset = _min_support_encodings[matrix[solution[i]].index][0];
      const uint32_t offset = _min_support_encodings[matrix[solution[i]].index][1];
      for ( uint32_t j = 0; j < _multiplicity; ++j )
      {
        if ( ( ( onset >> j ) & 1 ) )
        {
          tt |= isets[j];
        }
        if ( ( ( offset >> j ) & 1 ) )
        {
          care |= isets[j];
        }
      }

      care |= tt;

      _bound_set.push_back( tt );
      _care_set.push_back( care );
      _isets_onset.push_back( onset );
      _isets_offset.push_back( offset );

      /* for debugging */
      fmt::print( "[m] A raw BS function: " );
      kitty::print_hex( tt );
      fmt::print( "\n" );
    }

    /* for debugging */
    // fmt::print( "[m] `_isets_onset` contains: " );
    // for ( auto const& ele : _isets_onset )
    // {
    //   fmt::print( "{} ", ele );
    // }
    // fmt::print( "\n" );
    // fmt::print( "[m] `_isets_offset` contains: " );
    // for ( auto const& ele : _isets_offset )
    // {
    //   fmt::print( "{} ", ele );
    // }
    // fmt::print( "\n" );

    return true;
	}

	bool solve_encoding_heuristic( std::vector<TT> const& isets )
	{
		// std::vector<encoding_column> matrix;
		// matrix.reserve( _min_support_encodings.size() );
		// create_covering_matrix<true>( isets, matrix, true );

		// /* `solution[5]` stores the bound set size */
		// std::array<uint32_t, 6u> solution = solve_covering_heuristic( matrix );
		// if ( solution[0] == UINT32_MAX )
		// {
		// 	return false;
		// }

		// /* improve each solution entry by looking for a better choice */
		// while ( improve_covering( matrix, solution ) )
		// {}

		// _bound_set.clear();
		// _care_set.clear();
		// _isets_onset.clear();
		// _isets_offset.clear();

		// for ( uint32_t i = 0u; i < solution[5]; ++i )
		// {
		// 	TT tt, care;
		// 	const uint32_t onset = _min_support_encodings[matrix[solution[i]].index][0];
		// 	const uint32_t offset = _min_support_encodings[matrix[solution[i]].index][1];
		// 	for ( uint32_t j = 0u; j < _multiplicity; ++j)
		// 	{
		// 		if ( ( onset >> j ) & 1 )
		// 		{
		// 			tt |= isets[j];
		// 		}
		// 		if ( ( offset >> j ) & 1 )
		// 		{
		// 			care |= isets[j];
		// 		}
		// 	}
		// 	care |= tt;

		// 	_bound_set.push_back( tt );
		// 	_care_set.push_back( care );
		// 	_isets_onset.push_back( onset );
		// 	_isets_offset.push_back( offset );
		// }

		// return true;

    std::vector<encoding_column> matrix;
    matrix.reserve( _min_support_encodings.size() );
    _bound_set.clear();

    /* create covering matrix */
    create_covering_matrix<true>( isets, matrix, true );

    /* solve the covering problem: heuristic pass + local search */
    std::array<uint32_t, 6> solution = solve_covering_heuristic( matrix );

    /* check for failed decomposition */
    if ( solution[0] == UINT32_MAX )
    {
      return false;
    }

    /* improve solution with local search */
    while ( improve_covering( matrix, solution ) )
      ;

    /* compute best bound sets */
    uint32_t isets_support = _num_vars - _free_set_size;
    _care_set.clear();
    _isets_onset.clear();
    _isets_offset.clear();
    for ( uint32_t i = 0; i < solution[5]; ++i )
    {
      TT tt;
      TT care;

      const uint32_t onset = _min_support_encodings[matrix[solution[i]].index][0];
      const uint32_t offset = _min_support_encodings[matrix[solution[i]].index][1];
      for ( uint32_t j = 0; j < _multiplicity; ++j )
      {
        if ( ( ( onset >> j ) & 1 ) )
        {
          tt |= isets[j];
        }
        if ( ( ( offset >> j ) & 1 ) )
        {
          care |= isets[j];
        }
      }

      care |= tt;

      _bound_set.push_back( tt );
      _care_set.push_back( care );
      _isets_onset.push_back( onset );
      _isets_offset.push_back( offset );
    }

    return true;
	}

	template<bool use_heuristic>
	void create_covering_matrix( std::vector<TT> const& isets, std::vector<encoding_column>& matrix, bool sort )
	{
		// uint32_t num_combs = ( _multiplicity * ( _multiplicity - 1u ) ) / 2;
		// const uint32_t isets_support_size = _num_vars - _free_set_size;

		// /* insert dichotomies */
		// for ( uint32_t i = 0u; i < _min_support_encodings.size(); ++i )
		// {
		// 	const uint32_t onset = _min_support_encodings[i][0];
		// 	const uint32_t offset = _min_support_encodings[i][1];
		// 	uint64_t column[2] = { 0u, 0u };
		// 	TT tt, care;
		// 	uint32_t pair_pointer = 0u;
		// 	for ( uint32_t j = 0u; j < _multiplicity; ++j )
		// 	{
		// 		uint32_t onset_shift = ( onset >> j );
		// 		uint32_t offset_shift = ( offset >> j );
		// 		if ( onset_shift & 1 )
		// 		{
		// 			tt |= isets[j];
		// 		}
		// 		if ( offset_shift & 1 )
		// 		{
		// 			care |= isets[j];
		// 		}

		// 		for ( uint32_t k = j + 1u; k < _multiplicity; ++k )
		// 		{
		// 			if ( ( ( onset_shift & ( offset >> k ) ) | ( ( offset_shift & ( onset >> k ) ) ) ) & 1 )
		// 			{
		// 				column[pair_pointer >> 6u] |= ( uint64_t( 1u ) << ( pair_pointer & 0x3F ) );
		// 			}
		// 			++pair_pointer;
		// 		}
		// 	}

		// 	care |= tt;

		// 	uint32_t cost = 0u;
		// 	for ( uint32_t j = 0u; j < isets_support_size; ++j )
		// 	{
		// 		cost += is_in_support( tt, care, isets_support_size, j ) ? 1u : 0u;
		// 	}
		// 	if ( cost > _ps.lut_size )
		// 	{
		// 		continue;
		// 	}
		// 	if ( cost == 1u )
		// 	{
		// 		cost = 0u;
		// 	}

		// 	float sort_cost = 0.;
		// 	if ( use_heuristic )
		// 	{
		// 		sort_cost = 1.0f / ( __builtin_popcountl( column[0] ) + __builtin_popcountl( column[1] ) );
		// 	}
		// 	else
		// 	{
		// 		sort_cost = cost + ( ( num_combs - __builtin_popcountl( column[0] ) + __builtin_popcountl( column[1] ) ) << _num_vars );
		// 	}

		// 	matrix.emplace_back( encoding_column{ { column[0], column[1] }, i, cost, sort_cost } );
		// }

		// if ( !sort )
		// {
		// 	return;
		// }

		// if ( use_heuristic )
		// {
		// 	std::sort( matrix.begin(),  matrix.end(), [&]( encoding_column const& a, encoding_column const& b ){
		// 		return a.cost < b.cost;
		// 	} );
		// }
		// else
		// {
		// 	std::sort( matrix.begin(), matrix.end(), [&]( encoding_column const& a, encoding_column const& b) {
		// 		return a.sort_cost < b.sort_cost;
		// 	} );
		// }

    uint32_t combinations = ( _multiplicity * ( _multiplicity - 1 ) ) / 2;
    uint32_t iset_support = _num_vars - _free_set_size;

    /* insert dichotomies */
    for ( uint32_t i = 0; i < _min_support_encodings.size(); ++i )
    {
      uint32_t const onset = _min_support_encodings[i][0];
      uint32_t const offset = _min_support_encodings[i][1];

      /* compute function and distinguishable seed dichotomies */
      uint64_t column[2] = { 0, 0 };
      TT tt;
      TT care;
      uint32_t pair_pointer = 0;
      for ( uint32_t j = 0; j < _multiplicity; ++j )
      {
        auto onset_shift = ( onset >> j );
        auto offset_shift = ( offset >> j );
        if ( ( onset_shift & 1 ) )
        {
          tt |= isets[j];
        }

        if ( ( offset_shift & 1 ) )
        {
          care |= isets[j];
        }

        /* compute included seed dichotomies */
        for ( uint32_t k = j + 1; k < _multiplicity; ++k )
        {
          /* if are in diffent sets */
          if ( ( ( ( onset_shift & ( offset >> k ) ) | ( ( onset >> k ) & offset_shift ) ) & 1 ) )
          {
            column[pair_pointer >> 6u] |= UINT64_C( 1 ) << ( pair_pointer & 0x3F );
          }

          ++pair_pointer;
        }
      }

      care |= tt;

      /* compute cost */
      uint32_t cost = 0;
      for ( uint32_t j = 0; j < iset_support; ++j )
      {
        cost += ( kitty::has_var( tt, care, j ) ? 1 : 0 );
        // cost += is_in_support( tt, care, iset_support, j ) ? 1 : 0;
        // if ( !has_var_support( tt, care, iset_support, j ) )
        // {
        //   /* adjust truth table and care set */
        //   adjust_truth_table_on_dc( tt, care, iset_support, j );
        //   continue;
        // }
        // ++cost;
      }

      if ( cost == 0u )
      {
        /* must has at least one support */
        cost = UINT32_MAX;
      }

      /* discard solutions with support over LUT size */
      if ( cost > _ps.lut_size )
        continue;
      
      /* buffers have zero cost */
      if ( cost == 1 )
        cost = 0;

      float sort_cost = 0;
      if ( use_heuristic )
      {
        sort_cost = 1.0f / ( __builtin_popcountl( column[0] ) + __builtin_popcountl( column[1] ) );
      }
      else
      {
        sort_cost = cost + ( ( combinations - __builtin_popcountl( column[0] + __builtin_popcountl( column[1] ) ) ) << _num_vars );
      }

      /* insert */
      matrix.emplace_back( encoding_column{ { column[0], column[1] }, i, cost, sort_cost } );
    }

    if ( !sort )
    {
      return;
    }

    if ( use_heuristic )
    {
      std::sort( matrix.begin(), matrix.end(), [&]( encoding_column const& a, encoding_column const& b ) {
        return a.cost < b.cost;
      } );
    }
    else
    {
      std::sort( matrix.begin(), matrix.end(), [&]( encoding_column const& a, encoding_column const& b ) {
        return a.sort_cost < b.sort_cost;
      } );
    }
	}

	bool is_in_support( TT const& tt, TT const& care, uint32_t num_vars_local, uint8_t var_index )
	{
		// assert( var_index < num_vars_local );
		// assert( num_vars_local <= tt.num_vars() );
		// assert( tt.num_vars() == care.num_vars() );

		// const uint32_t num_blocks = ( num_vars_local <= 6u ) ? 1u : ( 1u << ( num_vars_local - 6u ) ); 
		// if ( num_vars_local <= 6u || var_index < 6u )
		// {
		// 	auto it_tt = std::begin( tt._bits );
		// 	auto it_care = std::begin( care._bits );
		// 	while ( it_tt != ( std::begin( tt._bits ) + num_blocks ) )
		// 	{
		// 		if ( ( ( ( *it_tt >> ( uint64_t( 1 ) << var_index ) ) ^ *it_tt ) &
    //            kitty::detail::projections_neg[var_index] &
    //            ( *it_care >> ( uint64_t( 1 ) << var_index ) ) & *it_care ) != 0 )
		// 		{
		// 			return true;
		// 		}
		// 		++it_tt;
		// 		++it_care;
		// 	}

		// 	return false;
		// }

		// const uint32_t step = 1u << ( var_index - 6u );
		// for ( uint32_t i = 0u; i < num_blocks; i += 2 * step )
		// {
		// 	for ( uint32_t j = 0u; j < step; ++j )
		// 	{
		// 		if ( ( ( tt._bits[i + j] ^ tt._bits[i + j + step] ) &
    //            care._bits[i + j] & care._bits[i + j + step] ) != 0 )
		// 		{
		// 			return true;
		// 		}
		// 	}
		// }

		// return false;

    assert( var_index < num_vars_local );
    assert( num_vars_local <= tt.num_vars() );
    assert( tt.num_vars() == care.num_vars() );

    const uint32_t num_blocks = num_vars_local <= 6 ? 1 : ( 1 << ( num_vars_local - 6 ) );
    if ( num_vars_local <= 6 || var_index < 6 )
    {
      auto it_tt = std::begin( tt._bits );
      auto it_care = std::begin( care._bits );
      while ( it_tt != std::begin( tt._bits ) + num_blocks )
      {
        if ( ( ( ( *it_tt >> ( uint64_t( 1 ) << var_index ) ) ^ *it_tt ) & kitty::detail::projections_neg[var_index] & ( *it_care >> ( uint64_t( 1 ) << var_index ) ) & *it_care ) != 0 )
        {
          return true;
        }
        ++it_tt;
        ++it_care;
      }

      return false;
    }

    const auto step = 1 << ( var_index - 6 );
    for ( auto i = 0u; i < num_blocks; i += 2 * step )
    {
      for ( auto j = 0; j < step; ++j )
      {
        if ( ( ( tt._bits[i + j] ^ tt._bits[i + j + step] ) & care._bits[i + j] & care._bits[i + j + step] ) != 0 )
        {
          return true;
        }
      }
    }

    return false;
	}

	std::array<uint32_t, 6u> solve_covering_exact( std::vector<encoding_column> const& matrix )
	{
		std::array<uint32_t, 6u> solution = { UINT32_MAX };
		uint32_t best_cost = UINT32_MAX;
		uint32_t num_combs = ( _multiplicity * ( _multiplicity - 1u ) ) / 2;
		
		if ( _multiplicity <= 2u )
		{
			solution[5] = 1u;
			solution[0] = 0u;
		}
		else
		{
			/* 2u < _multiplicity <= 4u */
			solution[5] = 2u;
			for ( uint32_t i = 0u; i < matrix.size() - 1u; ++i )
			{
				for ( uint32_t j = i + 1u; j < matrix.size(); ++j )
				{
					if ( matrix[i].cost + matrix[j].cost >= best_cost )
					{
						continue;
					}

					if ( __builtin_popcountl( matrix[i].column[0] | matrix[j].column[0] ) +
					     __builtin_popcountl( matrix[i].column[1] | matrix[j].column[1] ) == num_combs )
					{
						solution[0] = i;
						solution[1] = j;
						best_cost = matrix[i].cost + matrix[j].cost;
					}
				}
			}
		}

		return solution;
	}

	std::array<uint32_t, 6u> solve_covering_heuristic( std::vector<encoding_column>& matrix )
	{
		std::array<uint32_t, 6u> solution = { UINT32_MAX };
		uint32_t num_combs = ( _multiplicity * ( _multiplicity - 1u ) ) / 2;
		uint64_t column0 = 0u, column1 = 0u;
		float best_sort_cost = std::numeric_limits<float>::max();
		uint32_t best_index = 0u;

		for ( uint32_t i = 0u; i < matrix.size(); ++i )
		{
			if ( matrix[i].sort_cost < best_sort_cost )
			{
				best_index = i;
				best_sort_cost = matrix[i].sort_cost;
			}
		}

		/* select the `best_index`-th column */
		column0 = matrix[best_index].column[0];
		column1 = matrix[best_index].column[1];
		std::swap( matrix[0], matrix[best_index] );

		uint32_t pivot = 1u;
		while ( ( pivot < ( _ps.lut_size - _free_set_size ) ) &&
		        ( ( __builtin_popcountl( column0 ) + __builtin_popcountl( column1 ) ) != num_combs ) )
		{
			best_index = 0u;
			best_sort_cost = std::numeric_limits<float>::max();

			/* among the remaining columns, select the one that minimizes the sort cost */
			for ( uint32_t i = pivot; i < matrix.size(); ++i )
			{
				float sort_cost = 1.0f / ( __builtin_popcountl( matrix[i].column[0] & ~column0 ) + __builtin_popcountl( matrix[i].column[1] & ~column1 ) );
				if ( sort_cost < best_sort_cost )
				{
					best_index = i;
					best_sort_cost = sort_cost;
				}
			}

			column0 |= matrix[best_index].column[0];
			column1 |= matrix[best_index].column[1];
			std::swap( matrix[pivot], matrix[best_index] );
			++pivot;
		}

		if ( __builtin_popcountl( column0 ) + __builtin_popcountl( column1 ) == num_combs )
		{
			for ( uint32_t i = 0u; i < pivot; ++i )
			{
				solution[i] = i;
			}
			solution[5] = pivot;
		}

		return solution;
	}

	bool improve_covering( std::vector<encoding_column> const& matrix, std::array<uint32_t, 6>& solution )
	{
		uint32_t best_cost = 0u, current_cost = 0u;
		uint32_t num_elements = solution[5];
		uint32_t num_combs = ( _multiplicity * ( _multiplicity - 1u ) ) / 2;
		bool improved = false;

		for ( uint32_t i = 0u; i < num_elements; ++i )
		{
			best_cost += matrix[solution[i]].cost;
		}

		uint64_t column0 = 0u, column1 = 0u;
		/* for each currently selected column, look for a lower-cost alternative */
		for ( uint32_t i = 0u; i < num_elements; ++i )
		{
			// current_cost = 0u;
			current_cost = best_cost - matrix[solution[i]].cost;
			column0 = 0u;
			column1 = 0u;

			for ( uint32_t j = 0u; j < num_elements; ++j )
			{
				if ( j == i )
				{
					continue;
				}
				// current_cost += matrix[solution[j]].cost;
				column0 |= matrix[solution[j]].column[0];
				column1 |= matrix[solution[j]].column[1];
			}

			for ( uint32_t j = 0u; j < matrix.size(); ++j )
			{
				if ( ( __builtin_popcountl( column0 | matrix[j].column[0] ) + __builtin_popcountl( column1 | matrix[j].column[1] ) ) != num_combs )
				{
					continue;
				}

				if ( ( current_cost + matrix[j].cost ) < best_cost )
				{
					solution[i] = j;
					best_cost = current_cost + matrix[j].cost;
					improved = true;
				}
			}
		}

		return improved;
	}
	
	void compute_top_lut()
	{
		// const uint32_t num_top_vars = _bound_set.size() + _free_set_size;
    const uint32_t num_top_vars = _decomp_res.size() + _free_set_size;
		assert( num_top_vars <= _ps.lut_size );

    /* for debugging */
    fmt::print( "[m] Top LUT support size is {}\n", num_top_vars );

		kitty::dynamic_truth_table tt( num_top_vars );
		decomp_result decomp_res;
		for ( uint32_t i = 0u; i < _free_set_size; ++i )
		{
			decomp_res.support.push_back( _perm[i] );
		}

		std::vector<kitty::dynamic_truth_table> bs_vars;
		uint32_t offset = 0u;
    std::vector<uint32_t> ss_ind;
		for ( uint32_t i = 0u; i < _decomp_res.size(); ++i )
		{
			decomp_result const& decomp_res_i = _decomp_res[i];
			bs_vars.emplace_back( num_top_vars );
			kitty::create_nth_var( bs_vars[i], _free_set_size + i );

      /* for debugging */
      fmt::print( "[m] The {}th BS function's support size is {}\n", ( i + 1 ), decomp_res_i.support.size() );

			/* Add BS variables to the support of `decomp_res` */
			/* remove trivial BS function (buffer) */
			if ( decomp_res_i.support.size() == 1u )
			{
				decomp_res.support.push_back( decomp_res_i.support[0] );
				if ( decomp_res_i.tt._bits[0] & 1 )
				{
					bs_vars[i] = ~bs_vars[i];
				}
				// _decomp_res.erase( _decomp_res.begin() + i );
        ss_ind.push_back( i );
        ++offset;

        /* for debugging */
        fmt::print( "[m] Has a trivial BS function, removed.\n" );
			}
			else
			{
				decomp_res.support.push_back( _num_vars + i - offset );
			}
		}

    for ( auto it = ss_ind.rbegin(); it != ss_ind.rend(); ++it )
    {
      _decomp_res.erase( _decomp_res.begin() + *it );
    }

		for ( uint32_t i = 0u; i < _free_set.size(); ++i )
		{
			kitty::dynamic_truth_table fs_tt = kitty::shrink_to( _free_set[i], num_top_vars );
			
			for ( uint32_t j = 0u; j < bs_vars.size(); ++j )
			{
				if ( ( _isets_onset[j] >> i ) & 1 )
				{
					fs_tt &= bs_vars[j];
					continue;
				}
				if ( ( _isets_offset[j] >> i ) & 1 )
				{
					fs_tt &= ~bs_vars[j];
				}
			}

			tt |= fs_tt;
		}

		decomp_res.tt = tt;
		_decomp_res.push_back( decomp_res );

    /* for debugging */
    fmt::print( "[m] Supports of the top LUT are the " );
    for ( auto const& each_sup : decomp_res.support )
    {
      fmt::print( "{}th ", each_sup );
    }
    fmt::print( "signals\n" );

    /* for debugging */
    // if ( ( _decomp_res.size() - 1 ) != _bound_set.size() )

    // uint32_t top_vars = _bound_set.size() + _free_set_size;
    // assert( top_vars <= _ps.lut_size );

    // /* extend bound set functions with free_set_size LSB vars */
    // kitty::dynamic_truth_table tt( top_vars );

    // /* compute support */
    // _decomp_res.emplace_back();
    // for ( uint32_t i = 0; i < _free_set_size; ++i )
    // {
    //   _decomp_res.back().support.push_back( _perm[i] );
    // }

    // /* create functions for bound set */
    // std::vector<kitty::dynamic_truth_table> bound_set_vars;
    // auto res_it = _decomp_res.begin();
    // uint32_t offset = 0;
    // for ( uint32_t i = 0; i < _bound_set.size(); ++i )
    // {
    //   bound_set_vars.emplace_back( top_vars );
    //   kitty::create_nth_var( bound_set_vars[i], _free_set_size + i );

    //   /* add bound-set variables to the support, remove buffers (shared set) */
    //   if ( res_it->support.size() == 1 )
    //   {
    //     _decomp_res.back().support.push_back( res_it->support.front() );
    //     /* it is a NOT */
    //     if ( ( res_it->tt._bits[0] & 1 ) == 1 )
    //     {
    //       bound_set_vars[i] = ~bound_set_vars[i];
    //     }
    //     _decomp_res.erase( res_it );
    //     ++offset;
    //   }
    //   else
    //   {
    //     _decomp_res.back().support.push_back( _num_vars + i - offset );
    //     ++res_it;
    //   }
    // }

    // /* create composition function */
    // for ( uint32_t i = 0; i < _free_set.size(); ++i )
    // {
    //   kitty::dynamic_truth_table free_set_tt = kitty::shrink_to( _free_set[i], top_vars );

    //   /* find MUX assignments */
    //   for ( uint32_t j = 0; j < bound_set_vars.size(); ++j )
    //   {
    //     /* AND with ONSET or OFFSET */
    //     if ( ( ( _isets_onset[j] >> i ) & 1 ) )
    //     {
    //       free_set_tt &= bound_set_vars[j];
    //     }
    //     else if ( ( ( _isets_offset[j] >> i ) & 1 ) )
    //     {
    //       free_set_tt &= ~bound_set_vars[j];
    //     }
    //   }

    //   tt |= free_set_tt;
    // }

    // /* add top-level LUT to result */
    // _decomp_res.back().tt = tt;
	}
private:
	acdecomp_max_local_params _ps;
	TT _tt_res;
	kitty::dynamic_truth_table const& _tt_orig;
	uint32_t _num_vars = 0u;
	std::array<uint32_t, max_num_vars> _perm;
	uint32_t _multiplicity = UINT32_MAX;
	uint32_t _free_set_size = UINT32_MAX; /* FS support size */
	std::vector<TT> _free_set; /* FS functions */
	std::vector<TT> _bound_set; /* BS functions */
	std::vector<TT> _care_set;
	std::vector<uint64_t> _isets_onset;
	std::vector<uint64_t> _isets_offset;
	std::vector<std::array<uint32_t, 2u>> _min_support_encodings;
public:
	std::vector<decomp_result> _decomp_res;
};

} /* namespace mockturtle */