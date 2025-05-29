#pragma once

#include <ncurses.h>

namespace Netstring::Tui{
  void run(); // Open TUI and block until it is closed
  void update();
}
