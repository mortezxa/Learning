#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>

using namespace std;

const int PORT = 8888;
const int BUFFER_SIZE = 1024;

struct ClientInfo {
    int socket;
    string username;
    string ip;
    string pending_get_file;
};

vector<ClientInfo> clients;

void remove_client(int socket) {
    clients.erase(
        remove_if(clients.begin(), clients.end(),
                  [&](const ClientInfo& c) { return c.socket == socket; }),
        clients.end()
    );
}

bool username_exists(const string& name){
    for (auto& c : clients)
        if (c.username == name) return true;
    return false;
}

int main(){

    int server;
    struct sockaddr_in server_addr,client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_sd;

    mkdir("server_files", 0777);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0){
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Error setting socket options");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind failed");
        return 1;
    }

    if(listen(server, 5) < 0){
        perror("listen failed");
        return 1;
    }

    cout << "Server is listening on port " << PORT << "..." << endl;
    cout << "[INFO] Files will be stored in 'server_files' directory." << endl;

    while (true) {

        FD_ZERO(&readfds);
        FD_SET(server, &readfds);
        max_sd = server;

        for (auto& c : clients) {
            FD_SET(c.socket, &readfds);
            if (c.socket > max_sd) {
                max_sd = c.socket;
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            cout << "select error" << endl;
        }
        
        if (FD_ISSET(server, &readfds)) {
            int new_socket = accept(server, (struct sockaddr *)&client_addr, &client_len);
            if (new_socket < 0) {
                perror("accept failed");
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
            cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << endl;

            string ask = "ENTER_USERNAME: Please type your username: \n";
            send(new_socket, ask.c_str(), ask.size(), 0);

            clients.push_back({new_socket, "", client_ip, ""});
        }        

        for (size_t i = 0; i < clients.size(); i++) {
            int sd = clients[i].socket;

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int bytes_received = recv(sd, buffer, BUFFER_SIZE - 1, 0);

                if (bytes_received <= 0) {
                    cout << "Client disconnected: " 
                         << (clients[i].username.empty() ? "Unknown" : clients[i].username) 
                         << endl;
                    close(sd);
                    remove_client(sd);
                    i--;
                } 
                else {
                    buffer[bytes_received] = '\0';
                    string msg(buffer);
                    
                    msg.erase(msg.find_last_not_of(" \n\r\t") + 1);

                    if (clients[i].username == "") {
                        if (username_exists(msg) || msg.empty()) {
                            string err = "Username taken or invalid. Try another:\n";
                            send(sd, err.c_str(), err.size(), 0);
                        } else {
                            clients[i].username = msg;
                            string welcome = "Welcome " + msg + "! You are now registered.\n";
                            send(sd, welcome.c_str(), welcome.size(), 0);
                            cout << "User registered: " << msg << endl;
                        }
                        continue;
                    }
                    cout << "Received from " << clients[i].username << ": " << msg << endl;

                    if (msg.rfind("MSG ", 0) == 0) {
                        string text = msg.substr(4);
                        string broadcast = "[Group] " + clients[i].username + ": " + text + "\n";
                        for (auto& c : clients) {
                            if (c.socket != sd && !c.username.empty()) {
                                send(c.socket, broadcast.c_str(), broadcast.size(), 0);
                            }
                        }
                        send(sd, "Message sent to group.\n", 23, 0);
                    }
                    
                    else if (msg.rfind("PM ", 0) == 0) {
                        stringstream ss(msg);
                        string cmd, target_user, text, word;
                        ss >> cmd >> target_user;
                        while(ss >> word) text += word + " ";
                        
                        bool found = false;
                        for (auto& c : clients) {
                            if (c.username == target_user) {
                                string pm = "[Private from " + clients[i].username + "]: " + text + "\n";
                                send(c.socket, pm.c_str(), pm.size(), 0);
                                found = true;
                                break;
                            }
                        }
                        if(found) send(sd, "Private message sent.\n", 22, 0);
                        else send(sd, "User not found.\n", 16, 0);
                    }

                    else if (msg == "USERS") {
                        string user_list = "Online Users:\n";
                        for (auto& c : clients) {
                            if(!c.username.empty())
                                user_list += "- " + c.username + "\n";
                        }
                        send(sd, user_list.c_str(), user_list.size(), 0);
                    }
                    
                    else if (msg == "QUIT") {
                        send(sd, "BYE: Disconnecting.\n", 20, 0);
                        close(sd);
                        remove_client(sd);
                        i--;
                    }

                    else if (msg == "LIST") {
                        DIR *dir; 
                        struct dirent *ent;
                        string list = "\n--- Files on Server ---\n";
                        if ((dir = opendir("server_files")) != NULL) {
                            while ((ent = readdir(dir)) != NULL) {
                                if (ent->d_name[0] != '.') list += string(ent->d_name) + "\n";
                            }
                            closedir(dir);
                        } else {
                            list += "Error reading directory.\n";
                        }
                        list += "-----------------------\n";
                        send(sd, list.c_str(), list.size(), 0);
                    }

                    else if (msg.find("PUT ") == 0) {
                        stringstream ss(msg);
                        string dummy, filename;
                        long long filesize;
                        ss >> dummy >> filename >> filesize;

                        string reply = "READY_PUT " + filename + " " + to_string(filesize);
                        send(sd, reply.c_str(), reply.size(), 0);

                        ofstream outfile("server_files/" + filename, ios::binary);
                        long long received_total = 0;
                        char file_buf[4096];

                        while (received_total < filesize) {
                            int bytes = recv(sd, file_buf, sizeof(file_buf), 0);
                            if (bytes <= 0) break;
                            outfile.write(file_buf, bytes);
                            received_total += bytes;
                        }
                        outfile.close();
                        string success = "Server: File '" + filename + "' uploaded successfully.\n";
                        send(sd, success.c_str(), success.size(), 0);
                    }



                    else if (msg.find("GET ") == 0) {
                        string filename = msg.substr(4);
                        ifstream infile("server_files/" + filename, ios::binary | ios::ate);
                        if (!infile.is_open()) {
                            string err = "Server: ERROR - File not found.\n";
                            send(sd, err.c_str(), err.size(), 0);
                            continue;
                        }
                        long long filesize = infile.tellg();
                        infile.close();

                        clients[i].pending_get_file = filename;
                        string info = "FILE_INFO " + filename + " " + to_string(filesize);
                        send(sd, info.c_str(), info.size(), 0);
                    }



                    else if (msg == "READY_GET") {
                        string filename = clients[i].pending_get_file;
                        ifstream infile("server_files/" + filename, ios::binary);

                        long long filesize;
                        infile.seekg(0, ios::end); 
                        filesize = infile.tellg(); 
                        infile.seekg(0, ios::beg);

                        long long sent_total = 0;
                        char file_buf[4096];
                        while (sent_total < filesize) {
                            infile.read(file_buf, sizeof(file_buf));
                            int bytes_read = infile.gcount();
                            send(sd, file_buf, bytes_read, 0);
                            sent_total += bytes_read;
                        }
                        infile.close();
                        clients[i].pending_get_file = ""; 
                    }

                    else {
                        send(sd, "UNKNOWN_CMD: Invalid command.\n", 30, 0);
                    }                    
                }
            }
        }
    }
    close(server);
    return 0;
}


