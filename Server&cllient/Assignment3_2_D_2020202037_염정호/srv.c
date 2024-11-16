///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/28 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-2
// Description : Implementation of the server side for a simplified FTP-like protocol
///////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

char concatenate_output[4000];
// Function declarations
void bubbleSort(char *arr[], int n); // Bubble sort for sorting file lists
char *convert_str_to_addr(char *str, unsigned int *port);
void print_non_option(char *arr[], int n); // Print file names when there are no options
void write_code(int code, int control_Fd);
void print_data(const char *print); // Function to print execution results

void main(int argc, char **argv)
{
    int n;
    char *host_ip;
    int ls_error;
    char char_1[100];
    char temp[25];
    char *index;           // Pointer to divide data by spaces and store it
    int num_of_sindex = 0; // Number of elements in s_index array
    char *s_index[30];     // Array to store the separated data
    char input_form_client[100];
    unsigned int port_num;
    int control_fd, control_cli, data_fd;
    struct sockaddr_in control_addr, data_addr;

    // Create a control socket
    control_fd = socket(PF_INET, SOCK_STREAM, 0);

    // Set up the control address struct
    memset(&control_addr, 0, sizeof(control_addr));
    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Use any available address
    control_addr.sin_port = htons(atoi(argv[1]));     // Use the provided port number

    // Bind the control socket to the control address
    bind(control_fd, (struct sockaddr *)&control_addr, sizeof(control_addr));

    // Listen for incoming connections
    listen(control_fd, 5);

    int len = sizeof(control_addr);

    // Accept a connection from the client
    control_cli = accept(control_fd, (struct sockaddr *)&control_addr, &len);

    // Read data from the client
    while (1)
    {
        memset(input_form_client, 0, 100);
        n = read(control_cli, input_form_client, sizeof(input_form_client)); // Read data from the client
        input_form_client[n] = '\0';                                         // Null-terminate the input buffer
        index = strtok(input_form_client, " ");                              // Tokenize the input buffer
        num_of_sindex = 0;                                                   // Reset the index counter

        write(control_cli, " ", 2); // Acknowledge the received command

        memset(temp, 0, 25);
        read(control_cli, temp, 25); // Read the client's response
        temp[strlen(temp)] = '\0';

        // Print received address and port information (except for QUIT command)
        if (strcmp(index, "QUIT") != 0)
        {
            print_data(temp);
            print_data("\n");
        }

        // Convert the received string to an IP address and port number
        host_ip = convert_str_to_addr(temp, (unsigned int *)&port_num);

        // Tokenize the input command
        while (index != NULL) // Repeat until there are no more strings to tokenize
        {
            s_index[num_of_sindex] = index; // Store the tokenized string
            num_of_sindex++;                // Move to the next index in the array
            index = strtok(NULL, " ");      // Find the next token
        }

        // Check if the command is "NLST"
        if (strcmp(s_index[0], "NLST") == 0)
        {
            write_code(200, control_cli); // Send a success code
        }
        else if (strcmp(s_index[0], "QUIT") == 0)
        {
            write(control_cli, " ", 2); // Acknowledge the QUIT command
        }
        else
        {
            write_code(500, control_cli); // Send an error code
            return;
        }

        read(control_cli, char_1, sizeof(char_1)); // Read acknowledgment from client
        memset(char_1, 0, 100);

        // If the command is "NLST", process it
        if (strcmp(s_index[0], "NLST") == 0)
        {
            DIR *dp = NULL;          // Variable for directory traversal
            struct dirent *dirp = 0; // Structure to hold information about files within a directory
            char *dir = NULL;        // Variable to store the directory address to be searched

            char *files[100];   // Array to store the list of files
            int file_count = 0; // Integer to store the number of files in the list

            // Determine the directory to list
            if (num_of_sindex == 1)
            { // If only the command exists
                print_data("NLST\n");
                dir = "."; // Use the current directory
            }
            else if (num_of_sindex == 2) // If two arguments are received
            {
                print_data("NLST ");
                print_data(s_index[1]);
                print_data("\n");
                dir = s_index[1]; // Use the specified directory
            }

            // Try to open the directory
            if (opendir(dir) == NULL)
            {
                // Handle errors when opening the directory
                if (errno == EACCES)
                    strcpy(concatenate_output, "cannot access: Access denied\n");
                else
                    strcpy(concatenate_output, "No such directory\n");

                ls_error = 501;
                return;
            }
            else
            {
                dp = opendir(dir);            // Open the directory
                write_code(150, control_cli); // Send a code indicating data connection is opening
                read(control_cli, char_1, sizeof(char_1)); // Read acknowledgment from client
                memset(char_1, 0, 100);

                // Read files from the directory
                while ((dirp = readdir(dp)) != NULL)
                {
                    char file_path[300];                            // String to store the address of the file
                    struct stat file_stat;                          // String to store file information
                    sprintf(file_path, "%s/%s", dir, dirp->d_name); // Set the file location including the input arguments

                    files[file_count] = dirp->d_name; // Store the file name
                    stat(file_path, &file_stat);      // Get information about the file
                    if (S_ISDIR(file_stat.st_mode))
                    {
                        strcat(files[file_count], "/"); // Append '/' if it's a directory
                    }
                    file_count++; // Move to the next file
                }
                closedir(dp);

                bubbleSort(files, file_count); // Sort the list of file names

                print_non_option(files, file_count); // Print the sorted file names
                ls_error = 226;                      // Set the success code
            }
        }
        else if (strcmp(s_index[0], "QUIT") == 0)
        {
            print_data("QUIT\n"); // Print the QUIT command
            ls_error = 221;       // Set the success code for QUIT command
        }

        write(control_cli, "ready", strlen("ready")); // Indicate ready for data transfer

        read(control_cli, char_1, 100); // Read acknowledgment from client

        // Create a data socket
        data_fd = socket(AF_INET, SOCK_STREAM, 0);

        // Set up the data address struct
        memset(&data_addr, 0, sizeof(data_addr));
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(host_ip); // Use the host IP address
        data_addr.sin_port = htons(port_num);           // Use the port number

        // Connect to the client for data transfer
        connect(data_fd, (struct sockaddr *)&data_addr, sizeof(data_addr));

        // Send the concatenated output to the client
        if (ls_error != 221)
        {
            n = write(data_fd, concatenate_output, 4000); // Send file list to client
            read(data_fd, char_1, sizeof(char_1));        // Read acknowledgment from client
        }

        write_code(ls_error, data_fd); // Send the success or error code
        close(data_fd);                // Close the data connection

        if (strcmp(s_index[0], "QUIT") == 0)
        {
            close(control_cli); // Close the control connection
            break;
        }

        // Reset concatenate_output for the next command
        memset(concatenate_output, 0, sizeof(concatenate_output));
    }

    close(control_fd); // Close the control socket
    return;
}

