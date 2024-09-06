include ../common.mak
include ../commonp.mak

PROGRAM=GentleMan
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=fm_1.o fm_2-file_db.o fm_3-init.o fm_4-gui.o fm_5-filelist.o fm_6-anim.o fm_7-sorting.o fm_8-details.o fm_9-files.o fm_10-copy.o fm_11-prefs.o fm_12-plugin.o

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c CRFM -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: fm.rcp
	$(PILRC) fm.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag
