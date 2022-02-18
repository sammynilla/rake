
uint32_t
pack_colors(const uint8_t r, const uint8_t g, const uint8_t b) {
  const uint8_t a = 255;
  return (uint32_t)((a<<24) + (b<<16) + (g<<8) + r);
}
