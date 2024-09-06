MAINPATH = /usr

INCPATH = -I$(MAINPATH)/local/palmdev/sdk

INC = $(INCPATH) $(INCPATH)/Core $(INCPATH)/Core/UI $(INCPATH)/Core/System $(INCPATH)/Core/Hardware -I$(MAINPATH)/local/palmdev/TRG/include

LIB = -L$(MAINPATH)/m68k-palmos/lib

CC = $(MAINPATH)/bin/m68k-palmos-gcc

FLAGS = -O2

OBJ = wav_1.o
APPNAME = "GMPlugin-WavPlayer"
CRID = CRMF
RCP = wav.rcp
PRC = GM_WavPlayer.prc

all: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o palm_obj $(INC) $(LIB)
	$(MAINPATH)/bin/m68k-palmos-obj-res palm_obj
	$(MAINPATH)/bin/pilrc $(RCP)
	$(MAINPATH)/bin/build-prc $(PRC) $(APPNAME) $(CRID) *.grc *.bin -t GMPL
	rm *.bin
	rm *.grc
	rm *.o
	rm palm_obj
	ls -al *.prc

.c.o:
	$(CC) $(FLAGS) -c $< $(INC) $(LIB)
