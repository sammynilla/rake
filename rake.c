
#define _NO_CRT_STDIO_INLINE
#include <stdio.h>
#include <math.h>

#include "base_types.h"
#include "stacktrace.h"

#include "uc_rpm.h"

#define SCREEN_WIDTH  1024L
#define SCREEN_HEIGHT 512L
#define SCREEN_HSPLIT 512L
#define SCREEN_VSPLIT 0L
#define GET_SCREEN_SPLIT(a,b) ((a)-(b))

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

  if ((width == 0) && (height == 0))
    return;

  if ((width == 1) && (height == 1)) {
    uc_rpm_set(pBuf, x, y, color);
  } else {
    u32 i, j;
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        i32 cx = x+i;
        i32 cy = y+j;
        uc_rpm_set(pBuf, cx, cy, color);
      }
    }
  }
}

#define draw_pixel(pBuf, x, y) \
  fill_block((pBuf), (x), (y), 1, 1, SUNFLOWER)

internal void
draw_map(char *pBuf, const char *pMapBuf) {
  const i32 rect_w =
    GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT) / MAP_WIDTH_BLOCKS;
  const i32 rect_h =
    GET_SCREEN_SPLIT(SCREEN_HEIGHT, SCREEN_VSPLIT) / MAP_HEIGHT_BLOCKS;
  i32 rect_x, rect_y;
  i32 x, y;
  for (y = 0; y < MAP_HEIGHT_BLOCKS; y++) {
    for (x = 0; x < MAP_WIDTH_BLOCKS; x++) {
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
  const i32 rect_w =
    GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT) / MAP_WIDTH_BLOCKS;
  const i32 rect_h =
    GET_SCREEN_SPLIT(SCREEN_HEIGHT, SCREEN_VSPLIT) / MAP_HEIGHT_BLOCKS;
  fill_block(pBuf, (i32)(pUnit->x*rect_w), (i32)(pUnit->y*rect_h),
             pUnit->width, pUnit->height, color);
}

/* NOTE (sammynilla): Consider using this as a general ray cast function. */ 
internal r64
draw_ray(char *pBuf, const char *pMapBuf,
         const r32 x, const r32 y, const r64 angle, const r64 view_depth) {
  const i32 rect_w =
    GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT) / MAP_WIDTH_BLOCKS;
  const i32 rect_h =
    GET_SCREEN_SPLIT(SCREEN_HEIGHT, SCREEN_VSPLIT) / MAP_HEIGHT_BLOCKS;
  r64 t;
  size_t iter = 0;
  /* NOTE (sammynilla): Consider looking into intrinsics for speed. */
  for (t = 0; t < view_depth; t += 0.01) {
    r64 cx = x + t * cos(angle);
    r64 cy = y + t * sin(angle);
    iter++;
    if (pMapBuf[(i32)cx + (i32)cy * MAP_WIDTH_BLOCKS] != ' ')
      return t;

    if (!(iter % 10))
    {
      i32 px = (i32)(cx * rect_w);
      i32 py = (i32)(cy * rect_h);
      b32 not_oob =
        ((px > 0) && (px < GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT)) &&
         (py > 0) && (py < GET_SCREEN_SPLIT(SCREEN_HEIGHT, SCREEN_VSPLIT)));
      if (not_oob)
        draw_pixel(pBuf, px, py);
    }
  }

  /* Too far from characters view distance. */
  return 0;
}

internal void
draw_unit_fov(char *pBuf, const char *pMapBuf,
              struct unit *pUnit, r64 *pColumnsOut) {
  /* NOTE (sammynilla): The central FoV for most people is 50-60degrees. 
   * PI by itself will equate to a FoV of 180degrees.
   */
  const r64 fov = M_PI / 3.0f;
  const size_t ray_count = GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT);
  size_t i;
  for (i = 0; i < ray_count; ++i) {
    /* NOTE (sammynilla): Notes on the formula below (angle=radians):
     * 1. Subtracting (FoV*.5) from the angle gives us the first cone ray.
     * 2. Adding FoV to (FoV*.5) gives us the final cone ray length.
     * 3. Multiplying the previous FoV by the current iteration index and
     *    dividing it by the max iteration index provides us with the
     *    angle of the current ray iteration.
     * 4. Everything together using iteration provides a FoV cone.
     */
    r64 indexed_ray_angle =
      (pUnit->angle - (fov * .5)) + ((fov * i) / ray_count);
    r64 t =
      draw_ray(pBuf, pMapBuf,
               pUnit->x, pUnit->y, indexed_ray_angle, pUnit->view_depth);
    if (pColumnsOut != NULL)
      pColumnsOut[i] = t;
  }
}

int
main(void) {
  FILE *f;
  local_persist char rpm[UC_RPM_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)];

  uc_rpm_init(rpm, SCREEN_WIDTH, SCREEN_HEIGHT);

  Assert(sizeof(map) == MAP_SIZE(MAP_WIDTH_BLOCKS, MAP_HEIGHT_BLOCKS));

  draw_map(rpm, map);
  {
    local_persist r64 col[GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT)];

    struct unit player;
    player.x = 3.456f;
    player.y = 2.345f;
    player.width = player.height = 5;
    player.angle = 1.523;
    player.view_depth = 20.0;

    draw_unit(rpm, &player, WHITE);
    draw_unit_fov(rpm, map, &player, col);

    {
      const r64 fov = M_PI / 3.0f;
      const i32 hs = GET_SCREEN_SPLIT(SCREEN_WIDTH, SCREEN_HSPLIT);
      const i32 vs = GET_SCREEN_SPLIT(SCREEN_HEIGHT, SCREEN_VSPLIT);
      i32 i;
      for (i = 0; i < ArrayCount(col); ++i) {
        r64 t = col[i];
        if (t == 0) continue; /* Skip rendering if out of viewport. */

        {
          r64 indexed_ray_angle =
            player.angle-fov/2 + fov*i/hs;
          i32 ch = (i32)(vs / (t * cos(indexed_ray_angle-player.angle)));
          u8 color = UINT8_MAX - (u8)((t / player.view_depth) * UINT8_MAX);

          fill_block(rpm, hs+i, ((vs/2)-(ch/2)), 1, ch, pack_rgb(color, 0, color));
        }
      }
    }
  }

  f = fopen("./out.ppm", "wb");
  fwrite(rpm, sizeof(rpm), 1, f);
  fclose(f);

  return 0;
}
