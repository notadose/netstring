#pragma once

#include <glib.h>

namespace Netstring::Map{
  void onPacket();
  GString *getMapString();

  void setMapSize(guint16 width, guint16 height); // size in console
  void getMapSize(guint16 *wid_ptr, guint16 *hgt_ptr);
}
