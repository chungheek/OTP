#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "utility.h"

#define BUFF_SIZE 70000

// OTP encryption method
void encrypt2(char* plaintext, char* key, char* cipher, int length)
{
    int i = 0;
    for (i; i < length; i++)
    {
        int pt_char = plaintext[i];
        int key_char = key[i];

        // setup characters for 0 - 26 value
        pt_char = (pt_char == 32) ? 26 : pt_char - 65;
        key_char = (key_char == 32) ? 26 : key_char - 65;

        // Add message + key
        int mess_plus_key = pt_char + key_char;
        // Get mod 27
        int mod_27 = mod(mess_plus_key, 27);

        // Add 65 to get a random char from A-Z
        mod_27 = (mod_27 == 26) ? 32 : mod_27 + 65;
        cipher[i] = mod_27;
    }
    // Add termination character at end of cipher message
    cipher[i] = '@';
}

int main(int argc, char *argv[])
{
  int connectionSocket, charsRead;
  char buffer[BUFF_SIZE];
  char key_buff[BUFF_SIZE];
  char plaintext_buff[BUFF_SIZE];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  int totalSize = 0;

  // Check usage & args
  if (argc < 2)
  {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0)
  {
    serverError("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStructServer(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
           (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) < 0)
  {
    serverError("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while (1)
  {
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
    if (connectionSocket < 0)
      serverError("ERROR on accept");

    printf("SERVER: Connected to client running at host port %d\n", ntohs(clientAddress.sin_port));
    fflush(stdout);

    // Receive confirmation msg that ENC_CLIENT is connecting
    int charsRead = recv_all(connectionSocket, buffer);
    if (charsRead < 0) serverError("ERROR reading from socket for plaintext");
    if (strcmp(buffer, "ENC_CLIENT") != 0)
    {
        // Send an error message 'NO' to disconnect
        fprintf(stderr, "SERVER: Incorrect client!\n");
        int error = send_all(connectionSocket, "NO@", 3, 0);
    }
    else
    {
        // Tell client it's okay to connect
        int okay = send_all(connectionSocket, "OK@", 3, 0);

        // Receive plaintext message from client
        int charsRead = recv_all(connectionSocket, plaintext_buff);
        if (charsRead < 0) serverError("ERROR reading from socket for plaintext");
        // Set totalSize to length of plaintext_buff
        totalSize = strlen(plaintext_buff);
        printf("Received from client plaintext: %s\n", plaintext_buff);
        fflush(stdout);

        // Send a message that server received plaintext
        int send1 = send_all(connectionSocket, "I am the server, and I got your plaintext message@", 50, 0);

        // Receive key message from client
        charsRead = recv_all(connectionSocket, key_buff);
        if (charsRead < 0) serverError("ERROR reading from socket for key");
        printf("Received from client key: %s\n", key_buff);
        fflush(stdout);

        // Send a message that server received key
        int send2 = send_all(connectionSocket, "I am the server, and I got your key message@", 44, 0);

        // Receive wait message from client
        charsRead = recv_all(connectionSocket, buffer);
        if (charsRead < 0) serverError("ERROR reading from socket for key");
        printf("Received from client msg: %s\n", buffer);
        fflush(stdout);

        // Encrypt the message using plaintext and key buffers and send cipher message to client
        char* cipher = calloc(1, totalSize);
        encrypt2(plaintext_buff, key_buff, cipher, totalSize);
        int send_cipher = send_all(connectionSocket, cipher, strlen(cipher), 0);
        // Free cipher and set to null
        free(cipher);
        cipher = NULL;
    }
    close(connectionSocket);
  }
  // Close the listening socket
  close(listenSocket);
  return 0;
}