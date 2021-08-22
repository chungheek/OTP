#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>
#include "utility.h"

#define BUFF_SIZE 70000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

int main(int argc, char* argv[])
{
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char cipher_buff[BUFF_SIZE];
    char key_buff[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    int totalSize = 0; // used to see how many characters
    int cipher_size = 0;
    int key_size = 0;

    // Check usage & args
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s ciphertext key port\n", argv[0]);
        exit(0);
    }

    int ciphertext_fd = open(argv[1], O_RDWR);
    if (ciphertext_fd == -1)
    {
        printf("open() failed on \"%s\"\n", argv[1]);
        perror("Error");
        exit(1);
    }
    cipher_size = read(ciphertext_fd, cipher_buff, BUFF_SIZE);
    close(ciphertext_fd);

    int key_fd = open(argv[2], O_RDWR);
    if (key_fd == -1)
    {
        printf("open() failed on \"%s\"\n", argv[2]);
        perror("Error");
        exit(1);
    }
    key_size = read(key_fd, key_buff, BUFF_SIZE);
    close(key_fd);

    if (key_size < cipher_size)
    {
        printf("Key size is too short. Exiting program\n");
        fprintf(stderr, "Key size is too short. Exiting program\n", 39);
        exit(1);
    }

    if (isIncorrect(cipher_buff) == 1)
    {
        printf("The file %s contains incorrect letters\n", argv[1]);
        fprintf(stderr, "File contains incorrect letters. Exiting program\n", 49);
        exit(1);
    }

    totalSize = strlen(cipher_buff);
    cipher_buff[totalSize - 1] = '@';
    key_buff[totalSize - 1] = '@';

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
    {
        clientError("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStructClient(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("CLIENT: ERROR connecting");
        clientError("CLIENT: ERROR connecting");
    }

    // Let server know that you are a decryption client
    int dec_msg = send_all(socketFD, "DEC_CLIENT@", 11, 0);
    // Receive server message that it's okay to continue
    int server_recv = recv_all(socketFD, buffer);
    if (server_recv < 0) clientError("Client ERROR reading from socket for cipher");
    if (strcmp(buffer, "OK") != 0)
    {
        fprintf(stderr, "DEC_CLIENT tried to connect to incorrect server!\n");
        exit(2);
    }
    else
    {
        int first_msg = send_all(socketFD, cipher_buff, totalSize, 0);

        // Receive server message for ciphertext
        int first_recv = recv_all(socketFD, buffer);
        if (first_recv < 0) clientError("Client ERROR reading from socket for cipher");

        int sec_msg = send_all(socketFD, key_buff, totalSize, 0);

        // Receive server message for key
        int sec_recv = recv_all(socketFD, buffer);
        if (sec_recv < 0) clientError("Client ERROR reading from socket for key");

        // Send wait message
        int send_wait = send_all(socketFD, "Waiting for plain text@", 24, 0);

        // Receive server message for key
        int cipher_recv = recv_all(socketFD, buffer);
        if (cipher_recv < 0) clientError("Client ERROR reading from socket for plain text");
        printf("%s\n", buffer);
        fflush(stdout);
    }

    // Close the socket
    close(socketFD);

    return 0;
}