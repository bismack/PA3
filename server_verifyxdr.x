/*
 * server_verifyxdr.x  
 */
 
/*
 * Define a procedure
 *      rpc_initappendserver_1() 
 *
 */
 
struct server_verifyxdr {
	int N;
    int L;
    int M;
};

struct server_segment {
	int seg;
};


program VERIFY_PROG {
    version VERIFY_VERS {
    string RPC_INITVERIFYSERVER(server_verifyxdr) = 1;  /* procedure number = 1 */
    string RPC_GETSEG(server_segment) = 2;
    } = 1;                          /* version number = 1 */
} = 0x61234567;                     /* program number = 0x61234567 */