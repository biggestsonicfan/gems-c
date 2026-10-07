import re,os,struct,sys
import os
exec(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "elf.py")).read())
R2,R13=0x801F0520,0x801EF7E0
SRC=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "decomp")
pat=re.compile(r"\*\((float|double|int|uint|undefined4|undefined8|longlong|ulonglong|short|ushort|byte|char|undefined2|undefined|undefined1) \*\)\(unaff_r(2|13) \+ (-?0x[0-9a-f]+)\)")
pat2=re.compile(r"\(unaff_r(2|13) \+ (-?0x[0-9a-f]+)\)")
def lit(t,a):
    if t=="float":
        b=rd(a); u=struct.unpack(">I",b)[0]; f=struct.unpack(">f",b)[0]
        return "(%rf /*0x%08x*/)"%(f,u)
    if t=="double":
        b=rd(a,8); f=struct.unpack(">d",b)[0]; u=struct.unpack(">Q",b)[0]
        return "(%r /*0x%016x*/)"%(f,u)
    return None
for sub in ["FN","SHARC","SHARC/helpers"]:
    os.makedirs("ann/"+sub,exist_ok=True)
    for fn in os.listdir(SRC+"/"+sub):
        if not fn.endswith(".c"): continue
        s=open(SRC+"/"+sub+"/"+fn).read()
        def rep(m):
            t,r,off=m.group(1),m.group(2),int(m.group(3),16)
            a=(R2 if r=="2" else R13)+off
            if r=="2":
                l=lit(t,a)
                if l: return l
            v=rd(a,4)
            val="" if v is None else " =0x%08x"%struct.unpack(">I",v)[0]
            return "GCMEM_%s(0x%08x /*%s%s*/)"%(t,a,"sdata2" if r=="2" else "sdata",val)
        s=pat.sub(rep,s)
        s=pat2.sub(lambda m:"(0x%08x)"%((R2 if m.group(1)=="2" else R13)+int(m.group(2),16)),s)
        open("ann/"+sub+"/"+fn,"w").write(s)
