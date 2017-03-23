#include <iostream>
#include <string.h>
#include <omp.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <rpc/rpc.h>

extern "C" {
#include "server_appendxdr.h"
#include "server_verifyxdr.h"
}
using namespace std;

CLIENT *c1;
CLIENT *c2;

struct INPUT {
  int F, N, L, M;
  char c0, c1, c2;
} st;

int totalStringLength;
int numOfSegmentsSatisfied = 0;
bool static stringLengthMaxed = false;
char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

// Return if the string can be built from inputs.
int isPossible(int F, int N, int L) {
  if (N > 3) {
    return 1;
  } else if (F == 0 && L%2 == 0) {
    return 1;
  } else if (F == 1 && L != 1) {
    return 1;
  } else if (F == 2) {
    return 1;
  } else if (F == 3 && L%2 == 0) {
    return 1;
  } else {
    return 0;
  }
}

void writeOutputFile(string S) {
   ofstream textFile;
   textFile.open("out.txt");
   textFile << S << endl;
   textFile << numOfSegmentsSatisfied << endl;
   textFile.close();
}

bool validateInput(int F, int N, int L, int M) {

   bool isValid = true;

   printf("Parsing input parameters... %i %i %i %i\n", F, N, L, M);

   if (!(0 <= F and F <= 3) || !(3 <= N and N <= 8) || L<1 || M<1) {

      printf("usage: ./pa1.x i N L M c0 c1 c2\n");
      printf("where: \n");
      printf("    i (0 <= i <= 3) is the index of the property Fi which each segment of S needs to satisfy; \n");
      printf("    N (3 <= N <= 8) is the number of threads; \n");
      printf("    L is the length of each segment of S;\n");
      printf("    M (M divides N) is the number of segments in S to generate, and \n");
      printf("    c0, c1, and c2 are the letters to be used in the property check.\n");

      printf("\n");
      printf("Please check input parameter F\n");
      isValid = false;

   }

   return isValid;
}

void transferCompletedString(){
  //Connnect to UDP
  server_segment xdrmessage;
  xdrmessage.seg = 3;
  rpc_getseg_1(&xdrmessage, c2);

  //Begin String Transfer
  server_letter xdrmsg;
  xdrmsg.letter = alphabet[3];
  rpc_append_1(&xdrmsg, c1);

  string completedString;
  completedString = rpc_getseg_1(&xdrmessage, c2)[0];
  cout << "SRESULT: " << completedString << endl;
}

void update(server_letter xdrmessage, int thread_id) {
  if (*rpc_append_1(&xdrmessage, c1) == 0) {
    cout << thread_id << " : added" << endl;
  } else if (*rpc_append_1(&xdrmessage, c1) == -1) {
    stringLengthMaxed = true;
    cout << "STRING COMPLETE" << endl;
    // transferCompletedString();
  } else {
    cout << "UPDATE STRING: ERROR OCCURED" << endl;
  }
}

void RPCAppend() {
  int thread_id = omp_get_thread_num();
  
  server_letter xdrmessage; 
  xdrmessage.letter = alphabet[thread_id];

  while (stringLengthMaxed == false) {
      unsigned int microsleep = rand() % 500 + 100;
      usleep(microsleep);
      
      #pragma omp critical
      if (!stringLengthMaxed) {
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
  // transferCompletedString();
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
    else if (verify(sresult) == true) 
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
  writeOutputFile(sresult);
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

  if (!validateInput(st.F, st.N, st.L, st.M)) {
    printf("Exiting program...\n");
    return 0;
  }

  if (!isPossible(st.F, st.N, st.L)) {
    printf("Segments of length L and alphabet size N are not capable of satisfying property F.\n Please select different parameters.\n");
    return 0;
  }
  
  totalStringLength = st.M * st.L;

  //generates a seed of pseudo-random numbers
  srand(time(0));

  RPC_InitVerifyServer(argc, argv);
  RPC_InitAppendServer(argc, argv);
  transferCompletedString();
  RPC_GetSeg();
  clnt_destroy(c1);
  clnt_destroy(c2);
  exit(0);

  return(0);
}