/*
 * server_appendxdr.x  
 */
 
/*
 * Define a procedure
 *      str_server_append_1() takes a structure parameter and returns a string
 *
 */
 
struct server_appendxdr {
    int F;
    int L;
    int M;
    char c0;
    char c1;
    char c2;
};

struct server_letter {
    char letter;
};

program APPEND_PROG {
    version APPEND_VERS {
    string RPC_INITAPPENDSERVER(server_appendxdr) = 1;  /* procedure number = 1 */
    int RPC_APPEND(server_letter) = 2;
    } = 1;                          /* version number = 1 */
} = 0x31234567;                     /* program number = 0x31234567 */