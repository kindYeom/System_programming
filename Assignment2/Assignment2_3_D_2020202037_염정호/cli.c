///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/11 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-3
// Description : This program is a client-side implementation for a custom FTP-like protocol. It connects to a server specified by IP address and port number, sends commands entered by the user, and receives responses from the server. The supported commands include ls, dir, pwd, cd, mkdir, delete, rmdir, rename, and quit. Upon receiving SIGINT, it sends a quit command to the server and terminates.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define BUF_SIZE 256
void trim_whitespace(char *str);    // Function to remove leading and trailing whitespace
void print_data(const char *print); // Function to print execution results
int sockfd;                         // Socket descriptor

int main(int argc, char **argv)
{
    void sh_int(int);       // Signal handler for SIGINT
    signal(SIGINT, sh_int); /* Applying signal handler(sh_chld) for SIGINT */

    int n;
    int k; // Integer used for looping through multiple arguments
    size_t len;

    char data_in[BUF_SIZE];                                                                            // Buffer to store the user input
    char *command[] = {"ls", "dir", "pwd", "cd", "mkdir", "delete", "rmdir", "rename", "quit", NULL};  // List of commands
    char *convert[] = {"NLST", "LIST", "PWD", "CWD", "MKD", "DELE", "RMD", "RNFR&RNTO", "QUIT", NULL}; // Corresponding commands to be sent to the server

    char *divide;             // Pointer for dividing input string
    char *to_srv = NULL;      // String to be sent to server
    int PORT = atoi(argv[2]); // Port number
    char recv_from_srv[4000]; // Buffer to store server response

    int index_num = 0, cmd_num; // Index counter
    char *index[20] = {NULL};   // Array to store separated tokens

    struct sockaddr_in serv_addr; // Socket address structure

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open socket

    memset(&serv_addr, 0, sizeof(serv_addr));                          // Initialize address structure
    serv_addr.sin_family = AF_INET;                                    // Address family
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);                    // IP address
    serv_addr.sin_port = htons(atoi(argv[2]));                         // Port
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); // Connect to server

    while (1)
    {
        memset(data_in, 0, BUF_SIZE); // Reset buffer
        divide = "\0";
        fflush(stdout);
        write(STDOUT_FILENO, "> ", 2);                   // Print prompt
        ssize_t bytes_read = read(0, data_in, BUF_SIZE); // Read user input

        if (data_in[0] == '\n')
        {
            write(sockfd, "\n", strlen("\n")); // Send newline character to server
            if (!(read(sockfd, recv_from_srv, 4000) > 0))
            {
                close(sockfd);
                return 0;
            }
            else
            {
                memset(recv_from_srv, 0, 4000); // Clear receive buffer
                continue;
            }
        }

        data_in[strlen(data_in) - 1] = '\0'; // Remove newline character
        trim_whitespace(data_in);            // Remove leading and trailing whitespace
        divide = strtok(data_in, " ");       // Tokenize input string
        index_num = 0;

        while (divide != NULL)
        {
            index[index_num] = divide;  // Store token
            divide = strtok(NULL, " "); // Find next token
            index_num++;                // Move to next index
        }

        cmd_num = 0; // Initialize command index
        while (command[cmd_num] != NULL)
        {
            if (strcmp(command[cmd_num], index[0]) == 0) // Check if command exists
                break;
            cmd_num++;
        }

        if (command[cmd_num] == NULL)
        {
            to_srv = index[0]; // If command is not found, send it as is
        }

        else if (index_num == 1) // Command without arguments
        {
            print_data("\n"); // Print newline
            to_srv = (char *)malloc(strlen(convert[cmd_num]) + 1); // Allocate memory for converted command
            strcpy(to_srv, convert[cmd_num]); // Copy converted command
        }

        else if (index_num == 2) // Command with one argument
        {
            to_srv = (char *)malloc(strlen(convert[cmd_num]) + strlen(index[1]) + 2); // Allocate memory for command and argument
            strcat(to_srv, convert[cmd_num]); // Copy command
            strcat(to_srv, " "); // Add space
            strcat(to_srv, index[1]); // Copy argument
        }
        else if (index_num > 2) // Command with multiple arguments
        {
            to_srv = (char *)malloc(strlen(convert[cmd_num])); // Allocate memory for command
            len = strlen(to_srv);
            for (k = 1; k < index_num; k++)
                len = len + strlen(index[k]); // Calculate total length
            len = len + index_num - 1;

            to_srv = (char *)malloc(len); // Allocate memory for command and arguments

            strcat(to_srv, convert[cmd_num]); // Copy command

            for (k = 1; k < index_num; k++) // Append arguments
            {
                strcat(to_srv, " ");
                strcat(to_srv, index[k]);
            }
        }

        to_srv[strlen(to_srv)] = '\0'; // Null-terminate string

        if (write(sockfd, to_srv, strlen(to_srv)) != strlen(to_srv)) // Send to server
        {
            write(STDERR_FILENO, "write() error!!\n", sizeof("write() error!!\n"));
            exit(1);
        }

        if (n = read(sockfd, recv_from_srv, 4000) > 0) // Receive response from server
        {
            printf("%s\n", recv_from_srv);  // Print received data
            memset(recv_from_srv, 0, 4000); // Clear receive buffer
        }

        else
            break;
    }
    close(sockfd); // Close socket

    return 0;
}

void sh_int(int signum)
{
    write(sockfd, "QUIT", strlen("QUIT") + 1); // Send "QUIT" message to server
    close(sockfd);                             // Close socket
    exit(1);                                   // Exit program
}

void trim_whitespace(char *str)
{
    // Remove leading whitespace
    char *start = str;
    while (isspace(*start))
    {
        start++;
    }

    // Remove trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
    {
        end--;
    }
    *(end + 1) = '\0'; // Null-terminate string
}

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print);
    write(1, print, length);
    return;
}
