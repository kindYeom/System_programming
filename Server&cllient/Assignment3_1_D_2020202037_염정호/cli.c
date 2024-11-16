///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/17 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-1
///////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>

#define MAX_BUF 20
#define CONT_PORT 20001

void print_data(const char *print); // Function prototype for printing data to the standard output
void log_in(int sockfd);            // Function prototype for handling the login process

int main(int argc, char *argv[])
{
    int sockfd, n, p_pid;        // Socket file descriptor and other variables
    struct sockaddr_in servaddr; // Structure to hold server address information

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open socket

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));        // Zero out the structure
    servaddr.sin_family = AF_INET;                 // Set address family to Internet
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // Set IP address from command line arguments
    servaddr.sin_port = htons(atoi(argv[2]));      // Set port number from command line arguments

    // Connect to the server
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // Handle the login process
    log_in(sockfd);

    // Close the socket
    close(sockfd);
    return 0;
}

void log_in(int sockfd)
{
    int n;                                     // Variable to store number of bytes read
    char user[MAX_BUF], *passwd, buf[MAX_BUF]; // Buffers for user input and server messages
    memset(buf, 0, MAX_BUF);                   // Zero out the buffer

    // Read initial response from the server
    if (n = read(sockfd, buf, MAX_BUF) == 0)
    {
        print_data("server closed socket\n"); // If no data read, print message and return
    }

    // Handle server response
    if (strcmp(buf, "REJECTION") == 0)
    {
        print_data("** Connection refused **\n"); // Connection refused by the server
        close(sockfd);
        return;
    }
    else if (strcmp(buf, "ACCEPTED") == 0)
    {
        print_data("** It is connected to Server **\n"); // Connection accepted by the server
    }
    else
    {
        print_data("something wrong option\n"); // Unexpected response from the server
        return;
    }

    memset(buf, 0, MAX_BUF); // Zero out the buffer for the next use

    // Loop to handle user login attempts
    for (;;)
    {
        memset(user, 0, MAX_BUF); // Zero out the user buffer

        print_data("Input ID :");              // Prompt for user ID
        read(0, user, sizeof(user));           // Read user ID from standard input
        write(sockfd, user, strlen(user) + 1); // Send user ID to the server

        passwd = getpass("input passwd : ");


        write(sockfd, passwd, strlen(passwd) + 1); // Send password to the server

        // Read server response to login attempt
        n = read(sockfd, buf, MAX_BUF);
        buf[n] = '\0'; // Null-terminate the buffer
        if (!strcmp(buf, "OK"))
        {
            print_data("** User ‘");       // Print success message
            user[strlen(user) - 1] = '\0'; // Remove newline character from user input
            print_data(user);
            print_data("’ logged in **\n");

            close(sockfd); // Close the socket
            return;
        }
        else if (!strcmp(buf, "FAIL"))
        {
            print_data("** Log-in failed **\n"); // Print failure message
            memset(buf, 0, MAX_BUF);             // Zero out the buffer for next use
        }
        else
        {
            print_data("** Connection closed **\n"); // Print connection closed message
            return;
        }
    }
}

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print); // Get the length of the string
    write(1, print, length);    // Write the string to standard output
    return;
}
