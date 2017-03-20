#include <iostream>
#include <string.h>
#include <omp.h>
#include <unistd.h>
#include <algorithm>

#include <rpc/rpc.h>

extern "C" {
#include "server_appendxdr.h"
#include "server_verifyxdr.h"
}
using namespace std;

CLIENT *c1;
CLIENT *c2;

struct INPUT {
  int F,N,L,M;
  char c0, c1, c2;
} st;


bool static stringLengthMaxed = false;
char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

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

  cout << argv[8] << endl;
  server = argv[8];

  if ((c1 = clnt_create(server, APPEND_PROG, APPEND_VERS, "udp")) == NULL){
       clnt_pcreateerror(server);
    exit(2);
  }

  server_appendxdr xdrmessage;
  xdrmessage.F = st.F;
  xdrmessage.L = st.L;
  xdrmessage.M = st.M;
  xdrmessage.c0 = st.c0;
  xdrmessage.c1 = st.c1;
  xdrmessage.c2 = st.c2;
  
  if ((sresult = rpc_initappendserver_1(&xdrmessage, c1)) == NULL){
    clnt_perror(c1, server);
    exit(4);
  }
  
  #pragma omp parallel num_threads(st.N)
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
  xdrmessage.N = st.N;
  xdrmessage.L = st.L;
  xdrmessage.M = st.M;

  if ((sresult = rpc_initverifyserver_1(&xdrmessage, c2)) == NULL){
    clnt_perror(c2, server);
    exit(4);
  }
}

bool verify(string str) {
  size_t occurOfC0 = count(str.begin(), str.end(), st.c0);
  size_t occurOfC1 = count(str.begin(), str.end(), st.c1);
  size_t occurOfC2 = count(str.begin(), str.end(), st.c2);

  switch (st.F) {
    case 0:
      if (occurOfC0 + occurOfC1 == occurOfC2)
        return(true);
      break;
    case 1:
      if (occurOfC0 + (2 * occurOfC1) == occurOfC2)
        return(true);
      break;
    case 2:
      if (occurOfC0 * occurOfC1 == occurOfC2)
        return(true);
      break;
    case 3:
      if (occurOfC0 - occurOfC1 == occurOfC2) 
        return(true);
      break;
   }  
   return(false);
}   

int checkSegmentProp() {
  string sresult;
  int thread_id = omp_get_thread_num();
  int isSatisfied=0;
  server_segment xdrmessage;
  xdrmessage.seg = thread_id;

  while (true) {
    #pragma omp critical
    sresult = rpc_getseg_1(&xdrmessage, c2)[0];
    if (sresult[0]=='-') 
      break; 
    else if (verify(sresult)==true) 
      isSatisfied++; 

    cout << "THREAD: " << thread_id << " SUBTR: " << sresult << " RESULT: " << isSatisfied << "\n";
  }
  return (isSatisfied);
}

void RPC_GetSeg() {
  int isSatisfied=0;
  #pragma omp parallel num_threads(st.N) reduction(+:isSatisfied)
  isSatisfied += checkSegmentProp();

  string sresult;
  server_segment xdrmessage;
  xdrmessage.seg = -1;
  sresult = rpc_getseg_1(&xdrmessage, c2)[0];

  cout <<"---------STRING: " << sresult << " SEGMENTS SATISFIED: " << isSatisfied << "\n";
   // writeOutputFile();
}


int main(int argc, char *argv[]) {
  cerr <<"-------Program start---------\n";
  st.F = strtol(argv[1], NULL, 10);
  st.N = strtol(argv[2], NULL, 10);
  st.L = strtol(argv[3], NULL, 10);
  st.M = strtol(argv[4], NULL, 10);
  st.c0 = argv[5][0];
  st.c1 = argv[6][0];
  st.c2 = argv[7][0];


  RPC_InitVerifyServer(argc, argv);
  // RPC_InitAppendServer(argc, argv);
  RPC_GetSeg();
  // clnt_destroy(c1);
  clnt_destroy(c2);
  exit(0);
  return(0);
}