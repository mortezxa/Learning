#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/select.h>

using namespace std;

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8888;
const int BUFFER_SIZE = 1024;

int main() {

    int client;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    mkdir("client_files", 0777);
    
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        cerr << "Error creating client socket." << endl;
        return 1;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported." << endl;
        close(client);
        return 1;
    }

    if (connect(client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(client);
        return 1;
    }
    cout << "Connected to server " << SERVER_IP << ": " << SERVER_PORT << endl;
    cout << "Type your messages. Available commands: MSG, PM, USERS, LIST, PUT <file>, GET <file>, QUIT" << endl;
    cout << "[INFO] Put your files in 'client_files' to upload them." << endl;

    fd_set readfds;

    while(true){

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client, &readfds);
        int max_sd = client;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            cout << "Select error in client." << endl;
            break;
        }

        if (FD_ISSET(client, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0) {
                cout << "\nServer closed connection." << endl;
                break;
            }
            buffer[bytes_received] = '\0';
            string s_buffer(buffer);

            if (s_buffer.find("READY_PUT ") == 0) {
                stringstream ss(s_buffer);
                string dummy, filename;
                long long filesize;
                ss >> dummy >> filename >> filesize;

                cout << "Uploading '" << filename << "' (" << filesize << " bytes)..." << endl;
                
                ifstream infile("client_files/" + filename, ios::binary);
                long long sent_total = 0;
                char file_buf[4096];
                while (sent_total < filesize) {
                    infile.read(file_buf, sizeof(file_buf));
                    int bytes_read = infile.gcount();
                    send(client, file_buf, bytes_read, 0);
                    sent_total += bytes_read;
                }
                infile.close();
                cout << "Upload completed on client side." << endl;
            }

            else if (s_buffer.find("FILE_INFO ") == 0) {
                stringstream ss(s_buffer);
                string dummy, filename;
                long long filesize;
                ss >> dummy >> filename >> filesize;

                cout << "Downloading '" << filename << "' (" << filesize << " bytes)..." << endl;
                string ready_msg = "READY_GET";
                send(client, ready_msg.c_str(), ready_msg.size(), 0);

                ofstream outfile("client_files/" + filename, ios::binary);
                long long received_total = 0;
                char file_buf[4096];
                while (received_total < filesize) {
                    int bytes = recv(client, file_buf, sizeof(file_buf), 0);
                    if (bytes <= 0) break;
                    outfile.write(file_buf, bytes);
                    received_total += bytes;
                }
                outfile.close();
                cout << "Download complete!" << endl;
            }
            else {
                cout << "\n" << buffer;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            string cmd;
            getline(cin, cmd);

            if (cmd.empty()) {
                continue;
            }

            if (cmd.find("PUT ") == 0) {
                string filename = cmd.substr(4);
                ifstream infile("client_files/" + filename, ios::binary | ios::ate);
                if (!infile.is_open()) {
                    cout << "Error: File '" << filename << "' not found in 'client_files' folder." << endl;
                    continue; 
                }
                long long filesize = infile.tellg();
                infile.close();

                string put_req = "PUT " + filename + " " + to_string(filesize);
                send(client, put_req.c_str(), put_req.size(), 0);
                continue; 
            }

            send(client, cmd.c_str(), cmd.size(), 0);

            if (cmd == "QUIT") {
                cout << "Disconnecting..." << endl;
                break;
            }
        }        
    }

    close(client);
    cout << "Connection closed." << endl;    

    return 0;
}
