///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
// Date : 2024/06/03 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-3
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
long get_file_size(FILE *file);                                      // Function to get the size of a file.
void trim_whitespace(char *str);                                     //  * Removes leading and trailing whitespace from a string.
char *convert_addr_to_str(unsigned long ip_addr, unsigned int port); // Converts an IP address and port to a specific formatted string.
void print_data(char *print);                                        // Function to print execution results
int log_in(int sockfd);
void handle_int(int sig); //  Signal handler for handling the SIGINT signal.
int type = 0;             // set mode
int control_sockfd;       // control socket fd
/*
    Main function to run the FTP client.

    Parameters:
        argc: The number of command-line arguments.
        argv: Array of command-line argument strings.

    Description:
        This function initializes the client, connects to the server,
        logs in, and enters a loop to handle user commands.
        It also sets up signal handling for the SIGINT signal.
*/
void main(int argc, char **argv)
{
    signal(SIGINT, handle_int); // Set up signal handler for SIGINT (Ctrl+C)

    int n, k, p_pid; // Socket file descriptor and other variables
    int data_cli_fd, data_srv_fd;
    struct sockaddr_in control_srv, data_cli, data_srv; // Structure to hold server address information
    char buff[256];
    char *hostport;
    char user_input[256];                                                                                                                  // Buffer to store the user input
    char *command[] = {"ls", "dir", "pwd", "cd", "mkdir", "delete", "rmdir", "rename", "quit", "bin", "ascii", "get", "put", NULL};        // List of commands
    char *convert[] = {"NLST", "LIST", "PWD", "CWD", "MKD", "DELE", "RMD", "RNFR&RNTO", "QUIT", "TYPE I", "TYPE A", "RETR", "STOR", NULL}; // Corresponding commands to be sent to the server
    char *divide_input_string;                                                                                                             // Pointer for dividing input string
    char *to_srv = NULL;                                                                                                                   // String to be sent to the server
    char recv_from_srv[4000];                                                                                                              // Buffer to store server response
    size_t len_to_srv;
    int index_num = 0, cmd_num; // Index counter
    char *index[20] = {NULL};   // Array to store separated tokens

    srand(time(0)); // Seed the random number generator

    // Create a control socket
    control_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Set up the server address struct
    memset(&control_srv, 0, sizeof(control_srv));
    control_srv.sin_family = AF_INET;
    control_srv.sin_addr.s_addr = inet_addr(argv[1]);
    control_srv.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    connect(control_sockfd, (struct sockaddr *)&control_srv, sizeof(control_srv));

    log_in(control_sockfd); // Log in to the server

    ///////////////////////////Client connection completed//////////////

    while (1)
    {
        memset(user_input, 0, sizeof(user_input)); // Reset buffer
        divide_input_string = "\0";
        fflush(stdout);
        write(STDOUT_FILENO, "> ", 2);                                // Print prompt
        ssize_t bytes_read = read(0, user_input, sizeof(user_input)); // Read user input
        user_input[strlen(user_input) - 1] = '\0';                    // Remove newline character
        trim_whitespace(user_input);

        // Tokenize the input by spaces
        divide_input_string = strtok(user_input, " ");
        int index_num = 0;

        while (divide_input_string != NULL)
        {
            index[index_num] = divide_input_string;  // Store the cut string
            divide_input_string = strtok(NULL, " "); // Find the next token
            index_num++;                             // Move to the next array index
        }

        if (strcmp(index[0], "bin") == 0) // canage mode
            type = 0;
        else if (strcmp(index[0], "ascii") == 0)
            type = 1;

        cmd_num = 0; // Initialize command index

        while (command[cmd_num] != NULL)
        {
            if (strcmp(command[cmd_num], index[0]) == 0) // Check if command exists
                break;
            cmd_num++;
        }

        if (to_srv)
        {
            free(to_srv);
            to_srv = NULL;
        }

        if (command[cmd_num] == NULL)
        {
            to_srv = (char *)malloc(strlen(index[0]) + 1); // Allocate memory for converted command
            strcpy(to_srv, index[0]);                      // Copy converted command
        }

        else if (index_num == 1) // Command without arguments
        {
            print_data("\n");                                      // Print newline
            to_srv = (char *)malloc(strlen(convert[cmd_num]) + 1); // Allocate memory for converted command
            strcpy(to_srv, convert[cmd_num]);                      // Copy converted command
        }

        else if (index_num == 2) // Command with one argument
        {
            to_srv = (char *)malloc(strlen(convert[cmd_num]) + strlen(index[1]) + 2); // Allocate memory for command and argument
            strcat(to_srv, convert[cmd_num]);                                         // Copy command
            strcat(to_srv, " ");                                                      // Add space
            strcat(to_srv, index[1]);                                                 // Copy argument
        }
        else if (index_num > 2) // Command with multiple arguments
        {
            to_srv = (char *)malloc(strlen(convert[cmd_num])); // Allocate memory for command
            len_to_srv = strlen(to_srv);

            for (k = 1; k < index_num; k++)
                len_to_srv = len_to_srv + strlen(index[k]); // Calculate total length

            len_to_srv = len_to_srv + index_num - 1;

            to_srv = (char *)malloc(len_to_srv); // Allocate memory for command and arguments
            memset(to_srv, 0, sizeof(to_srv));
            strcat(to_srv, convert[cmd_num]); // Copy command

            for (k = 1; k < index_num; k++) // Append arguments
            {
                strcat(to_srv, " ");
                strcat(to_srv, index[k]);
            }
        }

        to_srv[strlen(to_srv)] = '\0';                     // Null-terminate string
        write(control_sockfd, to_srv, strlen(to_srv) + 1); // Send the command to the server

        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0 || strcmp(index[0], "get") == 0 || strcmp(index[0], "put") == 0)
        {
            read(control_sockfd, buff, sizeof(buff));
            memset(buff, 0, sizeof(buff));

            int random_port = 10000 + rand() % 20001; // Generate a random port for the data connection

            data_cli_fd = socket(AF_INET, SOCK_STREAM, 0);
            memset(&data_cli, 0, sizeof(data_cli));
            data_cli.sin_family = AF_INET;
            data_cli.sin_addr.s_addr = inet_addr(argv[1]);
            data_cli.sin_port = htons(random_port);

            hostport = convert_addr_to_str(data_cli.sin_addr.s_addr, data_cli.sin_port);

            write(control_sockfd, hostport, strlen(hostport));
            read(control_sockfd, recv_from_srv, 4000); // code 200
            print_data(recv_from_srv);
            memset(recv_from_srv, 0, 4000);

            // Bind the control socket to the control address
            bind(data_cli_fd, (struct sockaddr *)&data_cli, sizeof(data_cli));

            // Listen for incoming connections
            listen(data_cli_fd, 5);
        }

        int len = sizeof(data_srv);
        write(control_sockfd, " ", 2);
        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0 || strcmp(index[0], "get") == 0 || strcmp(index[0], "put") == 0)
            data_srv_fd = accept(data_cli_fd, (struct sockaddr *)&data_srv, &len);

        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0)
        {
            read(control_sockfd, recv_from_srv, 4000); // code150
            print_data(recv_from_srv);
            memset(recv_from_srv, 0, 4000);
        }
        else if (strcmp(index[0], "get") == 0)
        {
            int code_i;

            read(control_sockfd, recv_from_srv, 4000); // code 150

            code_i = atoi(recv_from_srv);
            print_data(recv_from_srv);
            memset(recv_from_srv, 0, 4000);
            char buffer[1024] = {0};
            int n;
            FILE *file;
            if (access(index[1], F_OK) == -1)
                write(control_sockfd, "OK", 3); // code 150
            else
                write(control_sockfd, "NO", 3); // code 150
            int bytes_received;
            int total_bytes_received = 0; // Variable to store the total number of received bytes
            if (code_i == 150)
            {

                if (type == 0)
                    file = fopen(index[1], "wb");
                else
                    file = fopen(index[1], "w");
                // Receive and save file content from the server
                while ((bytes_received = recv(data_srv_fd, buffer, sizeof(buffer), 0)) > 0)
                {
                    fwrite(buffer, 1, bytes_received, file);
                    total_bytes_received += bytes_received; // Accumulate the number of received bytes
                }
                char msg[256];
                sprintf(msg, "OK. %d bytes is received.\n", total_bytes_received); // Convert the total received bytes to a string
                print_data(msg);
                fclose(file);
            }
            // Use write to output the total received bytes
        }
        else if ((strcmp(index[0], "put") == 0))
        {
            read(control_sockfd, recv_from_srv, sizeof(recv_from_srv));
            print_data(recv_from_srv);                       // Concatenate output
            memset(recv_from_srv, 0, sizeof(recv_from_srv)); // code 150

            char buffer[1024];
            FILE *file;
            if (type == 0)
                file = fopen(index[1], "rb");
            else
                file = fopen(index[1], "r");

            if (file != NULL)
            {
                write(control_sockfd, "exist", strlen("exist")); // Notify the server that the file exists
                int file_size = 0;
                read(control_sockfd, buff, 3);
                if (strcmp(buff, "NO") == 0)
                {
                    close(data_srv_fd);
                    read(control_sockfd, recv_from_srv, sizeof(recv_from_srv)); // Read the final message from the server
                    print_data(recv_from_srv);
                    memset(recv_from_srv, 0, sizeof(recv_from_srv));
                    continue;
                }

                while (!feof(file))
                {
                    int bytes_read = fread(buffer, 1, sizeof(buffer), file); // send  file data
                    send(data_srv_fd, buffer, bytes_read, 0);
                    file_size = file_size + bytes_read;
                }

                fclose(file);
                close(data_srv_fd);

                read(control_sockfd, recv_from_srv, sizeof(recv_from_srv)); // Read the final message from the server

                print_data(recv_from_srv);
                char msg[256];
                sprintf(msg, "OK. %d bytes is received.\n", file_size); // Convert the total received bytes to a string
                print_data(msg);
                memset(recv_from_srv, 0, sizeof(recv_from_srv));
                continue;
            }
            else
            {
                write(control_sockfd, "fail", strlen("fail"));
                close(data_srv_fd);
                read(control_sockfd, recv_from_srv, sizeof(recv_from_srv)); // Read the final message from the server
                print_data(recv_from_srv);
                memset(recv_from_srv, 0, sizeof(recv_from_srv));
                continue;
            }
        }
        // Use write to output the total received bytes
        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0 || strcmp(index[0], "get") == 0 || strcmp(index[0], "put") == 0)
        // Data Connection Output
        {
            read(data_srv_fd, recv_from_srv, sizeof(recv_from_srv));
            print_data(recv_from_srv); // Concatenate output
            write(control_sockfd, " ", 2);
        }
        int i = strlen(recv_from_srv);
        memset(recv_from_srv, 0, sizeof(recv_from_srv));
        read(control_sockfd, recv_from_srv, sizeof(recv_from_srv)); // Read the final message from the server
        print_data(recv_from_srv);

        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0) // print data size if ls or dir
        {
            char str_size[200];
            sprintf(str_size, "OK. %d byte is received\n", i);
            if (i > 2)
                print_data(str_size);
            memset(str_size, 0, 200);
        }

        memset(recv_from_srv, 0, sizeof(recv_from_srv));
        if (strcmp(index[0], "ls") == 0 || strcmp(index[0], "dir") == 0 || strcmp(index[0], "get") == 0 || strcmp(index[0], "put") == 0)
        // if data fd open close it
        {
            close(data_cli_fd);
            close(data_srv_fd);
        }

        if (strcmp(index[0], "quit") == 0)
            exit(1);
    }

    // Generate a random port for the data connection

    return;
}

