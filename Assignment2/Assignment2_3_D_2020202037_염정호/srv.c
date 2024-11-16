///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/07 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-2
// Description :
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#define BUF_SIZE 256 // set buffer size

typedef struct // child process data
{
    pid_t pid;         // process id
    char *PORT_string; // prot
    time_t start_time; // create time
} child_data;

child_data child_s[10]; // child struct array
int num_of_child = 0;
pid_t pid; // pid

int j = 0;                // variable use in loop
int server_fd, client_fd; // file descriptor
char concatenate_output[4000];
void print_data(const char *print);                // Function to print execution results
void print_child(child_data *child, int i);        // print child list
int check_option(char *arr);                       // Check options, distinguish as 0, 1, or 2, 3
void print_error();                                // Print error messages for options
void print_non_option(char *arr[], int n, int cd); // Print when there are no ls options
void print_a(char *arr[], int n);                  // Print with ls option a
void print_l(char *filename[], int n);             // Print with ls option l
void print_al(char *arr[], int n);                 // Print with ls option l
void bubbleSort(char *arr[], int n);               // Bubble sort for sorting file lists
void sh_int(int signum);

int main(int argc, char **argv)
{

    void sh_chld(int); // Signal handler for SIGCHLD
    void sh_alrm(int); // Signal handler for SIGALRM
    void sh_int(int);  // Signal handler for SIGINT

    char BUFF[BUF_SIZE];                         // Input/output buffer
    int n;                                       // Number of bytes read/written
    struct sockaddr_in server_addr, client_addr; // Server and client address structures
    int server_fd, client_fd;                    // Server and client file descriptors
    int len;                                     // Size of client address
    int port;                                    // Port number

    // Port number
    char input_buf[100];   // Buffer to store input from client
    char *index;           // Pointer to divide data by spaces and store it
    char *s_index[20];     // Array to store the separated data
    int num_of_sindex = 0; // Number of elements in s_index array

    // Applying signal handlers for SIGCHLD and SIGALRM
    signal(SIGCHLD, sh_chld);
    signal(SIGALRM, sh_alrm);
    signal(SIGINT, sh_int);

    server_fd = socket(PF_INET, SOCK_STREAM, 0); // Get server file descriptor

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Get server address
    server_addr.sin_port = htons(atoi(argv[1]));     // Get port number

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); // Bind

    listen(server_fd, 5); // Listen for incoming connections

    while (1)
    {
        char PORT_string[10];                                                 // String to store the client's port number
        PORT_string[0] = '\0';                                                // Initialize the string
        len = sizeof(client_addr);                                            // Get the size of the client address structure
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len); // Accept a connection from the client

        if ((pid = fork()) < 0)
        {
        }
        else if (pid == 0)
        { // Child process: receive data from the client

            while (1)
            {
                n = read(client_fd, input_buf, sizeof(input_buf));         // Read data from the client
                input_buf[n] = '\0';                                       // Null-terminate the input buffer
                printf("%s                  [%d]\n", input_buf, getpid()); // Print the received data along with the child process ID
                index = strtok(input_buf, " ");                            // Tokenize the input buffer
                num_of_sindex = 0;                                         // Reset the index counter

                while (index != NULL) // Repeat until there are no more strings to tokenize
                {
                    s_index[num_of_sindex] = index; // Store the tokenized string
                    num_of_sindex++;                // Move to the next index in the array
                    index = strtok(NULL, " ");      // Find the next token
                }

                if (strcmp(s_index[0], "NLST") == 0) // If the command is ls
                {

                    DIR *dp = NULL;          // Variable for directory traversal
                    struct dirent *dirp = 0; // Structure to hold information about files within a directory
                    char *dir = NULL;        // Variable to store the directory address to be searched

                    char *files[100];   // Array to store the list of files
                    int file_count = 0; // Integer to store the number of files in the list
                    int option_i = 0;   // Store options as integers: 0 for none, 1 for a, 2 for l, 3 for al

                    if (num_of_sindex == 1)
                    {              // If only the command exists
                        dir = "."; // Output the current directory
                    }
                    else if (num_of_sindex == 2) // If two arguments are received
                    {
                        if (s_index[1][0] == '-') // If the second argument is an option
                        {
                            dir = ".";                           // Directory - current directory
                            option_i = check_option(s_index[1]); // Classify the option
                            if (option_i == -1)                  // Exception handling for options
                                print_error();                   // Print error
                        }
                        else
                            dir = s_index[1]; // If it's not an option, dir is the given address
                    }
                    else if (num_of_sindex == 3)
                    {
                        dir = s_index[2];                    // Store the directory name
                        option_i = check_option(s_index[1]); // Store the option
                        if (option_i == -1)
                        {
                            print_error(); // Print error if there's a problem with the option
                        }
                    }
                    else
                    {
                        strcat(concatenate_output, "too much input\n"); // Append '/' if it's a directory
                        write(client_fd, concatenate_output, strlen(concatenate_output));
                        memset(concatenate_output, 0, 4000); // recv_from_srv 초기화

                        continue;
                    }
                    // directory open
                    if (opendir(dir) == NULL)
                    {
                        if (errno == EACCES)
                            strcat(concatenate_output, "cannnot access : Access denied \n"); // If the directory cannot be accessed due to permission issues
                        else
                            strcat(concatenate_output, " No such directory\n"); // If the directory does not exist
                    }
                    else
                    {
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
                            file_count++; // Next file
                        }
                        closedir(dp); // Close the directory

                        bubbleSort(files, file_count); // Sort the list of file names

                        switch (option_i) // Execute functions according to the option
                        {
                        case 0:
                            print_non_option(files, file_count, client_fd); // If there's no option
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

                        n = strlen(concatenate_output);
                        concatenate_output[n] = '\0';
                    }
                }
                else if (strcmp(input_buf, "LIST") == 0)
                {
                    DIR *dp;                 // Pointer to directory stream for traversing directories
                    struct dirent *dirp = 0; // Pointer to structure holding information about directory entries
                    char *dir;               // String to store the directory path

                    char *files[1000];  // Array to store the list of files
                    int file_count = 0; // Counter to store the number of files

                    // Determine the directory path based on the received command and arguments
                    if (num_of_sindex == 1)      // If only the command is provided
                        dir = ".";               // Use the current directory
                    else if (num_of_sindex == 2) // If the command is followed by an argument
                    {
                        if (s_index[1][0] == '-') // If the argument is an option
                        {
                            // Print an error message for invalid options and continue to the next iteration
                            print_error();
                            write(client_fd, concatenate_output, strlen(concatenate_output));
                            memset(concatenate_output, 0, 4000); // Reset the output buffer
                            continue;                            // Skip to the next iteration of the loop
                        }
                        else
                            dir = s_index[1]; // Use the specified directory
                    }
                    else if (num_of_sindex > 2)                         // If there are too many arguments
                        strcpy(concatenate_output, "too much input\n"); // Print an error message

                    // Check if the directory can be opened
                    if (opendir(dir) == NULL) // If opening the directory fails
                    {
                        if (errno == EACCES)
                            strcpy(concatenate_output, "cannot access : Access denied \n"); // Print an error message for permission denied
                        else
                            strcpy(concatenate_output, " No such directory\n"); // Print an error message for directory not found
                    }
                    else // If the directory is successfully opened
                    {
                        dp = opendir(dir); // Open the directory

                        // Traverse the directory and collect information about files
                        while ((dirp = readdir(dp)) != NULL) // Loop through each entry in the directory
                        {
                            char file_path[300];   // String to store the full path of the file
                            struct stat file_stat; // Structure to store file information

                            // Create the full path of the file by concatenating the directory path and file name
                            sprintf(file_path, "%s/%s", dir, dirp->d_name);

                            // Store the file name in the array
                            files[file_count] = dirp->d_name;

                            // Retrieve file information
                            stat(file_path, &file_stat);

                            // If the file is a directory, append '/' to its name
                            if (S_ISDIR(file_stat.st_mode))
                            {
                                strcat(files[file_count], "/");
                            }

                            file_count++; // Increment the file count
                        }
                        closedir(dp); // Close the directory stream

                        // Sort the files alphabetically
                        bubbleSort(files, file_count);

                        // Print the list of files with detailed information (ls -al)
                        print_al(files, file_count);
                    }
                }

                else if (strcmp(input_buf, "PWD") == 0) // Command to print the current working directory
                {
                    if (num_of_sindex > 1) // If additional arguments are provided
                    {
                        if (s_index[1][0] == '-') // If the argument is an option
                            print_error();        // Print an error message for invalid options
                        else
                            strcat(concatenate_output, "argument is not required\n"); // Print a message indicating that an argument is not required
                    }
                    else // If no additional arguments are provided
                    {
                        strcat(concatenate_output, "Current working directory: "); // Print a message indicating that the following string is the current working directory
                        getcwd(BUFF, sizeof(BUFF));                                // Get the current working directory and store it in the BUFF array
                        strcat(concatenate_output, BUFF);                          // Append the current working directory to the output string
                        strcat(concatenate_output, "\n");                          // Append a newline character to the output string
                    }
                }

                else if (strcmp(input_buf, "CWD") == 0) // Command to change directory (cd command)
                {

                    if (s_index[1][0] == '-') // If an option is passed as an argument
                    {
                        print_error(); // Print an error message
                    }
                    else if (num_of_sindex != 2) // If the number of arguments is incorrect
                    {
                        strcat(concatenate_output, "wrong number of arguments\n"); // Append a message indicating an incorrect number of arguments
                    }

                    else if (chdir(s_index[1]) == -1) // If the directory change operation fails
                    {
                        strcat(concatenate_output, "Error: directory not found\n"); // Append an error message indicating that the directory was not found
                    }
                    else // If the directory change operation is successful
                    {
                        getcwd(BUFF, sizeof(BUFF));                                // Get the current working directory
                        strcat(concatenate_output, BUFF);                          // Append the current working directory
                        strcat(concatenate_output, " ");                           // Append a space
                        strcat(concatenate_output, " is the current directory\n"); // Append a message indicating the current directory
                    }
                }

                else if (strcmp(input_buf, "MKD") == 0) // Command to make directories
                {
                    if (num_of_sindex == 1)                                   // If no directory names are provided as arguments
                        strcat(concatenate_output, "argument is required\n"); // Indicate that an argument is required

                    else if (s_index[1][0] == '-') // If an option is provided instead of a directory name
                    {
                        print_error(); // Print an error message
                    }
                    else
                    {
                        j = 1;                    // Start checking arguments from index 1
                        while (j < num_of_sindex) // Loop through all received arguments
                        {
                            if (mkdir(s_index[j], 775) == -1) // Attempt to create a directory with permissions 775
                            {
                                strcat(concatenate_output, "Error: cannot create directory "); // Append an error message if directory creation fails
                                strcat(concatenate_output, s_index[j]);                        // Append the name of the directory causing the error
                                strcat(concatenate_output, "\n");                              // Add a newline character
                            }
                            else
                            {
                                strcat(concatenate_output, "MKD ");     // Append the command name "MKD"
                                strcat(concatenate_output, s_index[j]); // Append the name of the successfully created directory
                                strcat(concatenate_output, "\n");       // Add a newline character
                            }
                            j++; // Move to the next argument
                        }
                    }
                }
                else if (strcmp(input_buf, "DELE") == 0) // Command to delete files
                {
                    if (s_index[1] == NULL) // If no file names are provided as arguments
                    {
                        strcat(concatenate_output, "argument is required\n"); // Indicate that an argument is required
                    }
                    else if (s_index[1][0] == '-') // If an option is provided instead of a file name
                    {
                        print_error(); // Print an error message
                    }
                    else
                    {
                        j = 1;                    // Start checking arguments from index 1
                        while (j < num_of_sindex) // Loop through all received arguments
                        {
                            if (unlink(s_index[j]) == -1) // Attempt to delete the file
                            {
                                strcat(concatenate_output, "failed to delete: "); // Append a message indicating deletion failure
                                strcat(concatenate_output, s_index[j]);           // Append the name of the file causing the error
                                strcat(concatenate_output, "\n");                 // Add a newline character
                            }
                            else
                            {
                                strcat(concatenate_output, "DELE ");    // Append the command name "DELE"
                                strcat(concatenate_output, s_index[j]); // Append the name of the successfully deleted file
                                strcat(concatenate_output, "\n");       // Add a newline character
                            }
                            j++; // Move to the next argument
                        }
                    }
                }

                else if (strcmp(input_buf, "RMD") == 0) // Command to remove directories
                {
                    if (s_index[1] == NULL)                                   // If no directory names are provided as arguments
                        strcat(concatenate_output, "argument is required\n"); // Indicate that an argument is required

                    else if (s_index[1][0] == '-') // If an option is provided instead of a directory name
                    {
                        print_error(); // Print an error message
                    }
                    else
                    {
                        j = 1;                    // Start checking arguments from index 1
                        while (j < num_of_sindex) // Loop through all received arguments
                        {
                            if (rmdir(s_index[j]) == -1) // Attempt to remove the directory
                            {
                                strcat(concatenate_output, "failed to remove: "); // Append a message indicating removal failure
                                strcat(concatenate_output, s_index[j]);           // Append the name of the directory causing the error
                                strcat(concatenate_output, "\n");                 // Add a newline character
                                // Print an error if the directory doesn't exist
                            }
                            else
                            {
                                strcat(concatenate_output, "RMD ");     // Append the command name "RMD"
                                strcat(concatenate_output, s_index[j]); // Append the name of the successfully removed directory
                                strcat(concatenate_output, "\n");       // Add a newline character
                                // Print a success message
                            }
                            j++; // Move to the next argument
                        }
                    }
                }
                else if (strcmp(input_buf, "RNFR&RNTO") == 0) // Command to rename files or directories
                {
                    if (num_of_sindex != 3)
                        strcat(concatenate_output, "two arguments required\n"); // Indicate that two arguments are required
                    else if (s_index[1][0] == '-' || s_index[2][0] == '-')      // If an option is provided as an argument
                        print_error();                                          // Print an error message
                    else if (access(s_index[2], F_OK) != -1)
                        strcat(concatenate_output, "Error: name to change already exists\n"); // Indicate that the name to be changed already exists
                    else
                    {
                        rename(s_index[1], s_index[2]);         // Rename the file or directory
                        strcat(concatenate_output, "RNFR: ");   // Append the command name "RNFR"
                        strcat(concatenate_output, s_index[1]); // Append the old name
                        strcat(concatenate_output, "\n");       // Add a newline character
                        strcat(concatenate_output, "RNTO: ");   // Append the command name "RNTO"
                        strcat(concatenate_output, s_index[2]); // Append the new name
                        strcat(concatenate_output, "\n");       // Add a newline character
                    }
                }

                else if (strcmp(input_buf, "QUIT") == 0) // Handling the QUIT command
                {
                    if (num_of_sindex != 1) // Check if arguments are provided
                    {
                        if (s_index[1][0] == '-') // If the argument is an option
                            print_error();        // Print an error message
                        else
                            strcat(concatenate_output, "Error: argument is not required\n"); // Indicate that an argument is not required
                    }
                    else
                        exit(1); // Exit the program
                }
                else
                {
                    strcat(concatenate_output, "unknown instruction\n"); // Indicate that the instruction is unknown
                }

                write(client_fd, concatenate_output, strlen(concatenate_output)); // Send the response to the client
                memset(concatenate_output, 0, 4000);                              // Clear the concatenate_output buffer
                memset(BUFF, 0, 256);                                             // Clear the BUFF buffer
            }
        }
        else // Handling other instructions
        {
            write(STDOUT_FILENO, "==========Client info==========\n", strlen("==========Client info==========\n")); // Print client information
            write(STDOUT_FILENO, "client IP : ", strlen("Client IP : "));                                           // Print client IP
            write(STDOUT_FILENO, inet_ntoa(client_addr.sin_addr), strlen(inet_ntoa(client_addr.sin_addr)));         // Print client IP address
            write(STDOUT_FILENO, "\n\nclient prot : ", strlen("\n\nclient prot : "));                               // Print client port
            sprintf(PORT_string, "%d", client_addr.sin_port);                                                       // Save client port
            write(STDOUT_FILENO, PORT_string, strlen(PORT_string));                                                 // Print client port
            write(STDOUT_FILENO, "\n", 1);                                                                          // New line
            write(STDOUT_FILENO, "===============================\n", strlen("===============================\n")); // Divider
            child_s[num_of_child].pid = pid;                                                                        // Save child process ID
            child_s[num_of_child].PORT_string = PORT_string;                                                        // Save client port
            child_s[num_of_child].start_time = time(NULL);                                                          // Save child process start time
            num_of_child++;                                                                                         // Increment child process counter

            print_child(child_s, num_of_child); // Print child process details

            write(STDOUT_FILENO, "child Process ID : ", strlen("child Process ID : ")); // Print child process ID
            sprintf(PORT_string, "%d", pid);                                       // Save child process ID
            write(STDOUT_FILENO, PORT_string, strlen(PORT_string));                     // Print child process ID
            write(STDOUT_FILENO, "\n", 1);                                              // New line
        }

        close(client_fd); // Close client socket
    }

    return 0;
}

