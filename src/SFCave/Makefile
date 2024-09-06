include ../common.mak
include ../commonp.mak

PROGRAM=SFCave
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=SFCave.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c SFCv -n $(PROGRAM) resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) $(DLIB)
