///////////////////////////////////////////////////////////////////////
// File Name : cli.c //
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
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define BUF_SIZE 256

int main(int argc, char **argv)
{

    char buff[BUF_SIZE]; // i/o buff
    int n;
    int sockfd; // Socket descriptor 
    struct sockaddr_in serv_addr; // socket address data

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // open socket file descriptor

    memset(&serv_addr, 0, sizeof(serv_addr)); // reset address
    serv_addr.sin_family = AF_INET; // domain
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // address 
    serv_addr.sin_port = htons(atoi(argv[2])); // port
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //connect request to srv

    while (1)
    {
        memset(buff, 0, BUF_SIZE); // reset buff
        write(STDOUT_FILENO, "> ", 2); // print 
        read(STDIN_FILENO, buff, BUF_SIZE); // get input

        if (write(sockfd, buff, BUF_SIZE) > 0) //send input to srv
        {
            if (read(sockfd, buff, BUF_SIZE) > 0)
                printf("from server:%s", buff); // print recieved data from srv
            else
                break;
        }
        else
            break;
    }
    close(sockfd); // close descriptor
    return 0;
}