include ../common.mak
include ../commonp.mak

PROGRAM=DOSBox
OBJS=DOSBox.o
CUSTOMPFLAGS=-I../Libretro

PRC=$(VFS)/app_install/$(PROGRAM).prc

LIBRETRO=$(VFS)/app_card/PALM/Programs/Libretro
CORES=$(LIBRETRO)/cores
CORE=$(CORES)/dosbox_libretro$(SOEXT)
DOSBOX=$(LIBRETRO)/dosbox
CONF=$(DOSBOX)/dosbox.conf
DRIVEC=$(DOSBOX)/C

all: $(PRC) $(CORE) $(CONF) $(DRIVEC)

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) $(PROGRAM).res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c DOSb -n DOSBox *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

$(PROGRAM).res: $(PROGRAM).rcp
	$(PILRC) $(PROGRAM).rcp; touch $(PROGRAM).res

$(CONF): dosbox.conf
	mkdir -p $(DOSBOX)
	cp dosbox.conf $(CONF)

$(CORE): dosbox_libretro$(SOEXT)
	mkdir -p $(CORES)
	cp dosbox_libretro$(SOEXT) $(CORE)

$(DRIVEC):
	mkdir -p $(DRIVEC)

dosbox_libretro$(SOEXT):
	cd DOSBox; make; cd ..
	cp DOSBox/dosbox_libretro$(SOEXT) .

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) *.bin $(PROGRAM).res dosbox_libretro$(SOEXT)
	cd DOSBox; make clean; cd ..
