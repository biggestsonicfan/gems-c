#include "emu.h"
#include "cpu/i960/i960dis.h"
#include <cstdio>
#include <vector>
#include <sstream>
struct buf : util::disasm_interface::data_buffer {
  std::vector<uint8_t> m; uint32_t base;
  u8 r8(offs_t pc) const override { return pc-base<m.size()?m[pc-base]:0; }
  u16 r16(offs_t pc) const override { return r8(pc)|r8(pc+1)<<8; }
  u32 r32(offs_t pc) const override { return r16(pc)|r16(pc+2)<<16; }
  u64 r64(offs_t pc) const override { return r32(pc)|(u64)r32(pc+4)<<32; }
};
int main(int c,char**v){ // file start end
  FILE*f=fopen(v[1],"rb"); buf b; b.base=0; int ch; while((ch=fgetc(f))!=EOF) b.m.push_back(ch);
  uint32_t s=strtoul(v[2],0,0), e=strtoul(v[3],0,0);
  i960_disassembler d;
  for(uint32_t pc=s;pc<e;){ std::ostringstream o; uint32_t r=d.disassemble(o,pc,b,b);
    int n=r&0xffff; printf("%06x: %08x %s\n",pc,b.r32(pc),o.str().c_str()); pc+=n?n:4; }
}
