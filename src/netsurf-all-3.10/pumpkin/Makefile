include ../Makefile.common
ROOT=$(PUMPKINOS)

include $(ROOT)/src/common.mak
include $(ROOT)/src/commonp.mak

PROGRAM=NetSurf
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Nsrf -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): ../netsurf/nsfb$(SOEXT)
	cp ../netsurf/nsfb$(SOEXT) $(DLIB)

res.flag: resource.rcp data.zip
	$(PILRC) resource.rcp; touch res.flag

data.zip: data/welcome.html
	cd data; zip -r ../data.zip * ; cd ..

clean:
	rm -f $(PRC) $(PROGRAM).prc data.zip *.bin res.flag
