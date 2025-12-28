/***********************************************************************
 * File:       client.cpp
 * Created on: 11-15-2025
 * Due Date:   11-24-2025
 * Author:     Mohamed Abdelgawad
 * Major:      Computer Science
 *
 * Professor:  Prof. Walther
 * Course:     CPSC 328
 * Assignment: Group 4 Project Application: Wordle
 * Filename:   client.cpp
 *
 * Compile:    g++ client.cpp library.cpp -o client
 * Run:        ./client localhost [port]	//Defaul port is 5000
 *
 * Purpose:    Client Implementation
 ***********************************************************************/


#include "library.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>      
#include <netdb.h>      
#include <arpa/inet.h>  
#include <sys/socket.h> 
#include <unistd.h>     

using namespace std;

//default port used when user doesn't specify one
const int DEFAULT_PORT = 5000;
const int MAX_ATTEMPTS = 6;





/**********************************************************************
 * Function: to_lower_str
 * Purpose:  Convert all characters in a string to lowercase.
 *
 * Parameters:
 *   s - const string&; the input string to convert.
 *
 * Returns:
 *   A new string containing the lowercase version of s.
 *
 **********************************************************************/
static string to_lower_str(const string &s) {
    string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = (char)tolower(out[i]);
    }
    return out;
}





/**********************************************************************
 * Function: connect_to_server
 * Purpose:  Resolve the server hostname and establish a TCP connection
 *           to the given host and port.
 *
 * Parameters:
 *   host - const string&; server hostname or address.
 *   port - int; TCP port number.
 *
 * Returns:
 *   int - a connected socket file descriptor on success,
 *         or -1 if not.
 *
 **********************************************************************/
int connect_to_server(const string &host, int port) {
    struct addrinfo hints;
    struct addrinfo *res = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;       
    hints.ai_socktype = SOCK_STREAM;   

    string portStr = to_string(port);
    int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (status != 0) {
        cerr << "getaddrinfo error: " << gai_strerror(status) << endl;
        return -1;
    }

    int sockfd = -1;

    //try each result until one connects
    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;   //try next
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            sockfd = -1;
            continue;   //try next
        }

        //success
        break;
    }

    freeaddrinfo(res);
    return sockfd;
}





/**********************************************************************
 * Function: play_round
 * Purpose:  Play a single round with the server.
 *           Sends READY, receives the secret word, interacts with the
 *           user for up to MAX_ATTEMPTS guesses, and displays feedback.
 *
 * Parameters:
 *   sockfd - int; connected socket descriptor to the server.
 *
 * Returns:
 *   bool - true if the user chooses to play another round,
 *          false if the user chooses to quit or an error occurs.
 *
 **********************************************************************/
bool play_round(int sockfd) {
	//ANSI color codes
    const string GREEN  = "\033[42m\033[97m";
    const string YELLOW = "\033[43m\033[97m";
    const string GRAY   = "\033[100m\033[97m";
    const string RESET  = "\033[0m";
	
    //ask server for a new word
    if (!send_message(sockfd, "READY")) {
        cout << "Failed to send READY to server.\n";
        return false;
    }

    //receive secret word from server
    string secret;
    if (!receive_message(sockfd, secret)) {
        cout << "Failed to receive word from server.\n";
        return false;
    }

    secret = trim_whitespace(secret);
    secret = to_lower_str(secret);

    if (!validate_word(secret)) {
        cout << "Server sent invalid word: '" << secret << "'.\n";
        return false;
    }

    log_event("Client: received secret word from server.");

    cout << "\n=== New Round Started ===\n";
    cout << "Guess the 5-letter word. You have "
         << MAX_ATTEMPTS << " attempts.\n";	 
    cout << "------------------------------------------------------\n";
    cout << "Key: " << GREEN  << " A " << RESET << " = Correct Spot   "
					<< YELLOW << " B " << RESET << " = Wrong Spot   "
					<< GRAY   << " C " << RESET << " = Not in Word\n";
    cout << "------------------------------------------------------\n";

    bool won     = false;
    int attempts = 0;

    while (attempts < MAX_ATTEMPTS && !won) {
        string guess;

        //get a valid guess
        while (true) {
            cout << "\nEnter guess #" << (attempts + 1) << ": ";
            if (!getline(cin, guess)) {
                cout << "\nInput closed. Ending game.\n";
                return false;
            }

            guess = trim_whitespace(guess);
            guess = to_lower_str(guess);

            if (!validate_word(guess)) {
                cout << "Invalid guess. Please enter exactly "
                     << "5 alphabetic letters.\n";
                continue;
            }
            break;  //valid
        }

        attempts++;

        //use shared library to compare guess and word
        string pattern = compare_guess(guess, secret);

        cout << "\n   "; 
        for (int i = 0; i < 5; i++) {
            if (pattern[i] == '+') {
                //correct Spot (Green)
                cout << GREEN << " " << (char)toupper(guess[i]) << " " << RESET;
            } else if (pattern[i] == '?') {
                //wrong Spot (Yellow)
                cout << YELLOW << " " << (char)toupper(guess[i]) << " " << RESET;
            } else {
                //not in Word (Gray)
                cout << GRAY << " " << (char)toupper(guess[i]) << " " << RESET;
            }
        }
        cout << "\n\n";

        if (guess == secret) {
            won = true;
        } else {
            cout << "Attempts remaining: "
                 << (MAX_ATTEMPTS - attempts) << "\n";
        }
    }

    if (won) {
        cout << "\n*** Correct! You guessed the word in "
             << attempts << " tr"
             << (attempts == 1 ? "y" : "ies") << ". ***\n";
    } else {
        cout << "\n*** You ran out of tries. The correct word was '"
             << secret << "'. ***\n";
    }

    //ask if the user wants to play again
    while (true) {
        cout << "\nPlay again? (y/n): ";
        string resp;
        if (!getline(cin, resp)) {
            cout << "\nInput closed. Ending game.\n";
            return false;
        }

        resp = trim_whitespace(resp);
        if (resp.empty()) continue;

        char c = (char)tolower(resp[0]);
        if (c == 'y') return true;   //play another round
        if (c == 'n') return false;  //stop

        cout << "Please enter 'y' or 'n'.\n";
    }
}





int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        cout << "Usage: " << argv[0]
             << " <server-hostname> [port]\n";
        return 1;
    }

    string host = argv[1];
    int port    = DEFAULT_PORT;

    if (argc == 3) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            cout << "Invalid port number.\n";
            return 1;
        }
    }

    cout << "Connecting to " << host
         << " on port " << port << "...\n";

    int sockfd = connect_to_server(host, port);
    if (sockfd == -1) {
        cout << "Could not connect to server.\n";
        return 1;
    }

    log_event("Client connected to server.");

    //expect HELLO from server
    string msg;
    if (!receive_message(sockfd, msg)) {
        cout << "Failed to receive HELLO from server.\n";
        close_connection(sockfd);
        return 1;
    }

    msg = trim_whitespace(msg);
    if (msg != "HELLO") {
        cout << "Unexpected greeting from server: '"
             << msg << "'.\n";
        close_connection(sockfd);
        return 1;
    }

    cout << "Server says: " << msg << "\n";

    //Game loop: play rounds until user quits
    bool playMore = true;
    while (playMore) {
        playMore = play_round(sockfd);
    }

    //send BYE before closing connection
    if (!send_message(sockfd, "BYE")) {
        cout << "Warning: failed to send BYE to server.\n";
    }

    log_event("Client disconnecting from server.");
    close_connection(sockfd);

    cout << "\nThanks for playing! Goodbye.\n";
    return 0;
}
