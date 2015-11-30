/*
File: threadManager.h

class ThreadManager
 This object will serve as the manager for request, worker and statistics threads
*/
#ifndef _threadManager_H_                   // include file only once
#define _threadManager_H_

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <ctime>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "reqchannel.h"
#include "semaphore.h"

#define STDIN 0  // file descriptor for standard input

class ThreadManager
{
	//Declare ThreadManager's data members and helper functions
	public:
		//The constructor innitializes the control RequestChannel and whatever
		//other things that need to get done in the beginning
		ThreadManager(int req, int buf, int wok);
		~ThreadManager();

		void StartClient();

	private:
		//std::vector<RequestThread> v_requestThreads;
		std::vector<RequestPackage> v_requestBuffer1Results;
		std::vector<RequestPackage> v_requestBuffer2Results;
		std::vector<RequestPackage> v_requestBuffer3Results;

		std::vector<std::thread> v_requestThreads;
		std::vector<std::thread> v_workerThreads;
		std::vector<std::thread> v_staticticsThreads;
		std::vector<RequestChannel> v_workerChannels;
		RequestChannel* m_controlChannel;

		Semaphore* v_requestBuffer;
		Semaphore* v_responseBuffer1;
		Semaphore* v_responseBuffer2;
		Semaphore* v_responseBuffer3;

		std::thread* EventThread;

		int m_requestsPerPerson;
		int m_sizeOfBuffer;
		int m_numberOfRequestChannels;

		void enqueueRequestBuffer(string personRequested);
		void dequeueRequestBufferEnqueueResponseBuffer();
		void dequeueResponseBuffer1();
		void dequeueResponseBuffer2();
		void dequeueResponseBuffer3();

		void initRequestThreads();
		void initEventThread();
		void initStatisticsThreads();

		void checkClose();
		void clientCloser();

		void joinRequestThreads();
		void joinEventThread();
		void joinStatisticsThreads();

		void processResults(std::vector<RequestPackage> reqPacks);

		bool finish1;
		bool finish2;
		bool finish3;
};

#endif
