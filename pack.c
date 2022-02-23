
#include "base_types.h"

internal u32
pack_rgb(const u8 r, const u8 g, const u8 b) {
  const u8 a = 255;
  return (a<<24) + (b<<16) + (g<<8) + r;
}

internal u32
pack_rgba(const u8 r, const u8 g, const u8 b, const u8 a) {
  return (a<<24) + (b<<16) + (g<<8) + r;
}
