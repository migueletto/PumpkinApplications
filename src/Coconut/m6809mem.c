static UInt8 GetByte(Hardware *hardware, UInt16 a) {
  if (a <= hardware->ramsize)
    return hardware->m0[a & hardware->ramsize];
  if (a < 0xFF00)
    return hardware->m1[a & 0x7FFF];
  if (a < 0xFF40)
    return IO_readb(hardware, a & 0x23);
  if (a >= 0xFFF0)
    return IO_readb(hardware, a & 0xFF);

  return 0;
}

static void SetByte(Hardware *hardware, UInt16 a, UInt8 b) {
  if (a >= hardware->gp_begin && a < hardware->gp_end)
    hardware->dirty = 1;

  if (a <= hardware->ramsize)
    hardware->m0[a & hardware->ramsize] = b;
  else if (a < 0xFF00) {
    if (hardware->m1w)
      hardware->m1[a & 0x7FFF] = b;
  } else if (a < 0xFF40)
    IO_writeb(hardware, a & 0x23, b);
  else if (a >= 0xFFC0 && a < 0xFFE0)
    IO_sam(hardware, a - 0xFFC0);
}
