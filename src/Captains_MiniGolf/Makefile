include ../common.mak
include ../commonp.mak

PROGRAM=MiniGolf
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=sauce/appCoordinator.o sauce/database.o sauce/deviceInfo.o sauce/main.o \
     sauce/game/colors.o sauce/game/debugWindow.o sauce/game/drawhelper.o sauce/game/game.o sauce/game/gamesession.o sauce/game/inputPen.o sauce/game/level.o sauce/game/models.o sauce/game/movement.o sauce/game/polygon.o sauce/game/scoreWindow.o sauce/game/viewport.o \
     sauce/startscreen/about.o sauce/startscreen/startscreen.o

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c CGLF -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: resources/ui.rcp resources/graphicResources.rcp
	$(PILRC) resources/ui.rcp
	$(PILRC) resources/graphicResources.rcp
	touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag

