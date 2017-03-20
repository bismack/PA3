#include <iostream>
#include <string.h>
#include <omp.h>
#include <unistd.h>

#include <rpc/rpc.h>

extern "C" {
#include "server_appendxdr.h"
#include "server_verifyxdr.h"
}
using namespace std;

CLIENT *c1;
CLIENT *c2;
// INPUT int;

bool static stringLengthMaxed = false;
char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
int counter = 0;

void update(server_letter xdrmessage, int thread_id) {

  if (*rpc_append_1(&xdrmessage, c1) == 0) {
    cout << thread_id << " : added" << endl;
  } else if (*rpc_append_1(&xdrmessage, c1) == -1) {
    stringLengthMaxed = true;
    cout << "string has been completed" << endl;
  } else {
    cout << "err" << endl;
  }
}

void RPCAppend() {
  int thread_id = omp_get_thread_num();
  
  server_letter xdrmessage; 
  xdrmessage.letter = alphabet[thread_id];

  while (stringLengthMaxed == false) {
      unsigned int microsleep = rand() % 500 + 100;
      usleep(microsleep);
      
      if (!stringLengthMaxed) {
        #pragma omp critical
        update(xdrmessage, thread_id);
      }
   }
}

void RPC_InitAppendServer (int argc, char *argv[]) {

  char *server;
  char **sresult;


  // if (argc !=2){
  //   cerr <<"usage:" << argv[0] <<" hostname\n";
  //   exit(1);
  // }
  cout << argv[8] << endl;

  server = argv[8];

  if ((c1 = clnt_create(server, APPEND_PROG, APPEND_VERS, "udp")) == NULL){
       clnt_pcreateerror(server);
    exit(2);
  }

  server_appendxdr xdrmessage; //structure appendxdr defined in appendxdr.x

  // long temp_long = 1;
  // char *temp_str = (char*)"Client is testing";
  
  //initialize xdrmessage
  int N = strtol(argv[2], NULL, 10);
  xdrmessage.F = strtol(argv[1], NULL, 10);
  xdrmessage.L = strtol(argv[3], NULL, 10);
  xdrmessage.M = strtol(argv[4], NULL, 10);
  xdrmessage.c0 = argv[5][0];
  xdrmessage.c1 = argv[6][0];
  xdrmessage.c2 = argv[7][0];
  
  if ((sresult = rpc_initappendserver_1(&xdrmessage, c1)) == NULL){
    clnt_perror(c1, server);
    exit(4);
  }

  // cout << "Client call server successfully\n ";
  // cout << "Server send message back:\n " << server << " = " <<*sresult<<"\n";
  
  #pragma omp parallel num_threads(N)
  RPCAppend();

}

void RPC_InitVerifyServer (int argc, char *argv[]) {
  char *server;
  char **sresult;

  server = argv[9];

  if ((c2 = clnt_create(server, VERIFY_PROG, VERIFY_VERS, "udp")) == NULL){
       clnt_pcreateerror(server);
    exit(2);
  }

  server_verifyxdr xdrmessage;
  xdrmessage.N = strtol(argv[2], NULL, 10);
  xdrmessage.L = strtol(argv[3], NULL, 10);
  xdrmessage.M = strtol(argv[4], NULL, 10);

  if ((sresult = rpc_initverifyserver_1(&xdrmessage, c2)) == NULL){
    clnt_perror(c2, server);
    exit(4);
  }

}

void checkSegmentProp(int N) {
  char **sresult;
  int thread_id = omp_get_thread_num();
  // char *seg; 
  server_segment xdrmessage;
  xdrmessage.seg = thread_id;

  while (!sresult=='\0') {
    #pragma omp critical
    sresult = rpc_getseg_1(&xdrmessage, c2);
    cout << "THREAD: " << thread_id << " SEG" << xdrmessage.seg << " = " << *sresult << "\n";

  }

  // omp_set_lock(&mutex);
  // if (verifyF(inSt.F, inSt.c0, inSt.c1, inSt.c2, *sresult) == true) {
  //   numOfSegmentsSatisfied++;
  // }
  // omp_unset_lock(&mutex);


   // omp_set_lock(&mutex);
   // if (verifyF(inSt.F, inSt.c0, inSt.c1, inSt.c2, strSeg) == true) {
   //    numOfSegmentsSatisfied++;
   // }
   // omp_unset_lock(&mutex);

   // D(printf("-----THREAD: %i Substr: %s   Satisfied: %i\n", thread_id, strSeg.c_str(), numOfSegmentsSatisfied)); 
}

void RPC_GetSeg(int N, int M) {
  #pragma omp parallel num_threads(N)
  checkSegmentProp(N);
  counter++;
   // writeOutputFile();
}


int main(int argc, char *argv[]) {
  cerr <<"-------Program start---------\n";

  RPC_InitVerifyServer(argc, argv);
  RPC_InitAppendServer(argc, argv);
  RPC_GetSeg(strtol(argv[2], NULL, 10), strtol(argv[4], NULL, 10));
  clnt_destroy(c1);
  clnt_destroy(c2);
  exit(0);
  return(0);
}
