
internal u32
pack_colors(const u8 r, const u8 g, const u8 b) {
  const u8 a = 255;
  return (u32)((a<<24) + (b<<16) + (g<<8) + r);
}
