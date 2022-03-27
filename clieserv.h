#ifndef CLIESERV_H

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
//#include <signal.h>

#define MAX_COUPLE 6
#define Buffer_size 10
#define NAME_LEN 15


int flag = 0;
char name[32];

static int coupleCount = 0;
static int uid = 10;

int Socket(int domain, int type, int protocol);
int Accept(int sickfd, struct sockaddr *addr, socklen_t *addrlen);
void Connect(int socket, const struct sockaddr *addr, socklen_t addrlen);
void Inet_pton(int af, const char *src, void *dst);
void sendMessage(char *text, int uid);
int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arq);
void write_int(int sockfd, int msg);
void write_char(int sockfd, char *msg);

#endif