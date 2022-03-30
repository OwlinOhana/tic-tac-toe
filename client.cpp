#pragma once
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
using namespace std;

#define MAIN_PORT 3425
#define LAST_PORT 3427
#define SERV_ID "127.0.0.1"
#define DISCONNECTED "Opponent has disconnected from server"
extern const char CLIENT_CRASH_MSG;

int socket_settings(char const *id, uint16_t port);
bool check_server(int sock);
int wait_opponent_move(int *sock, char sign);
void chose_sign(int *sock, char *sign);
void wait_opponent_sign(int *sock, char sign);
void make_move(int *sock, char sign);
void client_handler(int sock);
void print_sign_part(int i, int j, int move);
void print_game_board(void);


const char CLIENT_CRASH_MSG = char(0x80);

char board[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int socket_settings(char const *id, uint16_t port) {
    struct sockaddr_in addr;
    struct hostent *hp;

	int sock = socket(AF_INET, SOCK_STREAM, 0);
  
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    hp = gethostbyname(id);
    
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return -1;

    return sock;
}

bool check_server(int sock)
{
    int status;

    send(sock, &status, sizeof(status), 0);

    int bytes_read = recv(sock, &status, sizeof(status), 0);

    if (CLIENT_CRASH_MSG == char(status))
    {
        cout << DISCONNECTED << endl;
        exit(0);
    }

    if(!bytes_read)
        return false;
    
    return true;
}

void chose_sign(int *sock, char *sign)
{
    bool is_avl_sign = true;

    for(int i = 0;; ++i)
    {
        system("clear");
        cout << " ~ " << "Choose your sign X or O" << " ~ " << endl << endl;

        try
        {
            if(i)
            {
                int start_game = 0;
                send(*sock, &start_game, sizeof(start_game), 0);
            }

            string msg = "";
            do {
                if(!is_avl_sign)
                {
                    system("clear");
                    cout << " ~ " << "Choose your sign X or O" << " ~ " << endl << endl;
                    cout << " ~ " << msg << " ~ " << endl;
                }

                string buf;
                cout << "> " << flush;
                getline(cin, buf);
                *sign = toupper(buf[0]);
                send(*sock, sign, sizeof(*sign), 0);

                if(!recv(*sock, &is_avl_sign, sizeof(is_avl_sign), 0))
                    throw -1;
                if (CLIENT_CRASH_MSG == char(is_avl_sign))
                {
                    cout << DISCONNECTED << endl;
                    exit(0);
                }

                if (*sign != 'X' || *sign != 'O')
                    msg = "Invalid input! Try again";
                else
                    msg = "This sign has already been choosen";
            }
            while(!is_avl_sign);

           break;
        }
        catch(int)
        { 
            for (int i = MAIN_PORT; i <= LAST_PORT; ++i)
            {    
                *sock = socket_settings(SERV_ID, i);
                
                if(*sock != -1)
                    break;
            }
            if(*sock == -1)
            {
                perror("All servers aren't acceptable");
                exit(2);
            }
        }
    }
}

void wait_opponent_sign(int *sock, char sign)
{
    bool begin_game = false;

    for (int i = 0; ; ++i)
    {
        try
        {
            if(i)
            {
                int start_game = 0;
                bool is_avl_sign = false;
     
                send(*sock, &start_game, sizeof(start_game), 0);
                send(*sock, &sign, sizeof(sign), 0);

                if(!recv(*sock, &is_avl_sign, sizeof(is_avl_sign), 0))
                    throw -1;
                if (CLIENT_CRASH_MSG == char(is_avl_sign))
                {
                    cout << DISCONNECTED << endl;
                    exit(0);
                }
            }

            if(!recv(*sock, &begin_game, sizeof(begin_game), 0))
                throw -1;
            if (CLIENT_CRASH_MSG == char(begin_game))
            {
                cout << DISCONNECTED << endl;
                exit(0);
            }

           break;
        }
        catch(int)
        { 
            for (int i = MAIN_PORT; i <= LAST_PORT; ++i)
            {             
                *sock = socket_settings(SERV_ID, i);
                
                if(*sock != -1)
                    break;
            }
            if(*sock == -1)
            {
                perror("All servers aren't acceptable");
                exit(2);
            }
        }
    }
}

int wait_opponent_move(int *sock, char sign)
{
    char winner = -1;

    int move = -1;

    for (int i = 0; ; ++i)
    {
        try
        {
            if(i)
            {
                int start_game = 1;

                send(*sock, &start_game, sizeof(start_game), 0);
                send(*sock, &sign, sizeof(sign), 0);
            }

            if(!recv(*sock, &winner, sizeof(winner), 0))
                throw -1;
            if (CLIENT_CRASH_MSG == char(winner))
            {
                cout << DISCONNECTED << endl;
                exit(0);
            }
            if(!recv(*sock, &move, sizeof(move), 0))
                throw -1;
            if (CLIENT_CRASH_MSG == char(move))
            {
                cout << DISCONNECTED << endl;
                exit(0);
            }
           
            
            if((move != 0) || (move!= -1))
                board[move] = (sign == 'X') ? 'O' : 'X';

            system("clear");
            
            cout << "~ " << "You are " << sign << " player" << " ~ " << endl << endl;
            print_game_board();

            if(winner != -1)
            {
                if(winner == 0)
                    cout << " ~ " << "Draw!" << " ~ " << endl;
                else
                    cout << " ~ " << winner << " Won!" << " ~ " << endl;
                
                move = 0;            
                
                if(!check_server(*sock))
                    throw -1;

                send(*sock, &move, sizeof(move), 0);
                    
                return 1;
            }

            break;
        }
        catch(int)
        { 
            for (int i = MAIN_PORT; i <= LAST_PORT; ++i)
            {               
                *sock = socket_settings(SERV_ID, i);
                
                if(*sock != -1)
                    break;
            }
            if(*sock == -1)
            {
                perror("All servers aren't acceptable");
                exit(2);
            }
        }
    }

    return 0;
}

void make_move(int *sock, char sign)
{
    int move = -1;

    bool is_val_1 = false;
    bool is_val_2 = false;
   
    for (int i = 0; ; ++i)
    {
        try
        {
            if(i)
            {
                int start_game = 1;

                send(*sock, &start_game, sizeof(start_game), 0);
                send(*sock, &sign, sizeof(sign), 0);
            }

            string msg = "Enter move number";
            do {
                system("clear");
                cout << " ~ " << "You are " << sign << " player" << " ~ " << endl << endl;
                print_game_board();
                cout << endl << " ~ " << msg << " ~ " << endl;           

                string buf;
                cout << "> " << flush;
                getline(cin, buf);
                move = atoi(buf.c_str());
            
                if(!check_server(*sock))
                    throw -1;

                send(*sock, &move, sizeof(move), 0);
            
                if(move == 0)
                {
                    msg = "Please pick a value between 1 and 9";
                    continue;
                }
            
                if(!recv(*sock, &is_val_1, sizeof(is_val_1), 0))
                    throw -1;

                if (CLIENT_CRASH_MSG == char(is_val_1))
                {
                    cout << DISCONNECTED << endl;
                    exit(0);
                }
                if(!recv(*sock, &is_val_2, sizeof(is_val_2), 0))
                    throw -1;
                if (CLIENT_CRASH_MSG == char(is_val_2))
                {
                    cout << DISCONNECTED << endl;
                    exit(0);
                }

                if(!is_val_1)
                    msg = "Please pick a value between 1 and 9";
                
                if(!is_val_2)
                    msg = "This move has already been done";
            }
            while(!is_val_1 || !is_val_2);

            board[move] = sign;

            break;
        }
        catch(int)
        { 
            for (int i = MAIN_PORT; i <= LAST_PORT; ++i)
            {            
                *sock = socket_settings(SERV_ID, i);
                
                if(*sock != -1)
                    break;
            }
            if(*sock == -1)
            {
                perror("All servers aren't acceptable");
                exit(2);
            }
        }
    }
}

void client_handler(int sock)
{
    char sign;
    chose_sign(&sock, &sign);

    cout << " ~ " << "Waiting for opponent..." << " ~ " << endl;
   
    wait_opponent_sign(&sock, sign);

    system("clear");

    while(1) 
    {
        system("clear");
        
        cout << "~ " << "You are " << sign << " player" << " ~ " << endl << endl;
        
        print_game_board();

        char winner = -1;
        
        recv(sock, &winner, sizeof(winner), 0);
        if (CLIENT_CRASH_MSG == char(winner))
        {
            cout << DISCONNECTED << endl;
            exit(0);
        }
        
        if(winner != -1)
        {
            if(winner == 0)
                cout << " ~ " << "Draw!" << " ~ " << endl;
            else
                cout << " ~ " << winner << " won!" << " ~ " << endl;
            break;
        }

        cout << " ~ " << "Waiting for opponent..." << " ~ " << endl;

        if(wait_opponent_move(&sock, sign) == 1)
            break;

        make_move(&sock, sign);
    }
}


void print_game_board(void) {
	cout << "     |     |     " << endl;
  	cout << "  " << board[1] << "  |  " << board[2] << "  |  " << board[3]
       << endl;
	
	cout << "_____|_____|_____" << endl;
	cout << "     |     |     " << endl;

	cout << "  " << board[4] << "  |  " << board[5] << "  |  " << board[6]
		<< endl;

	cout << "_____|_____|_____" << endl;
	cout << "     |     |     " << endl;

	cout << "  " << board[7] << "  |  " << board[8] << "  |  " << board[9]
		<< endl;

	cout << "     |     |     " << endl << endl;
}

int main(int argc, char const **argv) {
	int sock = socket_settings(SERV_ID, MAIN_PORT);
    client_handler(sock);
    close(sock);
	return 0;
}
