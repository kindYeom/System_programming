///////////////////////////////////////////////////////////////////////
// File Name : kw2020202037_opt.c //
// Date : 2024/03/31 //
// OS : Ubuntu 20.04.6 LTS 64bits
//
// Author : Yeom jung ho //
// Student ID : 2020202037 //
// ----------------------------------------------------------------- //
// Title : System Programming Assignment #1-1 ( ftp server ) //
// 
///////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
// vairable set
    int aflag = 0, bflag = 0; // falg set
    char *cvalue = NULL; // cvalue
    int index, c;//index -> num of input c->input
    opterr = 0;
///////////////////////////////////////////////////////////////////////

    while ((c = getopt(argc, argv, "abc: ")) != -1) // 입력값 확인
    {

        switch (c) //c값에 따라 변수 설정
        {
        case 'a':
            aflag  = 1;
            break;
        case 'b':
            bflag  = 1;
            break;
        case 'c':
            cvalue = optarg;
            break;
        default:	

            break;
				
        }
    }
    printf("aflag = %d, bflag = %d,cvalue = %s\n", aflag, bflag, cvalue); // 입력 값 출력
	
    
      for (index = optind; index < argc; index++) // 잘못 들어온 명령어들 출력
       printf ("Non-option argument %s\n", argv[index]);

       
    return 0;
    
    
    
}




