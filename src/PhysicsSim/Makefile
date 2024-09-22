include ../common.mak
include ../commonp.mak

PROGRAM=PhysicsSim

ZIG=zig
ZIGLIB=zig-out/lib/lib$(PROGRAM)$(SOEXT)

PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c PhyS -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(ZIGLIB)
	cp $(ZIGLIB) $(DLIB)

$(ZIGLIB): main.zig $(SRC)libpumpkin/pumpkin.zig
	$(ZIG) build
	touch $(ZIGLIB)

res.flag: resource.rcp
	$(PILRC) resource.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res.flag $(ZIGLIB)
