#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";
 
  RouteEntry entry {
    .prefix = route_prefix,
    .prefix_length = prefix_length,
    .next_hop = next_hop,
    .interface_num = interface_num
  };
  insert_route(entry);
}
void Router::insert_route(const RouteEntry &entry) {
  TrieNode *cur = &_root;

  for (uint8_t i = 0; i < entry.prefix_length; i++) {
    uint8_t bit = (entry.prefix >> (32 - i - 1)) & 1;
    if (!cur->child[bit]) {
      cur->child[bit] = std::make_unique<TrieNode>();
    }
    cur = cur->child[bit].get();
  }
  cur->route_info = entry;
}

std::optional<RouteEntry> Router::find_longest_prefix_match(uint32_t dst) const {
  const TrieNode *cur = &_root;
  std::optional<RouteEntry> best_match = std::nullopt;

  for (int i = 0; i < 32; i++) {
    if (cur->route_info.has_value()) {
      best_match = cur->route_info;
    }
    uint8_t bit = (dst >> (31 - i)) & 1;
    if (!cur->child[bit]) {
      break;
    }
    cur = cur->child[bit].get();
  }

  if (cur->route_info.has_value()) {
    best_match = cur->route_info;
  }

  return best_match;
}

void Router::route()
{

  for (auto &iface : _interfaces) {
    auto &incoming = iface->datagrams_received();

    while (!incoming.empty()) {
      InternetDatagram datagram = std::move(incoming.front());
      incoming.pop();

      if (datagram.header.ttl <= 1) {
        continue;
      }

      datagram.header.ttl -= 1;
      datagram.header.compute_checksum();

      uint32_t dst_addr = datagram.header.dst;
      auto route_opt = find_longest_prefix_match(dst_addr);

      if (!route_opt.has_value()) 
      {
        continue;
      }

      const auto &route = route_opt.value();
      Address next_hop_addr = route.next_hop.has_value()? route.next_hop.value(): Address::from_ipv4_numeric(dst_addr);
      interface(route.interface_num)->send_datagram(datagram, next_hop_addr);
    }
  }
}
