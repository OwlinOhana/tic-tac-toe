#include "server.hpp"
#include <cstring>
#include <fstream>

int Sendto(int sockfd, const void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t addrlen)
{
  int res = sendto(sockfd, buf, len, flags, addr, addrlen);
  if (res < 0)
    perror("Sendto failed\n");
  return res;
}

int Resvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen)
{
  int res = recvfrom(sockfd, buf, len, flags, addr, addrlen);
  if (res < 0)
    perror("resvfrom failed\n");
  return res; 
}

int Recv(int sockfd, void *buf, size_t len)
{
  int res = read(sockfd, buf, len);
  if (res < 0)
    perror("resvfrom failed\n");
  return res;
}

void handleClient(int port)
{
  char *ip = "127.0.0.1";

  struct sockaddr_in serverAdr = {0};
  struct sockaddr_in clientAddr = {0};
  pthread_t tid;

  int server = Socket(AF_INET, SOCK_STREAM, 0); //протокол
  serverAdr.sin_family = AF_INET;
  serverAdr.sin_addr.s_addr = inet_addr(ip);
  serverAdr.sin_port = htons(port);

  Bind(server, (struct sockaddr *) &serverAdr, sizeof serverAdr);

  Listen(server, 6);

  int leaveFlag = 1;
  int player = 0;

  Game *game1 = (Game *)malloc(sizeof(Game));//структура игроков

  while (player < 2)
  {
    while (leaveFlag == 1)
    {
      socklen_t clieLen = sizeof(clientAddr);
      int connfd = Accept(server, (struct sockaddr *) &clientAddr, &clieLen);
      game1->addres1 = clientAddr;
      game1->sockfd = connfd;
      game1->uidPlayer1 = uid++;
      
      char buf[Buffer_size];
      char name[NAME_LEN];

      if(recv(game1->sockfd, name, NAME_LEN, 0) <= 0 || strlen(name) <  2 || strlen(name) >= NAME_LEN-1)
      {
        printf("Didn't enter the name.\n");
        close(game1->sockfd);
        leaveFlag = 1;
      } 
      else
      {
        strcpy(game1->namePlayer1, name);
        leaveFlag = 0;
        // sprintf(buf, "%s has joined\n", cli->name);
        // printf("%s", buf);
        // sendMessage(buf, cli->uid);
      } 
      free(buf);
      free(name);
    }
    player++;
    
    //собираем данные о втором игроке
    leaveFlag = 1;
    write_char(game1->sockfd, "HLD");
    while (leaveFlag == 1)
    {
      socklen_t clieLen = sizeof(clientAddr);
      int connfd = Accept(server, (struct sockaddr *) &clientAddr, &clieLen);
      game1->addres2 = clientAddr;
      game1->sockfd1 = connfd;
      game1->uidPlayer2 = uid++;
      
      char buf[Buffer_size];
      char name[NAME_LEN];

      if(recv(game1->sockfd1, name, NAME_LEN, 0) <= 0 || strlen(name) <  2 || strlen(name) >= NAME_LEN-1)
      {
        printf("Didn't enter the name.\n");
        leaveFlag = 1;
        close(game1->sockfd1);
      } 
      else
      {
        strcpy(game1->namePlayer2, name);
        leaveFlag = 0;
        // sprintf(buf, "%s has joined\n", cli->name);
        // printf("%s", buf);
        // sendMessage(buf, cli->uid);
      } 
      free(buf);
      free(name);
    }
    player++;
    write_char(game1->sockfd, "SRT");
    write_char(game1->sockfd1, "SRT");

  }

   for(int i = 0; i < 10; i++ )
      game1->square[i] = board[i];  
  
  Pthread_create(&tid, NULL, &runGame, (void *)game1);
}

// void saveStatusGame(void *arg, struct sockaddr *addr, socklen_t addrlen, int sockfd)
// {
//   ofstream dataBase;
//   Game *cli = (Game *)arg;
//   dataBase.open("dataBase.txt", std::ofstream::out | std::ofstream::trunc);
//   for(int i = 0; i < 10; i++ )
//     dataBase << cli->square[i] << " ";  
//   dataBase << endl;
//   dataBase.close();
//   Sendto(sockfd, (char*)cli, sizeof(*cli), 0, addr, addrlen);
// }


void saveStatusGame(void *arg)
{
  ofstream dataBase;
  Game *cli = (Game *)arg;
  dataBase.open("dataBase.txt", std::ofstream::out | std::ofstream::trunc);
  for(int i = 0; i < 10; i++ )
    dataBase << cli->square[i] << " ";  
  dataBase << endl;
  dataBase.close();
}

