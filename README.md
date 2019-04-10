# C-C-
Amazing
Before using this interface, we need to configure something. You can learn something from my Chinese blog. The link of the blog is as follows:

Https://blog.csdn.net/qq_41915225/article/details/88877250

Of course, you can also open the link directly, http://ffmpeg.org/download.html#releases, and download the version you need.

Next you need to type the following commands in turn:

Tar-jxvf ***. tar. bz2

## Compile dynamic libraries, not static libraries, not to mention the compilation configuration of documents:

/ configure -- prefix = buildout -- enable-shared -- disable-static -- disable-doc

## Increase compilation configuration supported by x264

/ configure -- prefix = buildout -- enable-shared -- disable-static -- disable-doc -- enable-gpl -- enable-libx264

## Now you can enter make

Make - J # If compiled slowly, you can add the parameter - J

Make install

Finally, you can copy the header files and libraries of the FFmpeg system library under the path installed above to. / FFmpegLib directory.

You can also directly modify the FILE parameters in Makefile below.

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

