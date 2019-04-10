#include "RtspStream.h"
#include "Realization.h"

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
#include <cstdio>
#include <unistd.h>
#include <error.h>
#include <cstring>
#include <pthread.h>
#ifdef MAKESTREAMFILE
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
#endif

static std::string GetErr(int err,std::string& ErrString)
{
	char tmp[64] = {0};
	av_make_error_string(tmp,AV_ERROR_MAX_STRING_SIZE,err);
	std::string ErrTmp = tmp;
	ErrString = ErrTmp;
	return ErrTmp;
}

RtspStream::RtspStream(const char* URL,MyObs *obj)
{
	p_realization = new Realization(URL,obj);
}

RtspStream::~RtspStream()
{
	if(p_realization)
	{
		delete p_realization;
		p_realization = nullptr;
	}
}

bool RtspStream::UpdataUrl(const char* url)
{
	this->p_realization->UpdataUrl(url);
	return true;
}

void RtspStream::startRecvStream()
{
	this->p_realization->startRecvStream();
	/*
	if(inputRul.empty())
	{
		return;
	}
	int err = pthread_create(&pthreadId,NULL,ReadingThread,(void*)this);
	if(-1 == err)
	{
		std::cout<<strerror(err)<<std::endl;
		return;
	}
	//pthread_join(pthreadId,NULL);
	*/
}

void RtspStream::stopRecvStream()
{
	this->p_realization->stopRecvStream();
	/*
	b_stop_status = true;
	if(pthreadId)
	{
		if(!pthread_join(pthreadId,NULL))
		{
			close(pthreadId);
			pthreadId = 0;
		}
	}
	CloseInputStream();
	*/
}

Realization::Realization(const char *URL,MyObs* obj)
{
	inputRul = URL;
	p_inputAVFormatCxt = nullptr;
	p_bsfcH264 = nullptr;
	videoStreamIndex = -1;
	audioStreamIndex = -1;
	b_stop_status = false;
	b_InputInited = false;
	frame_rate = 25;
	pthreadId = 0;
	ErrString = "NO ERROR";
	obs = obj;
	avformat_network_init();
	av_register_all();

}

Realization::~Realization()
{
	stopRecvStream();
}

bool Realization::UpdataUrl(const char*URL)
{
	inputRul = URL;
	return true;
}

void* Realization::ReadingThread(void* arg)
{
	Realization *pRtsp = (Realization*)arg;
	pRtsp->run();
	std::cout<<"ReadingThread exited!"<<std::endl;
}

void Realization::startRecvStream()
{
	if(inputRul.empty())
	{
		return;
	}
	int err = pthread_create(&pthreadId,NULL,ReadingThread,(void*)this);
	if(-1 == err)
	{
		std::cout<<strerror(err)<<std::endl;
		return;
	}
	//pthread_join(pthreadId,NULL);
}

void Realization::stopRecvStream()
{
	b_stop_status = true;
	if(pthreadId)
	{
		if(!pthread_join(pthreadId,NULL))
		{
			close(pthreadId);
			pthreadId = 0;
		}
	}
	CloseInputStream();
}
void Realization::run()
{
	b_stop_status = false;
	OpenInputStream();
	readAndMux();
	CloseInputStream();
}

void Realization::readAndMux()
{
	AVPacket pkt = {0};
	av_init_packet(&pkt);
#ifdef MAKESTREAMFILE
	int audio = open("recv_audio.mp3",O_APPEND|O_CREAT|O_WRONLY,0644);
	int video = open("recv_video.h264",O_APPEND|O_CREAT|O_WRONLY,0644);
#endif
	while(true)
	{
		if(b_stop_status)
		{
			break;
		}
		int err = av_read_frame(p_inputAVFormatCxt,&pkt);
		if(err < 0)
		{
			std::cout<<__FILE__<<" "<<__func__<<" "<<__LINE__<<" : "<<GetErr(err,ErrString)<<std::endl;
			return;
		}
		if(pkt.stream_index == videoStreamIndex)
		{
			av_bitstream_filter_filter(p_bsfcH264,p_inputAVFormatCxt->streams[videoStreamIndex]->codec,NULL,&pkt.data,&pkt.size,pkt.data,pkt.size,pkt.pts);
			//处理视频
#ifdef MAKESTREAMFILE
			write(video,pkt.data,pkt.size);
#endif
			obs->OnVideoData(pkt.data,pkt.size);
		}
		else if(pkt.stream_index == audioStreamIndex)
		{
#ifdef MAKESTREAMFILE
			write(audio,pkt.data,pkt.size);
#endif
			obs->OnAudioData(pkt.data,pkt.size);
		}
	}

#ifdef MAKESTREAMFILE
	close(audio);
	close(video);
#endif
	av_free_packet(&pkt);
}

bool Realization::OpenInputStream()
{
	if(p_inputAVFormatCxt)
	{
		std::cout<<"have AVFormat,not open Stream"<<std::endl;
		return false;
	}
	p_bsfcH264 = av_bitstream_filter_init("h264_mp4toannexb");
	if(!p_bsfcH264)
	{
		std::cout<<"can not creat H264-MP4TOANNEXB"<<std::endl;
		return false;
	}

	int err = avformat_open_input(&p_inputAVFormatCxt,inputRul.c_str(),NULL,NULL);
	if(err < 0)
	{
		std::cout<<__FILE__<<" "<<__func__<<" "<<__LINE__<<" : "<<GetErr(err,ErrString)<<std::endl;
		return false;
	}
	err = avformat_find_stream_info(p_inputAVFormatCxt,0);
	if(err < 0)
	{
		std::cout<<__FILE__<<" "<<__func__<<" "<<__LINE__<<" : "<<GetErr(err,ErrString)<<std::endl;
		return false;
	}
	av_dump_format(p_inputAVFormatCxt,0,inputRul.c_str(),0);
	
	for(size_t i = 0;i<p_inputAVFormatCxt->nb_streams;++i)
	{
		AVStream *in_stream = p_inputAVFormatCxt->streams[i];
		if(in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = i;
			if(in_stream->avg_frame_rate.den != 0 &&
			   in_stream->avg_frame_rate.num != 0)
			{
				frame_rate = in_stream->avg_frame_rate.num/in_stream->avg_frame_rate.den;
			}
		}
		else if(in_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStreamIndex = i;
		}
	}
	b_InputInited = true;
	return true;
}

bool Realization::CloseInputStream()
{
	if(p_inputAVFormatCxt)
	{
		avformat_close_input(&p_inputAVFormatCxt);
		p_inputAVFormatCxt = nullptr;
	}
	if(p_bsfcH264)
	{
		av_bitstream_filter_close(p_bsfcH264);
		p_bsfcH264 = nullptr;
	}
	b_InputInited = false;
	return true;
}

