///////////////////////////////////////////////////////////////////////
// File Name : kw2020202037_ls.c //
// Date : 2024/04/16 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-3 ( sra ) //
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// InsertNode //
// ================================================================= //
// Input:
//
//
//
// NLST LIST PWD CWD CDUP MKD DELE RMD RNFR & RNTO QUITZ
///////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void print_error();                        // Print error messages for options
void print_data(const char *print);        // Print results using write
void bubbleSort(char *arr[], int n);       // Bubble sort for sorting file lists
void print_non_option(char *arr[], int n); // Print when there are no ls options
void print_a(char *arr[], int n);          // Print with ls option a
void print_l(char *filename[], int n);     // Print with ls option l
void print_al(char *arr[], int n);         // Print with ls option l
int check_option(char *arr);               // Check options, distinguish as 0, 1, or 2, 3

int main(int argc, char *argv[])
{

    char output_buf[200];                 // String used for outputting results on the screen
    char input_buf[100];                  // String to store data received from the client
    int i = 0;                            // Variable used in a for loop
    int j = 0;                            // Variable used in a nested for loop
    read(0, input_buf, 100);              // Store data received from the CLI
    char *index = strtok(input_buf, " "); // Divide data by spaces and store it
    char *s_index[20];                    // Store the separated data

    if (input_buf[0] == '\0')
        return 0;

    while (index != NULL) // Repeat until there are no more strings to cut
    {
        s_index[i] = index;        // Store the cut string
        i++;                       // Next array
        index = strtok(NULL, " "); // Find the next token
    }

    if (strcmp(input_buf, "NLST") == 0) // If the command is ls
    {

        DIR *dp;                 // Variable for directory traversal
        struct dirent *dirp = 0; // Structure to hold information about files within a directory
        char *dir;               // Variable to store the directory address to be searched

        char *files[100];   // Array to store the list of files
        int file_count = 0; // Integer to store the number of files in the list
        int option_i = 0;   // Store options as integers: 0 for none, 1 for a, 2 for l, 3 for al

        if (i == 1)      // If only the command exists
            dir = ".";   // Output the current directory
        else if (i == 2) // If two arguments are received
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
        else if (i == 3)
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

        if (opendir(dir) == NULL)
        {
            if (errno == ENOTDIR)
                print_data(" No such directory\n"); // If the directory does not exist
            else if (errno == EACCES)
                print_data("cannnot access : Access denied \n"); // If there's no read permission
            return 0;
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
        switch (option_i)              // Execute functions according to the option
        {
        case 0:
            print_non_option(files, file_count); // If there's no option
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
        }
        print_data("\n");
    }

   else if (strcmp(input_buf, "LIST") == 0)
{
    DIR *dp;                 // Variable for directory traversal
    struct dirent *dirp = 0; // Structure to hold information about files within a directory
    char *dir;               // String to store the address of the file

    char *files[1000];  // Array to store the list of files
    int file_count = 0; // Integer to store the number of files

    if (i == 1)    // If only the command is entered
        dir = "."; // Current directory
    else if (i == 2)
    {
        if (s_index[1][0] == '-') // Exception handling for options
        {
            print_error();
            return 0;
        }
        else
            dir = s_index[1]; // Store the address
    }
    else
    {
        print_data("too much input\n");
        return 0;
    }

    if (opendir(dir) == NULL)
    {
        if (errno == ENOTDIR)
            print_data(" No such directory\n"); // If the directory does not exist
        else if (errno == EACCES)
            print_data("cannnot access : Access denied \n"); // If there's no read permission
        return 0;
    }
    else
        dp = opendir(dir); // Open the directory

    while ((dirp = readdir(dp)) != NULL) // Files inside the directory
    {
        char file_path[300];                            // String to store the address of the file
        struct stat file_stat;                          // Structure to store file information
        sprintf(file_path, "%s/%s", dir, dirp->d_name); // Create the file path

        files[file_count] = dirp->d_name; // Store the file name
        stat(file_path, &file_stat);      // Get the file information via the path
        if (S_ISDIR(file_stat.st_mode))
        {
            strcat(files[file_count], "/"); // Append '/' if it's a directory
        }
        file_count++;
    }
    closedir(dp);

    bubbleSort(files, file_count); // Sort the files

    print_al(files, file_count); // ls -al
}
/////////////////////print working directory//////////////////////////////////////////
else if (strcmp(input_buf, "PWD") == 0) // Print the current working directory
{
    if (i > 1)
    {
        if (s_index[1][0] == '-')
            print_error(); // When an option is passed as an argument
        else
            print_data("argument is not required\n"); // When more than one argument is given
        return 0;
    }

    print_data("Current working directory: ");
    print_data(getcwd(output_buf, sizeof(output_buf))); // Print the current directory
    print_data("\n");
}
//*****************************complete************************************
else if (strcmp(input_buf, "CWD") == 0) // cd command
{

    if (s_index[1][0] == '-')
    {
        print_error(); // When an option is passed as an argument
        return 0;
    }
    else if (i != 2)
    {
        print_data("worng number of argument\n"); // When more than two arguments are given
        return 0;
    }

    if (chdir(s_index[1]) == -1)
    {
        print_data("Error : directory not found\n");
        return 0;
    }
    print_data(getcwd(output_buf, sizeof(output_buf)));
    print_data(" is current directory\n");
}
//*****************************complete************************************

///////////////////make directory/////////////////////
else if (strcmp(input_buf, "CDUP") == 0)
{

    if (i != 2)
    {
        if (s_index[2][0] == '-')
            print_error();
        else
            print_data("worng number of argument\n");
        return 0;
    }

    if (chdir(s_index[1]) == -1)
    {
        print_data("no parent directory\n");
        return 0;
    }
    print_data(getcwd(output_buf, sizeof(output_buf)));
    print_data(" is current directory\n");
}
//*****************************complete************************************
else if (strcmp(input_buf, "MKD") == 0)
{
    if (i == 1) // If no arguments are provided
    {

        print_data("argument is required \n");
        return 0;
    }
    else if (s_index[1][0] == '-') // Option exception
    {
        print_error();
        return 0;
    }
    j = 1; // Check all received arguments
    while (j < i)
    {
        if (mkdir(s_index[j], 775) == -1) // Directory creation with permissions 775
        {
            print_data("Error cannot create directory '");
            print_data(s_index[j]);
            print_data("\n");
            // Print error if failed
        }
        else
        {
            print_data("MKD  ");
            print_data(s_index[j]);
            print_data("\n");
            // Print result if successful
        }
        j++; // Next argument
    }
}
//*****************************complete************************************
//////////////////////DELETE////////////////////

else if (strcmp(input_buf, "DELE") == 0) // Command to delete files
{

    if (s_index[1] == NULL) // If no arguments are provided
    {
        print_data("argument is required\n");
        return 0;
    }
    else if (s_index[1][0] == '-') // Error if an option is provided
    {
        print_error();
        return 0;
    }

    j = 1;

    while (j < i) // Delete all files received as arguments
    {
        if (unlink(s_index[j]) == -1)
        {
            print_data("failed to delete '");
            print_data(s_index[j]);
            print_data("'\n");
            // Print error if the file doesn't exist
        }

        else
        {

            print_data("DELE  ");
            print_data(s_index[j]);
            print_data("'\n");
            // Print success message if successful
        }

        j++; // Next file
    }
}
/////////////////////REMOVE DIRECTORY//////////////////////////////////////////
else if (strcmp(input_buf, "RMD") == 0)
{
    if (s_index[1] == NULL) // If no arguments are provided
    {
        print_data("argument is required\n");
        return 0;
    }
    else if (s_index[1][0] == '-') // Error if an option is provided
    {
        print_error();
        return 0;
    }

    j = 1;
    while (j < i) // Traverse all files received as arguments
    {
        if (rmdir(s_index[j]) == -1)
        {
            print_data("failed to remove '");
            print_data(s_index[j]);
            print_data("'\n");
            // Print error if the directory doesn't exist
        }
        else
        {
            print_data("RMD ");
            print_data(s_index[j]);
            print_data("'\n");
            // Print success message
        }
        j++;
    }
}
//*****************************complete************************************

else if (strcmp(input_buf, "RNFR&RNTO") == 0)
{
    if (i != 3)
    {
        print_data("two argument required\n"); // When there are not two arguments

        return 0;
    }
    else if (s_index[1][0] == '-' || s_index[2][0] == '-') // If an option is provided as an argument
    {
        print_error();
        return 0;
    }
    if (access(s_index[2], F_OK) != -1)
    {
        print_data("Error: name to chage already exist\n");
        return 0;
    }
    rename(s_index[1], s_index[2]);

    print_data("RNFR : ");
    print_data(s_index[1]);
    print_data(" RNTO : ");
    print_data(s_index[2]);
    print_data("\n");
    // Print if the name change is successful
    return 0;
}

else if (strcmp(input_buf, "QUIT") == 0) //
{
    if (i != 1) // If arguments are provided
    {
        if (s_index[1][0] == '-') // If the argument is an option
            print_error();
        else
            print_data("Error: argument is not required\n"); // If it's an argument other than an option
        return 0;
    }
    print_data("QUIT success\n"); // Print success message
    return 0;
}
else
    return 0;

return 0;
}


void print_error() // Function to print error messages
{
    write(1, "invalid option\n", strlen("invalid option\n"));
    return;
}

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print);
    write(1, print, length);
    return;
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

