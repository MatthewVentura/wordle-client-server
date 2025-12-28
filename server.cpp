/***********************************************************************
 * File:       server.cpp
 * Created on: 11-15-2025
 * Due Date:   11-17-2025
 * Author:     Mohamed Abdelgawad
 * Major:      Computer Science
 *
 * Professor:  Prof. Walther
 * Course:     CPSC 328
 * Assignment: Group 4 Project Application: Wordle
 * Filename:   server.cpp
 *
 * Compile:    g++ server.cpp library.cpp -o server
 * Run:        ./server [port]		//Defaul port is 5000
 *
 * Purpose:    Server Implementation
 ***********************************************************************/


#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>      
#include <cstring>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>   
#include <errno.h>      

#include "library.h"

using namespace std;

const int DEFAULT_PORT = 5000;
const string WORD_FILE = "words.txt";


/**********************************************************************
 * Function: sigchld_handler
 * Purpose:  handler to prevent zombie processes when children exit.
 *
 * Parameters:
 *   s - int; the signal number
 *
 * Returns:
 *   NULL
 *
 **********************************************************************/
void sigchld_handler(int s) {
    (void)s; //quiet unused variable warning
    int saved_errno = errno;
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        //reap all finished children
    }
    errno = saved_errno;
}



/**********************************************************************
 * Function: handle_client
 * Purpose:  Handle all communication with client:
 *           send "HELLO", respond to "READY"/"WORD" with a random word,
 *           and close the connection on "BYE"/"QUIT".
 *
 * Parameters:
 *   client_sock - int; the connected client socket file descriptor.
 *   word_bank   - const vector<string>&; list of valid words the server
 *                 can send to the client.
 *
 * Returns:
 *   NULL. function runs until the client disconnects or sends
 *   "BYE"/"QUIT" then closes the client socket.
 *
 **********************************************************************/
void handle_client(int client_sock, const vector<string>& word_bank) {
    //send HELLO upon connection
    if (!send_message(client_sock, "HELLO")) {
        cout << "Error sending HELLO to client." << endl;
        close_connection(client_sock);
        return;
    }

    bool running = true;
    while (running) {
        string msg;

        //wait for a message from the client
        if (!receive_message(client_sock, msg)) {
            cout << "Client disconnected or error receiving." << endl;
            break;
        }

        msg = trim_whitespace(msg);

        if (msg == "READY" || msg == "WORD") {
            //select random word and send it
            string random_word = get_random_word(word_bank);
            if (!send_message(client_sock, random_word)) {
                cout << "Error sending word to client." << endl;
                break;
            }
            cout << "Sent word '" << random_word << "' to client." << endl;
        }
        else if (msg == "BYE" || msg == "QUIT") {
            //handle BYE/QUIT
            cout << "Client sent BYE/QUIT. Closing session." << endl;
            running = false;
        }
        else {
            cout << "Unknown message from client: '" << msg << "'" << endl;
        }
    }

    close_connection(client_sock);
}

int main(int argc, char *argv[]) {
    //load word bank
    vector<string> word_bank;
    if (!load_words(WORD_FILE, word_bank)) {
        cerr << "Error: Could not load words from " << WORD_FILE << endl;
        return 1;
    }
    if (word_bank.empty()) {
        cerr << "Error: Word bank is empty." << endl;
        return 1;
    }
    cout << "Loaded " << word_bank.size() << " words." << endl;

    //port
    int port = DEFAULT_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            cerr << "Invalid port. Using default " << DEFAULT_PORT << endl;
            port = DEFAULT_PORT;
        }
    }

    //create listening socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    //allow port reuse
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("setsockopt");
        
    }

    //bind
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    //listen
    if (listen(sockfd, 10) < 0) {
        perror("Listen failed");
        close(sockfd);
        return 1;
    }

    //reaper for dead child processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // Reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        perror("sigaction");
        close(sockfd);
        return 1;
    }

    cout << "Server listening on port " << port << "..." << endl;

    //accept loop
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t sin_size = sizeof(client_addr);
        int new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);

        if (new_fd == -1) {
            perror("Accept failed");
            continue;
        }

        cout << "Received connection from "
             << inet_ntoa(client_addr.sin_addr) << endl;

        //fork for concurrency
        pid_t pid = fork();
        if (pid == 0) {
            //child process
            close(sockfd); //child doesn't need the listening socket
            handle_client(new_fd, word_bank);
            exit(0);
        }

        //parent process
        close(new_fd); //parent doesn't use this client socket
    }

    close(sockfd);
    return 0;
}