// Convert IP address and port to a string in the desired format
char *convert_addr_to_str(unsigned long ip_addr, unsigned int port)
{
    // Structure to hold the IP address
    struct in_addr ip_addr_struct;
    ip_addr_struct.s_addr = ip_addr;

    // Convert the port from network byte order to host byte order
    int random_port = ntohs(port);

    // Buffer to hold the IP address string
    char ip_str[INET_ADDRSTRLEN];
    char *addr;

    // Convert the IP address to a string
    if (inet_ntop(AF_INET, &ip_addr_struct, ip_str, INET_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }

    // Extract the upper and lower 8 bits of the port number
    uint8_t upper_8_bits = (random_port >> 8) & 0xFF;
    uint8_t lower_8_bits = random_port & 0xFF;

    // Allocate memory for the result string
    addr = (char *)malloc(INET_ADDRSTRLEN + 20);
    if (addr == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Variables to hold the individual bytes of the IP address
    int ip1, ip2, ip3, ip4;
    sscanf(ip_str, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);

    // Format the IP address and port into the result string
    snprintf(addr, INET_ADDRSTRLEN + 20, "%s %d,%d,%d,%d,%u,%u", "PORT", ip1, ip2, ip3, ip4, upper_8_bits, lower_8_bits);

    return addr;
}

// Function to print execution results
void print_data(char *print)
{
    int length = strlen(print);
    write(1, print, length);
    return;
}

// Function to trim leading and trailing whitespace from a string
void trim_whitespace(char *str)
{
    // Pointer to the start of the string
    char *start = str;

    // Remove leading whitespace
    while (isspace((unsigned char)*start))
    {
        start++;
    }

    // Pointer to the end of the string
    char *end = start + strlen(start) - 1;

    // Remove trailing whitespace
    while (end > start && isspace((unsigned char)*end))
    {
        end--;
    }

    // Null-terminate the trimmed string
    *(end + 1) = '\0';

    // Move the trimmed string to the original buffer
    if (start != str)
    {
        memmove(str, start, end - start + 2); // +2 to include the null terminator
    }
}

int log_in(int sockfd)
{
    int n;                            // Variable to store the number of bytes read
    char user[20], *passwd, buf[256]; // Buffers for user input and server messages
    char log_in_data[30];             // Buffer for login data
    memset(buf, 0, 256);              // Zero out the buffer

    // Read the initial response from the server
    if ((n = read(sockfd, buf, sizeof(buf))) == 0)
    {
        print_data("server closed socket\n"); // If no data read, print a message and return
    }
    buf[strlen(buf) - 1] = '\0'; // Trim the trailing newline
    n = atoi(buf);               // Convert the response to an integer
    if (n == 431)
    {
        print_data(buf); // Connection refused by the server
        exit(1);
        return 0;
    }
    else if (n == 220)
    {
        print_data(buf); // Connection accepted by the server
    }
    else
    {
        print_data("something wrong option\n"); // Unexpected response from the server
        return 0;
    }
    print_data("\n");
    memset(buf, 0, 256); // Zero out the buffer for the next use

    // Loop to handle user login attempts
    for (;;)
    {
        memset(user, 0, 20); // Zero out the user buffer

        print_data("NAME :");                                        // Prompt for the user ID
        read(0, user, sizeof(user));                                 // Read the user ID from standard input
        snprintf(log_in_data, sizeof(log_in_data), "USER %s", user); // Construct the login data
        write(sockfd, log_in_data, strlen(log_in_data) + 1);         // Send the user ID to the server
        memset(buf, 0, sizeof(buf));
        read(sockfd, buf, sizeof(buf));
        print_data(buf);
        n = atoi(buf);
        if (n == 430)
            continue;
        else if (n == 530)
            exit(1);

        passwd = getpass("Password : ");                               // Prompt for the password
        snprintf(log_in_data, sizeof(log_in_data), "PASS %s", passwd); // Construct the login data
        write(sockfd, log_in_data, strlen(log_in_data) + 1);           // Send the password to the server
        memset(buf, 0, sizeof(buf));
        read(sockfd, buf, sizeof(buf));
        print_data(buf);
        n = atoi(buf);
        if (n == 430)
            continue;
        else if (n == 530)
            exit(1);
        else
            return 1;
    }
}

long get_file_size(FILE *file)
{
    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    long size = ftell(file);  // Get the current position of the file pointer, which gives the size of the file
    fseek(file, 0, SEEK_SET); // Move the file pointer back to the beginning of the file
    return size;              // Return the size of the file
}

void handle_int(int sig)
{
    write(control_sockfd, "QUIT", strlen("QUIT") + 1); // Send "QUIT" message to the control socket
    write(control_sockfd, " ", 2);                     // Send a space character to ensure message separation
    exit(1);                                           // Exit the program
}