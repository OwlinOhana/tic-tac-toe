#include "client.c"

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    perror("ERROR, no port provided\n");
    exit(1);
  }

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);


	printf("Please enter your name: ");
  fgets(name, 32, stdin);
  str_trim_lf(name, strlen(name));

	if (strlen(name) > 32 || strlen(name) < 2)
  {
		printf("Name must be less than 30 and more than 2 characters.\n");
		exit(1);
	}

  struct sockaddr_in server = {0};

  int sockfd = Socket(AF_INET, SOCK_STREAM, 0); //протокол
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_port = htons(port);

  // Connect to Server
  Connect(sockfd, (struct sockaddr *)&server, sizeof server); 
 	// Send name
	send(sockfd, name, NAME_LEN, 0);
  char msg[4];
	printf("\n**********Welcome**********\n");
 
  do {
    recv_msg(sockfd, msg);
    if (!strcmp(msg, "HLD"))
      printf("Waiting for a second player...\n");
  } while (strcmp(msg, "SRT") );

  while(1) 
  {
    RecvByServer(sockfd, msg);
    if (!strcmp(msg, "TRN")) 
    { 
	    printf("Your move...\n");
	    if(SendFromServer(sockfd)==2)
        break;
    }
    else if (!strcmp(msg, "INV"))
    {
      printf("That position has already been played. Try again.\n"); 
      if(SendFromServer(sockfd)==2)
        break;
    }  
     
    // else if (!strcmp(msg, "CNT")) 
    // { 
    //   int num_players = recv_int(sockfd);
    //   printf("There are currently %d active players.\n", num_players); 
    // }
    else if (!strcmp(msg, "WAT"))  
      printf("Waiting for other players move...\n");
        
    else if(strcmp(name, msg) == 0)
    {
      printf("You win!\n");
      break;
    }
    else if(strcmp(name, msg) != 0)
    {
      printf("You lost.\n");
      break;
    }
    else 
    {
      printf("Sorry, game end");
      break;
    }
  }
         
		close(sockfd);

	return 0;
}