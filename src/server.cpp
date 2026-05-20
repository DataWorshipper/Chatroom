#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include "protocol.hpp"
using  namespace std;

vector<int>active_clients;
mutex clients_mutex;
void handle_client(int client_fd)
{
    string incoming_msg;
    while(chat_protocol::receive_message(client_fd,incoming_msg))
    {
        cout<<"Data Received"<<client_fd<<":"<<incoming_msg<<endl;
        clients_mutex.lock();
        for(int other_client_fd:active_clients)
        {
            if(other_client_fd!=client_fd)
            {
                if(!(chat_protocol::send_message(other_client_fd,incoming_msg)))
                cerr<<"Failed to broadcast message to client"<<other_client_fd<<endl;
            }
        }
        clients_mutex.unlock();
    }
    cout << "Client " << client_fd << " disconnected." << endl;
    clients_mutex.lock();
    active_clients.erase(remove(active_clients.begin(), active_clients.end(), client_fd), active_clients.end());
    clients_mutex.unlock();
    
    close(client_fd);

}
int main(int argc,char* argv[])
{

    int backlog_size = 100; 
    if (argc > 1) {
        backlog_size = atoi(argv[1]);
    }
    cout<<"Starting server"<<endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(server_fd==-1)
    {
        cerr<<"Failed to create socket";
        close(server_fd);
        return 1;
    }
    
    cout<<"socket created"<<server_fd<<endl;
    
    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8080);
    server_addr.sin_addr.s_addr=INADDR_ANY;
    int bind_return=bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(bind_return==-1)
    {
        cerr<<"Failed to bind to port 8080";
        close(server_fd);
        return 1;
    }
    int listen_ret_value=listen(server_fd,backlog_size);
    if(listen_ret_value==-1)
    {
        cerr<<"Failed to listen to any incoming connections";
        close(server_fd);
        return 1;

    }
    else
    cout<<"Listening  on port 8080"<<endl;
    while(true)
    {
    sockaddr_in client_addr;
    socklen_t client_size=sizeof(client_addr);
    int client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&client_size);
    if(client_fd==-1)
    {
        cerr<<"Not accepted the request";
        continue;
    }
    cout<<"Connected to server successfully";

    clients_mutex.lock();
    active_clients.push_back(client_fd);
    clients_mutex.unlock();
    thread(handle_client,client_fd).detach();
    
}
close(server_fd);


    return 0;

}



