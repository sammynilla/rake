
#ifndef RPM_H
#define RPM_H

/* NOTE (sammynilla): PPM image format specification taken from 'man ppm'
 * http://ailab.eecs.wsu.edu/wise/P1/PPM.html
 */

#define RPM_SIZE(w, h) ((h) * ((w) * 3) + (3) + (10 + 1) + (10 + 1) + (4))

static unsigned long
rpm_size(const long width, const long height) {
  enum { MAGIC_NUMBER = 3, SIZE_DATA = 11, MAX_VAL = 4 };
  const int header = (MAGIC_NUMBER + (SIZE_DATA * 2) + MAX_VAL);
  /* TODO (sammynilla): Add in value overflow sanitizing checks. */
  if (width < 1 || height < 1)
    return 0; /* Illegal size */

  return (unsigned long)(height * (width * 3) + header);
}

static void 
rpm_set(void *buf, long x, long y, unsigned long color) {
  enum { MAGIC_NUMBER = 3, SIZE_DATA = 11, MAX_VAL = 4 };
  enum { DIGIT_TO_ASCII = 48 };
  enum { DIGIT_0 = 48, DIGIT_9 = 57 };
  unsigned char *p = (unsigned char *)buf;
  unsigned long width = 0;

  p += MAGIC_NUMBER;
  // CALCULATE WIDTH
  for (; (((int)*p >= DIGIT_0) && ((int)*p <= DIGIT_9)); *p++) {
    if ((int)*p == DIGIT_0) continue;
    int val = ((int)*p) - DIGIT_TO_ASCII;
    width = (width * 10) + val;
  }
  *p++;

  p += SIZE_DATA + MAX_VAL;
  p += y * (width * 3) + x * 3;
  /* NOTE (sammynilla): RGB based format
   * RAWBITS ppm requires values from 0 to MAXVALS (255 in our implementation).
   */
  p[0] = (color >>  0) & 255;
  p[1] = (color >>  8) & 255;
  p[2] = (color >> 16) & 255;
}

/* NOTE (sammynilla): rpm_init()
 * This function is responsible for initialization the ppm file's header.
 * The expected buffer should be zero-initialized before use.
 */
static void
rpm_init(void *buf, long width, long height) {
  typedef unsigned char uchar;
  enum { 
    ASCII_0 = 0x30, ASCII_2 = 0x32, ASCII_5 = 0x35, ASCII_6 = 0x36,
    ASCII_P = 0x50, ASCII_COMMA = 0x2C, ASCII_NEW_LINE = 0x0A,
  };
  enum { BASE_MOD = 1, MAX_DIGIT = 10, DIGIT_TO_ASCII = 48 };

  uchar *p = (uchar *)buf;

  // MAGIC NUMBER
  *p++ = ASCII_P;
  *p++ = ASCII_6;
  *p++ = ASCII_NEW_LINE;

  // WIDTH,HEIGHT
  {
    const uchar separator[2] = { ASCII_COMMA, ASCII_NEW_LINE };
    uchar ascii[MAX_DIGIT];
    uchar *d;
    int i, j;
    for (i = 0; i < 2; ++i) {
      unsigned long mod = BASE_MOD;
      unsigned long ret = 0;
      long s = i ? height : width;
      const int digit_count =
        (s >= 1000000000) ? 10 : (s >= 100000000) ? 9 : (s >= 10000000) ? 8 :
        (s >= 1000000) ? 7 : (s >= 100000) ? 6 : (s >= 10000) ? 5 :
        (s >= 1000) ? 4 : (s >= 100) ? 3 : (s >= 10) ? 2 : 1;
      for (j = 0; j < MAX_DIGIT - digit_count; ++j) { ascii[j] = ASCII_0; }
      for (j = 0; j < digit_count; ++j) {
        s -= ret;
        mod *= 10;
        ret = s % mod;
        ascii[MAX_DIGIT-(j+1)] = (uchar)((ret / (mod * 0.1)) + DIGIT_TO_ASCII);
      }
      d = ascii;
      while (*d) { *p++ = *d++; }
      *p++ = separator[i];
    }
  }

  // MAXVAL
  *p++ = ASCII_2;
  *p++ = ASCII_5;
  *p++ = ASCII_5;
  *p++ = ASCII_NEW_LINE;
}

#endif /* RPM_H */
