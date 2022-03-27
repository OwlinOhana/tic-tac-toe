#include "clieserv.h"

int checkHod(int hod);
void str_trim_lf (char* arr, int length); 
int Recv(int sockfd, void *buf, size_t len, int flags);
void saveStatusGame(void *arg, struct sockaddr *addr, socklen_t addrlen, int sockfd);
int printBoardClient(int cli, char *file);
int SendFromServer(int sockfd);
void RecvByServer(int sockfd, char * msg);