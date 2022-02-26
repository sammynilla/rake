
#include "base_types.h"

#define MAGENTA         pack_rgb(255,0,255)
#define NEON_GREEN      pack_rgb(0,255,0)
#define CORNFLOWER_BLUE pack_rgb(145,178,217)
#define SUNFLOWER       pack_rgb(255, 218, 3)
#define WHITE           pack_rgb(255,255,255)

internal u32
pack_rgb(const u8 r, const u8 g, const u8 b) {
  const u8 a = 255;
  return (a<<24) + (b<<16) + (g<<8) + r;
}

internal u32
pack_rgba(const u8 r, const u8 g, const u8 b, const u8 a) {
  return (a<<24) + (b<<16) + (g<<8) + r;
}
