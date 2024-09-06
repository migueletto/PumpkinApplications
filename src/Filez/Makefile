include ../common.mak
include ../commonp.mak

PROGRAM=Filez
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=DetailsVFSForm.o DetailsRForm.o SelectionChecker.o \
     FilterRForm.o FolderDetails.o SetAttributes.o \
     HexForm.o InformationForm.o CardSet.o \
     Main.o MenuForm.o PrefForm.o RecListForm.o \
     PreferencesForm.o Chooser.o TreeView.o \
     TreeViewForm.o Item.o ItemFolder.o ItemFile.o ItemSet.o \
     Stuph.o Tabs.o BusyIndicator.o UI.o \
     resize.o DIA.o

CUSTOMPFLAGS=-fpermissive
CXXFLAGS=$(CFLAGS)
CC=g++

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Filz -n $(PROGRAM) *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res.flag: filez.rcp
	$(PILRC) filez.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res.flag

