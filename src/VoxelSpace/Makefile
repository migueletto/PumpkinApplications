include ../common.mak
include ../commonp.mak

PROGRAM=Voxels
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=main.o player.o display.o input.o noise.o

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c VoxS -n Voxels *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res.flag: resource.rcp
	$(PILRC) resource.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag
