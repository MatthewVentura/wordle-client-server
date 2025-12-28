#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>
using namespace std;

/***********************************************************************/
/*                                                                     */
/* Function name:  trim_whitespace                                     */
/* Description:    Removes spaces, tabs, and newlines from the start   */
/*                 and end of a string.                                */
/* Parameters:     const string &str: original string to be cleaned –  */
/*                 input                                               */
/* Return Value:   string – the trimmed version of the original        */
/*                 string                                              */
/*                                                                     */
/***********************************************************************/
string trim_whitespace(const string &str);


/***********************************************************************/
/*                                                                     */
/* Function name:  validate_word                                       */
/* Description:    Checks if a word is exactly 5 alphabetic letters.   */
/* Parameters:     const string &word: word to be checked – input      */
/* Return Value:   bool – true if the word is 5 letters A–Z, false     */
/*                 otherwise                                           */
/*                                                                     */
/***********************************************************************/

bool validate_word(const string &word);

/***********************************************************************/
/*                                                                     */
/* Function name:  compare_guess                                       */
/* Description:    Compares a player’s guess to the target word and    */
/*                 builds a feedback pattern using '+', '?', and '_'.  */
/* Parameters:     const string &guess: player’s guessed word – input  */
/*                 const string &target: correct word – input          */
/* Return Value:   string – pattern showing correctness of each letter */
/*                                                                     */
/***********************************************************************/

string compare_guess(const string &guess, const string &target);

/***********************************************************************/
/*                                                                     */
/* Function name:  load_words                                          */
/* Description:    Reads words from a text file into a vector for the  */
/*                 word bank. Only keeps valid 5-letter words.         */
/* Parameters:     const string &path: name/path of the word file –    */
/*                 input                                               */
/*                 vector<string> &out_words: vector that will store   */
/*                 all valid words – output                            */
/* Return Value:   bool – true if at least one word was loaded, false  */
/*                 if the file could not be opened or had no valid     */
/*                 words                                               */
/*                                                                     */
/***********************************************************************/

bool   load_words(const string &path, vector<string> &out_words);

/***********************************************************************/
/*                                                                     */
/* Function name:  get_random_word                                     */
/* Description:    Picks and returns one random word from the word     */
/*                 vector.                                             */
/* Parameters:     const vector<string> &words: list of valid words –  */
/*                 input                                               */
/* Return Value:   string – a single randomly chosen word, or empty    */
/*                 string if the list is empty                         */
/*                                                                     */
/***********************************************************************/

string get_random_word(const vector<string> &words);

/***********************************************************************/
/*                                                                     */
/* Function name:  send_message                                        */
/* Description:    Sends a full line of text over a socket, ending     */
/*                 with a newline character.                           */
/* Parameters:     int sockfd: socket file descriptor – input          */
/*                 const string &data: message to send – input         */
/* Return Value:   bool – true if the whole message was sent           */
/*                 successfully, false on error                        */
/*                                                                     */
/***********************************************************************/

bool send_message(int sockfd, const string &data);

/***********************************************************************/
/*                                                                     */
/* Function name:  receive_message                                     */
/* Description:    Reads characters from a socket until a newline is   */
/*                 found and stores that line as the message.          */
/* Parameters:     int sockfd: socket file descriptor – input          */
/*                 string &out: string to store the received message – */
/*                 output                                              */
/* Return Value:   bool – true if a line was received, false on error  */
/*                 or closed connection                                */
/*                                                                     */
/***********************************************************************/

bool receive_message(int sockfd, string &out);

/***********************************************************************/
/*                                                                     */
/* Function name:  log_event                                           */
/* Description:    Writes a message to a log file with a timestamp at  */
/*                 the front.                                          */
/* Parameters:     const string &msg: message to be written to the     */
/*                 log – input                                         */
/* Return Value:   void – no return value                              */
/*                                                                     */
/***********************************************************************/

void log_event(const string &msg);

/***********************************************************************/
/*                                                                     */
/* Function name:  error_exit                                          */
/* Description:    Prints an error message and exits the program in a  */
/*                 controlled way.                                     */
/* Parameters:     const string &msg: message to display before exit – */
/*                 input                                               */
/* Return Value:   void – this function does not return (program       */
/*                 terminates)                                         */
/*                                                                     */
/***********************************************************************/

void error_exit(const string &msg);

/***********************************************************************/
/*                                                                     */
/* Function name:  close_connection                                    */
/* Description:    Closes a socket if it is a valid file descriptor.   */
/* Parameters:     int sockfd: socket file descriptor to close – input */
/* Return Value:   void – no return value                              */
/*                                                                     */
/***********************************************************************/

void close_connection(int sockfd);

#endif
