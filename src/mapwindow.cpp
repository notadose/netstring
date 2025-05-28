#include <chafa.h>
#include <stdio.h>
#include <cairo.h>
#include <math.h>
#include <tins/tins.h>
#include <iostream>

#include "mapwindow.hpp"
#include "sniffer.hpp"

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

void Netstring::Map::onPacket() {
  std::deque<Netstring::Sniffer::packetInfo> packets = Netstring::Sniffer::getPackets();
  std::vector<Tins::IP::address_type> hosts = Netstring::Sniffer::getKnownHosts();

  const int PIX_WIDTH = 400;
  const int PIX_HEIGHT = 400;
  const int N_CHANNELS = 4;

  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32,
    PIX_WIDTH,
    PIX_HEIGHT
  );

  cairo_t *cr = cairo_create(surface);

  float src_xpos;
  float src_ypos;

  float dest_xpos;
  float dest_ypos;

  getHostLocation(packets[0].src, &src_xpos, &src_ypos);
  getHostLocation(packets[0].dest, &dest_xpos, &dest_ypos);

  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
  cairo_move_to(cr, PIX_WIDTH*src_xpos, PIX_HEIGHT*src_ypos);
  cairo_set_line_width(cr, 2.0);
  cairo_line_to(cr, PIX_WIDTH*dest_xpos, PIX_HEIGHT*dest_ypos);
  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

  std::for_each(hosts.begin(), hosts.end(), [&](Tins::IP::address_type host) -> void{
    float xpos;
    float ypos;

    getHostLocation(host, &xpos, &ypos);
    
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
  chafa_canvas_config_set_geometry(config, 23, 10);

  /* Create canvas */
  ChafaCanvas *canvas = chafa_canvas_new(config);

  /* Draw pixels to canvas */
  chafa_canvas_draw_all_pixels(canvas, CHAFA_PIXEL_RGBA8_UNASSOCIATED, pixels,
                               PIX_WIDTH, PIX_HEIGHT, PIX_WIDTH * N_CHANNELS);

  /* Generate a string that will show the canvas contents on a terminal */
  GString *gs = chafa_canvas_print(canvas, NULL);

  /* Print the string */
  fwrite(gs->str, sizeof(char), gs->len, stdout);
  fputc('\n', stdout);

  /* Free resources */
  g_string_free(gs, TRUE);
  chafa_canvas_unref(canvas);
  chafa_canvas_config_unref(config);
  chafa_symbol_map_unref(symbol_map);
}

void Netstring::Map::drawCircle() {

  const int PIX_WIDTH = 400;
  const int PIX_HEIGHT = 400;
  const int N_CHANNELS = 4;

  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32,
    PIX_WIDTH,
    PIX_HEIGHT
  );

  cairo_t *cr = cairo_create(surface);

  cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 12.0);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

  cairo_translate(cr, PIX_WIDTH*0.5, PIX_HEIGHT*0.5);
  cairo_arc(cr, 0, 0, PIX_WIDTH * 0.4, 0, 2.0*M_PI);
  cairo_stroke_preserve(cr);

  cairo_fill(cr);

  cairo_destroy(cr);
  cairo_surface_flush(surface);
  const guint8 *pixels2 = cairo_image_surface_get_data(surface);



  /* Specify the symbols we want */
  ChafaSymbolMap *symbol_map = chafa_symbol_map_new();
  chafa_symbol_map_add_by_tags(symbol_map, CHAFA_SYMBOL_TAG_ALL);

  /* Set up a configuration with the symbols and the canvas size in characters
   */
  ChafaCanvasConfig *config = chafa_canvas_config_new();
  chafa_canvas_config_set_symbol_map(config, symbol_map);
  chafa_canvas_config_set_dither_mode(config, CHAFA_DITHER_MODE_NONE);
  chafa_canvas_config_set_color_extractor(config, CHAFA_COLOR_EXTRACTOR_MEDIAN);
  chafa_canvas_config_set_pixel_mode(config, CHAFA_PIXEL_MODE_SYMBOLS);
  chafa_canvas_config_set_canvas_mode(config, CHAFA_CANVAS_MODE_FGBG);
  chafa_canvas_config_set_geometry(config, 23, 10);

  /* Create canvas */
  ChafaCanvas *canvas = chafa_canvas_new(config);

  /* Draw pixels to canvas */
  chafa_canvas_draw_all_pixels(canvas, CHAFA_PIXEL_RGBA8_UNASSOCIATED, pixels2,
                               PIX_WIDTH, PIX_HEIGHT, PIX_WIDTH * N_CHANNELS);

  /* Generate a string that will show the canvas contents on a terminal */
  GString *gs = chafa_canvas_print(canvas, NULL);

  /* Print the string */
  fwrite(gs->str, sizeof(char), gs->len, stdout);
  fputc('\n', stdout);

  /* Free resources */
  g_string_free(gs, TRUE);
  chafa_canvas_unref(canvas);
  chafa_canvas_config_unref(config);
  chafa_symbol_map_unref(symbol_map);
}
