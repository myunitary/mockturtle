#pragma once

#include "../utils/node_map.hpp"
#include "topo_view.hpp"

#include <cstdlib>

namespace mockturtle
{

template<typename Ntk>
class ownership_view : public Ntk
{
public:
  using storage = typename Ntk::storage;
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

  explicit ownership_view( Ntk const& ntk, uint32_t num_parties = 2u )
    : Ntk( ntk ),
      _num_parties( num_parties )
  {
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( has_is_and_v<Ntk>, "Ntk does not implement the is_and method" );
    static_assert( has_is_dead_v<Ntk>, "Ntk does not implement the is_dead method" );
    static_assert( has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
    static_assert( has_num_pis_v<Ntk>, "Ntk does not implement the num_pis method" );

    assert( _num_parties <= 32u );

    init_ownership();
  }

  ownership_view() = default;

  void init_ownership() const
  {
    std::for_each( this->_storage->nodes.begin(), this->_storage->nodes.end(), [this]( auto& n ) {
      n.data[1].h1 = 0u;
    } );
  }

  void assign_ownership( std::vector<uint32_t> pis_ownership = {} ) const
  {
    const uint32_t num_pis = this->num_pis();
    uint32_t party_ind = 0u, threshold = 0u;

    if ( pis_ownership.empty() )
    {
      /* default PI ownership assignment */
      fmt::print( "[i] Primary input ownership unavaialable. " );
      fmt::print( "Assuming a symmetric {}-party setting...\n", _num_parties );

      threshold = num_pis / _num_parties;
      uint32_t index = 0u;
      std::for_each( this->_storage->inputs.begin(), this->_storage->inputs.end(), [&]( auto& n ) {
        if ( ( index % threshold == 0u ) && ( index != 0u ) )
        {
          ++party_ind;
        }
        this->_storage->nodes[n].data[1].h1 = ( 1 << party_ind );
        ++index;
      } );
    }
    else
    {
      /* user-specified PI ownership assignment */
      /* [3,1,2] */
      uint8_t num_parties = pis_ownership.size();
      assert( num_parties <= 32u );
      threshold = pis_ownership[0];

      uint32_t index = 0u;
      std::for_each( this->_storage->inputs.begin(), this->_storage->inputs.end(), [&]( auto& n ) {
        if ( index == threshold )
        {
          if ( party_ind < num_parties - 1 )
          {
            ++party_ind;
            threshold += pis_ownership[party_ind];
          }
          else
          {
            fmt::print( "[i] The user-specified PI ownership is incomplete. " );
            fmt::print( "Assigning the remaining PIs to the last party...\n" );
            threshold = num_pis;
          }
        }
        this->_storage->nodes[n].data[1].h1 = ( 1 << party_ind );
        ++index;
      } );
      
      if ( threshold > num_pis )
      {
        fmt::print( "[i] The user-specified PI ownership does not match the PI set size. " );
        fmt::print( "Redudant specification is omitted.\n" );
      }
    }

    propogate_ownership();
  }

  void propogate_ownership() const
  {
    auto r = range<uint64_t>( 1u, this->_storage->nodes.size() );
    detail::foreach_element_if( r.begin(), r.end(), [this] ( auto const& n ) { return !this->is_pi( n ) && !this->is_dead( n ); }, [this]( auto const& n ) {
      this->_storage->nodes[n].data[1].h1 |= get_ownership( this->_storage->nodes[n].children[0] );
      this->_storage->nodes[n].data[1].h1 |= get_ownership( this->_storage->nodes[n].children[1] );
    } );
  }

  void assign_ownership_local( node const& n, uint32_t ownership ) const
  {
    this->_storage->nodes[n].data[1].h1 = ownership;
  }

  void propogate_ownership_local( node const& n ) const
  {
    this->_storage->nodes[n].data[1].h1 = 0u;
    this->_storage->nodes[n].data[1].h1|= get_ownership( this->_storage->nodes[n].children[0] );
    this->_storage->nodes[n].data[1].h1 |= get_ownership( this->_storage->nodes[n].children[1] );
  }

  uint32_t get_ownership( node const& n ) const
  {
    return this->_storage->nodes[n].data[1].h1;
  }

  uint32_t get_ownership( signal const& f ) const
  {
    return get_ownership( this->get_node( f ) );
  }

  uint32_t is_local( node const& n ) const
  {
    const uint32_t ownership = get_ownership( n );
    if ( __builtin_popcount( ownership ) > 1u )
    {
      return 0u;
    }
    else if ( __builtin_popcount( ownership ) != 1u )
    {
      fmt::print( "[e] Something is wrong with ownership assignment!\n" );
      abort();
    }
    return ( __builtin_ctz( ownership ) + 1u );
  }

  signal create_and( signal a, signal b )
  {
    if ( a.index > b.index )
    {
      std::swap( a, b );
    }
    if ( a.index == b.index )
    {
      return ( ( a.complement == b.complement ) ? a : this->get_constant( false ) );
    }
    else if ( a.index == 0u )
    {
      return ( ( a.complement == false ) ? this->get_constant( false ) : b );
    }

    signal output = this->_create_node( a, b );
    propogate_ownership_local( this->get_node( output ) );
    
    return output;
  }

  signal create_xor( signal a, signal b )
  {
    if ( a.index < b.index )
    {
      std::swap( a, b );
    }

    bool f_compl = a.complement != b.complement;
    a.complement = b.complement = false;

    if ( a.index == b.index )
    {
      return this->get_constant( f_compl );
    }
    else if ( b.index == 0 )
    {
      return a ^ f_compl;
    }

    signal output = this->_create_node( a, b ) ^ f_compl;
    propogate_ownership_local( this->get_node( output ) );

    return output;
  }

  signal clone_node( ownership_view<Ntk> const& other, node const& source, std::vector<signal> const& children )
  {
    assert( children.size() == 2u );
    if ( other.is_and( source ) )
    {
      return create_and( children[0u], children[1u] );
    }
    else
    {
      return create_xor( children[0u], children[1u] );
    }
  }

private:
  uint32_t _num_parties;
};

template<typename Ntk>
class owner_view : public Ntk
{
public:
  using storage = typename Ntk::storage;
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

