#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>

// Send all method
// https://stackoverflow.com/a/14184228/6889483
int send_all(int socket, char* buffer, size_t length, int flags)
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
int recv_all(int socket, char* buffer)
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

// Set up the address struct for the server socket
void setupAddressStructServer(struct sockaddr_in* address, int portNumber)
{

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

// Set up the address struct
void setupAddressStructClient(struct sockaddr_in* address, int portNumber)
{
    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Host name should be localhost
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Checks to see if plaintext has any incorrect letters 1 = true, 0 = false.
int isIncorrect(char* string)
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

// mod
//https://stackoverflow.com/a/4003287/6889483
int mod(int a, int b)
{
    int result = a % b;
    if (result < 0)
        result += b;
    return result;
}

// Error function used for reporting issues
void serverError(const char* msg)
{
    perror(msg);
    exit(1);
}

// Error function used for reporting issues
void clientError(const char *msg)
{
  perror(msg);
  exit(0);
}

