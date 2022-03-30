#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

using namespace std;

#define MAIN_PORT 3425
#define SERV_ID "127.0.0.1"

extern const char CLIENT_CRASH_MSG;

typedef struct sockets
{
    vector<int> *clients;
    vector<int> *opt_servs;
} sockets;

int server_socket_settings(char const *id, uint16_t port);
int client_socket_settings(char const *id, uint16_t port);
void listen_current_server(int sock);
void *opt_server_handler(void *socks);
void *main_server_handler(void *socks);
bool avalible_cell_validate(int move);
bool border_validate(int number);
char game_over_validate(void);
bool check_connection(int sock, int sock1);
bool check_listening_server(int sock);
void send_msg_to_listening_server(vector<int> *new_opt_servs, int move, char sign, int flag);



const char CLIENT_CRASH_MSG = char(0x80);
char board[10];

vector<char> signs;
set<int> dead_socks;

void init_game_field()
{
    for(int i = 1; i < 10; ++i)
        board[i] = ' ';
}

bool validate_sign(char sign) {
    bool result = true;
    if (sign != 'X' && sign != 'O')
        result = false;
//    else if (!signs.size() % 2)
//        result = false;
    else if (find(signs.begin(), signs.end(), sign) != signs.end())
        result = false;
    return result;
}

int server_socket_settings(char const *id, uint16_t port) { //подключение сервера к сокету
    int listener;

    struct sockaddr_in addr;
    struct hostent *hp;
    
    listener = socket(AF_INET, SOCK_STREAM, 0);

    if(listener < 0) {
        perror("socket");

        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    hp = gethostbyname(id);
    
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    int optval = 1;
    if ((setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int))) == -1) {
            close(listener);
            perror("ERROR  SWM : Set Socket ReUSED ERROR \n");
            return NULL;
    }
    if(::bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 2);
    return listener;
}

