include ../common.mak
include ../commonp.mak

PROGRAM=iRogue
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=iRogue.o \
     Globals.o chuvmey_nolib.o color.o dead.o display.o hit.o \
     inventory.o level.o lib_md.o lib_monster.o lib_object.o main.o \
     message.o monster.o move.o my_object.o object.o pack.o random.o \
     ring.o room.o save.o spec_hit.o throw.o trap.o use.o win.o zap.o

CUSTOMPFLAGS=-DNON_INTERNATIONAL -DI_AM_COLOR

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c RoGe -n iRogue *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res.flag: iRogue.rcp
	$(PILRC) iRogue.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag
