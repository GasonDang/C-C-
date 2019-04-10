# C-C-
Amazing
#ifndef _RTSP_STREAM_H__
#define _RTSP_STREAM_H__
extern "C"
{
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libavcodec/avcodec.h"
	#include "libavfilter/avfilter.h"
	#include "libavutil/time.h"
}
#include <iostream>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define LOG() \
	int file = open("runlog.log",O_APPEND|O_CREAT|O_WRONLY,0644);\
	char msg[1024] = {0};\
	sprintf(msg,"[DEBUG] %s %s %d\r\n",__FILE__,__func__,__LINE__);\
	write(file,msg,strlen(msg));\
	close(file);\

class MyObs
{
	public:
		virtual void OnVideoData(const unsigned char* data, int len) = 0;
		virtual void OnAudioData(const unsigned char* data, int len) = 0;
};
/*define MAKESTREAMFILE can creat file about the video or audio file*/
//#define MAKESTREAMFILE
class Realization;
class RtspStream
{
public:
	RtspStream(){}
	RtspStream(const char*,MyObs*);
	virtual ~RtspStream();
	bool UpdataUrl(const char*);
	void startRecvStream();
	void stopRecvStream();
private:
	Realization *p_realization;
};
#endif
