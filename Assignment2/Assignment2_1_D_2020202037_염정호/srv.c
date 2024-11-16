///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/01 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #2-1 ( srv ) //
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////



#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
char concatenate_output[4000];

int check_option(char *arr); // Check options, distinguish as 0, 1, or 2, 3
void print_data(const char *print);
void print_error();                                // Print error messages for options
void print_non_option(char *arr[], int n, int cd); // Print when there are no ls options
void print_a(char *arr[], int n);                  // Print with ls option a
void print_l(char *filename[], int n);             // Print with ls option l
void print_al(char *arr[], int n);                 // Print with ls option l
void send_to_cli(char *output_s, int cd);
void bubbleSort(char *arr[], int n); // Bubble sort for sorting file lists

int main(int argc, char *argv[])
{
    // SOCKET

    struct sockaddr_in server, client;
    unsigned int PORT = atoi(argv[1]);
    char PORT_string[10];

    int n;
    int sd, cd, clientlen = sizeof(client);
    char input_buf[100];

    // OTHERS
    char *index;       // Divide data by spaces and store it
    char *s_index[20]; // Store the separated data
    int num_of_sindex = 0;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    memset((char *)&server, '\0', sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    printf("%d\n", PORT);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)))
    {
        perror("bind");
        exit(1);
    }

    if (listen(sd, 5))
    {
        perror("listen");
        exit(1);
    }

    if ((cd = accept(sd, (struct sockaddr *)&client, &clientlen)) == -1)
    {
        perror("accept");
        exit(1);
    }
    else
    {
        print_data("========= Client info========= \n");
        print_data("client IP: ");
        print_data(inet_ntoa(client.sin_addr));
        print_data("\n");
        print_data("client port: ");
        sprintf(PORT_string, "%d", client.sin_port); // save port
        print_data(PORT_string);
        print_data("\n");
        print_data("========================= \n");
    }
    for (;;)
    {
        n = read(cd, input_buf, sizeof(input_buf));
        input_buf[n] = '\0';
        print_data(input_buf);
        print_data("\n");

        index = strtok(input_buf, " ");
        num_of_sindex = 0;

        while (index != NULL) // Repeat until there are no more strings to cut
        {
            s_index[num_of_sindex] = index; // Store the cut string
            num_of_sindex++;                // Next array
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
                    {
                        print_error(); // Print error
                        return 0;
                    }
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
                    return 0;
                }
            }
            else
            {
                print_data("too much input\n"); // If too many arguments are passed
                return 0;
            }
            // directory open
            if (opendir(dir) == NULL)
            {
                if (errno == ENOTDIR)
                    print_data(" No such directory\n"); // If the directory does not exist
                else if (errno == EACCES)
                    print_data("cannnot access : Access denied \n"); // If there's no read permission
            }
            else
                dp = opendir(dir); // Traverse the directory

            while ((dirp = readdir(dp)) != NULL) // Get information about files in the directory
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
                file_count++; // Next file
            }
            closedir(dp);

            bubbleSort(files, file_count); // Sort the list of file names

            switch (option_i) // Execute functions according to the option
            {
            case 0:
                print_non_option(files, file_count, cd); // If there's no option
                break;
            case 1:
                print_a(files, file_count); // If the option is a
                break;
            case 2:
                print_l(files, file_count); // If the option is l
                break;
            case 3:
                print_al(files, file_count); // If the option is al
                break;
            default:
                strcpy(concatenate_output, "invaild option");
                break;
            }

            write(cd, concatenate_output, strlen(concatenate_output));
            memset(concatenate_output, 0, 4000); // recv_from_srv 초기화
        }
        else if (strcmp(s_index[0], "QUIT") == 0)
        {
            strcpy(concatenate_output, "Program QUIT!!!\n");
            write(cd, concatenate_output, strlen(concatenate_output));
            memset(concatenate_output, 0, 4000); // recv_from_srv 초기화
            close(cd);
            close(sd);
            break;
        }
    }

    return 0;
}

/// END MAIN////

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print);
    write(1, print, length);
    return;
}

int check_option(char *arr) // Function to check options
{
    int num = 0; // 0 if there are no options
    for (int i = 1; i < strlen(arr); i++)

        if (arr[i] == 'a')
            num = num + 1;
        else if (arr[i] == 'l')
            num = num + 2;
        else
            print_data("invaild option");

    return num;
}

void print_error() // Function to print error messages
{
    write(1, "invalid option\n", strlen("invalid option\n"));
    return;
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