void *runGame(void *arg)
{
  char buf[Buffer_size];
  char *sendInfo;
  char name[NAME_LEN];
  int leaveFlags = 0;

  Game *cli = (Game *)arg;

  int choice = 0;

  while(1)
  {
    if (leaveFlags)
     break;
    
    write_char(cli->sockfd, "TRN");
    Recv(cli->sockfd, buf, Buffer_size);
    write_char(cli->sockfd1, "WAT");
    
    if (strcmp(buf, "exit") == 0)
    {
      sendInfo = "End Game";
      break;
    }

    choice = atoi(buf);
    game(choice, 1);
    saveStatusGame(cli);
    printBoard(cli);
    
    if(cli->status == 1)
    {
      strcpy(sendInfo, cli->namePlayer1);
      break;
    }

    if(cli->status == 2)
    { 
      strcpy(sendInfo, cli->namePlayer2);
      break;
    }
    
    bzero(buf, Buffer_size);

    write_char(cli->sockfd1, "TRN");
    Recv(cli->sockfd1, buf, Buffer_size);
    write_char(cli->sockfd, "WAT");

    if (strcmp(buf, "exit") == 0)
    {
      sendInfo = "End Game";
      break;
    }
    
    choice = atoi(buf);
    game(choice, 2);
    saveStatusGame(cli);
    printBoard(cli);

    if(cli->status == 1)
    {
      strcpy(sendInfo, cli->namePlayer1);
      break;
    } else if(cli->status == 2)
      { 
        strcpy(sendInfo, cli->namePlayer2);
        break;
      }

    bzero(buf, Buffer_size);
  }
  
  write_char(cli->sockfd, sendInfo);
  write_char(cli->sockfd1, sendInfo);
  
  close(cli->sockfd1);
  close(cli->sockfd);
 
  free(cli);

  pthread_detach(pthread_self());

 return 0;
}


void game(int choice, int player, struct Game Game) 
{
  string mark, value;
  int i = 0;
  mark = (player == 1) ? "X" : "O";
  if (choice == 1 && Game.square[1] == "1") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[1] = value;
  } else if (choice == 2 && Game.square[2] == "2") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[2] = value;
  } else if (choice == 3 && Game.square[3] == "3") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[3] = value;
  } else if (choice == 4 && Game.square[4] == "4") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[4] = value;
  } else if (choice == 5 && Game.square[5] == "5") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[5] = value;
  } else if (choice == 6 && Game.square[6] == "6") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[6] = value;

  } else if (choice == 7 && Game.square[7] == "7") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[7] = value;
  } else if (choice == 8 && Game.square[8] == "8") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[8] = value;
  } else if (choice == 9 && Game.square[9] == "9") 
  {
    value = "\x1B[31m" + mark;
    value = value + "\033[0m";
    Game.square[9] = value;
  } else 
    cout << "Invalid move ";
  
  i = checkwin();
  if (i == 1)
  {
    if(player == 1)
      Game.status = 1;
    else
     Game.status = 2;
  }
}

int checkwin(struct Game Game) 
{
  if (Game.square[1] == Game.square[2] && Game.square[2] == Game.square[3])
    return 1;
  else if (Game.square[4] == Game.square[5] && Game.square[5] == Game.square[6])
    return 1;
  else if (Game.square[7] == Game.square[8] && Game.square[8] == Game.square[9])
    return 1;
  else if (Game.square[1] == Game.square[4] && Game.square[4] == Game.square[7])
    return 1;
  else if (Game.square[2] == Game.square[5] && Game.square[5] == Game.square[8])
    return 1;
  else if (Game.square[3] == Game.square[6] && Game.square[6] == Game.square[9])
    return 1;
  else if (Game.square[1] == Game.square[5] && Game.square[5] == Game.square[9])
    return 1;
  else if (Game.square[3] == Game.square[5] && Game.square[5] == Game.square[7])
    return 1;
  else if (Game.square[1] != "1" && Game.square[2] != "2" && Game.square[3] != "3" &&
           Game.square[4] != "4" && Game.square[5] != "5" && Game.square[6] != "6" &&
           Game.square[7] != "7" && Game.square[8] != "8" && Game.square[9] != "9")

    return 0;
  else
    return -1;
}

void printBoard(void *arg) 
{
  Game *game = (Game *)arg;
  ofstream Board;
  Board.open("Board.txt", std::ofstream::out | std::ofstream::trunc);
 
  Board << "\n\nTic Tac Toe\n\n";

  Board << "Player 1 (X)  -  Player 2 (O)" << endl << endl;
  Board << endl;

  Board << "     |     |     " << endl;
  Board << "  " << game->square[1] << "  |  " << game->square[2] << "  |  " << game->square[3]
       << endl;

  Board << "_____|_____|_____" << endl;
  Board << "     |     |     " << endl;

  Board << "  " << game->square[4] << "  |  " << game->square[5] << "  |  " << game->square[6]
       << endl;

  Board << "_____|_____|_____" << endl;
  Board << "     |     |     " << endl;

  Board << "  " << game->square[7] << "  |  " << game->square[8] << "  |  " << game->square[9]
       << endl;

  Board << "     |     |     " << endl << endl;
  Board.close();
}