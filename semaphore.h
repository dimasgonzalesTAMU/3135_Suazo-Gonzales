/*
    File: semaphore.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 08/02/11

*/

#ifndef _semaphore_H_                   // include file only once
#define _semaphore_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <mutex>
#include <condition_variable>
#include <string>
#include <ctime>
#include "BoundedBuffer.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

struct RequestPackage
{
	std::string personRequested;
	int requestNumber;
	std::string serverResponse;
	clock_t requestEnqued;
	clock_t requestDequed;
	clock_t requestReplied;
	clock_t requestProcessed;
};

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   S e m a p h o r e  */
/*--------------------------------------------------------------------------*/

class Semaphore {
private:
  /* -- INTERNAL DATA STRUCTURES
     You may need to change them to fit your implementation. */

  // int             value;
  // pthread_mutex_t m;
  // pthread_cond_t  c;
  BoundedBuffer<RequestPackage>* bf_mp4;
  std::mutex the_mutex;
  std::condition_variable the_notfull_cvar;
  std::condition_variable the_notempty_cvar;
  bool bIsDone;

public:

  /* -- CONSTRUCTOR/DESTRUCTOR */

  Semaphore(int _sizeOfBuffer);

  ~Semaphore();

  /* -- SEMAPHORE OPERATIONS */

  void P(RequestPackage _content);
  RequestPackage V();

	bool isDone();
	void setDone(bool areWeDone);
	int returnNumberOfElems();
};


#endif
