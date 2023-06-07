#pragma once

#include <cstdint>

#include <mockturtle/networks/xag.hpp>
#include <mockturtle/views/topo_view.hpp>

namespace mockturtle
{

void determine_and_group( xag_network& xag, xag_network::node const& n, 
													xag_network::node const& root )
{
	if ( xag.value( n ) == 0u ) 
	{
		if ( n != root )
		{
			/* remark this node as traversed */
			xag.incr_value( n );
			/* update the size of this AND group */
			xag.incr_value( root );
		}
		else
		{
			xag.set_value( n, 2u );
		}

		/* recursively trace its fanin */
		xag.foreach_fanin( n, [&]( auto const& f ) {
			auto const child = xag.get_node( f );
			if ( xag.is_and( child ) && !xag.is_complemented( f ) && ( xag.fanout_size( child ) == 1u ) )
			{
				determine_and_group( xag, child, root );
			}
		} );
	}
}

uint32_t merge_split_xag( xag_network const& xag )
{
	uint32_t t_count{ 0u };
	topo_view<xag_network> xag_topo{ xag };
	xag_topo.clear_values();

	xag_topo.foreach_gate_reverse( [&]( auto const& n ) {
		if ( xag_topo.is_and( n ) && xag_topo.value( n ) == 0u )
		{
			determine_and_group( xag_topo, n, n );
			uint32_t fanin_size = xag_topo.value( n );
			while ( fanin_size > 2u )
			{
				t_count += 6u;
				fanin_size -= 2u;
			}
			if ( fanin_size > 1u )
			{
				t_count += 4u;
				--fanin_size;
			}
		}
	} );

	xag_topo.clear_values();
	return t_count;
}


} /* namespace mockturtle */