// Convert a string to an IP address and port number
char *convert_str_to_addr(char *str, unsigned int *port)
{
    char *addr;
    int ip1, ip2, ip3, ip4;
    unsigned int upper_8_bits, lower_8_bits;
    addr = (char *)malloc(20);

    // Extract IP address and port number components from the string
    sscanf(str, "%d,%d,%d,%d,%u,%u", &ip1, &ip2, &ip3, &ip4, &upper_8_bits, &lower_8_bits);

    // Convert the IP address to dot-decimal notation
    snprintf(addr, INET_ADDRSTRLEN, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    // Combine the upper and lower 8 bits to form the port number
    *port = (upper_8_bits << 8) | lower_8_bits;

    return addr;
}

// Print file names when there are no options
void print_non_option(char *arr[], int n)
{
    int word = 0;

    for (int i = 0; i < n; i++)
    {
        if (arr[i][0] != '.') // Exclude hidden directories
        {
            strcat(concatenate_output, arr[i]);
            strcat(concatenate_output, "             ");
            word++;

            if (word > 5) // Change line every 5 words
            {
                word -= 5;
                strcat(concatenate_output, "\n");
            }
        }
    }
    strcat(concatenate_output, "\n");
}

// Bubble sort for sorting file names in a directory
void bubbleSort(char *arr[], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (strcmp(arr[j], arr[j + 1]) > 0)
            {
                char *temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Write a response code to the client
void write_code(int code, int control_Fd)
{
    switch (code)
    {
    case 150:
        write(control_Fd, "150 Opening data connection for directory list.", strlen("150 Opening data connection for directory list."));
        print_data("150 Opening data connection for directory list.\n"); // Data connection opening
        break;
    case 200:
        write(control_Fd, "200 Port command performed successful", strlen("200 Port command performed successful"));
        print_data("200 Port command performed successful\n"); // Port command success
        break;
    case 221:
        write(control_Fd, "221 Goodbye.", strlen("221 Goodbye."));
        print_data("221 Goodbye.\n"); // Transfer complete
        break;
    case 226:
        write(control_Fd, "226 Complete transmission.", strlen("226 Complete transmission."));
        print_data("226 Complete transmission.\n"); // Transfer complete
        break;
    case 500:
        write(control_Fd, "500: Syntax error, command unrecognized", strlen("500: Syntax error, command unrecognized"));
        print_data("500: Syntax error, command unrecognized\n"); // Command unrecognized
        break;
    case 501:
        write(control_Fd, "501: Syntax error in parameters or arguments", strlen("501: Syntax error in parameters or arguments"));
        print_data("501: Syntax error in parameters or arguments\n"); // Parameter error
        break;
    }
    return;
}

// Function to print execution results
void print_data(const char *print)
{
    int length = strlen(print);
    write(1, print, length); // Print to standard output
    return;
}
