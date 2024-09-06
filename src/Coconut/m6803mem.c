static UInt8 GetByte(Hardware *hardware, UInt16 a) {
  if (a < 0x0020)
    return m6803_getreg(hardware->cpu, a & 0xFF);
  if (a >= 0x0080 && a < 0x0100)
    return hardware->m0[a];
  if (a == 0xBFFF)
    return IO_readb(hardware, a & 0xFF);
  if (a >= 0x4000 && a <= (0x4000 + hardware->ramsize))
    return hardware->m0[a - 0x3000];
  if (a >= 0xE000)
    return hardware->m1[a & 0x7FFF];

  return 0;
}

static void SetByte(Hardware *hardware, UInt16 a, UInt8 b) {
  if ((a-0x3000) >= hardware->gp_begin && (a-0x3000) < hardware->gp_end)
    hardware->dirty = 1;

  if (a < 0x0020)
    m6803_setreg(hardware->cpu, a & 0xFF, b);
  else if (a >= 0x0080 && a < 0x0100)
    hardware->m0[a] = b;
  else if (a == 0xBFFF)
    return IO_writeb(hardware, a & 0xFF, b);
  else if (a >= 0x4000 && a <= (0x4000 + hardware->ramsize))
    hardware->m0[a - 0x3000] = b;
}
