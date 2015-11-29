#include "threadManager.h"

ThreadManager::ThreadManager(int RequestsPerPerson, int SizeOfBuffer, int NumberOfRequestChannels)
{

	m_requestsPerPerson = RequestsPerPerson;
	m_sizeOfBuffer = SizeOfBuffer;
	m_numberOfRequestChannels = NumberOfRequestChannels;

	finish1 = false;
	finish2 = false;
	finish3 = false;
	// printf("This application supports:\n\tRequests Per Person: %d\n\tBuffer Size: %d\n\tTotal Worker Threads: %d\n",m_requestsPerPerson, m_sizeOfBuffer, m_numberOfRequestChannels);

	// printf("Establishing control channel... ");
	m_controlChannel = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
  // printf("done.\n");

	v_requestBuffer = new Semaphore(m_sizeOfBuffer);
	v_responseBuffer1 = new Semaphore(m_sizeOfBuffer);
	v_responseBuffer2 = new Semaphore(m_sizeOfBuffer);
	v_responseBuffer3 = new Semaphore(m_sizeOfBuffer);
}

ThreadManager::~ThreadManager()
{
	delete m_controlChannel;
	delete v_requestBuffer;
	delete v_responseBuffer1;
	delete v_responseBuffer2;
	delete v_responseBuffer3;
}

void ThreadManager::StartClient()
{
	printf("Started Client\n ");

	initEventThread();

	initStatisticsThreads();

	initRequestThreads();

	checkClose();
	//clientCloser();
}

void ThreadManager::enqueueRequestBuffer(string personRequested)
{
	for(int requestNum = 1; requestNum < m_requestsPerPerson + 1; requestNum++)
	{
		RequestPackage rqstPckg;
		rqstPckg.personRequested = personRequested;
		rqstPckg.requestNumber = requestNum;
		rqstPckg.requestEnqued = clock();

		v_requestBuffer->P(rqstPckg);
	}
}

void ThreadManager::dequeueRequestBufferEnqueueResponseBuffer()
{
	vector<RequestChannel*> v_allRequestChannels;
	vector<int> v_readInts;
	vector<int> v_writeInts;
	int maxReadVal = -1;
	int maxWriteVal = -1;

	fd_set readfs;
	fd_set writefs;

	FD_ZERO(&readfs);
	FD_ZERO(&writefs);

	FD_SET(STDIN, &readfs);
	FD_SET(STDIN, &writefs);

	// We need to make and save the new request channels from the given size
	for (int i = 0; i < m_numberOfRequestChannels; ++i) {
		// Create new channels server side;
		string strServerThreadRequest = m_controlChannel->send_request("newthread");

		RequestChannel* dataChan = new RequestChannel(strServerThreadRequest, RequestChannel::CLIENT_SIDE);
		//Instantiate our request channels
		v_allRequestChannels.push_back(dataChan);

		int readInt = v_allRequestChannels[i]->read_fd();
 		v_readInts.push_back(readInt);
		if (readInt > maxReadVal) {
			maxReadVal = readInt;
		}

		int writeInt = v_allRequestChannels[i]->write_fd();
		v_writeInts.push_back(writeInt);
		if (writeInt > maxWriteVal) {
			maxWriteVal = writeInt;
		}

		// We need to save all of the read/write file descriptors from all of the request channels
		FD_SET(v_allRequestChannels[i]->read_fd(), &readfs);
		FD_SET(v_allRequestChannels[i]->write_fd(), &writefs);
	}
	

	while(!v_requestBuffer->isDone())
	{
		fd_set read_dup = readfs;
		fd_set write_dup = writefs;

		int wNum = select(maxWriteVal + 1, NULL, &write_dup, NULL, NULL);

		int writePlace = -1;
		for (int i = 0; i < m_numberOfRequestChannels; ++i) {
			if (FD_ISSET(v_allRequestChannels[i]->write_fd(), &writefs)) {
				writePlace = i;
				break;
			}
		}

		if (writePlace == -1) continue;

		RequestPackage newPackage = v_requestBuffer->V();
		newPackage.requestDequed = clock();
		if (v_requestBuffer->isDone()) break;

		// Using the open channel found using the select, we get the file descriptor and call cread and cwrite to get the information, which will then be saved to 
		v_allRequestChannels[writePlace]->cwrite("data " + newPackage.personRequested);

		int rNum = select(maxReadVal + 1, &read_dup, NULL, NULL, NULL);
		

		if (rNum < 1 && wNum < 1) continue;
		//We need to check all of the file descriptors using select to ensure which channel we will use.
		int readPlace = -1;
		for (int i = 0; i < m_numberOfRequestChannels; ++i) {
			if (FD_ISSET(v_allRequestChannels[i]->read_fd(), &read_dup)) {
				readPlace = i;
				break;
			}
		}

		if (readPlace == -1) continue;

		string strReply = v_allRequestChannels[readPlace]->cread();
		//string strReply = "99";
		newPackage.serverResponse = strReply;
		newPackage.requestReplied = clock();


		if (newPackage.personRequested == "Joe Smith"){
			v_responseBuffer1->P(newPackage);
		}
		else if (newPackage.personRequested == "Jane Smith"){
			v_responseBuffer2->P(newPackage);
		}
		else if (newPackage.personRequested == "John Doe"){
			v_responseBuffer3->P(newPackage);
		}
	}

	//We need to quit all of the request channels.
	for (auto& t : v_allRequestChannels) t->send_request("quit");
}

