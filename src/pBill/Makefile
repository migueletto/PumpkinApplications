include ../common.mak
include ../commonp.mak

PROGRAM=pBill
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=sauce/Bucket.o sauce/Cable.o sauce/Computer.o sauce/Game.o sauce/Horde.o sauce/Library.o sauce/Monster.o sauce/Network.o sauce/Picture.o sauce/Scorelist.o sauce/Spark.o sauce/UI.o sauce/colors.o sauce/deviceInfo.o sauce/drawhelper.o sauce/pBill.o
CUSTOMPFLAGS=-DHIRESBUILD

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c pBil -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: resources/ui.rcp resources/graphicResources.rcp
	$(PILRC) -D PALMHIRES resources/ui.rcp
	$(PILRC) -D PALMHIRES resources/graphicResources.rcp
	touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag

