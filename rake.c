
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <math.h>

#include "base_types.h"
#include "stacktrace.h"

#include "rpm.h"

#define SCREEN_WIDTH  512L
#define SCREEN_HEIGHT 512L

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

#include "packed_colors.c"

internal void
fill_block(char *pBuf, const i32 x, const i32 y,
           const u32 width, const u32 height, const u32 color) {

  Assert((width != 0) && (height != 0));

  if ((width == 1) && (height == 1)) {
    rpm_set(pBuf, x, y, color);
    return;
  }

  {
    u32 i, j;
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        i32 cx = x+i;
        i32 cy = y+j;
        rpm_set(pBuf, cx, cy, color);
      }
    }
  }
}

#define draw_pixel(pBuf, x, y) \
  fill_block((pBuf), (x), (y), 1, 1, WHITE)

internal void
draw_map(char *pBuf, const char *pMapBuf) {

  const i32 rect_w = SCREEN_WIDTH / MAP_WIDTH_BLOCKS;
  const i32 rect_h = SCREEN_HEIGHT / MAP_HEIGHT_BLOCKS;
  i32 rect_x, rect_y;
  i32 x, y;
  for (y = 0; y < MAP_WIDTH_BLOCKS; y++) {
    for (x = 0; x < MAP_HEIGHT_BLOCKS; x++) {
      if (pMapBuf[x+y*MAP_WIDTH_BLOCKS] == ' ') continue; /* skip empty spaces */
      rect_x = x * rect_w;
      rect_y = y * rect_h;
      fill_block(pBuf, rect_x, rect_y, rect_w, rect_h, NEON_GREEN);
    }
  }
}

struct unit {
  r32 x, y;
  i32 width, height;
  r64 angle;
  r64 view_depth;
};

internal void
draw_unit(char *pBuf, struct unit *pUnit, const u32 color) {
  const i32 rect_w = SCREEN_WIDTH / MAP_WIDTH_BLOCKS;
  const i32 rect_h = SCREEN_HEIGHT / MAP_HEIGHT_BLOCKS;
  fill_block(pBuf, (i32)(pUnit->x*rect_w), (i32)(pUnit->y*rect_h),
             pUnit->width, pUnit->height, color);
}

internal void
draw_ray(char *pBuf, const char *pMapBuf,
         const r32 x, const r32 y, const r64 angle, const r64 view_depth) {
  const i32 rect_w = SCREEN_WIDTH / MAP_WIDTH_BLOCKS;
  const i32 rect_h = SCREEN_HEIGHT / MAP_HEIGHT_BLOCKS;
  r64 t;
  /* NOTE (sammynilla): Consider looking into intrinsics for speed. */
  for (t = 0; t < view_depth; t += .05) {
    r64 cx = x + t * cos(angle);
    r64 cy = y + t * sin(angle);

    if (pMapBuf[(i32)cx + (i32)cy * MAP_WIDTH_BLOCKS] != ' ') break;

    {
      i32 px = (i32)(cx * rect_w);
      i32 py = (i32)(cy * rect_h);
      b32 not_oob =
        ((px > 0) && (px < SCREEN_WIDTH) && (py > 0) && (py < SCREEN_HEIGHT));
      if (not_oob)
        draw_pixel(pBuf, px, py);
    }
  }
}

internal void
draw_unit_fov(char *pBuf, const char *pMapBuf, struct unit *pUnit) {
  /* NOTE (sammynilla): The central FoV for most people is 50-60degrees. 
   * PI by itself will equate to a FoV of 180degrees.
   */
  const r64 fov = M_PI / 3;
  size_t i;
  for (i = 0; i < SCREEN_WIDTH; ++i) {
    /* NOTE (sammynilla): Notes on the formula below (angle=radians):
     * 1. Subtracting (FoV*.5) from the angle gives us the first cone ray.
     * 2. Adding FoV to (FoV*.5) gives us the final cone ray length.
     * 3. Multiplying the previous FoV by the current iteration index and
     *    dividing it by the max iteration index provides us with the
     *    angle of the current ray iteration.
     * 4. Everything together using iteration provides a FoV cone.
     */
    r64 indexed_ray_angle =
      (pUnit->angle - (fov * .5)) + ((fov * i) / SCREEN_WIDTH);
    draw_ray(pBuf, pMapBuf,
             pUnit->x, pUnit->y, indexed_ray_angle, pUnit->view_depth);
  }
}

int
main(void) {
  FILE *f;
  local_persist char rpm[RPM_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)];

  Assert(sizeof(map) == MAP_SIZE(MAP_WIDTH_BLOCKS, MAP_HEIGHT_BLOCKS));

  rpm_init(rpm, SCREEN_WIDTH, SCREEN_HEIGHT);

  draw_map(rpm, map);

  {
    struct unit player;
    player.x = 3.456f;
    player.y = 2.345f;
    player.width = player.height = 5;
    player.angle = 1.25;
    player.view_depth = 20.0;

    draw_unit(rpm, &player, WHITE);
    draw_unit_fov(rpm, map, &player);
  }

  f = fopen("./out.ppm", "wb");
  fwrite(rpm, sizeof(rpm), 1, f);
  fclose(f);

  return 0;
}
