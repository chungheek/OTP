#ifndef UTILITY_H
#define UTILITY_H

int send_all(int socket, char* buffer, size_t length, int flags);
int recv_all(int socket, char* buffer);
void setupAddressStructServer(struct sockaddr_in* address, int portNumber);
void setupAddressStructClient(struct sockaddr_in* address, int portNumber);
int isIncorrect(char* string);
int mod(int a, int b);
void serverError(const char *msg);
void clientError(const char *msg);
#endif
