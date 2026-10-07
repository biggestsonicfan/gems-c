import struct,sys
import os
D=open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "stf.elf"),"rb").read()
shoff=struct.unpack(">I",D[0x20:0x24])[0]; shnum=struct.unpack(">H",D[0x30:0x32])[0]
secs=[]
for i in range(shnum):
    h=D[shoff+i*40:shoff+i*40+40]
    name,typ,flags,addr,off,size=struct.unpack(">IIIIII",h[:24])
    if typ==1: secs.append((addr,off,size))
def rd(a,n=4):
    for s,o,z in secs:
        if s<=a<s+z: return D[o+a-s:o+a-s+n]
    return None
def u32(a): b=rd(a); return struct.unpack(">I",b)[0] if b else None
