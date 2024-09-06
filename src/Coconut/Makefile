include ../common.mak
include ../commonp.mak

PROGRAM=Coconut
PRC=$(VFS)/app_install/$(PROGRAM).prc

OPTMACHINES=-DF_APPLE -DF_AQUARIUS -DF_ATOM -DF_CGENIE -DF_COLECO -DF_CPM -DF_JUPITER -DF_MSX -DF_NES -DF_ORIC -DF_VIC20
CUSTOMPFLAGS=-DUNIX -DLSB_FIRST $(OPTMACHINES)

M6809OBJS=m6809armlet.o m6809.o
M6803OBJS=m6803armlet.o m6803.o
M6502OBJS=m6502armlet.o m6502.o
Z80OBJS=z80armlet.o z80.o

OBJS=main.o palm.o cpu.o machine.o debug.o db.o \
     misc.o snapshot.o dsk.o bin.o cas.o kbd.o snapz80.o dac.o \
     coco.o mc10.o spectrum.o mc1000.o vz.o ay8910.o \
     ti9918.o msx.o sn76489.o coleco.o \
     cpm.o console.o lpt.o auxp.o cpmdsk.o screen.o vt100.o \
     apple.o vic20.o oric.o atom.o cgenie.o cgdsk.o m6845.o \
     aquarius.o nes.o jupiter.o io.o \
     $(M6809OBJS) \
     $(M6803OBJS) \
     $(M6502OBJS) \
     $(Z80OBJS)

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c CoCn -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: resource.rcp
	$(PILRC) resource.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag
