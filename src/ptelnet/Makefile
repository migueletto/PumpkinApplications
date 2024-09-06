include ../common.mak
include ../commonp.mak

PROGRAM=ptelnet
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=ptelnet.o serial.o network.o telnet.o vt100.o tn3270.o screen.o db.o memo.o log.o misc.o error.o

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Ptln -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: ptelnet.rcp 4x6.txt 5x9.txt 8x14.txt 12x22.txt
	$(PILRC) ptelnet.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag

