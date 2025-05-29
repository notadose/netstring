#include <chafa.h>
#include <stdio.h>
#include <cairo.h>
#include <math.h>
#include <tins/tins.h>
#include <mutex>
#include <shared_mutex>

#include "mapwindow.hpp"
#include "sniffer.hpp"

struct color{
  float r;
  float g;
  float b;
  float a;
};

const color HOSTS_COLOR = color{
  .r=1.0,.g=1.0,.b=1.0,.a=1.0
};

const color LOCALHOST_COLOR = color{
  .r=1.0,.g=0.0,.b=0.0,.a=1.0
};

const color LINE_COLOR = color{
  .r=1.0,.g=0.0,.b=0.0,.a=1.0
};

const color BG_COLOR = color{
  .r=0.0,.g=0.0,.b=0.0,.a=1.0
};

void getHostLocation(Tins::IP::address_type &host, float *loc_x, float *loc_y){
  unsigned seed = 0;
  size_t leastsize = sizeof(host);
  if(sizeof(seed) < leastsize){
    leastsize = sizeof(seed);
  }
  std::memcpy(&seed, &host, leastsize);
  std::srand(seed);

  *loc_x = float(std::rand()) / RAND_MAX;
  *loc_y = float(std::rand()) / RAND_MAX;
}

guint16 map_term_width = 20;
guint16 map_term_height = 20;

void Netstring::Map::setMapSize(guint16 width, guint16 height){
  map_term_width = width;
  map_term_height = height;
}

void Netstring::Map::getMapSize(guint16 *wid_ptr, guint16 *hgt_ptr){
  *wid_ptr = map_term_width;
  *hgt_ptr = map_term_height;
}

std::shared_mutex stringMutex;
GString *mapString = nullptr;

void Netstring::Map::onPacket() {
  std::deque<Netstring::Sniffer::packetInfo> packets = Netstring::Sniffer::getPackets();
  std::vector<Tins::IP::address_type> hosts = Netstring::Sniffer::getKnownHosts();

  const int PIX_WIDTH = (map_term_width * 400)/23;
  const int PIX_HEIGHT = (map_term_height * 400)/10;
  const int N_CHANNELS = 4;

  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32,
    PIX_WIDTH,
    PIX_HEIGHT
  );

  cairo_t *cr = cairo_create(surface);
  cairo_set_source_rgba(cr, BG_COLOR.g, BG_COLOR.b, BG_COLOR.r, BG_COLOR.a);

  cairo_paint(cr); // Fill the screen with the bg color

  float src_xpos;
  float src_ypos;

  float dest_xpos;
  float dest_ypos;

  getHostLocation(packets[0].src, &src_xpos, &src_ypos);
  getHostLocation(packets[0].dest, &dest_xpos, &dest_ypos);

  cairo_set_source_rgba(cr, LINE_COLOR.g, LINE_COLOR.b, LINE_COLOR.r, LINE_COLOR.a);
  cairo_move_to(cr, PIX_WIDTH*src_xpos, PIX_HEIGHT*src_ypos);
  cairo_set_line_width(cr, 2.0);
  cairo_line_to(cr, PIX_WIDTH*dest_xpos, PIX_HEIGHT*dest_ypos);
  cairo_stroke(cr);

  std::for_each(hosts.begin(), hosts.end(), [&](Tins::IP::address_type host) -> void{
    float xpos;
    float ypos;

    getHostLocation(host, &xpos, &ypos);

    cairo_set_source_rgba(cr, HOSTS_COLOR.g, HOSTS_COLOR.b, HOSTS_COLOR.r, HOSTS_COLOR.a);

    if(host == Netstring::Sniffer::getLocalAddress()){
      cairo_set_source_rgba(cr, LOCALHOST_COLOR.g, LOCALHOST_COLOR.b, LOCALHOST_COLOR.r, LOCALHOST_COLOR.a);
    }
    
    cairo_arc(cr, PIX_WIDTH*xpos, PIX_HEIGHT*ypos, 8, 0, 2.0*M_PI);

    cairo_fill(cr);
  });

  cairo_destroy(cr);
  cairo_surface_flush(surface);

  const guint8 *pixels = cairo_image_surface_get_data(surface);

  /* Specify the symbols we want */
  ChafaSymbolMap *symbol_map = chafa_symbol_map_new();
  chafa_symbol_map_add_by_tags(symbol_map, CHAFA_SYMBOL_TAG_ALL);

  /* Set up a configuration with the symbols and the canvas size in characters
   */
  ChafaCanvasConfig *config = chafa_canvas_config_new();
  chafa_canvas_config_set_symbol_map(config, symbol_map);
  chafa_canvas_config_set_dither_mode(config, CHAFA_DITHER_MODE_NONE);
  chafa_canvas_config_set_color_extractor(config, CHAFA_COLOR_EXTRACTOR_MEDIAN);
  chafa_canvas_config_set_pixel_mode(config, CHAFA_PIXEL_MODE_KITTY);
  chafa_canvas_config_set_canvas_mode(config, CHAFA_CANVAS_MODE_FGBG);
  chafa_canvas_config_set_geometry(config, map_term_width, map_term_height);

  /* Create canvas */
  ChafaCanvas *canvas = chafa_canvas_new(config);

  /* Draw pixels to canvas */
  chafa_canvas_draw_all_pixels(canvas, CHAFA_PIXEL_RGBA8_UNASSOCIATED, pixels,
                               PIX_WIDTH, PIX_HEIGHT, PIX_WIDTH * N_CHANNELS);

  std::unique_lock mLock(stringMutex);

  if(mapString != nullptr){
    g_string_free(mapString, TRUE);
  }

  mLock.unlock();
  
  /* Generate a string that will show the canvas contents on a terminal */
  mapString = chafa_canvas_print(canvas, NULL);

  /* Free resources */
  chafa_canvas_unref(canvas);
  chafa_canvas_config_unref(config);
  chafa_symbol_map_unref(symbol_map);
}

GString *Netstring::Map::getMapString(){
  std::shared_lock sharedLock(stringMutex);
  return g_string_new(mapString->str);
}

