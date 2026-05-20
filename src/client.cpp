#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <cstring>
#include "protocol.hpp"
using namespace std;

void receive_messages(int  server_fd)
{   
    string incoming_msg;
    while (chat_protocol::receive_message(server_fd, incoming_msg)) {
        
      
        cout << "\n[Incoming]: " << incoming_msg << "\n> " << flush;
    }
    cout << "\n[System] Connection lost or server shut down.\n";
    close(server_fd);
    exit(0); 
}

int main() {
    cout << "--- Starting C++ Chat Client ---" << endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { cerr << "Failed to create socket\n"; return 1; }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); 

    if (connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Connection to server failed!\n";
        close(server_fd);
        return 1;
    }

    cout << "Connected successfully to the chatroom!" << endl;
    cout << "Type a message and hit Enter. Type 'exit' to quit.\n\n> " << flush;

    thread(receive_messages, server_fd).detach();

   
    string user_input;
    while (getline(cin, user_input)) {
        if (user_input == "exit") {
            break;
        }
        
        if (user_input.empty()) {
            cout << "> " << flush;
            continue;
        }

       
        if (!chat_protocol::send_message(server_fd, user_input)) {
            cerr << "Failed to send message packet.\n";
            break;
        }
        
        cout << "> " << flush; 
    }

    close(server_fd);
    cout << "Goodbye!" << endl;
    return 0;
}
