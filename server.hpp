#include "clieserv.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

void game(int choice, int player);
int checkwin();
void printBoard(void *arg); 

string board[10] = {"o", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

struct Game
{
	string square[10];
  char namePlayer1 [NAME_LEN];
  char namePlayer2 [NAME_LEN];
  struct sockaddr_in addres1;
  struct sockaddr_in addres2;
  int sockfd;
  int sockfd1;
  int uidPlayer1;
  int uidPlayer2;
  int status;
};

void handleClient();
void *runGame(void *arg);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Recv(int sockfd, void *buf, size_t len);
int Sendto(int sockfd, const void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t addrlen);
int Resvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen);
