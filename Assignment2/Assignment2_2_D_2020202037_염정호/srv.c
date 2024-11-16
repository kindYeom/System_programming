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

#define BUF_SIZE 256 // set buffer size



int main(int argc, char **argv)
{

    void sh_chld(int); // signal handler for SIGCHLD
    void sh_alrm(int); // signal handler for SIGALRM

    char buff[BUF_SIZE]; // i/o buffer
    int n;//
    struct sockaddr_in server_addr, client_addr; // server and client address
    int server_fd, client_fd; // file descriptor
    int len; //size of client address
    int port; // port number

    signal(SIGCHLD, sh_chld); /* Applying signal handler(sh_alrm) for SIGALRM */
    signal(SIGALRM, sh_alrm); /* Applying signal handler(sh_chld) for SIGCHLD */

    server_fd = socket(PF_INET, SOCK_STREAM, 0); // get server file desrcriptor

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // get adder
    server_addr.sin_port = htons(atoi(argv[1])); // get port number

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); // bind

    listen(server_fd, 5); // get requist

    while (1)
    {
        char PORT_string[10];
        PORT_string[0] = '\0';
        pid_t pid;
        len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len); // connect to server

   
        if ((pid = fork()) < 0)
        {
        }
        else if (pid == 0)
        { // get data from client
            sleep(1); // delay 1 second
           write(STDOUT_FILENO, "child Process ID : ", strlen("child Process ID : "));
            sprintf(PORT_string, "%d", getpid());
            write(STDOUT_FILENO, PORT_string, strlen(PORT_string));
            write(STDOUT_FILENO, "\n", 1);

            while (1)
            {
                if (read(client_fd, buff, BUF_SIZE) > 0)
                {
                    if (strcmp(buff, "QUIT\n") != 0)
                        write(client_fd, buff, BUF_SIZE);
                    else
                        alarm(1);
                }
            }
        }
        else
        {
            write(STDOUT_FILENO, "==========Client info==========\n", strlen("==========Client info==========\n"));
            write(STDOUT_FILENO, "client IP : ", strlen("Client IP : "));
            write(STDOUT_FILENO, inet_ntoa(client_addr.sin_addr), strlen(inet_ntoa(client_addr.sin_addr)));
            write(STDOUT_FILENO, "\n\nclient prot : ", strlen("\n\nclient prot : "));
            sprintf(PORT_string, "%d", client_addr.sin_port); // save port
            write(STDOUT_FILENO, PORT_string, strlen(PORT_string));
            write(STDOUT_FILENO, "\n", 1);
            write(STDOUT_FILENO, "===============================\n", strlen("===============================\n"));
        }
        close(client_fd);
    }

    return 0;
}

void sh_chld(int signum)
{
    printf("Status of Child process was changed.\n"); //print
    wait(NULL);
}

void sh_alrm(int signum) //print alarm
{
    printf("Child Process(PID : %d) will be terminated.\n", getpid());
    exit(1);
}