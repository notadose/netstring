#include <chafa.h>
#include <cairo.h>
#include <math.h>
#include <stdexcept>
#include <thread>

#include "mapwindow.hpp"
#include "sniffer.hpp"
#include "tui.hpp"

void onPacket(){
  // std::cout << "src: " << packets[0].src << '\n';
  // std::cout << "dst: " << packets[0].dest << '\n';
  // std::cout << std::endl;

  Netstring::Map::onPacket();

  GString *str = Netstring::Map::getMapString();

  /* Print the string */
  // fwrite(str->str, sizeof(char), str->len, stdout);
  // fputc('\n', stdout);

  Netstring::Tui::update();

  g_string_free(str, TRUE);
}

int main(int argc, char *argv[]) {
  if(argc < 2){
    throw std::invalid_argument("no interface specified");
  }

  Netstring::Sniffer::start_sniffing(argv[1], onPacket);

  Netstring::Tui::run();

  Netstring::Sniffer::stop_sniffing();

  return 0;
}