  explicit owner_view( uint32_t num_parties = 2u )
    : Ntk(),
      _num_parties( num_parties ),
      _ownerships( *this )
  {
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_size_v<Ntk>, "Ntk does not implement the size method" );
    static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
    static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
    static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );

    assert( _num_parties <= 32u );

    add_event = Ntk::events().register_add_event( [this]( auto const& n ) { on_add( n ); } );
  }

  /* `per_pi_ownership` controls how the passed `pis_ownership` shall be interpreted: */
  /* If is false, the i-th entry indicates "the number of pis belonging to the i-th party"; */
  /* If is true, the i-th entry indicates "the ownership of the i-th pi." */
  explicit owner_view( Ntk const& ntk, uint32_t num_parties = 2u, std::vector<uint32_t> pis_ownership = {}, bool per_pi_ownership = false )
    : Ntk( ntk ),
      _num_parties( num_parties ),
      _ownerships( ntk )
  {
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_size_v<Ntk>, "Ntk does not implement the size method" );
    static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
    static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
    static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );

    assert( _num_parties <= 32u );

    init_ownership();
    if ( !per_pi_ownership )
    {
      assign_ownership( pis_ownership );
    }
    else
    {
      assign_ownership_per_pi( pis_ownership );
    }
    
    add_event = Ntk::events().register_add_event( [this]( auto const& n ) { on_add( n ); } );
  }

  owner_view( owner_view<Ntk> const& other )
    : Ntk( other ),
      _num_parties( other._num_parties ),
      _ownerships( other._ownerships )
  {
    add_event = Ntk::events().register_add_event( [this]( auto const& n ) { on_add( n ); } );
  }

  owner_view<Ntk>& operator=( owner_view<Ntk> const& other )
  {
    Ntk::events().release_add_event( add_event );
    this->_num_parties = other._num_parties;
    this->_ownerships = other._ownerships;
    this->_storage = other._storage;
    this->_events = other._events;
    add_event = Ntk::events().register_add_event( [this]( auto const& n ) { on_add( n ); } );

    return *this;
  }

  ~owner_view()
  {
    Ntk::events().release_add_event( add_event );
  }

  Ntk toNtk() const
  {
    Ntk ntk;
    node_map<typename Ntk::signal, owner_view<Ntk>> old_to_new{ *this };
    old_to_new[this->get_constant( false )] = ntk.get_constant( false );
    this->foreach_pi( [&]( auto const& pi ) {
      auto new_pi = ntk.create_pi();
      old_to_new[pi] = new_pi;
    } );
    topo_view{ *this }.foreach_gate( [&]( auto const& n, auto index ) {
      /* this implementation is dedicated for 2-in-degree DAGs */
      std::vector<signal> children_old = { this->_storage->nodes[n].children[0], this->_storage->nodes[n].children[1] };
      std::vector<typename Ntk::signal> children_new = { ntk.get_constant( false ), ntk.get_constant( false ) };
      children_new[0] = this->is_complemented( children_old[0] ) ? ntk.create_not( old_to_new[children_old[0]] ) : old_to_new[children_old[0]];
      children_new[1] = this->is_complemented( children_old[1] ) ? ntk.create_not( old_to_new[children_old[1]] ) : old_to_new[children_old[1]];
      old_to_new[n] = ntk.clone_node( *this, n, children_new);
    } );
    this->foreach_po( [&]( auto const& po ) {
      ntk.create_po( this->is_complemented( po ) ? ntk.create_not( old_to_new[this->get_node( po )] ) : old_to_new[this->get_node( po )] );
    } );

    return ntk;
  }

  void init_ownership()
  {
    _ownerships.reset( 0 );
  }

  void assign_ownership( std::vector<uint32_t> pis_ownership )
  {
    this->incr_trav_id();
    const uint32_t num_pis = this->num_pis();
    uint32_t party_ind = 0u, threshold = 0u;

    if ( pis_ownership.empty() )
    {
      /* default PI ownership assignment */
      fmt::print( "[i] Primary input ownership unavaialable. " );
      fmt::print( "Assuming a symmetric {}-party setting...\n", _num_parties );

      threshold = num_pis / _num_parties;
      this->foreach_pi( [&]( auto const& pi, auto index ) {
        if ( ( index % threshold == 0u ) && ( index != 0u ) )
        {
          ++party_ind;
        }
        assign_ownership_local( pi, ( 1 << party_ind ) );

        /* for debugging */
        // fmt::print( "[m] Assigning ownership {} to PI {}...\n", ( 1 << party_ind ), ( index + 1 ) );
      } );
    }
    else
    {
      /* user-specified PI ownership assignment */
      /* [3,1,2] */
      assert( _num_parties == pis_ownership.size() );
      threshold = pis_ownership[0];

      this->foreach_pi( [&]( auto const& pi, auto index ) {
        if ( index == threshold )
        {
          if ( party_ind < _num_parties - 1 )
          {
            ++party_ind;
            threshold += pis_ownership[party_ind];
          }
          else
          {
            fmt::print( "[i] The user-specified PI ownership is incomplete. " );
            fmt::print( "Assigning the remaining PIs to the last party...\n" );
            threshold = num_pis;
          }
        }
        assign_ownership_local( pi, ( 1 << party_ind ) );
      } );
      
      if ( threshold > num_pis )
      {
        fmt::print( "[i] The user-specified PI ownership does not match the PI set size. " );
        fmt::print( "Redudant specification is omitted.\n" );
      }
    }

    if ( this->num_pos() > 0u )
    {
      /* a complete network */
      propogate_ownership();
    }
    else
    {
      /* an under-construction network */
      propogate_ownership_interm();
    }  
  }

  void assign_ownership_per_pi( std::vector<uint32_t> pis_ownership )
  {
    this->incr_trav_id();
    if ( pis_ownership.size() != this->num_pis() )
    {
      fmt::print( "[e] During per-PI ownership assignment, given {} os, while there are {} pis...\n", pis_ownership.size(), this->num_pis() );
    }
    assert( pis_ownership.size() == this->num_pis() );
    this->foreach_pi( [&]( auto const& pi, auto index ) {
      assign_ownership_local( pi, pis_ownership[index] );
    } );

    if ( this->num_pos() > 0u )
    {
      propogate_ownership();
    }
    else
    {
      propogate_ownership_interm();
    }
  }

  void propogate_ownership()
  {
    topo_view<Ntk>( *this ).foreach_node( [&]( auto const& n ) {
      if ( this->is_constant( n ) || this->is_pi( n ) )
      {
        return true;
      }
      propogate_ownership_local( n );
      return true;
    } );
  }

  /* ownership propogation approach for intermediate (i.e., under construction) network */
  /* thus, it's assumed that the incomplete network is inherently topological */
  void propogate_ownership_interm()
  {
    this->foreach_node( [this]( auto const& n ) {
      if ( this->is_constant( n ) || this->is_pi( n ) )
      {
        return true;
      }

      uint32_t ownership = 0u;
      this->foreach_fanin( n, [&]( auto const& fi ) {
        ownership |= this->get_ownership( this->get_node( fi ) );
      } );

      this->assign_ownership_local( n, ownership );
      return true;
    } );
  }

  void propogate_ownership_local( node const& n )
  {
    uint32_t ownership = 0u;
    this->foreach_fanin( n, [&]( auto const& fi ) {
      ownership |= this->get_ownership( this->get_node( fi ) );
    } );
    this->assign_ownership_local( n, ownership );
  }

  void assign_ownership_local( node const& n, uint32_t ownership )
  {
    _ownerships[n] = ownership;
  }

  uint32_t get_ownership( node const& n ) const
  {
    return _ownerships[n];
  }

  uint32_t get_ownership( signal const& f ) const
  {
    return get_ownership( this->get_node( f ) );
  }

  uint32_t is_local( node const& n ) const
  {
    const uint32_t ownership = get_ownership( n );
    if ( __builtin_popcount( ownership ) > 1u )
    {
      return 0u;
    }
    // else if ( __builtin_popcount( ownership ) != 1u )
    // {
    //   fmt::print( "[e] Something is wrong with ownership assignment!\n" );
    //   abort();
    // }
    return ( __builtin_ctz( ownership ) + 1u );
  }

  void on_add( node const& n )
  {
    _ownerships.resize();

    uint32_t ownership = 0u;
    this->foreach_fanin( n, [&]( auto const& fi ) {
      ownership |= this->get_ownership( this->get_node( fi ) );
    } );

    this->assign_ownership_local( n, ownership );
  }

  void profile_ntk()
  {
    fmt::print( "[m] Profiling current network:\n" );
    this->foreach_pi( [this]( auto const& pi, auto i ) {
      fmt::print( "PI{}({}) ", ( i + 1 ), _ownerships[pi] );
    } );
    fmt::print( "\n" );
    this->foreach_gate( [this]( auto const& n ) {
      fmt::print( "n{}({}) ", this->node_to_index( n ), _ownerships[n] );
    } );
    fmt::print( "\n" );
  }

  void resize_ownerships()
  {
    _ownerships.resize();
  }

