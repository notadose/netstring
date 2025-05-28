#pragma once
#include <string>
#include <tins/tins.h>
#include <functional>
#include <deque>

namespace Netstring::Sniffer{
  struct packetInfo{
    Tins::IP::address_type src;
    Tins::IP::address_type dest;
  };

  void start_sniffing(std::string iface_name, std::function<void()> on_packet);
  const std::vector<Tins::IP::address_type> &getKnownHosts();
  const std::deque<packetInfo> &getPackets();
}
