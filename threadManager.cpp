#include "threadManager.h"

ThreadManager::ThreadManager(int RequestsPerPerson, int SizeOfBuffer, int NumberOfWorkers)
{

	m_requestsPerPerson = RequestsPerPerson;
	m_sizeOfBuffer = SizeOfBuffer;
	m_numberOfWorkers = NumberOfWorkers;

	finish1 = false;
	finish2 = false;
	finish3 = false;
	// printf("This application supports:\n\tRequests Per Person: %d\n\tBuffer Size: %d\n\tTotal Worker Threads: %d\n",m_requestsPerPerson, m_sizeOfBuffer, m_numberOfWorkers);

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

	initWorkerThreads();

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

void ThreadManager::dequeueRequestBufferEnqueueResponseBuffer(string strRequestChannel)
{
	RequestChannel dataChan(strRequestChannel, RequestChannel::CLIENT_SIDE);

	while(!v_requestBuffer->isDone()){
		RequestPackage newPackage = v_requestBuffer->V();
		newPackage.requestDequed = clock();
		if (v_requestBuffer->isDone()) break;
		string strReply = dataChan.send_request("data " + newPackage.personRequested);
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

	string reply7 = dataChan.send_request("quit");
	// cout << "Reply to request 'quit' is '" << reply7 << "'" << endl;
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
	std::thread requestThread1 (&ThreadManager::enqueueRequestBuffer, this, "Joe Smith");
	std::thread requestThread2 (&ThreadManager::enqueueRequestBuffer, this, "Jane Smith");
	std::thread requestThread3 (&ThreadManager::enqueueRequestBuffer, this, "John Doe");
	requestThread1.join();
	requestThread2.join();
	requestThread3.join();
}



void ThreadManager::initWorkerThreads(){
	for (int i = 0; i < m_numberOfWorkers; ++i){
		string strServerThreadRequest = m_controlChannel->send_request("newthread");
		// cout << "Reply to request 'newthread' is " << strServerThreadRequest << "'" << endl;

		v_workerThreads.push_back(std::thread(&ThreadManager::dequeueRequestBufferEnqueueResponseBuffer, this, strServerThreadRequest));
	}
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
			printf("Loading...\n");
			i = 0;
		}
		i++;

	}

	clientCloser();
}

void ThreadManager::clientCloser(){
	std::string strUserInput;
	// while (cin>>strUserInput){
	// 	if (strUserInput == "quit"){
	// 		break;
	// 	}
	// }

	v_requestBuffer->setDone(true);
	v_responseBuffer1->setDone(true);
	v_responseBuffer2->setDone(true);
	v_responseBuffer3->setDone(true);

	joinRequestThreads();
	// printf("NOTICE: Request threads complete!\n");
	joinWorkerThreads();
	// printf("NOTICE: Worker threads complete!\n");
	joinStatisticsThreads();
	// printf("NOTICE: Statistics threads complete!\n");

	string reply = m_controlChannel->send_request("quit");
	// cout << "Reply to request 'quit' is '" << reply << "'" << endl;

	usleep(100000); //Just for print formatting.

	processResults(v_requestBuffer1Results);
	processResults(v_requestBuffer2Results);
	processResults(v_requestBuffer3Results);

	printf("\n ---------------------------------- \n Thanks for using our program!\n ---------------------------------- \n");
}


void ThreadManager::joinRequestThreads(){

}

void ThreadManager::joinWorkerThreads(){
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
