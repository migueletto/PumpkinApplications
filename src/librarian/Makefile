include ../common.mak
include ../commonp.mak

PROGRAM=Librarian
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=librarian.o librarianDB.o librarianTransfer.o

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c LFlb -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: librarian.rcp
	$(PILRC) librarian.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag
