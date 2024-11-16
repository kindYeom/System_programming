///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/05/28 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-2
///////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Function declarations
void trim_whitespace(char *str);
char *convert_addr_to_str(unsigned long ip_addr, unsigned int port);
void print_data(const char *print); // Function to print execution results

void main(int argc, char **argv)
{
    int n;
    size_t read_n;
    char *to_srv = NULL; // Send instruction to server
    char *divide;        // Divide input
    char *hostport;
    char *index[30] = {NULL}; // Index separate
    char user_input[100];
    struct sockaddr_in temp, sock;
    int control_sockfd, data_sockfd, data_srv;
    char client_output[4000];
    char totalBytesReadStr[20];

    // Create a control socket
    control_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Set up the server address struct
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(argv[1]);
    sock.sin_port = htons(atoi(argv[2]));

    srand(time(0));
    // Connect to the server
    connect(control_sockfd, (struct sockaddr *)&sock, sizeof(sock));
    // Generate a random port for the data connection

    // User command input
    while (1)
    {

        memset(user_input, 0, 100);
        read(STDIN_FILENO, user_input, 100);
        user_input[strlen(user_input) - 1] = '\0'; // Remove newline character

        // Remove leading and trailing whitespace
        trim_whitespace(user_input);

        // Tokenize the input by spaces
        divide = strtok(user_input, " ");
        int index_num = 0;

        // Store each token in the index array
        while (divide != NULL)
        {
            index[index_num] = divide;  // Store the cut string
            divide = strtok(NULL, " "); // Find the next token
            index_num++;                // Move to the next array index
        }

        // Process the 'ls' command
        if (strcmp(index[0], "ls") == 0)
        {
            if (index_num == 1) // 'ls' command without arguments
            {
                to_srv = (char *)malloc(strlen("NLST") + 1); // Allocate memory and insert data
                strcpy(to_srv, "NLST");
            }
            else if (index_num == 2) // 'ls' command with one argument
            {
                to_srv = (char *)malloc(strlen("NLST") + strlen(index[1]) + 2); // Allocate memory and insert data
                strcpy(to_srv, "NLST ");
                strcat(to_srv, index[1]);
            }
        }
        else if (strcmp(index[0], "quit") == 0)
        {
            to_srv = (char *)malloc(strlen("QUIT") + 1); // Allocate memory and insert data
            strcpy(to_srv, "QUIT");
        }
        else
        {
            to_srv = (char *)malloc(strlen("wrong") + 1); // Allocate memory and insert data
            strcpy(to_srv, "wrong");
        }
        write(control_sockfd, to_srv, strlen(to_srv) + 1); // Send the command to the server
        free(to_srv);                                      // Free the allocated memory
        to_srv = NULL;                                     // Reset the pointer

        read(control_sockfd, client_output, 4000); // Read the server response
        memset(client_output, 0, 4000); // Clear the output buffer

        int random_port = 10000 + rand() % 20001; // Generate a random port for the data connection

        // Set up the temporary address struct for the data connection
        memset(&temp, 0, sizeof(temp));
        temp.sin_family = AF_INET;
        temp.sin_addr.s_addr = inet_addr(argv[1]);
        temp.sin_port = htons(random_port);
        // Convert the address to the desired string format
        hostport = convert_addr_to_str(temp.sin_addr.s_addr, temp.sin_port);
        if (strcmp(index[0], "quit") != 0)
        {
            print_data("converting to ");
            print_data(hostport);
            print_data("\n");
        }

        // Send the host and port information to the server
        write(control_sockfd, hostport, strlen(hostport));

        // Wait for the server's response
        while (1)
        {
            n = read(control_sockfd, client_output, sizeof(client_output)); // Read from the control socket
            client_output[n] = '\0'; // Ensure null-terminated string

            if (strcmp(client_output, "ready") == 0)
                break; // Break the loop if the server is ready

            print_data(client_output);
            print_data("\n");
            memset(client_output, 0, sizeof(client_output));
            write(control_sockfd, " ", 1); // Send acknowledgment
        }

        memset(client_output, 0, 4000); // Clear the output buffer

        // Create the data socket
        data_sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (data_sockfd < 0)
        {
            perror("socket error");
            exit(1);
        }

        // Set socket option SO_REUSEADDR
        int optval = 1;

        if (setsockopt(data_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        // Bind the data socket to the temporary address
        if (bind(data_sockfd, (struct sockaddr *)&temp, sizeof(temp)) < 0)
        {
            perror("bind error");
            exit(1);
        }

        // Listen for incoming connections
        if (listen(data_sockfd, 5) < 0)
        {
            perror("listen error");
            exit(1);
        }

        int len = sizeof(temp);

        write(control_sockfd, " ", 1); // Send acknowledgment

        // Accept a connection from the server
        data_srv = accept(data_sockfd, (struct sockaddr *)&temp, &len);
        if (data_srv < 0)
        {
            perror("accept error");
            exit(1);
        }

        // Read data from the server
        if (!(strcmp(index[0], "quit") == 0))
        {
            read(data_srv, client_output, sizeof(client_output)); // Read from the data socket
            print_data(client_output);
            n = strlen(client_output);

            memset(client_output, 0, 4000); // Clear the output buffer
            write(data_srv, " ", 1); // Send acknowledgment

            read(data_srv, client_output, sizeof(client_output)); // Read from the data socket
            print_data(client_output);
            print_data("\n");

            // Check if the server response indicates success
            if (atoi(client_output) == 226)
            {
                print_data("OK. ");
                sprintf(totalBytesReadStr, "%d", n); // Convert the number of bytes received to a string
                print_data(totalBytesReadStr);
                print_data(" bytes received.\n");
            }
            close(data_srv); // Close the data connection
        }
        else
        {
            read(data_srv, client_output, sizeof(client_output)); // Read the final message from the server
            print_data(client_output);

            break; // Exit the loop for the "quit" command
        }
        close(data_sockfd); // Close the data socket
        close(data_srv); // Close the data connection
        memset(client_output, 0, 4000); // Clear the output buffer
        memset(totalBytesReadStr, 0, 20); // Clear the byte count buffer
    }

    return;
}

// Convert IP address and port to a string in the desired format
char *convert_addr_to_str(unsigned long ip_addr, unsigned int port)
{
    struct in_addr ip_addr_struct;
    ip_addr_struct.s_addr = ip_addr;

    int random_port = ntohs(port);

    char ip_str[INET_ADDRSTRLEN];
    char *addr;

    if (inet_ntop(AF_INET, &ip_addr_struct, ip_str, INET_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }

    uint8_t upper_8_bits = (random_port >> 8) & 0xFF;
    uint8_t lower_8_bits = random_port & 0xFF;
    addr = (char *)malloc(INET_ADDRSTRLEN + 20);

    if (addr == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int ip1, ip2, ip3, ip4;
    sscanf(ip_str, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);

    // Format the IP address and port
    snprintf(addr, INET_ADDRSTRLEN + 20, "%d,%d,%d,%d,%u,%u", ip1, ip2, ip3, ip4, upper_8_bits, lower_8_bits);

    return addr;
}

// Function to print execution results
void print_data(const char *print)
{
    int length = strlen(print);
    write(1, print, length);
    return;
}

// Function to trim leading and trailing whitespace from a string
void trim_whitespace(char *str)
{
    // Delete front blank
    char *start = str;
    while (isspace(*start))
    {
        start++;
    }

    // Remove back blank
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
    {
        end--;
    }
    *(end + 1) = '\0';
}
