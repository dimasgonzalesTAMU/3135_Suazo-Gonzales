#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <exception>
#include "threadManager.h"
#include "semaphore.h"

/*Global Options*/
int RequestsPerPerson;
int SizeOfBuffer;
int NumberOfRequestChannels;
void getOption(int argc, char *argv[ ])
{
    char option;

    //Specifying the expected options
    while ((option = getopt(argc, argv, "n:b:w:")) != -1)
    {
        switch (option) {
              case 'n' :
                  RequestsPerPerson = atoi(optarg);
                  break;
			  case 'b' :
                  SizeOfBuffer = atoi(optarg);
                  break;
			  case 'w' :
                  NumberOfRequestChannels = atoi(optarg);
                  break;
              case '?':
                  if (optarg == "n" || optarg == "b" || optarg == "w")
                    fprintf (stderr, "Option -%c requires an argument.\n", optarg);
                  else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n",  optarg);
              default:
                  abort ();
          }
    }

    if(RequestsPerPerson == 0) RequestsPerPerson = 10000;
    if(SizeOfBuffer == 0) SizeOfBuffer = 100;
    if(NumberOfRequestChannels == 0) NumberOfRequestChannels = 10;

}

int main(int argc, char **argv)
{

	getOption(argc, argv);
    pid_t pid = fork();
    try
    {
        int retVal = 0;
        if (pid == 0)// child process
        {
            retVal = execl("./dataserver","./dataserver", NULL);

            if(retVal == -1){
                throw "DataServer Execution failed";
            }
        }
        else if (pid > 0) // parent process
        {
    		ThreadManager threadManager(RequestsPerPerson, SizeOfBuffer, NumberOfRequestChannels);
            // ThreadManager threadManager(100000, 100, 40);
            threadManager.StartClient();
        }
        else
        {
            // fork failed
            printf("fork() failed!\n");
            return 1;
        }
    }
    //TODO figure out how to printf exceptions
    // catch (exception& e)
    // {
    //     printf("%s\n",e.what().c_str());
    // }
    catch(char const* e)
    {
        printf("%s\n", e);
    }
    catch(...)
    {
        printf("Catched an unknown error\n");
    }


	return 0;
}
