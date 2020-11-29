#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>

#define BUFF_SIZE 70000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Checks to see if plaintext has any incorrect letters 1 = true, 0 = false.
int isIncorrect(char *string)
{
  int i = 0;
  for (i; i < strlen(string); i++)
  {
    char c = string[i];
    int isLetter = 0;
    if (c >= 65 && c <= 90)
      isLetter = 1;
    int isSpace = 0;
    if (c == 32)
      isSpace = 1;
    int isCrNl = 0;
    if (c == '\n')
      isCrNl = 1;
    // If the character is not within acceptable bounds then return
    // that string is incorrect
    if (isLetter == 0 && isSpace == 0 && isCrNl == 0)
      return 1;
  }
  return 0;
}

// Error function used for reporting issues
void error(const char *msg)
{
  perror(msg);
  exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in *address, int portNumber)
{
  // Clear out the address struct
  memset((char *)address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Host name should be localhost
  struct hostent *hostInfo = gethostbyname("localhost");
  if (hostInfo == NULL)
  {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char *)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Send all method
// https://stackoverflow.com/a/14184228/6889483
int send_all(int socket, char *buffer, size_t length, int flags)
{
    // Use charsWritten to keep track of how many characters are written
    ssize_t charsWritten;
    // Use buffIdx to advance buffer pointer
    char* buffIdx = buffer;
    // Keep on looping until length reachs 0
    while (length > 0)
    {
        charsWritten = send(socket, buffIdx, length, flags);
        if (charsWritten <= 0)
            return -1;
        buffIdx += charsWritten;
        length -= charsWritten;
    }
    return 0;
}

// Receive all method (similar to send_all method)
// Uses termination character '@' to stop receving message
// https://stackoverflow.com/a/16256724/6889483
int recv_all(int socket, char *buffer)
{
    memset(buffer, '\0', BUFF_SIZE);
    ssize_t charsWritten;
    char* buffIdx = buffer;
    int length = BUFF_SIZE;
    // Will stop looping until termination character is in buffer
    while (strchr(buffer, '@') == NULL)
    {
        // Keep on looping until buffIdx is equal to greater than totalSize of message
        charsWritten = recv(socket, buffIdx, length, 0);
        if (charsWritten <= 0)
            return -1;
        buffIdx += charsWritten;
        length -= charsWritten;
    }
    // Replace the termination character with a null character in buffer before returning
    buffer[charsWritten - 1] = '\0';
    return 0;
}


int main(int argc, char *argv[])
{
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char plaintext_buff[BUFF_SIZE];
  char key_buff[BUFF_SIZE];
  char buffer[BUFF_SIZE];
  int totalSize = 0; // used to see how many characters
  int plaintext_size = 0;
  int key_size = 0;

  // Check usage & args
  if (argc < 4)
  {
    fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  int plaintext_fd = open(argv[1], O_RDWR);
  if (plaintext_fd == -1)
  {
    perror("Error");
    exit(1);
  }
  plaintext_size = read(plaintext_fd, plaintext_buff, BUFF_SIZE);
  close(plaintext_fd);

  int key_fd = open(argv[2], O_RDWR);
  if (key_fd == -1)
  {
    perror("Error");
    exit(1);
  }
  key_size = read(key_fd, key_buff, BUFF_SIZE);
  close(key_fd);

  if (key_size < plaintext_size)
  {
    fprintf(stderr, "Key size is too short. Exiting program\n", 39);
    exit(1);
  }

  if (isIncorrect(plaintext_buff) == 1)
  {
    fprintf(stderr, "File contains incorrect letters. Exiting program\n", 49);
    exit(1);
  }

  totalSize = strlen(plaintext_buff);
  plaintext_buff[totalSize-1] = '@';
  key_buff[totalSize-1] = '@';

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0)
  {
    error("CLIENT: ERROR opening socket");
  }

  // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server
  if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
  {
    printf("CLIENT: ERROR connecting");
    error("CLIENT: ERROR connecting");
  }
  // Let server know that you are an encryption client
  int enc_msg = send_all(socketFD, "ENC_CLIENT@", 11, 0);
  // Receive server message that it's okay to continue
  int server_recv = recv_all(socketFD, buffer);
  if (server_recv < 0) error("Client ERROR reading from socket for cipher");
  if (strcmp(buffer, "OK") != 0)
  {
      fprintf(stderr, "ENC_CLIENT tried to connect to incorrect server!\n");
      exit(2);
  }
  else
  {
      int first_msg = send_all(socketFD, plaintext_buff, totalSize, 0);

      // Receive server message for plaintext
      int first_recv = recv_all(socketFD, buffer);
      if (first_recv < 0) error("ERROR reading from socket for plaintext");
      //printf("Received from server: %s\n", buffer);
      //fflush(stdout);

      int sec_msg = send_all(socketFD, key_buff, totalSize, 0);

      // Receive server message for key
      int sec_recv = recv_all(socketFD, buffer);
      if (sec_recv < 0) error("ERROR reading from socket for key");
      //printf("Received from server: %s\n", buffer);
      //fflush(stdout);

      // Send wait message
      int send_wait = send_all(socketFD, "Waiting for cipher text@", 24, 0);

      // Receive server message for cipher
      int cipher_recv = recv_all(socketFD, buffer);
      if (cipher_recv < 0) error("ERROR reading from socket for cipher");
      printf("%s\n", buffer);
      fflush(stdout);
  }



  // Close the socket
  close(socketFD);
  return 0;
}