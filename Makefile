CC=g++
CCFLAGS=-g -Wall -fopenmp 
# DEPS= append.h valinput.h choose.h
# OBJS= msg.o verify.o valinput.o choose.o

# ifeq ($(txt), 1)
# OPTS = -DDEBUG
# endif

all: prog.x

# %.o: %.cpp $(DEPS)
# 	$(CC) $(OPTS) $(CCFLAGS) -c -o $@ $<

# msg.x: $(OBJS) new
# 	$(CC) $(CCFLAGS) $(OBJS) -o msg.x 
# 	./msg.x 0 3 6 3 a b c 

prog.x: new
	rpcgen server_appendxdr.x
	rpcgen server_verifyxdr.x
	cc -c *.c	
	$(CC) $(CCFLAGS) -o client client.C server_appendxdr_xdr.o server_appendxdr_clnt.o server_verifyxdr_xdr.o server_verifyxdr_clnt.o
	$(CC) $(CCFLAGS) -o server_append server_append.C server_appendxdr_xdr.o server_appendxdr_svc.o
	$(CC) $(CCFLAGS) -o server_verify server_verify.C server_verifyxdr_xdr.o server_verifyxdr_svc.o
	# ./client 0 3 5 3 a b c 127.0.0.1 127.0.0.1
# test:
# 	./msg.x 0 3 4 3 a b c 
# 	echo "./msg.x 0 3 4 3 a b c">>out.txt
# 	cat out.txt>>total.txt
# 	./msg.x 1 3 5 3 a b c 
# 	echo "./msg.x 1 3 5 3 a b c">>out.txt
# 	cat out.txt>>total.txt
# 	./msg.x 2 3 3 3 a b c 
# 	echo "./msg.x 2 3 3 3 a b c">>out.txt
# 	cat out.txt>>total.txt
# 	./msg.x 3 3 4 3 a b c 
# 	echo "./msg.x 3 3 4 3 a b c">>out.txt
# 	cat out.txt>>total.txt
	
new:

clean:
	rm *.o *.c client server_append server_verify server_appendxdr.h server_verifyxdr.h
