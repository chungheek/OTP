CC = gcc
CFLAGS=-g -Wall -std=gnu99

all: keygen enc_server enc_client dec_server dec_client

keygen:
	${CC} ${CFLAGS} -o keygen keygen.c

enc_server:
	${CC} ${CFLAGS} -o enc_server enc_server.c

enc_client:
	${CC} ${CFLAGS} -o enc_client enc_client.c

dec_server:
	${CC} ${CFLAGS} -o dec_server dec_server.c

dec_client:
	${CC} ${CFLAGS} -o dec_client dec_client.c

clean:
	-rm *.o keygen enc_server enc_client dec_server dec_client