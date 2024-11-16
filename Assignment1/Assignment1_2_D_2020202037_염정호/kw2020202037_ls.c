///////////////////////////////////////////////////////////////////////
// File Name : kw2020202037_ls.c //
// Date : 2024/04/10 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-2 ( ftp server ) //
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// InsertNode //
// ================================================================= //
// Input: dp* -> Pointer used to store directory addresses
// dirp -> read file data;
// Purpose: Processing input factors //
///////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[])
{

    DIR *dp;   //
    struct dirent *dirp = 0;

    // If there is only one factor


////////////////////////////check input////////////////////////////
    if (argc == 1)
        dp = opendir("."); // Current Directory
    // If you receive a directory as a factor
    else if (argc > 2){
        printf("only one directory path can be processed\n");
    return 0;
}
    else
    {
        
////////////////////////////////////////////////////////
        if ((opendir(argv[1])) == NULL)

        {
            ////////////////////////////error print////////////////////////////
           if(errno == ENOTDIR)
               printf("kw2020202037_ls: cannot access '%s' : No such directory\n", argv[1]); // Error output when directory does not dir
            else if (errno == EACCES)
                printf("kw2020202037_ls: cannnot access '%s' : Access denied \n", argv[1]); // Error output when you do not have access
            else
                printf("kw2020202037_ls: cannot access '%s' : No such directory\n", argv[1]); // Error output when directory does not exist.
            return 0;
            ////////////////////////////////////////////////////////
        }

        dp = opendir(argv[1]); // Save the directory address when no error occurs
    }

    // Browse all files in the directory
    while ((dirp = readdir(dp)) != 0) //
        printf("%s\n", dirp->d_name); // print file name
    
    if (closedir(dp))
        printf("closed");
}

