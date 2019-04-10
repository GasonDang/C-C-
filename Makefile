FILE = ../FFmpegLib
FFMPEGINCLUDE = $(FILE)/include/
FFMPEGLIB = $(FILE)/lib/
PTHREAD = -lpthread
CC = gcc -std=gnu99 -Wall -g
CPP = g++ -std=c++11 -Wall -g
FFMPEGLINK = -lavformat -lavfilter -lavcodec -lswscale -lavutil -lswresample

INCLUDE = ./include 
LIB = ./lib

DYN = g++ -fPIC -shared
STA = g++ -static ## ar -r
LINK = RtspStream.o
CPPLINK = RtspStream.cpp
HLINK = RtspStream.h
ALL = libRtspStream.so libRtspStream.a testFFmpeg

all:$(ALL)

libRtspStream.so:$(CPPLINK)
	$(DYN) -o $@ $^
libRtspStream.a:$(LINK)
	ar -r $@ $^
testFFmpeg:$(LINK)
	$(CPP) -o $@ testFFmpeg.cpp $^ -I $(FFMPEGINCLUDE) \
							-L $(FFMPEGLIB) $(FFMPEGLINK) \
							$(PTHREAD) ## -D MAKESTREAMFILE
$(LINK):$(CPPLINK)
	$(CPP) -c $^

clean:
	rm -rf $(ALL) *.o tags

install:
	cp *.h $(INCLUDE)
	cp *.so *.a $(LIB) -r 

setEnvir:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(FFMPEGLIB)
setTags:
	ctags -R
	chmod 0666 tags

