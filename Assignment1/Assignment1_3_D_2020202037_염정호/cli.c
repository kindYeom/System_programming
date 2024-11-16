///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/04/16 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-3 ( cli ) //
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// InsertNode //
// ================================================================= //
// Input:
//
//
//
//
///////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>

int main(int argc, char *argv[])
{
    int i = 0;
    int k;                                                                                             // Integer used for looping through multiple arguments
    char *to_srv = NULL;                                                                               // String to be sent to the server
    size_t len;                                                                                        // Variable to store the size of the output string
    char *command[] = {"ls", "dir", "pwd", "cd", "mkdir", "delete", "rmdir", "rename", "quit", NULL};  // List of commands
    char *convert[] = {"NLST", "LIST", "PWD", "CWD", "MKD", "DELE", "RMD", "RNFR&RNTO", "QUIT", NULL}; // List of commands to be sent to the server

    while (command[i] != NULL)
    {
        if (strcmp(command[i], argv[1]) == 0) // Check if the command exists in the array
            break;
        i++;
    }

    if (command[i] == NULL)
    {
        write(0, "known option or instruction\n", strlen("known option or instruction\n")); // If an unknown command is received
        write(1, "\0", 1);
        return 0;
    }
    else if (strcmp(argv[1], "cd") == 0) // If the command is cd and the argument is ..
    {

        if (strcmp(argv[2], "..") == 0)
            convert[i] = "CDUP";
    }

    if (argc == 2) // Command + something
    {
        to_srv = (char *)malloc(strlen(convert[i]) + 1); // Expand the length of the array and insert data
        strcpy(to_srv, convert[i]);
    }

    else if (argc == 3) // Command + something
    {
        to_srv = (char *)malloc(strlen(convert[i]) + strlen(argv[2]) + 2); // Expand the length of the array and insert data
        strcat(to_srv, convert[i]);
        strcat(to_srv, " ");
        strcat(to_srv, argv[2]);
    }
    else if (argc > 3) // Command with options and targets
    {
        to_srv = (char *)malloc(strlen(convert[i])); // Expand according to convert.
        len = strlen(to_srv);
        for (k = 2; k < argc; k++)
            len = len + strlen(argv[k]); // Expand the length of the array and insert data

        len = len + argc - 1;

        to_srv = (char *)malloc(len); // Expand the length of the array and insert data

        strcat(to_srv, convert[i]); // Append

        for (int j = 2; j < argc; j++)
        {
            strcat(to_srv, " ");
            strcat(to_srv, argv[j]);
        }
    }
    len = strlen(to_srv);
    write(0, to_srv, len + 1); // String to be output
    write(0, "\n", 1);
    write(1, to_srv, len + 1); // String to be sent to the server
    return 0;
}

