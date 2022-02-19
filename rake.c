
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "base_types.h"
#include "rpm.h"

/* Sample map data from the lesson plan. */
global const char map[] =  "0000222222220000"\
                           "1              0"\
                           "1     111111   0"\
                           "1     0        0"\
                           "0     0  1110000"\
                           "0     3        0"\
                           "0   10000      0"\
                           "0   0   11100  0"\
                           "0   0   0      0"\
                           "0   0   1  00000"\
                           "0       1      0"\
                           "2       1      0"\
                           "0       0      0"\
                           "0 0000000      0"\
                           "0              0"\
                           "0002222222200000";

#define MAP_SIZE(w,h) (((w)*(h))+1)
#define MAP_WIDTH_BLOCKS  16
#define MAP_HEIGHT_BLOCKS 16

#include "pack.c"

#define MAGENTA         pack_colors(255,0,255)
#define NEON_GREEN      pack_colors(0,255,0)
#define CORNFLOWER_BLUE pack_colors(145,178,217)
#define WHITE           pack_colors(255,255,255)

internal void
fill_block(char *pixels, const long x, const long y,
           const i32 width, const i32 height, const u32 color) {

  long i, j;
  for (j = 0; j < height; ++j) {
    for (i = 0; i < width; ++i) {
      long cx = x+i;
      long cy = y+j;
      rpm_set(pixels, cx, cy, color);
    }
  }
}

#define draw_player(buf, x, y, h, w) \
  fill_block((buf), (x), (y), (w), (h), WHITE) 

internal void
draw_map(char *pixels, const char *map_data, 
         const i32 width, const i32 height) {

  const i32 rect_w = width / MAP_WIDTH_BLOCKS;
  const i32 rect_h = height / MAP_HEIGHT_BLOCKS;
  long x, y;
  long rect_x, rect_y;
  for (y = 0; y < MAP_WIDTH_BLOCKS; y++) {
    for (x = 0; x < MAP_HEIGHT_BLOCKS; x++) {
      if (map_data[x+y*MAP_WIDTH_BLOCKS] == ' ') continue; /* skip empty spaces */
      rect_x = x * rect_w;
      rect_y = y * rect_h;
      fill_block(pixels, rect_x, rect_y, rect_w, rect_h, NEON_GREEN);
    }
  }
}
  
int
main(void) {
  enum { WIDTH = 512L, HEIGHT = 512L };
  enum { PLAYER_WIDTH = 5, PLAYER_HEIGHT = 5 };

  FILE *f;
  local_persist char rpm[RPM_SIZE(WIDTH, HEIGHT)];

  assert(sizeof(map) == MAP_SIZE(MAP_WIDTH_BLOCKS, MAP_HEIGHT_BLOCKS));

  rpm_init(rpm, WIDTH, HEIGHT);
  {
    const float px = 3.456f;
    const float py = 2.345f;
    const i32 rect_w = WIDTH / MAP_WIDTH_BLOCKS;
    const i32 rect_h = HEIGHT / MAP_HEIGHT_BLOCKS;

    draw_map(rpm, map, WIDTH, HEIGHT);
    draw_player(rpm, (long)px*rect_w, (long)py*rect_h, 
                PLAYER_WIDTH, PLAYER_HEIGHT);
  }

  f = fopen("./out.ppm", "wb");
  fwrite(rpm, sizeof(rpm), 1, f);
  fclose(f);

  return 0;
}
