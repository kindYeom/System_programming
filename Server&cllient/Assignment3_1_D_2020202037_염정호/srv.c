///////////////////////////////////////////////////////////////////////
// File Name : srv.c //
// Date : 2024/05/17 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom Jung Ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #3-1
// Description :
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include<pwd.h>

#define MAX_BUF 20

// Function declarations
int user_match(char *user, char *passwd);
int log_auth(int connfd);
void print_data(const char *print);



int main(int argc, char *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    FILE *fp_checkIP; // FILE stream to check client’s IP

    char check_ip[20], *client_ip;
    int i = 0, j = 0;
    char stri[10];

    // Create a socket for the server
    listenfd = socket(PF_INET, SOCK_STREAM, 0); // Get server file descriptor

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Get server address
    servaddr.sin_port = htons(atoi(argv[1]));     // Get port number

    // Bind the socket to the server address
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // Listen for incoming connections
    listen(listenfd, 5);
    for (;;)
    {
        int len = sizeof(cliaddr);
        // Accept a connection from a client
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        client_ip = inet_ntoa(cliaddr.sin_addr);

        // Print client connection details
        printf("**Client is trying to connect **\n");
        printf("- IP:   %s\n", client_ip);
        printf("Port:   %d\n", cliaddr.sin_port);

        // Open the access file to check client's IP
        fp_checkIP = fopen("access.txt", "r");

        if (fp_checkIP == NULL)
        {
            printf("**file \"access.txt\" doesn't exist**");
            return 0;
        }

        // Check if the client IP matches any entry in the access file
        while (fgets(check_ip, sizeof(check_ip), fp_checkIP) != NULL)
        {
            if (check_ip[strlen(check_ip)] == '\n')
            {
                check_ip[strlen(check_ip)] = '\0';
            }
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
                        i = i + 1;
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
            write(connfd, "ACCEPTED", strlen("ACCEPTED") + 1);
            printf("** Client is connected **\n");
        }
        else
        {
            // If client IP is not matched, reject connection
            write(connfd, "REJECTION", strlen("REJECTION") + 1);
            printf("**It is NOT authenticated client**\n");
            continue;
        }

        // Authenticate user log-in
        if (log_auth(connfd) == 0)
        { // if 3 times fail (ok : 1, fail : 0)
            printf("** Fail to log-in **\n");
        }
    }
}

int log_auth(int connfd)
{
    char user[MAX_BUF], passwd[MAX_BUF];
    int n, count = 1;
    char count_s[2];
    while (1)
    {
        memset(user, 0, MAX_BUF);
        memset(passwd, 0, MAX_BUF);

        // Read user credentials from client
        read(connfd, user, MAX_BUF);
        user[strlen(user) - 1] = '\0';
        read(connfd, passwd, MAX_BUF);

        // Print log-in attempt information
        print_data("** User is trying to log-in (");
        sprintf(count_s, "%d", count);
        print_data(count_s);
        print_data("/3) **\n");

        // Match user credentials
        if ((n = user_match(user, passwd)) == 1)
        {
            write(connfd, "OK", MAX_BUF);
            print_data("** Success to log-in **\n");
            break;
        }
        else if (n == 0)
        {
            print_data("** Log-in failed **\n");
            if (count >= 3)
            {
                write(connfd, "DISCONNECTION", MAX_BUF);
                return 0;
            }
            write(connfd, "FAIL", MAX_BUF);
            count++;
            continue;
        }
    }
    return 1;
}

int user_match(char *user, char *passwd)
{
    FILE *fp;
    struct passwd *pw;

    char line[200];
    char *ptr;

    // Open the file containing the user credentials
    fp = fopen("passwd", "r");

    // Check if the file was opened successfully
    if (fp == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    // Read each line from the file
    while ((pw =fgetpwent(fp))!=NULL)
    {

        // Compare the provided user ID and password with the stored values
        if (strcmp(user, pw->pw_name) == 0)
        {
            // If user ID matches, check the password
            if (strcmp(passwd, pw->pw_passwd) == 0)
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

void print_data(const char *print) // Function to print execution results
{
    int length = strlen(print);
    write(1, print, length);
    return;
}