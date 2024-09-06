static UInt8 GetByte(Hardware *hardware, UInt16 a) {
  UInt32 sw;

  switch (hardware->memmode) {
    case 1:	// spectrum
      if (a >= 0x8000)
        return hardware->m0[a & 0x7FFF];
      return hardware->m1[a];

    case 2:	// cpm
      if (a < 0x8000)
        return hardware->m0[a];
      return hardware->m1[a & 0x7FFF];

    case 3:	// mc1000
      if (a < 0x4000)
        return hardware->m0[a];

      if (a >= 0xC000)
        return hardware->m1[a & 0x7FFF];

      if (a < 0x8000) {
        if (hardware->banksw[0])
          return hardware->m2[a - 0x4000];
        return 0xFF;
      }

      if (hardware->banksw[1])
        return hardware->m2[a - 0x4000];

      if (a >= 0x8000 && a < 0x9800)
        return hardware->m1[a & 0x7FFF];

      if (hardware->banksw[0])
        return hardware->m2[a - 0x4000];

      return 0xFF;

    case 4:	// vz300
      if (a >= 0x8000)
        return hardware->m0[a & 0x7FFF];
      if (a >= 0x6800 && a < 0x7000)
        return IO_readb(hardware, a);
      return hardware->m1[a];

    case 5:	// cgenie
      if (a < 0x8000)
        return hardware->m1[a];
      if (a >= 0xF800 && a < 0xF900)
        return IO_readb(hardware, a);	// porta artificial
      return hardware->m0[a & 0x7FFF];

    case 6:	// jupiter
      if (a < 0x8000)
        return hardware->m1[a];
      return hardware->m0[a & 0x7FFF];

    case 7:	// aquarius
      if (a < 0x8000)
        return hardware->m1[a];
      if (a >= 0xC000)
        return hardware->m0[a & 0x7FFF] ^ hardware->m0[0xFF];
      return 0xFF;

    case 8:	// msx
      // bank0: RAM    8000-FFFF
      // bank1: R0M    0000-7FFF (BIOS + BASIC)
      // bank2: RAM    0000-7FFF
      // bank3: CART A 4000-BFFF

      sw = a >> 14;

      switch (sw) {
        case 0:	// 0000-3FFF
          switch (hardware->banksw[sw]) {
            case 0:	// BIOS
              return hardware->m1[a];
            case 1:	// CART A
              break;
            case 2:	// CART B
              break;
            case 3:	// RAM
              return hardware->m2[a];
          }
          break;
        case 1:	// 4000-7FFF
          switch (hardware->banksw[sw]) {
            case 0:	// BASIC
              return hardware->m1[a];
            case 1:	// CART A
              if (hardware->m3)
                return hardware->m3[a - 0x4000];
              break;
            case 2:	// CART B
              break;
            case 3:	// RAM
              return hardware->m2[a];
          }
          break;
        case 2:	// 8000-BFFF
          switch (hardware->banksw[sw]) {
            case 0:	// N/A
              break;
            case 1:	// CART A
              if (hardware->m3)
                return hardware->m3[a - 0x4000];
              break;
            case 2:	// CART B
              break;
            case 3:	// RAM
              return hardware->m0[a & 0x7FFF];
          }
          break;
        case 3:	// C000-FFFF
          switch (hardware->banksw[sw]) {
            case 0:	// N/A
              break;
            case 1:	// CART A
              break;
            case 2:	// CART B
              break;
            case 3:	// RAM
              return hardware->m0[a & 0x7FFF];
          }
      }
      break;

    case 9:	// coleco
      if (a >= 0x8000)
        return hardware->m0[a & 0x7FFF];
      if (a >= 0x6000)
        return hardware->m1[a & 0x63FF];
      return hardware->m1[a];

    case 10:	// sord
      if (a < 0x8000)
        return hardware->m1[a];
      return hardware->m0[a & 0x7FFF];
  }

  return 0;
}

