#include <functional>
#include <tins/tins.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>

#include "sniffer.hpp"

std::function<void()> nextOnPacket;


const unsigned int MAX_PACKETS = 1024;

std::mutex packetsMutex; // For packets, knownHosts, and hostKnownMap
std::deque<Netstring::Sniffer::packetInfo> packets;
// First packets in deque should be the newest
std::vector<Tins::IP::address_type> knownHosts;

std::unordered_map<Tins::IP::address_type, bool> hostKnownMap;

void handleHost(const Tins::IP::address_type &host){
  if(hostKnownMap.find(host) != hostKnownMap.end()){
    return;
  }
  hostKnownMap[host] = true;
  knownHosts.push_back(host);
}

bool packet_callback(const Tins::PDU &pdu){
  const Tins::IP *ip_layer = pdu.find_pdu<Tins::IP>();
  if(ip_layer == 0) return true; // TODO: IPV6 in the future

  const Tins::IP::address_type source_addr = ip_layer->src_addr();
  const Tins::IP::address_type dest_addr = ip_layer->dst_addr();

  handleHost(source_addr);
  handleHost(dest_addr);

  Netstring::Sniffer::packetInfo info = {
    .src = source_addr,
    .dest = dest_addr
  };

  std::unique_lock<std::mutex> lock(packetsMutex);

  packets.push_front(info);
  if(packets.size() > MAX_PACKETS){
    packets.pop_back();
  }

  nextOnPacket();

  lock.unlock();

  return true;
}

void Netstring::Sniffer::start_sniffing(
  std::string iface_name,
  std::function<void()> on_packet
){
  nextOnPacket = on_packet;
  Tins::NetworkInterface iface;
  Tins::NetworkInterface::Info info;

  Tins::SnifferConfiguration config;
  config.set_immediate_mode(true);

  Tins::Sniffer(iface_name, config).sniff_loop(packet_callback);

  std::cout << "exit" << std::endl;
}

const std::vector<Tins::IP::address_type> &Netstring::Sniffer::getKnownHosts(){
  return knownHosts;
}

const std::deque<Netstring::Sniffer::packetInfo> &Netstring::Sniffer::getPackets(){
  return packets;
}
