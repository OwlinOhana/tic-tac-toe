#include "clieserv.h"

void str_trim_lf (char* arr, int length) 
{
  for (int i = 0; i < length; i++)
    if (arr[i] == '\n') 
    {
      arr[i] = '\0';
      break;
    }
  
}

int checkHod(int hod)
{
  int A[9];
  int count = 0;
  for (int i = 0; i < 10; i++)
    A[i] = i + 1;

  int i = 0;
  while( i!= 9)
  {
    if(A[i] == hod)
    {
      count++;
      break;
    }
    i++;
  }
  return count;  
}

int printBoardClient(char *file)
{
  int counter = 264;
  FILE *fp;
  if(fp == 0)
  {
    perror("Error: open file for sending");
    return -1;
  }

  char *buff = (char *)malloc((counter+1) * sizeof(char));

  while(!feof(fp))
    fgets(buff, counter, fp);

  fclose(fp);
  free(buff);
  return 0;
}

int SendFromServer(int sockfd)
{
  char message[Buffer_size] = {};
  int i = 0;
  FILE *fp;
  fp = fopen("Board.txt", "r");
  printBoardClient(fp);
  while(i != 2) 
  {
    fgets(message, Buffer_size, stdin);
    str_trim_lf(message, Buffer_size);

    if (strcmp(message, "exit") == 0)
    {
      write_char(sockfd, message);
      break;
    } 

    else if(checkHod(atoi(message)) == 1)
      write_int(sockfd, atoi(message));
    else
    {
      printf("Invalid move. Try again \n");
      i++;
    }
  }
  bzero(message, Buffer_size + NAME_LEN);
  return i;
}

void RecvByServer(int sockfd, char * msg)
{
  memset(msg, 0, 4);
  int res = read(sockfd, msg, 3);
    
  if (res < 0)
    perror("resvfrom failed\n");
}

