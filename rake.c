
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define global static
#define internal static
#define local_persist static

#define i32 int32_t

#define MAGENTA         pack_colors(255,0,255)
#define NEON_GREEN      pack_colors(0,255,0)
#define CORNFLOWER_BLUE pack_colors(145,178,217)

#include "rpm.h"
#include "pack.c"

#define MAP_SIZE(w,h) (((w)*(h))+1)

/* Sample map data from the lesson plan. */
global const char map[] =  "0000222222220000"\
                           "1              0"\
                           "1      11111   0"\
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

int
main(void) {
  enum { WIDTH = 512L, HEIGHT = 512L };
  enum { MAP_WIDTH = 16, MAP_HEIGHT = 16 };

  FILE *f;
  local_persist char rpm[RPM_SIZE(WIDTH, HEIGHT)];

  rpm_init(rpm, WIDTH, HEIGHT);

  assert(sizeof(map) == MAP_SIZE(MAP_WIDTH, MAP_HEIGHT));

  {
    long x, y;
    /* for (y = 0; y < HEIGHT; y++) { */
    /*   for (x = 0; x < WIDTH; x++) { */
    /*     uint8_t r = (uint8_t)(255 * y / HEIGHT); */
    /*     uint8_t g = (uint8_t)(255 * x / WIDTH); */
    /*     uint8_t b = (uint8_t)(120); */
    /*     rpm_set(rpm, x, y, pack_colors(r, g, b)); */
    /*   } */
    /* } */
    {
      const i32 rect_w = WIDTH / MAP_WIDTH;
      const i32 rect_h = HEIGHT / MAP_HEIGHT;
      long rect_x, rect_y;
      for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
          if (map[x+y*MAP_WIDTH] == ' ') continue; /* skip empty spaces */
          rect_x = x * rect_w;
          rect_y = y * rect_h;
          {
            long i, j;
            for (j = 0; j < rect_h; ++j) {
              for (i = 0; i < rect_w; ++i) {
                long cx = rect_x+i;
                long cy = rect_y+j;
                assert((cx < WIDTH) && (cy < HEIGHT));
                rpm_set(rpm, cx, cy, NEON_GREEN);
              }
            }
          }
        }
      }
    }
  }

  f = fopen("./out.ppm", "wb");
  fwrite(rpm, sizeof(rpm), 1, f);
  fclose(f);

  return 0;
}