void sh_chld(int signum)
{
    int i = 0;
    pid_t child_id;

    child_id = wait(NULL);                       // Wait for any child process to terminate
    printf("Client(%d) is Relased\n", child_id); // Print message indicating released client

    while (i < num_of_child)
    {
        if (child_s[i].pid == child_id)
        {
            // Remove terminated child process from the array
            for (int j = i; j < num_of_child - 1; j++)
            {
                child_s[j] = child_s[j + 1];
            }
            break;
        }
        i++;
    }
    num_of_child--; // Decrement the count of terminated child processes
}

void sh_alrm(int signum)
{
    print_child(child_s, num_of_child); // Print information of child processes
}

void print_child(child_data *child, int i)
{
    printf("Current Number of Clients: %d\n", i); // Print the current number of clients
    printf("   pid     PORT     TIME\n");         // Print the header for PID, PORT, and TIME

    int j = 0;
    while (j < i)
    {
        printf("  %d     %s      %ld\n", child[j].pid, child[j].PORT_string, time(NULL) - child[j].start_time); // Print PID, PORT, and elapsed time
        j++;
    }
    alarm(10); // Reset the alarm
    return;
}

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print);
    write(1, print, length); // Write the given string to stdout
}

void sh_int(int signum)
{
    exit(1); // Exit the process
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
            strcat(concatenate_output, "invalid option\n"); // Append an error message indicating an invalid option
            num = num + 100;                                // Set num to a value greater than 2 to indicate an invalid option
        }
    return num; // Return the total value of options found
}

void print_error() // Function to print error messages
{
    strcat(concatenate_output, "invalid option\n"); // Append an error message indicating an invalid option
}

void print_non_option(char *arr[], int n, int cd) // When there are no options
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

void print_a(char *arr[], int n)
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

void print_al(char *filename[], int n)
{
    int word = 0;
    struct stat file_stat;
    for (int i = 0; i < n; i++)
    {
        stat(filename[i], &file_stat); // Check file permissions and save them to a string
        char mode_str[11];             // String to store permissions
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
        strcat(concatenate_output, filename[i]);
        strcat(concatenate_output, "\n");
    }
}

void print_l(char *filename[], int n)
{
    int word = 0;
    struct stat file_stat;
    for (int i = 0; i < n; i++)
    {
        if (filename[i][0] != '.') // Exclude hidden files and print the rest. Other variables serve the same purpose as print_al.
        {
            stat(filename[i], &file_stat); // Check file permissions and save them to a string
            char mode_str[11];             // String to store permissions
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
            strcat(concatenate_output, filename[i]);
            strcat(concatenate_output, "\n");
        }
    }
}

void bubbleSort(char *arr[], int n) // Bubble sort used for sorting file names in a directory
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