int client_socket_settings(char const *id, uint16_t port) {
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

bool check_connection(int sock, int sock1 = -1)
{
    int status = 1;

    if(!recv(sock, &status, sizeof(status), 0))
    {
        if(sock1 != -1)
        {
            send(sock1, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
            
            return false;
        }
        else
            return false;
    }
    send(sock, &status, sizeof(status), 0);
    return true;
}

bool check_listening_server(int sock)
{
    int status;

    send(sock, &status, sizeof(status), 0);

    int bytes_read = recv(sock, &status, sizeof(status), 0);

    if(!bytes_read)
    {
        dead_socks.emplace(sock);
        return false;
    }
    
    return true;
}

void listen_current_server(int sock)
{
    int move;

    char sign;

    while(1)
    {
        if(!check_connection(sock))
            break;

        recv(sock, &move, sizeof(move), 0);

        if(move == 0)
        {
            init_game_field();
            continue;
        }

        recv(sock, &sign, sizeof(sign), 0);
        board[move] = sign;
    }
}


void *opt_server_handler(void *socks)
{
    char sign;

    int move = -1;
    int start_game = -1;

    int current_player = 'X';

    sockets new_socks = *(sockets *)socks;

    vector<int> new_clients = *new_socks.clients;
    vector<int> new_opt_servs;

    if(new_socks.opt_servs != nullptr)
        new_opt_servs = *new_socks.opt_servs;

    int curr_sock = new_clients[new_clients.size() - 1];

    int other_sock;

    bool is_avl_sign = false;

    if(new_clients.size() % 2)
        other_sock = curr_sock + 1;
    else if(!(new_clients.size() % 2))
        other_sock = curr_sock - 1;

    if(!recv(curr_sock, &start_game, sizeof(start_game), 0))
    {
        send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
        goto opt_serv_fin;
    }

    if(start_game)
    {
        if(!recv(curr_sock, &sign, sizeof(sign), 0))
        {
            send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
            goto opt_serv_fin;
        }
    }

    if(!start_game)
    {
        do
        {
            if(!recv(curr_sock, &sign, sizeof(sign), 0))
            {
                send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
                goto opt_serv_fin;
            }
            
            is_avl_sign = validate_sign(sign);
            if (is_avl_sign) {
                signs.push_back(sign);
            }
            send(curr_sock, &is_avl_sign, sizeof(is_avl_sign), 0);
            
        } while(!is_avl_sign);

        while(signs.size() % 2) {
            sleep(1);
        }

        bool begin_game = true;
        send(curr_sock, &begin_game, sizeof(begin_game), 0);

        char not_over = -1;
        send(curr_sock, &not_over, sizeof(not_over), 0);
        
        if(sign == current_player)
        {
            char winner = -1;
            send(curr_sock, &winner, sizeof(winner), 0);
            send(curr_sock, &move, sizeof(move), 0);
        }
    }

    while(1)
    {
        bool is_val_1 = false;
        bool is_val_2 = false;
        
        do
        {
            if(!check_connection(curr_sock, other_sock))
                goto opt_serv_fin;

            if(!recv(curr_sock, &move, sizeof(move), 0))
            {
                send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
                goto opt_serv_fin;
            }
             
            if(move == 0)
                goto opt_serv_fin;
            
            is_val_1 = border_validate(move);
            is_val_2 = is_val_1 ? avalible_cell_validate(move) : true;

            send(curr_sock, &is_val_1, sizeof(is_val_1), 0);
            send(curr_sock, &is_val_2, sizeof(is_val_2), 0);
        }
        while(!is_val_1 && !is_val_2);

        board[move] = sign;

        char winner = game_over_validate();
  
        send(curr_sock, &winner, sizeof(winner), 0);
        send(other_sock, &winner, sizeof(winner), 0);
        send(other_sock, &move, sizeof(move), 0);
         
        if(new_socks.opt_servs != nullptr)
            send_msg_to_listening_server(&new_opt_servs, move, sign, 0);

        if(winner != -1)
            break;
    }

opt_serv_fin:
    send_msg_to_listening_server(&new_opt_servs, move, sign, -1);
    
    init_game_field();
    
    return 0;
}



void *main_server_handler(void *socks)
{
    char sign;

    int current_player = 'X';
    int move = -1;

    char not_over = -1;

    bool begin_game = true;
    bool is_avl_sign = false;
 

    sockets new_socks = *(sockets *)socks;

    vector<int> new_clients = *new_socks.clients;
    vector<int> new_opt_servs = *new_socks.opt_servs;

    int curr_sock = new_clients[new_clients.size() - 1];

    int other_sock;

    if(new_clients.size() % 2)
        other_sock = curr_sock + 1;
    else if(!(new_clients.size() % 2))
        other_sock = curr_sock - 1;
    
    do {
        if(!recv(curr_sock, &sign, sizeof(sign), 0)) {
            send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
            goto main_serv_fin;
        }
        
        is_avl_sign = validate_sign(sign);
        if (is_avl_sign) {
            signs.push_back(sign);
        }
        send(curr_sock, &is_avl_sign, sizeof(is_avl_sign), 0);
    } while(!is_avl_sign);

    while(signs.size() % 2) {
        sleep(1);
    }

    send(curr_sock, &begin_game, sizeof(begin_game), 0);
    send(curr_sock, &not_over, sizeof(not_over), 0);
    
    if(sign == current_player) {
        char winner = -1;
        
        send(curr_sock, &winner, sizeof(winner), 0);
        send(curr_sock, &move, sizeof(move), 0);
    }

    while(1)
    {
        bool is_val_1 = false;
        bool is_val_2 = false;
        do {
            if(!check_connection(curr_sock, other_sock))
                goto main_serv_fin;
             
            if(!recv(curr_sock, &move, sizeof(move), 0))
            {
                send(other_sock, &CLIENT_CRASH_MSG, sizeof(CLIENT_CRASH_MSG), 0);
                goto main_serv_fin;
            }
        
            if(move == 0)
                goto main_serv_fin;
            
            is_val_1 = border_validate(move); //проверка корректности введенного номера клетки
            is_val_2 = is_val_1 && avalible_cell_validate(move); //проверка доступности клетки (может поменять на фалс)

            send(curr_sock, &is_val_1, sizeof(is_val_1), 0);
            send(curr_sock, &is_val_2, sizeof(is_val_2), 0);
        } while(!is_val_1 && !is_val_2);

        board[move] = sign;

        char winner = game_over_validate();
     
        send(curr_sock, &winner, sizeof(winner), 0);
        send(other_sock, &winner, sizeof(winner), 0);
        send(other_sock, &move, sizeof(move), 0); // oao

        send_msg_to_listening_server(&new_opt_servs, move, sign, 0);

        if(winner != -1)
            break;
    }

main_serv_fin:
    send_msg_to_listening_server(&new_opt_servs, move, sign, -1);
    
    init_game_field();
    
    return 0;
}

void send_msg_to_listening_server(vector<int> *new_opt_servs, int move, char sign, int flag)
{
    for (int i : *new_opt_servs)
    {
        if(dead_socks.find(i) == dead_socks.end()) // не найден
        {
            if(check_listening_server(i))
            {
                if(!flag)
                {
                    send(i, &move, sizeof(move), 0);
                    send(i, &sign, sizeof(sign), 0);
                }
                else
                    send(i, &flag, sizeof(flag), 0);
            }
        }
    }
}

bool avalible_cell_validate(int move)
{
    if(board[move] == 'X' || board[move] == 'O')
        return false;
    return true;
}

bool border_validate(int number)
{
    if(number >= 1 && number <= 9)
        return true;
    else
        return false;
}

char game_over_validate()
{
    if(board[1] == board[2] && board[1] == board[3] && board[1] != ' ' && !isdigit(board[1]))
        return board[1];
    else if(board[1] == board[5] && board[1] == board[9] && board[1] != ' ' && !isdigit(board[1]))
        return board[1];
    else if(board[1] == board[4] && board[1] == board[7] && board[1] != ' ' && !isdigit(board[1]))
        return board[1];
    else if(board[4] == board[5] && board[4] == board[6] && board[4] != ' ' && !isdigit(board[4]))
        return board[4];
    else if(board[2] == board[5] && board[2] == board[8] && board[2] != ' ' && !isdigit(board[2]))
        return board[2];
    else if(board[3] == board[6] && board[3] == board[9] && board[3] != ' ' && !isdigit(board[3]))
        return board[3];
    else if(board[7] == board[8] && board[7] == board[9] && board[7] != ' ' && !isdigit(board[7]))
        return board[7];
    else if(board[3] == board[5] && board[3] == board[7] && board[3] != ' ' && !isdigit(board[3]))
        return board[3];

    for(int i = 1; i < 10; ++i) {
      if(board[i] == ' ')
          return -1;
    }
    return 0;
}

#include <signal.h>

int main(int argc, char const **argv) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
    
    int listener, sock, sock_1;

    vector<int> clients;
    vector<int> opt_servs;

    sockets socks;
    init_game_field();
    pthread_t pid;

    switch(argc) {
        case 1:
            listener = server_socket_settings(SERV_ID, MAIN_PORT);

            while(opt_servs.size() != 2) {
                sock = accept(listener, NULL, NULL);

                if(sock < 0) {
                    perror("accept");
                    exit(1);
                }

                opt_servs.push_back(sock); //подключаем два сервера
            }
            for(int i = 0; i < 2; i++) {  //поменять на два клиента
                sock = accept(listener, NULL, NULL);
               
                if(sock < 0) {
                    perror("accept");
                    exit(1);
                }

                clients.push_back(sock);
                
                socks.clients = &clients;
                socks.opt_servs = &opt_servs;

                if(pthread_create(&pid, NULL, main_server_handler, (void *)&socks) < 0)
                {
                    perror("could not create thread");

                    exit(2);
                }

                pthread_detach(pid);
            }
            while (1){
                sleep(1);
            }
        default:
            string param = argv[1];

            if (param == "--opt_serv_1")
            {
                listener = server_socket_settings(SERV_ID, MAIN_PORT + 1);
                
                sock = client_socket_settings(SERV_ID, MAIN_PORT);
          
                while(opt_servs.size() != 1)
                {
                   sock_1 = accept(listener, NULL, NULL);
                   
                    if(sock_1 < 0)
                    {
                        perror("accept");
                        exit(1);
                    }

                    opt_servs.push_back(sock_1);
                }

                if(sock != -1)
                    listen_current_server(sock);

                while(1)
                {
                    sock = accept(listener, NULL, NULL);
                   
                    if(sock < 0)
                    {
                        perror("accept");
                        exit(1);
                    }

                    clients.push_back(sock);

                    socks.clients = &clients;
                    socks.opt_servs = &opt_servs;
            
                    if(pthread_create(&pid, NULL, opt_server_handler, (void *)&socks) < 0)
                    {
                        perror("could not create thread");

                        exit(2);
                    }

                    pthread_detach(pid);
                }
            }
            else if (param == "--opt_serv_2")
            {
                listener = server_socket_settings(SERV_ID, MAIN_PORT + 2);
                
                sock = client_socket_settings(SERV_ID, MAIN_PORT);
                sock_1 = client_socket_settings(SERV_ID, MAIN_PORT + 1);
                
                if (sock != -1)
                    listen_current_server(sock);
                
                if (sock_1 != -1)
                    listen_current_server(sock_1);

                while(1)
                {
                    sock = accept(listener, NULL, NULL);
                   
                    if(sock < 0)
                    {
                        perror("accept");
                        exit(1);
                    }

                    clients.push_back(sock);
                      
                    socks.clients = &clients;
                    socks.opt_servs = nullptr;

                    if(pthread_create(&pid, NULL, opt_server_handler, (void *)&socks) < 0)
                    {
                        perror("could not create thread");

                        exit(2);
                    }

                    pthread_detach(pid);
                }
            }
    }

    return 0;
}
