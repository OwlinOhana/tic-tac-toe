#include "clieserv.h"

int Socket(int domain, int type, int protocol)
{
  int res = socket(domain, type, protocol);
  if (res == -1)
  {
    perror("socket failed\n");
    exit(1);
  }
  return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int res = bind(sockfd, addr, addrlen);
  if (res == -1)
  {
    perror("bind failed\n");
    exit(1);
  }
}

void Listen(int sockfd, int backlog)
{
  int res = listen(sockfd, backlog);
  if (res == -1)
  {
    perror("listen failed");
    exit(1);
  }
}

int Accept(int sickfd, struct sockaddr *addr, socklen_t *addrlen)
{
  int res = accept(sickfd, addr, addrlen);
  if (res == -1)
  {
    perror("accept failed\n");
    exit(1);
  }
  return res;
}


void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int res = connect(sockfd, addr, addrlen);
  if (res == -1)
  {
    perror("connect failed\n");
    exit(1);
  }
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arq)
{
  int res = pthread_create(thread, attr, start_routine, arq);
  if (res != 0)
  {
    perror("pthread_create failed\n");
    exit(1);
  }
  return res;
}

void write_int(int sockfd, int msg)
{
  int res = write(sockfd, &msg, sizeof(int));
  if (res < 0)
    perror("resvfrom failed\n");
}

void write_char(int sockfd, char *msg)
{
  int res = write(sockfd, msg, sizeof(msg));
  if (res < 0)
    perror("resvfrom failed\n");
}