static void SetByte(Hardware *hardware, UInt16 a, UInt8 b) {
  UInt32 sw;

  switch (hardware->memmode) {
    case 1:	// spectrum
      if (a >= 0x8000)
        hardware->m0[a & 0x7FFF] = b;
      else if (a >= 0x4000)
        hardware->m1[a] = b;
      break;

    case 2:	// cpm
      if (a < 0x8000)
        hardware->m0[a] = b;
      else
        hardware->m1[a & 0x7FFF] = b;
      break;

    case 3:	// mc1000
      if (a < 0x4000) {
        hardware->m0[a] = b;
        return;
      }

      if (a < 0x8000) {
        if (hardware->banksw[0])
          hardware->m2[a - 0x4000] = b;
        return;
      }

      if (a >= 0xC000)
        return;

      if (hardware->banksw[1]) {
        hardware->m2[a - 0x4000] = b;
        return;
      }

      if (a >= 0x8000 && a < 0x9800) {
        hardware->m1[a & 0x7FFF] = b;
        hardware->dirty = 1;
      }

      if (hardware->banksw[0])
        hardware->m2[a - 0x4000] = b;
      break;

    case 4:	// vz300
      if (a >= 0x6800 && a < 0x7000)
        IO_writeb(hardware, 0x6800, b);
      else if (a >= 0x7000 && a < 0x7800) {
        hardware->m1[a] = b;
        hardware->dirty = 1;
      } else if (a >= 0x7800 && a < 0x8000)
        hardware->m1[a] = b;
      else if (a >= 0x8000 && a < 0xB800)
        hardware->m0[a & 0x7FFF] = b;
      break;

    case 5:	// cgenie
      if (a >= 0x4000 && a < 0x8000) {
        if (hardware->m1[a] != b) {
          hardware->m1[a] = b;
          hardware->dirty = 1;
        }
      } else if (a >= 0x8000 && a < 0xC000) {
        hardware->m0[a & 0x7FFF] = b;
      } else if (a >= 0xF000 && a < 0xF400) {
        b |= 0xF0;
        if (hardware->m0[a & 0x7FFF] != b) {
          hardware->m0[a & 0x7FFF] = b;
          hardware->dirty = 1;
        }
      } else if (a >= 0xF400 && a < 0xF800) {
        if (hardware->m0[a & 0x7FFF] != b) {
          hardware->m0[a & 0x7FFF] = b;
          hardware->dirty = 1;
        }
      }
      break;

    case 6:	// jupiter
      if (a >= 0x2300 && a < 0x2400)
        hardware->m1[a] = b;
      else if (a >= 0x2400 && a < 0x2700) {
        if (hardware->m1[a] != b) {
          hardware->dirty = 1;
          hardware->m1[a] = b;
        }
      } else if (a >= 0x2700 && a < 0x2800)
        hardware->m1[a] = b;
      else if (a >= 0x2C00 && a < 0x3000) {
        if (hardware->m1[a] != b) {
          hardware->dirty = 1;
          hardware->m1[a] = b;
        }
      } else if (a >= 0x3C00 && a < 0x8000)
        hardware->m1[a] = b;
      else
        hardware->m0[a & 0x7FFF] = b;
      break;

    case 7:	// aquarius
      if (a >= 0x3000 && a < 0x3800) {
        if (hardware->m1[a] != b) {
          hardware->dirty = 1;
          hardware->m1[a] = b;
        }
      } else if (a >= 0x3800 && a < 0x4000) {
        hardware->m1[a] = b;
      } else if (a >= 0x4000 && a <= hardware->ramsize) {
        hardware->m1[a] = b;
      }
      break;

    case 8:	// msx
      // bank2: RAM 0000-7FFF
      // bank1: R0M 0000-7FFF (BIOS + BASIC)
      // bank0: RAM 8000-FFFF

      sw = a >> 14;

      switch (sw) {
        case 0:	// 0000-3FFF
        case 1:	// 4000-7FFF
          switch (hardware->banksw[sw]) {
            case 0:	// BIOS ou BASIC
            case 1:	// CART A
            case 2:	// CART B
              break;
            case 3:	// RAM
              hardware->m2[a] = b;
          }
          break;
        case 2:	// 8000-BFFF
        case 3:	// C000-FFFF
          switch (hardware->banksw[sw]) {
            case 0:	// N/A
            case 1:	// CART A
            case 2:	// CART B
              break;
            case 3:	// RAM
              hardware->m0[a & 0x7FFF] = b;
          }
      }
      break;

    case 9:	// coleco
      if (a >= 0x6000 && a < 0x8000)
        hardware->m1[a & 0x63FF] = b;
      break;

    case 10:	// sord
      if (a >= 0x7000 && a < 0x8000)
        hardware->m1[a] = b;
  }
}
