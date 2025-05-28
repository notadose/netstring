#include <chafa.h>
#include <cairo.h>
#include <math.h>
#include <stdexcept>
#include <iostream>

#include "mapwindow.hpp"
#include "sniffer.hpp"

void onPacket(){
  const std::deque<Netstring::Sniffer::packetInfo> &packets =
    Netstring::Sniffer::getPackets();

  // std::cout << "src: " << packets[0].src << '\n';
  // std::cout << "dst: " << packets[0].dest << '\n';
  // std::cout << std::endl;

  Netstring::Map::onPacket();
}

int main(int argc, char *argv[]) {
  if(argc < 2){
    throw std::invalid_argument("no interface specified");
  }

  Netstring::Sniffer::start_sniffing(argv[1], onPacket);

  return 0;
}
