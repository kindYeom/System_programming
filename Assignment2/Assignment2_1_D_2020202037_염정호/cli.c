///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/01 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 ( cli ) //
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////

//
///////////////////////////////////////////////////////////////////////
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define MAX_BUFF 256

void trim_whitespace(char *str);    // Function to remove blank
void print_data(const char *print); // Function to print execution results

int main(int argc, char *argv[])
{
        int n;
        int k; // Integer used for looping through multiple arguments
        size_t len;

        char data_in[MAX_BUFF];                                                                                // Variable to store the size of the output string
        char *command[] = {"ls", /* "dir", "pwd", "cd", "mkdir", "delete", "rmdir", "rename",*/ "quit", NULL}; // List of commands
        char *convert[] = {"NLST", /*"LIST", "PWD", "CWD", "MKD", "DELE", "RMD", "RNFR&RNTO",*/ "QUIT", NULL}; // List of commands to be sent to the server

        int sd;                    // sever directroy
        struct sockaddr_in server; // socket address
        char *divide;              // divde input
        char *to_srv = NULL;       // send intr to srv
        int PORT = atoi(argv[2]);  // get port number
        char recv_from_srv[4000];  // string get output from srv

        int index_num = 0, cmd_num; // num of index
        char *index[20] = {NULL};   // inedx seprate

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // make socket
        {
                perror("socket"); // error
                exit(1);
        }

        memset((char *)&server, '\0', sizeof(server)); // reset server data
        server.sin_family = AF_INET;                   // domain
        server.sin_addr.s_addr = inet_addr(argv[1]);   // addr srv
        server.sin_port = htons(PORT);                 // prot

        if (connect(sd, (struct sockaddr *)&server, sizeof(server)))
        {
                perror("connect");
                exit(1);
        }
        // link socket

        for (;;)
        {
                memset(data_in, 0, MAX_BUFF); // data_in reset
                divide = "\0";
                fflush(stdout);                                  // clean buffer
                ssize_t bytes_read = read(0, data_in, MAX_BUFF); // get input

                data_in[strlen(data_in) - 1] = '\0';
                trim_whitespace(data_in);      // remove " "
                divide = strtok(data_in, " "); // Divide data by spaces and store it
                index_num = 0;
                while (divide != NULL)
                {
                        index[index_num] = divide;  // Store the cut string
                        divide = strtok(NULL, " "); // Find the next token
                        index_num++;                // Next array
                }

                cmd_num = 0; // number of cmd
                while (command[cmd_num] != NULL)
                {
                        if (strcmp(command[cmd_num], index[0]) == 0) // Check if the command exists in the array
                                break;
                        cmd_num++;
                }

                if (command[cmd_num] == NULL)
                {
                        write(0, "known option or instruction\n", strlen("known option or instruction\n")); // If an unknown command is received
                        write(1, "\0", 1);
                        return 0;
                }

                if (index_num == 1) // Command
                {
                        print_data("\n");
                        to_srv = (char *)malloc(strlen(convert[cmd_num]) + 1); // Expand the length of the array and insert data
                        strcpy(to_srv, convert[cmd_num]);
                }

                else if (index_num == 2) // Command + something
                {
                        to_srv = (char *)malloc(strlen(convert[cmd_num]) + strlen(index[1]) + 2); // Expand the length of the array and insert data
                        strcat(to_srv, convert[cmd_num]);
                        strcat(to_srv, " ");
                        strcat(to_srv, index[1]);
                }
                else if (index_num > 2) // Command with options and targets
                {
                        to_srv = (char *)malloc(strlen(convert[cmd_num])); // Expand according to convert.
                        len = strlen(to_srv);
                        for (k = 1; k < index_num; k++)
                                len = len + strlen(index[k]); // Expand the length of the array and insert data
                        len = len + index_num - 1;

                        to_srv = (char *)malloc(len); // Expand the length of the array and insert data

                        strcat(to_srv, convert[cmd_num]); // Append

                        for (k = 1; k < index_num; k++) // make output data
                        {
                                strcat(to_srv, " ");
                                strcat(to_srv, index[k]);
                        }
                }

                if (write(sd, to_srv, strlen(to_srv)) != strlen(to_srv)) // send to srv
                {
                        write(STDERR_FILENO, "write() error!!\n", sizeof("write() error!!\n"));
                        exit(1);
                }

                if ((n = read(sd, recv_from_srv, 4000 - 1)) < 0)
                {
                        write(STDERR_FILENO, "read() error\n", sizeof("read() error\n"));
                        exit(1);
                }
                recv_from_srv[n] = '\0';
                print_data(recv_from_srv);
                if (!strcmp(to_srv, "QUIT"))
                {
                        close(sd);
                        return 0;
                }
        }
}

void print_data(const char *print) // Function to print execution results
{
        int length = strlen(print);
        write(1, print, length);
        return;
}

void trim_whitespace(char *str)
{
        // delete front blank
        char *start = str;
        while (isspace(*start))
        {
                start++;
        }

        // remove back blank
        char *end = start + strlen(start) - 1;
        while (end > start && isspace(*end))
        {
                end--;
        }
        *(end + 1) = '\0';
}