///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/06/03 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-3

///////////////////////////////////////////////////////////////////////
#include <pwd.h>
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
#include <grp.h> // 그룹 관련 함수 및 구조체 정의

char code_buff[1024];
char concatenate_output[4000];
char ip_port[30];
int type = 0;
// Function declarations
int log_auth(int connfd);
void bubbleSort(char *arr[], int n); // Bubble sort for sorting file lists
char *convert_str_to_addr(char *str, unsigned int *port);
void write_code(int code, int control_Fd, char intr[400]);
int user_match(char *user, char *passwd, int TYPE);
void write_log(char *log);
int check_option(char *arr); // Check options, distinguish as 0, 1, or 2, 3
void print_non_option(char *arr[100], int n);
void print_a(char *arr[100], int n);  // Print with ls option a
void print_l(char *arr[100], int n);  // Print with ls option l
void print_al(char *arr[100], int n); // Print with ls option l
void print_data(const char *print);   // Function to print execution results
void handle_sigint(int sig);

FILE *LogBook;
char cli_user_name[20] = " ";
int ctrl_cli_fd;
int main(int argc, char **argv)
{

    signal(SIGINT, handle_sigint);

    int code = 0;
    int control_fd, data_fd;
    struct sockaddr_in control_addr, control_cli, data_addr;
    FILE *fp_checkIP; // FILE stream to check client’s IP
    char trash_buf[10];
    char check_ip[20], *client_ip, *host_ip;
    char temp[30];
    pid_t pid; // pid
    unsigned int port_num;
    int i = 0, j = 0, n;
    time_t rawtime;
    struct tm *timeinfo;
    char time_buf[256];
    char log_buf[400];
    char read_buf[256];
    char *index;           // Pointer to divide data by spaces and store it
    char *s_index[20];     // Array to store the separated data
    int num_of_sindex = 0; // Number of elements in s_index array

    LogBook = fopen("logfile", "a");
    if (LogBook == NULL)
    {
        perror("로그 파일 열기 실패");
        return 1;
    }

    // 현재 시간 가져오기 및 초기화
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // 시간 형식 지정 및 로그 작성
    strftime(time_buf, sizeof(time_buf), "%a %b %d %H:%M:%S %Y", timeinfo);
    sprintf(log_buf, "%s %s", time_buf, "Server is started\n");

    // 로그 파일에 쓰기
    fprintf(LogBook, "%s", log_buf);
    fflush(LogBook);
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

    for (;;)
    {
        int len = sizeof(control_cli);
        // Accept a connection from a client
        ctrl_cli_fd = accept(control_fd, (struct sockaddr *)&control_cli, &len);

        if ((pid = fork()) < 0)
        {
        }
        else if (pid == 0)
        { // Child process: receive data from the client

            client_ip = inet_ntoa(control_cli.sin_addr);
            sprintf(ip_port, "[%s :%d]", client_ip, ntohs(control_cli.sin_port));
            // Open the access file to check client's IP
            fp_checkIP = fopen("access.txt", "r");

            if (fp_checkIP == NULL)
            {
                print_data("**file \"access.txt\" doesn't exist**");
                return 1;
            }

            // Check if the client IP matches any entry in the access file
            while (fgets(check_ip, sizeof(check_ip), fp_checkIP) != NULL)
            {
                check_ip[strcspn(check_ip, "\n")] = '\0'; // Remove the newline character

                j = 0;
                i = 0;

                while ((check_ip[i] != '\0') && (client_ip[j] != '\0'))
                {
                    if (check_ip[i] == '*')
                    {
                        while (client_ip[j] != '.' && client_ip[j] != '\0')
                            j++;

                        if (client_ip[j] == '\0')
                        {
                            i++;
                            break;
                        }
                        j = j + 1;
                        i = i + 2;
                        continue;
                    }

                    if (check_ip[i] == client_ip[j])
                    {
                        i++;
                        j++;
                    }
                    else
                    {
                        break;
                    }
                }
                if ((check_ip[i] == '\0') && (client_ip[j] == '\0'))
                {
                    i = 100;
                    break;
                }
            }

            // If client IP is matched, allow connection
            if (i == 100)
            {
                write_code(220, ctrl_cli_fd, " ");
            }
            else
            {
                // If client IP is not matched, reject connection
                write_code(431, ctrl_cli_fd, " ");
                fclose(fp_checkIP);
                close(ctrl_cli_fd);
                exit(1);
            }

            fclose(fp_checkIP);

            if (log_auth(ctrl_cli_fd) == 0)
            { // if 3 times fail (ok : 1, fail : 0)
                exit(1);
            }

            while (1)
            {
                strcpy(concatenate_output, " "); // Print a message indicating that the following string is the current working directory
                memset(code_buff, 0, sizeof(code_buff));
                memset(read_buf, 0, 256);
                n = read(ctrl_cli_fd, read_buf, 256); // Read data from the client/
                read_buf[n] = '\0';
                print_data("input : ");
                if (strcmp(read_buf, "CWD ..") == 0)
                    print_data("CDUP");
                else
                    print_data(read_buf);
                print_data("\n");
                write_log(read_buf);

                index = strtok(read_buf, " "); // Tokenize the input buffer

                num_of_sindex = 0;
                while (index != NULL) // Repeat until there are no more strings to tokenize
                {
                    s_index[num_of_sindex] = index; // Store the tokenized string
                    num_of_sindex++;                // Move to the next index in the arrays
                    index = strtok(NULL, " ");      // Find the next token
                }

                if (strcmp(s_index[0], "NLST") == 0 || strcmp(s_index[0], "LIST") == 0 || strcmp(s_index[0], "RETR") == 0 || strcmp(s_index[0], "STOR") == 0)
                {
                    write(ctrl_cli_fd, " ", 2); // Write a space to the control connection
                    memset(temp, 0, 30);        // Clear the temp buffer

                    read(ctrl_cli_fd, temp, sizeof(temp));                          // Read the client's response
                    temp[strlen(temp)] = '\0';                                      // Ensure null-termination of the string
                    host_ip = convert_str_to_addr(temp, (unsigned int *)&port_num); // Convert the string to host IP address

                    write_code(200, ctrl_cli_fd, " "); // Send response code 200 to client

                    data_fd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket for data connection

                    memset(&data_addr, 0, sizeof(data_addr));       // Clear the data_addr structure
                    data_addr.sin_family = AF_INET;                 // Set address family to IPv4
                    data_addr.sin_addr.s_addr = inet_addr(host_ip); // Set the host IP address
                    data_addr.sin_port = htons(port_num);           // Set the port number

                    read(ctrl_cli_fd, trash_buf, sizeof(trash_buf));                    // Read acknowledgment from client
                    connect(data_fd, (struct sockaddr *)&data_addr, sizeof(data_addr)); // Initiate data connection
                }

                if (strcmp(s_index[0], "NLST") == 0) // If the command is NLST
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    code = 150;                      // Set response code to 150
                    DIR *dp = NULL;                  // Variable for directory traversal
                    struct dirent *dirp = 0;         // Structure to hold information about files within a directory
                    char *dir = " ";                 // Initialize dir variable to store the directory address to be searched

                    char *files[100];   // Array to store the list of files
                    int file_count = 0; // Initialize file_count to store the number of files in the list
                    int option_i = 0;   // Store options as integers: 0 for none, 1 for a, 2 for l, 3 for al

                    if (num_of_sindex == 1)
                    {              // If only the command exists
                        dir = "."; // Set dir to the current directory
                    }
                    else if (num_of_sindex == 2) // If two arguments are received
                    {
                        if (s_index[1][0] == '-') // If the second argument is an option
                        {
                            dir = ".";                           // Set dir to the current directory
                            option_i = check_option(s_index[1]); // Classify the option
                            if (option_i > 99)                   // Exception handling for options
                                code = 501;                      // Set response code to 501
                        }
                        else
                            dir = s_index[1]; // Set dir to the given address
                    }
                    else if (num_of_sindex == 3)
                    {
                        dir = s_index[2];                    // Set dir to the given directory name
                        option_i = check_option(s_index[1]); // Store the option
                        if (option_i > 99)
                            code = 501; // Set response code to 501
                    }
                    else
                    {
                        code = 501; // Set response code to 501
                    }
                    write_code(code, ctrl_cli_fd, " "); // Send response code to client

                    // Directory open
                    if (opendir(dir) == NULL || code != 150)
                    {
                        code = 550; // Set response code to 550
                        if (errno == EACCES)
                            strcpy(code_buff, "NLSTA"); // Set error code_buff to "NLSTA"
                        else
                            strcpy(code_buff, "NLSTD"); // Set error code_buff to "NLSTD"
                    }
                    else
                    {
                        code = 226;        // Set response code to 226
                        dp = opendir(dir); // Traverse the directory

                        while ((dirp = readdir(dp)) != NULL) // Get information about files in the directory
                        {
                            char file_path[300];                            // String to store the address of the file
                            struct stat file_stat;                          // Structure to store file information
                            sprintf(file_path, "%s/%s", dir, dirp->d_name); // Set the file location including the input arguments

                            files[file_count] = dirp->d_name; // Store the file name
                            stat(file_path, &file_stat);      // Get information about the file
                            if (S_ISDIR(file_stat.st_mode))
                            {
                                strcat(files[file_count], "/"); // Append '/' if it's a directory
                            }
                            file_count++; // Move to the next file
                        }
                        closedir(dp); // Close the directory

                        bubbleSort(files, file_count); // Sort the list of file names

                        switch (option_i) // Execute functions according to the option
                        {
                        case 0:
                            print_non_option(files, file_count); // If there's no option
                            break;
                        case 1:
                            print_a(files, file_count); // If the option is 'a'
                            break;
                        case 2:
                            print_l(files, file_count); // If the option is 'l'
                            break;
                        case 3:
                            print_al(files, file_count); // If the option is 'al'
                            break;
                        default:
                            strcpy(concatenate_output, "invalid option"); // If an invalid option is provided
                            break;
                        }

                        n = strlen(concatenate_output); // Get the length of concatenate_output
                        concatenate_output[n] = '\0';   // Ensure null-termination of the string
                    }
                }
                else if (strcmp(s_index[0], "LIST") == 0) // Command to list files in a directory
                {
                    code = 150;                      // Set response code to 150
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    DIR *dp = NULL;                  // Variable for directory traversal
                    struct dirent *dirp = 0;         // Structure to hold information about files within a directory
                    char *dir = " ";                 // Initialize dir variable to store the directory address to be searched

                    char *files[100];   // Array to store the list of files
                    int file_count = 0; // Initialize file_count to store the number of files in the list
                    int option_i = 0;   // Store options as integers: 0 for none, 1 for a, 2 for l, 3 for al

                    if (num_of_sindex == 1)
                    {              // If only the command exists
                        dir = "."; // Set dir to the current directory
                    }
                    else if (num_of_sindex == 2) // If two arguments are received
                    {
                        if (s_index[1][0] == '-') // If the second argument is an option
                        {
                            code = 501; // Set response code to 501
                        }
                        else
                            dir = s_index[1]; // If it's not an option, dir is the given address
                    }
                    else
                        code = 501;                     // Set response code to 501
                    write_code(code, ctrl_cli_fd, " "); // Send response code to client

                    // Directory open
                    if (opendir(dir) == NULL || code != 150)
                    {
                        code = 550; // Set response code to 550
                        if (errno == EACCES)
                            strcpy(code_buff, "NLSTA"); // Set error code_buff to "NLSTA"
                        else
                            strcpy(code_buff, "NLSTD"); // Set error code_buff to "NLSTD"
                    }
                    else
                    {
                        code = 226;        // Set response code to 226
                        dp = opendir(dir); // Traverse the directory

                        while ((dirp = readdir(dp)) != NULL) // Get information about files in the directory
                        {
                            char file_path[300];                            // String to store the address of the file
                            struct stat file_stat;                          // Structure to store file information
                            sprintf(file_path, "%s/%s", dir, dirp->d_name); // Set the file location including the input arguments

                            files[file_count] = dirp->d_name; // Store the file name
                            stat(file_path, &file_stat);      // Get information about the file
                            if (S_ISDIR(file_stat.st_mode))
                            {
                                strcat(files[file_count], "/"); // Append '/' if it's a directory
                            }
                            file_count++; // Move to the next file
                        }
                        closedir(dp); // Close the directory

                        bubbleSort(files, file_count); // Sort the list of file names

                        print_al(files, file_count); // If the option is 'al'
                    }

                    n = strlen(concatenate_output); // Get the length of concatenate_output
                    concatenate_output[n] = '\0';   // Ensure null-termination of the string
                }
                else if (strcmp(s_index[0], "PWD") == 0) // Command to print the current working directory
                {
                    strcpy(concatenate_output, " "); // Print a message indicating that the following string is the current working directory
                    if (num_of_sindex > 1)           // If additional arguments are provided
                    {
                        code = 501; // Set response code to 501
                    }
                    else // If no additional arguments are provided
                    {
                        getcwd(code_buff, 256); // Get the current working directory
                        code = 257;             // Set response code to 257
                    }
                }
                else if (strcmp(s_index[0], "CWD") == 0) // Command to change directory (cd command)
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    strcpy(code_buff, "CWD");        // Set code_buff to "CWD"
                    if (s_index[1][0] == '-')        // If an option is passed as an argument
                        code = 501;                  // Set response code to 501
                    else if (num_of_sindex != 2)     // If the number of arguments is incorrect
                        code = 501;                  // Set response code to 501
                    else if (chdir(s_index[1]) == -1)
                    {                                                                                   // If the directory change operation fails
                        code = 1000;                                                                    // Set response code to 1000
                        sprintf(code_buff, "550 %s: Can’t find such file or directory.\n", s_index[1]); // Set code_buff with error message
                    }
                    else // If the directory change operation is successful
                    {
                        getcwd(trash_buf, sizeof(code_buff)); // Get the current working directory
                        strcpy(code_buff, "CWD");             // Set code_buff to "CWD"
                        code = 250;                           // Set response code to 250
                    }
                }
                else if (strcmp(s_index[0], "MKD") == 0) // Command to make directories
                {
                    memset(code_buff, 0, 1024);      // Clear code_buff
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    if (num_of_sindex == 1)          // If no directory names are provided as arguments
                        code = 501;                  // Set response code to 501
                    else if (s_index[1][0] == '-')   // If an option is provided instead of a directory name
                        code = 501;                  // Set response code to 501
                    else
                    {
                        j = 1;                    // Start checking arguments from index 1
                        code = 1000;              // Set response code to 1000
                        while (j < num_of_sindex) // Loop through all received arguments
                        {
                            if (mkdir(s_index[j], 775) == -1) // Attempt to create a directory with permissions 775
                            {
                                strcat(code_buff, "550 ");                       // Append an error message if directory creation fails
                                strcat(code_buff, s_index[j]);                   // Append the name of the directory causing the error
                                strcat(code_buff, ": can’t create directory\n"); // Add a newline character
                            }
                            else
                                strcat(code_buff, "250 MKD command performed successfully\n"); // Append success message

                            j++; // Move to the next argument
                        }
                    }
                }
                else if (strcmp(s_index[0], "DELE") == 0) // Command to delete files
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    if (s_index[1] == NULL)          // If no file names are provided as arguments
                        code = 501;                  // Set response code to 501
                    else if (s_index[1][0] == '-')   // If an option is provided instead of a file name
                        code = 501;                  // Set response code to 501
                    else
                    {
                        memset(code_buff, 0, 512); // Clear code_buff
                        j = 1;                     // Start checking arguments from index 1
                        code = 1000;               // Set response code to 1000
                        while (j < num_of_sindex)  // Loop through all received arguments
                        {
                            if (unlink(s_index[j]) == -1) // Attempt to delete the file
                            {
                                strcat(code_buff, "550 ");                                  // Append an error message if deletion fails
                                strcat(code_buff, s_index[j]);                              // Append the name of the file causing the error
                                strcat(code_buff, ": Can’t find such file or directory\n"); // Add a newline character
                            }
                            else
                                strcat(code_buff, "250 DELE command performed successfully\n"); // Append success message

                            j++; // Move to the next argument
                        }
                    }
                }
                else if (strcmp(s_index[0], "RMD") == 0) // Command to remove directories
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    if (s_index[1] == NULL)          // If no directory names are provided as arguments
                        code = 501;                  // Set response code to 501
                    else if (s_index[1][0] == '-')   // If an option is provided instead of a directory name
                        code = 501;                  // Set response code to 501
                    else
                    {
                        memset(code_buff, 0, 512); // Clear code_buff
                        j = 1;                     // Start checking arguments from index 1
                        code = 1000;               // Set response code to 1000
                        while (j < num_of_sindex)  // Loop through all received arguments
                        {
                            if (rmdir(s_index[j]) == -1) // Attempt to remove the directory
                            {
                                strcat(code_buff, "550 ");                       // Append an error message if removal fails
                                strcat(code_buff, s_index[j]);                   // Append the name of the directory causing the error
                                strcat(code_buff, ": can’t remove directory\n"); // Add a newline character
                            }
                            else
                                strcat(code_buff, "250 RMD command performed successfully\n"); // Append success message

                            j++; // Move to the next argument
                        }
                    }
                }
                else if (strcmp(s_index[0], "RNFR&RNTO") == 0) // Command to rename files or directories
                {
                    strcpy(concatenate_output, " ");         // Initialize concatenate_output
                    memset(code_buff, 0, sizeof(code_buff)); // Clear code_buff
                    code = 1000;                             // Set response code to 1000

                    // Check if the correct number of arguments is provided
                    if (num_of_sindex != 3)
                        code = 501;                                        // Set response code to 501
                    else if (s_index[1][0] == '-' || s_index[2][0] == '-') // If an option is provided as an argument
                        code = 501;                                        // Set response code to 501
                    else
                    {
                        // Check if the file or directory to be renamed exists
                        if (access(s_index[1], F_OK) == -1)
                        {
                            strcat(code_buff, "550 ");                                  // Append an error message if the source file or directory doesn't exist
                            strcat(code_buff, s_index[1]);                              // Append the name of the file or directory causing the error
                            strcat(code_buff, ": Can’t find such file or directory\n"); // Add a newline character
                            strcat(code_buff, "550 ");                                  // Append an error message if the target file or directory doesn't exist
                            strcat(code_buff, s_index[2]);                              // Append the name of the file or directory causing the error
                            strcat(code_buff, ": can’t be renamed\n");                  // Add a newline character
                        }
                        else
                        {
                            strcat(code_buff, "350: File exists, ready to rename\n"); // Append the command name "RNFR"
                            // Check if the target file or directory already exists
                            if (access(s_index[2], F_OK) != -1)
                            {
                                strcat(code_buff, "550 ");                 // Append an error message if the target file or directory already exists
                                strcat(code_buff, s_index[2]);             // Append the name of the file or directory causing the error
                                strcat(code_buff, ": can’t be renamed\n"); // Add a newline character
                            }
                            else
                            {
                                rename(s_index[1], s_index[2]);                    // Rename the file or directory
                                strcat(code_buff, "250: RNTO command succeeds\n"); // Append success message
                                strcat(concatenate_output, "\n");                  // Add a newline character
                            }
                        }
                    }
                }
                else if (strcmp(s_index[0], "TYPE") == 0) // Command to set transfer mode
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    code = 201;                      // Set response code to 201

                    // Check if the correct number of arguments is provided
                    if (num_of_sindex != 2)
                        code = 501; // Set response code to 501

                    // Set transfer mode based on the argument
                    if (s_index[1][0] == 'I')
                        type = 0; // Binary mode
                    else
                        type = 1; // ASCII mode
                }
                else if (strcmp(s_index[0], "RETR") == 0) // Command to retrieve file from server
                {
                    strcpy(concatenate_output, " ");         // Initialize concatenate_output
                    memset(code_buff, 0, sizeof(code_buff)); // Clear code_buff
                    strcpy(code_buff, "file");               // Set file type
                    code = 150;                              // Set response code to 150

                    char buffer[1024];
                    FILE *file;

                    // Open the file in the appropriate mode
                    if (type == 0)
                        file = fopen(s_index[1], "rb"); // Binary mode
                    else
                        file = fopen(s_index[1], "r"); // ASCII mode

                    if (file == NULL) // Check if the file exists
                    {
                        code = 550;                              // Set response code to 550
                        memset(code_buff, 0, sizeof(code_buff)); // Clear code_buff
                        strcpy(code_buff, "NLSTA");              // Set error message
                    }

                    write_code(code, ctrl_cli_fd, code_buff); // Write response code to client
                    memset(trash_buf, 0, sizeof(trash_buf));
                    read(ctrl_cli_fd, trash_buf, 3);
                    print_data(trash_buf);
                    if (strcmp(trash_buf, "NO") == 0)
                        code == 550;

                    if (code == 150) // If file retrieval starts
                    {
                        while (!feof(file))
                        {
                            int bytes_read = fread(buffer, 1, sizeof(buffer), file);
                            send(data_fd, buffer, bytes_read, 0); // Send file data to client
                        }

                        fclose(file);   // Close the file
                        close(data_fd); // Close the data connection
                    }

                    // Set appropriate response code based on file retrieval success
                    if (code == 150)
                        code = 226; // File transfer successful
                    else
                    {
                        code = 550;                 // File transfer failed
                        strcpy(code_buff, "NLSTD"); // Set error message
                    }
                }

                else if (strcmp(s_index[0], "STOR") == 0) // Command to store file on server
                {
                    write_code(150, ctrl_cli_fd, "file"); // Send acknowledgment to client
                    strcpy(concatenate_output, " ");      // Initialize concatenate_output
                    char buffer[1024] = {0};              // Buffer for receiving data
                    int bytes_received;

                    FILE *file;
                    memset(trash_buf, 0, sizeof(trash_buf));         // Clear trash_buf
                    read(ctrl_cli_fd, trash_buf, sizeof(trash_buf)); // Read acknowledgment from client

                    if (strcmp(trash_buf, "exist") == 0) // Check if the file exists
                    {
                        if (access(s_index[1], F_OK) == -1)
                            write(ctrl_cli_fd, "OK", 3); // code 150
                        else
                        {
                            write(ctrl_cli_fd, "NO", 3);       // code 150
                            write_code(550, ctrl_cli_fd, " "); // Send error code to client
                            continue;
                        }
                        // Open the file in the appropriate mode
                        if (type == 0)
                            file = fopen(s_index[1], "wb"); // Binary mode
                        else
                            file = fopen(s_index[1], "w"); // ASCII mode

                        while ((bytes_received = recv(data_fd, buffer, sizeof(buffer), 0)) > 0) // Receive data from client
                        {
                            fwrite(buffer, 1, bytes_received, file); // Write received data to file
                            fflush(file);                            // Flush the buffer to write data to the file
                        }

                        write_code(226, ctrl_cli_fd, " "); // Send acknowledgment to client
                        continue;
                    }
                    else
                    {
                        write_code(550, ctrl_cli_fd, " "); // Send error code to client
                        continue;
                    }
                }
                else if (strcmp(s_index[0], "QUIT") == 0) // Handling the QUIT command
                {
                    strcpy(concatenate_output, " ");                                // Initialize concatenate_output
                    write(data_fd, concatenate_output, sizeof(concatenate_output)); // Send acknowledgment to client
                    print_data(concatenate_output);                                 // Print acknowledgment message

                    read(ctrl_cli_fd, trash_buf, 2);   // Read acknowledgment from client
                    write_code(221, ctrl_cli_fd, " "); // Send acknowledgment to client
                    close(data_fd);                    // Close the data connection
                    close(ctrl_cli_fd);                // Close the control connection
                    exit(1);                           // Exit the program
                }
                else
                {
                    strcpy(concatenate_output, " "); // Initialize concatenate_output
                    code = 500;                      // Set response code to 500
                }

                if (strcmp(s_index[0], "NLST") == 0 || strcmp(s_index[0], "LIST") == 0 || strcmp(s_index[0], "RETR") == 0 || strcmp(s_index[0], "STOR") == 0)
                {
                    n = write(data_fd, concatenate_output, strlen(concatenate_output)); // Send file list to client
                }

                read(ctrl_cli_fd, trash_buf, 2); // Read acknowledgment from client

                s_index[0] = "df";                        // Set default value for s_index[0]
                write_code(code, ctrl_cli_fd, code_buff); // Write response code to client
                memset(code_buff, 0, sizeof(code_buff));  // Clear code_buff
                close(data_fd);                           // Close the data connection
            }
        }
    }
    close(control_fd);
    return 0;
}