int check_option(char *arr) // Function to check options
{
    int num = 0; // 0 if there are no options
    for (int i = 1; i < strlen(arr); i++)

        if (arr[i] == 'a')
            num = num + 1;
        else if (arr[i] == 'l')
            num = num + 2;
        else
            return -1; // Incorrect option

    return num;
}

void print_non_option(char *arr[], int n) // When there are no options
{
    int word = 0;
    for (int i = 0; i < n; i++)
    {
        if (arr[i][0] != '.') // Exclude hidden directories
        {
            write(1, arr[i], strlen(arr[i]));
            write(1, "             ", 12);
            word++;
        }
        if (word > 5) // Change line every 5 words
        {
            word -= 5;
            write(1, "\n", 1);
        }
    }
}

void print_a(char *arr[], int n)
{
    int word = 0;
    for (int i = 0; i < n; i++) // Print all files
    {
        write(1, arr[i], strlen(arr[i]));
        write(1, "             ", 12);
        word++;
        if (word > 5)
        {
            word -= 5;
            write(1, "\n", 1);
        }
    }
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
        write(1, mode_str, strlen(mode_str));           // Print permissions
        write(1, " ", 1);

        sprintf(nlink_str, "%ld ", file_stat.st_nlink); // Store long int value in nlink_str string to print it
        write(1, nlink_str, strlen(nlink_str));

        write(1, " ", 1);

        write(1, pw->pw_name, strlen(pw->pw_name) + 1); // Print user name
        write(1, " ", 1);
        write(1, gr->gr_name, strlen(gr->gr_name)); // Print group name
        write(1, " ", 1);

        char size_str[20];
        sprintf(size_str, "%ld", (long)file_stat.st_size); // Store file size, an integer, as a string
        write(1, size_str, strlen(size_str));

        write(1, " ", 1);
        char time_str[100]; // Variable to store time here
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
        write(1, time_str, strlen(time_str));
        write(1, " ", 1);
        write(1, filename[i], strlen(filename[i])); // Print file name
        write(1, "\n", 1);
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
            stat(filename[i], &file_stat);
            char mode_str[11];
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

            struct passwd *pw = getpwuid(file_stat.st_uid);
            struct group *gr = getgrgid(file_stat.st_gid);
            char nlink_str[100];
            write(1, mode_str, strlen(mode_str));
            write(1, " ", 1);

            sprintf(nlink_str, "%ld ", file_stat.st_nlink);
            write(1, nlink_str, strlen(nlink_str));

            write(1, " ", 1);

            write(1, pw->pw_name, strlen(pw->pw_name) + 1);
            write(1, " ", 1);
            write(1, gr->gr_name, strlen(gr->gr_name));
            write(1, " ", 1);
            char size_str[20];
            sprintf(size_str, "%ld", (long)file_stat.st_size);
            write(1, size_str, strlen(size_str));
            write(1, " ", 1);
            char time_str[100]; // Variable to store time here
            strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
            write(1, time_str, strlen(time_str));
            write(1, " ", 1);
            write(1, filename[i], strlen(filename[i]));
            write(1, "\n", 1);
        }
    }
}
