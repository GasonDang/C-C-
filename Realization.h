#ifndef _REALIZATION_H__
#define _REALIZATION_H__

extern "C"
{
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libavcodec/avcodec.h"
	#include "libavfilter/avfilter.h"
	#include "libavutil/time.h"
}
#include <pthread.h>
#include <iostream>
#include "RtspStream.h"

class Realization
{
public:
	Realization (){}
	Realization(const char*,MyObs*);
	virtual ~Realization();
	bool UpdataUrl(const char*);
	void startRecvStream();
	void stopRecvStream();
private:
	static void* ReadingThread(void* arg); 
	void run();
	void readAndMux();
	
	bool OpenInputStream();
	bool CloseInputStream();
private:
	std::string inputRul;
	AVFormatContext* p_inputAVFormatCxt;
	AVBitStreamFilterContext* p_bsfcH264;
	int videoStreamIndex;
	int audioStreamIndex;
	bool b_stop_status;
	bool b_InputInited;
	int frame_rate;
	std::string ErrString;
	pthread_t pthreadId;
	MyObs *obs;
};
#endif
