#include <ncurses.h>
#include <atomic>

#include "tui.hpp"
#include "mapwindow.hpp"

WINDOW *mapwindow;

std::atomic<bool> running(false);

void Netstring::Tui::run(){ // Start the TUI, blocking until the program should exit
  initscr();
  noecho();
  cbreak();

  mapwindow = newwin(LINES, COLS, 1, 1);

  Netstring::Map::setMapSize(getmaxx(mapwindow) - 4, getmaxy(mapwindow) - 4);

  refresh();

  running = true;

  getch();

  endwin();
}

void Netstring::Tui::update(){
  if(!running.load()) return;

  Netstring::Map::setMapSize(getmaxx(mapwindow) - 4, getmaxy(mapwindow) - 4);
  
  werase(mapwindow);

  refresh();

  printf("\033[%i;%iH", getcurx(mapwindow), getcury(mapwindow));
  fflush(stdout);
  Netstring::Map::drawMap();

}
