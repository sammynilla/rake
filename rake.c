
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <stdint.h>
#include <math.h>
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
           const u32 width, const u32 height, const u32 color) {

  assert((width != 0) && (height != 0));

  if ((width == 1) && (height == 1)) {
    rpm_set(pixels, x, y, color);
    return;
  }
  
  {
    u32 i, j;
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        long cx = x+i;
        long cy = y+j;
        rpm_set(pixels, cx, cy, color);
      }
    }
  }
}

#define draw_pixel(buf, x, y) \
  fill_block((buf), (x), (y), 1, 1, WHITE)
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
  enum { RAY_DEPTH = 20 };

  FILE *f;
  local_persist char rpm[RPM_SIZE(WIDTH, HEIGHT)];

  assert(sizeof(map) == MAP_SIZE(MAP_WIDTH_BLOCKS, MAP_HEIGHT_BLOCKS));

  rpm_init(rpm, WIDTH, HEIGHT);

  draw_map(rpm, map, WIDTH, HEIGHT);

  {
    /* Player Properties */
    float player_x = 3.456f;
    float player_y = 2.345f;
    double player_angle = 1.25;
    const i32 rect_w = WIDTH / MAP_WIDTH_BLOCKS;
    const i32 rect_h = HEIGHT / MAP_HEIGHT_BLOCKS;
    draw_player(rpm, (long)(player_x*rect_w), (long)(player_y*rect_h),
                PLAYER_WIDTH, PLAYER_HEIGHT);

    {
      /* NOTE (sammynilla): The central FoV for most people is 50-60degrees. 
       * PI by itself will equate to a FoV of 180degrees.
       */
      const double fov = M_PI / 3;
      size_t i;
      for (i = 0; i < WIDTH; ++i) {
        double t;
        /* NOTE (sammynilla): Notes on the formula below (angle=radians):
         * 1. Subtracting (FoV*.5) from the angle gives us the first cone ray.
         * 2. Adding FoV to (FoV*.5) gives us the final cone ray length.
         * 3. Multiplying the previous FoV by the current iteration index and
         *    dividing it by the max iteration index provides us with the
         *    angle of the current ray iteration.
         * 4. Everything together using iteration provides a FoV cone.
         */
        double angle = (player_angle - (fov * .5)) + ((fov * i) / WIDTH);
        /* NOTE (sammynilla): Consider looking into intrinsics for speed. */ 
        for (t = 0; t < RAY_DEPTH; t += .05) {
          double cx = player_x + t * cos(angle);
          double cy = player_y + t * sin(angle);

          if (map[(int)cx + (int)cy * MAP_WIDTH_BLOCKS] != ' ') break;

          {
            long x = (long)(cx * rect_w);
            long y = (long)(cy * rect_h);
            b32 not_oob = ((x > 0) && (x < WIDTH) && (y > 0) && (y < HEIGHT));
            if (not_oob)
              draw_pixel(rpm, x, y);
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
