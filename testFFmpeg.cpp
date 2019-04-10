#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>


#include "RtspStream.h"
class TestObject:public MyObs
{
	
	public:
		TestObject(const char* audioFilename,const char* videoFilename)
		{
			audiofd	= open(audioFilename,O_APPEND|O_CREAT|O_WRONLY,0644);
			videofd = open(videoFilename,O_APPEND|O_CREAT|O_WRONLY,0644);
		}
		~TestObject()
		{
			close(audiofd);
			close(videofd);
	//		LOG();
		}
		void OnVideoData(const unsigned char* data, int dataSize)
		{
			write(videofd,data,dataSize);
		}
		void OnAudioData(const unsigned char* data, int dataSize)
		{
			write(audiofd,data,dataSize);
		}
	private:
		int audiofd,videofd;
};

int main(int argc,char *argv[])
{

	if(argc != 2)
	{
		std::cout<<argv[0]<<" , URL"<<std::endl;
		return -1;
	}
	TestObject obj("recvAudio1.mp3","recvVideo2.h264");
	MyObs *test = &obj;
	RtspStream rtspWork(argv[1],test);
	rtspWork.startRecvStream();
	rtspWork.stopRecvStream();
	return 0;
}

