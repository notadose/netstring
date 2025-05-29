#include <functional>
#include <tins/tins.h>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>

#include "sniffer.hpp"

std::function<void()> nextOnPacket;


const unsigned int MAX_PACKETS = 1024;

Tins::IP::address_type localAddress;

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

std::atomic<bool> shouldStop(false);

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

  return !shouldStop.load();
}

std::unique_ptr<Tins::Sniffer> sniffer = nullptr;
std::thread sniffThread;

void Netstring::Sniffer::start_sniffing(
  std::string iface_name,
  std::function<void()> on_packet
){
  shouldStop = false;
  nextOnPacket = on_packet;
  if(sniffer != nullptr) return;

  sniffThread = std::thread([&]() -> void {
    Tins::NetworkInterface iface = iface_name;
    Tins::NetworkInterface::Info info = iface.info();

    localAddress = info.ip_addr;

    Tins::SnifferConfiguration config;
    config.set_immediate_mode(true);

    sniffer = std::make_unique<Tins::Sniffer>(iface_name, config);
    sniffer->sniff_loop(packet_callback);
  });
}

void Netstring::Sniffer::stop_sniffing(){
  if(!sniffer) return;
  shouldStop = true;

  sniffer->stop_sniff();

  if(sniffThread.joinable()){
    sniffThread.join();
  }

  sniffer.reset();
  //stop_sniff() should kill sniff_thread
}

const std::vector<Tins::IP::address_type> &Netstring::Sniffer::getKnownHosts(){
  return knownHosts;
}

const std::deque<Netstring::Sniffer::packetInfo> &Netstring::Sniffer::getPackets(){
  return packets;
}

const Tins::IP::address_type &Netstring::Sniffer::getLocalAddress(){
  return localAddress;
}