void ThreadManager::dequeueResponseBuffer1(){
	while(!v_responseBuffer1->isDone()){
		RequestPackage newPackage = v_responseBuffer1->V();
		newPackage.requestProcessed = clock();
		if (v_responseBuffer1->isDone()) break;
		v_requestBuffer1Results.push_back(newPackage);

		if (v_requestBuffer1Results.size() == m_requestsPerPerson)
		{
			finish1 = true;
			break;
		}
	}
}

void ThreadManager::dequeueResponseBuffer2(){
	while(!v_responseBuffer2->isDone()){
		RequestPackage newPackage = v_responseBuffer2->V();
		newPackage.requestProcessed = clock();
		if (v_responseBuffer2->isDone()) break;
		v_requestBuffer2Results.push_back(newPackage);

		if (v_requestBuffer2Results.size() == m_requestsPerPerson)
		{
			finish2 = true;
			break;
		}
	}


}

void ThreadManager::dequeueResponseBuffer3(){
	while(!v_responseBuffer3->isDone()){
		RequestPackage newPackage = v_responseBuffer3->V();
		newPackage.requestProcessed = clock();
		if (v_responseBuffer3->isDone()) break;
		v_requestBuffer3Results.push_back(newPackage);

		if (v_requestBuffer3Results.size() == m_requestsPerPerson)
		{
			finish3 = true;
			break;
		}
	}
}

void ThreadManager::initRequestThreads(){
	v_requestThreads.push_back(std::thread (&ThreadManager::enqueueRequestBuffer, this, "Joe Smith"));
	v_requestThreads.push_back(std::thread (&ThreadManager::enqueueRequestBuffer, this, "Jane Smith"));
	v_requestThreads.push_back(std::thread (&ThreadManager::enqueueRequestBuffer, this, "John Doe"));
	
}



void ThreadManager::initEventThread(){
	EventThread = new std::thread(&ThreadManager::dequeueRequestBufferEnqueueResponseBuffer, this);
}

void ThreadManager::initStatisticsThreads(){
	v_staticticsThreads.push_back(std::thread (&ThreadManager::dequeueResponseBuffer1, this));
	v_staticticsThreads.push_back(std::thread (&ThreadManager::dequeueResponseBuffer2, this));
	v_staticticsThreads.push_back(std::thread (&ThreadManager::dequeueResponseBuffer3, this));
}

void ThreadManager::checkClose(){
	int i = 0;
	while (finish1 == false || finish2 == false || finish3 == false){
		if(i%1000000==0){
			//printf("Loading...\n");
			i = 0;
		}
		i++;

	}

	clientCloser();
}

void ThreadManager::clientCloser(){
	std::string strUserInput;

	v_requestBuffer->setDone(true);
	v_responseBuffer1->setDone(true);
	v_responseBuffer2->setDone(true);
	v_responseBuffer3->setDone(true);

	joinRequestThreads();
	joinEventThread();
	joinStatisticsThreads();

	string reply = m_controlChannel->send_request("quit");

	usleep(100000); //Just for print formatting.

	processResults(v_requestBuffer1Results);
	processResults(v_requestBuffer2Results);
	processResults(v_requestBuffer3Results);

	printf("\n ---------------------------------- \n Thanks for using our program!\n ---------------------------------- \n");
}


void ThreadManager::joinRequestThreads(){
	for (auto& t : v_requestThreads) t.join();
}

void ThreadManager::joinEventThread(){
	for (auto& t: v_workerThreads) t.join();
}

void ThreadManager::joinStatisticsThreads(){
	for (auto& t: v_staticticsThreads) t.join();
}

void ThreadManager::processResults(std::vector<RequestPackage> requestPackages)
{
	std::string personRequested = requestPackages[0].personRequested;
	float averagetimeInRequestBuffer = 0.00;
	float averagetimeForReply = 0.00;
	float averagetimeInResponseBuffer = 0.00;
	std::vector<double> v_responseDistribution(5,0);
 	for(auto& pack: requestPackages){
		averagetimeInRequestBuffer += (((float)(pack.requestDequed - pack.requestEnqued))/CLOCKS_PER_SEC);
		averagetimeForReply += (((float)(pack.requestReplied - pack.requestDequed))/CLOCKS_PER_SEC);
		averagetimeInResponseBuffer += (((float)(pack.requestProcessed - pack.requestDequed))/CLOCKS_PER_SEC);

 		int serverResponse = std::stoi(pack.serverResponse);
		if(serverResponse > 80) v_responseDistribution[0]++;
		else if(serverResponse > 60) v_responseDistribution[1]++;
		else if(serverResponse > 40) v_responseDistribution[2]++;
		else if(serverResponse > 20) v_responseDistribution[3]++;
		else if(serverResponse > 0) v_responseDistribution[4]++;
	}

	averagetimeInRequestBuffer = averagetimeInRequestBuffer/requestPackages.size();
	averagetimeForReply = averagetimeForReply/requestPackages.size();
	averagetimeInResponseBuffer = averagetimeInResponseBuffer/requestPackages.size();

	for(auto& resp: v_responseDistribution){
		resp = (resp/requestPackages.size())*100;
	}

	printf("\n%s spent an average of:\n\t%f seconds in the Request Buffer\n\t%f seconds waiting for a Reply\n\t%f seconds in the Response Buffer\n",
		personRequested.c_str(), averagetimeInRequestBuffer, averagetimeForReply, averagetimeInResponseBuffer);

	printf("With %lu responses the replies were distributed as shown below...(one * means about %lu responses)", requestPackages.size(), requestPackages.size()/100 );
	int scale = 0;
	for(auto& count: v_responseDistribution){
		int range = scale*20;
		if(range==0) printf("\nValues greater than 00:");
		else printf("\nValues greater than %i:", range);
		for(int i = 0; i < count; i++){
			printf("*");
		}
		scale++;
	}
	printf("\n");

}
