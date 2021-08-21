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

// OTP decryption method
void decrypt(char* cipher, char* key, char* plain, int length)
{
    int i = 0;
    for (i; i < length; i++)
    {
        int cipher_char = cipher[i];
        int key_char = key[i];

        cipher_char = (cipher_char == 32) ? 26 : cipher_char - 65;
        key_char = (key_char == 32) ? 26 : key_char - 65;

        int ciph_min_key = cipher_char - key_char;
        int mod_27 = mod(ciph_min_key, 27);

        mod_27 = (mod_27 == 26) ? 32 : mod_27 + 65;

        plain[i] = mod_27;
    }
    // Add termination character at end of cipher message
    plain[i] = '@';
}

int main(int argc, char* argv[])
{
    int connectionSocket, charsRead;
    char buffer[BUFF_SIZE];
    char key_buff[BUFF_SIZE];
    char cipher_buff[BUFF_SIZE];
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
        (struct sockaddr*)&serverAddress,
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
        connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
        if (connectionSocket < 0)
            serverError("ERROR on accept");

        printf("SERVER: Connected to client running at host port %d\n", ntohs(clientAddress.sin_port));
        fflush(stdout);

        // Receive confirmation msg that ENC_CLIENT is connecting
        int charsRead = recv_all(connectionSocket, buffer);
        if (charsRead < 0) serverError("ERROR reading from socket for plaintext");
        if (strcmp(buffer, "DEC_CLIENT") != 0)
        {
            // Send an error message 'NO' to disconnect
            fprintf(stderr, "DEC SERVER: Incorrect client!\n");
            int error = send_all(connectionSocket, "NO@", 3, 0);
        }
        else
        {
            // Tell client it's okay to connect
            int okay = send_all(connectionSocket, "OK@", 3, 0);

            // Receive cipher text message from client
            int charsRead = recv_all(connectionSocket, cipher_buff);
            if (charsRead < 0) serverError("ERROR reading from socket for plaintext");
            // Set totalSize to length of cipher_buff
            totalSize = strlen(cipher_buff);
            printf("Received from client cipher: %s\n", cipher_buff);
            fflush(stdout);

            // Send a message that server received plaintext
            int send1 = send_all(connectionSocket, "I am the dec server, and I got your cipher text message@", 56, 0);

            // Receive key message from client
            charsRead = recv_all(connectionSocket, key_buff);
            if (charsRead < 0) serverError("ERROR reading from socket for key");
            printf("Received from client key: %s\n", key_buff);
            fflush(stdout);

            // Send a message that server received key
            int send2 = send_all(connectionSocket, "I am dec the server, and I got your key message@", 48, 0);

            // Receive wait message from client
            charsRead = recv_all(connectionSocket, buffer);
            if (charsRead < 0) serverError("ERROR reading from socket for key");
            printf("Received from client msg: %s\n", buffer);
            fflush(stdout);

            // Decrypt the message using plaintext and key buffers and send cipher message to client
            char* plain = calloc(1, totalSize);
            decrypt(cipher_buff, key_buff, plain, totalSize);
            int send_plain = send_all(connectionSocket, plain, strlen(plain), 0);
            // Free plain and set to null
            free(plain);
            plain = NULL;
        }
        close(connectionSocket);
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}