private:
  uint32_t _num_parties;
  node_map<uint32_t, Ntk> _ownerships;
  std::shared_ptr<typename network_events<Ntk>::add_event_type> add_event;
}; 

// template<class Ntk>
// owner_view( Ntk const& ) -> owner_view<Ntk>;

// template<typename Ntk>
// class enhanced_owner_view : public Ntk
// {
// public:
//   static constexpr size_t MAX_PARTIES = 32;
//   using ownership_count_t = std::array<uint32_t, MAX_PARTIES>;
//   using ownership_t = std::unordered_map<uint8_t, std::unordered_set<uint32_t>>;
//   using storage = typename Ntk::storage;
//   using node = typename Ntk::node;
//   using signal = typename Ntk::signal;

//   explicit enhanced_owner_view()
//     : Ntk(),
//       _ownerships( *this ),
//       _ownerships_count( *this )
//   {
//     static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
//     static_assert( has_size_v<Ntk>, "Ntk does not implement the size method" );
//     static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
//     static_assert( has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
//     static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
//     static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );

//     add_event = Ntk::events().register_add_event( [this]( auto const& n ) { on_add( n ); } );
//   }

// private:
//   node_map<ownership_t, Ntk> _ownerships;
//   node_map<ownership_count_t, Ntk> _ownerships_count;
//   std::shared_ptr<typename network_events<Ntk>::add_event_type> add_event;
// };

} /* namespace mockturtle */