char *convert_str_to_addr(char *str, unsigned int *port) // Convert a string to an IP address and port number
{
    char *addr;
    int ip1, ip2, ip3, ip4;
    unsigned int upper_8_bits, lower_8_bits;
    str = str + 5;
    addr = (char *)malloc(20);

    // Extract IP address and port number components from the string
    sscanf(str, "%d,%d,%d,%d,%u,%u", &ip1, &ip2, &ip3, &ip4, &upper_8_bits, &lower_8_bits);

    // Convert the IP address to dot-decimal notation
    snprintf(addr, INET_ADDRSTRLEN, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

    // Combine the upper and lower 8 bits to form the port number
    *port = (upper_8_bits << 8) | lower_8_bits;

    return addr;
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
void write_code(int code, int control_Fd, char *intr)
{

    char message[1024]; // Buffer to hold the message to be sent
    FILE *file;
    struct tm *tm_info;
    time_t now;
    char time_str[256];
    char line[256];

    switch (code)
    {
    case 150:
        if (strcmp(intr, "file") == 0)
        {
            if (type == 0)
                strcpy(message, "150 Opening binary data connection for directory list.\n");
            else
                strcpy(message, "150 Opening ascii data connection for directory list.\n");
        }
        else
            strcpy(message, "150 Opening data connection for directory list.\n");
        break;
    case 200:
        strcpy(message, "200 PORT command performed successfully.\n");
        break;
    case 201:
        if (type == 0)
            strcpy(message, "201 Type set to I.\n");
        else
            strcpy(message, "201 Type set to A.\n");
        break;
    case 220:
        setenv("TZ", "Asia/Seoul", 1);
        tzset();
        // Get the current time
        time(&now);
        tm_info = localtime(&now);

        // Format the time as a string
        strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Z %Y", tm_info);
        file = fopen("motd", "r");

        fgets(line, sizeof(line), file);
        fclose(file);
        line[strlen(line) - 3] = '\0';
        snprintf(message, sizeof(message), "220 %s %s %s\n", line, time_str, ")  ready\n"); // Combine line and time_str into message
        break;
    case 221:
        strcpy(message, "221 Goodbye.\n");
        break;
    case 226:
        strcpy(message, "226 Complete transmission.\n");
        break;

    case 230:
        snprintf(message, sizeof(message), "230 User %s logged in.\n", intr);
        break;
    case 250:
        snprintf(message, sizeof(message), "250 %s command performed successfully\n", intr);
        break;
    case 257:
        snprintf(message, sizeof(message), "257 \"%s\" is current directory\n", intr);
        break;
    case 331:
        strcpy(message, "331 Password is required for username.\n");
        break;
    case 350:
        strcpy(message, "350 File exists, ready to rename\n");
        break;
    case 430:
        strcpy(message, "430 Invalid username or password\n");
        break;
    case 431:
        strcpy(message, "431 This client can’t access. Close the session.\n");
        break;
    case 500:
        strcpy(message, "500: Syntax error, command unrecognized\n");
        break;
    case 501:
        strcpy(message, "501: Syntax error in parameters or arguments\n");
        break;
    case 530:
        strcpy(message, "530 Failed to log-in\n");
        break;
    case 550:
        if (strcmp(intr, "NLSTA") == 0)
            strcpy(message, "Failed to access\n");
        else
            strcpy(message, "550 Failed transmission.\n");
        break;
    case 1000:
        strcpy(message, intr);
        break;
    }
    write(control_Fd, message, strlen(message));
    print_data(message); // Data connection opening
    write_log(message);
    memset(message, 0, sizeof(message));
    return;
}

int log_auth(int connfd)
{
    char user[20], passwd[20];
    int n, count = 1;
    char count_s[2];
    char *user_trimmed;
    char *passwd_trimmed;
    while (1)
    {

        memset(user, 0, sizeof(user));
        memset(passwd, 0, sizeof(passwd));

        // Read user credentials from client
        read(connfd, user, sizeof(user) - 1);

        user[strcspn(user, "\n")] = '\0'; // Remove the newline character
        user_trimmed = user + 5;
        print_data(user_trimmed);
        print_data("\n");
        n = user_match(user_trimmed, " ", 0);
        if (n == 1)
            write_code(331, connfd, " ");
        else
        {
            if (count >= 3)
            {
                write_code(530, connfd, " ");
                return 0;
            }
            write_code(430, connfd, " ");
            count++;
            continue;
        }
        read(connfd, passwd, sizeof(passwd) - 1);
        write_log(passwd);
        passwd[strcspn(passwd, "\n")] = '\0'; // Remove the newline character
        passwd_trimmed = passwd + 5;
        n = user_match(user_trimmed, passwd_trimmed, 1);
        if (n == 1)
        {
            write_code(230, connfd, user_trimmed);
            break;
        }
        else
        {
            if (count >= 3)
            {
                write_code(530, connfd, " ");
                return 0;
            }
            write_code(430, connfd, " ");
            count++;
            continue;
        }
    }
    strcpy(cli_user_name, user_trimmed);
    return 1;
}

// Function to print execution results
void print_data(const char *print)
{
    int length = strlen(print);
    write(STDOUT_FILENO, print, length); // Print to standard output
    return;
}

int user_match(char *user, char *passwd, int TYPE)
{
    FILE *fp;
    char line[200];
    char *stored_user, *stored_passwd;
    // Open the file containing the user credentials
    fp = fopen("passwd", "r");

    // Check if the file was opened successfully
    if (fp == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    // Read each line from the file
    while (fgets(line, sizeof(line), fp) != NULL)
    {

        line[strcspn(line, "\n")] = '\0'; // Remove the newline character

        stored_user = strtok(line, ":");
        stored_passwd = strtok(NULL, ":");

        // Compare the provided user ID and password with the stored values
        if (TYPE == 0)
        {
            if (strcmp(user, stored_user) == 0)
            {
                fclose(fp); // Close the file before returning
                return 1;   // Return 1 if both match
            }
        }
        else
        {
            if (strcmp(passwd, stored_passwd) == 0 && strcmp(user, stored_user) == 0)
            {
                fclose(fp); // Close the file before returning
                return 1;   // Return 1 if both match
            }
        }
    }

    // Close the file if end is reached without finding a match
    fclose(fp);
    return 0; // Return 0 if no match is found
}

void write_log(char *log)
{
    time_t rawtime;
    struct tm *timeinfo;
    char time_buf[256];
    char log_buf[400];

    // 현재 시간 가져오기 및 초기화
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // 시간 형식 지정
    strftime(time_buf, sizeof(time_buf), "%a %b %d %H:%M:%S %Y", timeinfo);

    // 로그 메시지 작성
    snprintf(log_buf, sizeof(log_buf), "%s %s %s", time_buf, ip_port, log);

    // 로그 메시지 끝에 개행 문자가 없는 경우 추가
    if (log_buf[strlen(log_buf) - 1] != '\n')
    {
        size_t len = strlen(log_buf);
        log_buf[len] = '\n';
        log_buf[len + 1] = '\0';
    }

    // 로그 메시지 출력 (또는 파일에 쓰기)
    fprintf(LogBook, "%s", log_buf);
    fflush(LogBook);
    return;
}

void print_non_option(char *arr[100], int n) // When there are no options
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

void print_a(char *arr[100], int n)
{
    int word = 0;
    for (int i = 0; i < n; i++) // Print all files
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
    strcat(concatenate_output, "\n");
}

void print_l(char *arr[100], int n)
{
    int word = 0;
    struct stat file_stat;
    char file_name[400][50];

    for (int i = 0; i < n; i++)
    {
        strcpy(file_name[i], arr[i]);
    }

    for (int i = 0; i < n; i++)
    {
        if (file_name[i][0] != '.') // Exclude hidden files and print the rest. Other variables serve the same purpose as print_al.
        {
            stat(file_name[i], &file_stat); // Check file permissions and save them to a string
            char mode_str[11];              // String to store permissions
            mode_str[0] = (S_ISDIR(file_stat.st_mode)) ? 'd' : '-';
            mode_str[1] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
            mode_str[2] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
            mode_str[3] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
            mode_str[4] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
            mode_str[5] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
            mode_str[6] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
            mode_str[7] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
            mode_str[8] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
            mode_str[9] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
            mode_str[10] = '\0';

            struct passwd *pw = getpwuid(file_stat.st_uid); // Structure to get user name
            struct group *gr = getgrgid(file_stat.st_gid);  // Structure to get group id
            char nlink_str[100];                            // Variable used for file output

            strcat(concatenate_output, mode_str);
            strcat(concatenate_output, " ");

            sprintf(nlink_str, "%ld ", file_stat.st_nlink); // Store long int value in nlink_str string to print it

            strcat(concatenate_output, nlink_str);
            strcat(concatenate_output, " ");

            strcat(concatenate_output, pw->pw_name);
            strcat(concatenate_output, " ");
            strcat(concatenate_output, gr->gr_name);
            strcat(concatenate_output, " ");

            char size_str[20];
            sprintf(size_str, "%ld", (long)file_stat.st_size); // Store file size, an integer, as a string
            strcat(concatenate_output, size_str);
            strcat(concatenate_output, " ");

            char time_str[100]; // Variable to store time here
            strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));

            strcat(concatenate_output, time_str);
            strcat(concatenate_output, " ");
            strcat(concatenate_output, file_name[i]);
            strcat(concatenate_output, "\n");
            memset(file_name, 0, 30);
        }
    }
}

void print_al(char *arr[100], int n)
{
    int word = 0;
    struct stat file_stat;
    char file_name[400][30];

    for (int i = 0; i < n; i++)
    {
        strcpy(file_name[i], arr[i]);
    }

    for (int i = 0; i < n; i++)
    {

        stat(file_name[i], &file_stat); // Check file permissions and save them to a string
        char mode_str[11];              // String to store permissions
        mode_str[0] = (S_ISDIR(file_stat.st_mode)) ? 'd' : '-';
        mode_str[1] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
        mode_str[2] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
        mode_str[3] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
        mode_str[4] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
        mode_str[5] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
        mode_str[6] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
        mode_str[7] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
        mode_str[8] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
        mode_str[9] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
        mode_str[10] = '\0';
        struct passwd *pw = getpwuid(file_stat.st_uid); // Structure to get user name
        struct group *gr = getgrgid(file_stat.st_gid);  // Structure to get group id
        char nlink_str[100];                            // Variable used for file output

        strcat(concatenate_output, mode_str);
        strcat(concatenate_output, " ");

        sprintf(nlink_str, "%ld ", file_stat.st_nlink); // Store long int value in nlink_str string to print it

        strcat(concatenate_output, nlink_str);
        strcat(concatenate_output, " ");

        strcat(concatenate_output, pw->pw_name);
        strcat(concatenate_output, " ");
        strcat(concatenate_output, gr->gr_name);
        strcat(concatenate_output, " ");

        char size_str[20];
        sprintf(size_str, "%ld", (long)file_stat.st_size); // Store file size, an integer, as a string
        strcat(concatenate_output, size_str);
        strcat(concatenate_output, " ");

        char time_str[100]; // Variable to store time here
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));

        strcat(concatenate_output, time_str);
        strcat(concatenate_output, " ");
        strcat(concatenate_output, file_name[i]);
        strcat(concatenate_output, "\n");
        memset(file_name, 0, 30);
    }
}

int check_option(char *arr) // Function to check options
{
    int num = 0;                          // Initialize num to 0; it will remain 0 if there are no options
    for (int i = 1; i < strlen(arr); i++) // Iterate over the characters of the input string, starting from index 1

        if (arr[i] == 'a')      // If the character is 'a'
            num = num + 1;      // Increment num by 1
        else if (arr[i] == 'l') // If the character is 'l'
            num = num + 2;      // Increment num by 2
        else
        {
            num = num + 100; // Set num to a value greater than 2 to indicate an invalid option
        }
    return num; // Return the total value of options found
}
void handle_sigint(int sig)
{
    write_log("221 Goodbye.");
    exit(1); // Exit the programs
}
