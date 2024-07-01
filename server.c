#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_LEN 100

/**
 * The error function prints out a given message and then closes
 * the program
 *
 * @param msg A given message to print
 */
void error(const char *msg) 
{
    printf("Error: %s\n", msg);
    exit(1);
}

/**
 * Get and return the port number
 * 
 * @param argv The given arguments when the .exe file is run 
 * @return The parsed port number
 */
int get_port_num (char *argv[]) 
{
    int num;
    // parse the port number from a string to an int
    if (sscanf(argv[1], "%d", &num) != 1) 
        error("Failed to obtain port number");
    
    if (num < 1024) // port number cannot be less then 1024
        return -1; 

    return num;
}

/**
 * Create a TCP socket
 *
 * @return the file descriptor of the socket
 */
int create_socket() 
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // check for socket creation failure
    if (fd == -1) 
        error("Unable to create socket"); 

    printf("Socket was created!\n"); 
    return fd;
}

/**
 * Create a socket address with the given port number
 *
 * @param port_num The port number
 * @return a sockaddr_in stucture that represents the address
 */
struct sockaddr_in create_address(int port_num) 
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num); // port number
    addr.sin_addr.s_addr = INADDR_ANY; // any address	

    printf("Address created\n");
    return addr;
}

/**
 * Binds the address to the TCP socket
 *
 * @param addr the previously created socket address
 * @param fd the file descripter
 */
void bind_address(int fd, struct sockaddr_in addr) 
{
    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    // check for binding failure
    if (ret < 0) 
        error("Failed to bind address to socket");

    printf("Binding was successful\n");
}

/**
 * Listens for incoming connections with the socket
 *
 * @param fd the file descriptor of the socket
 */
void listen_for_incoming_connection(int fd) 
{
    if (listen(fd, SOMAXCONN) < 0)
        error("Failed when listening for connections");

    printf("Listening successful\n");
}

/**
 * Accepts the clients connection to the socket
 *
 * @param fd The file descriptor for the socket 
 * @return The file descriptor of the client's accepted connection
 */
int accept_client (int fd) 
{
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    // accepct the incoming cliet connection
    int client_fd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);

    // check for acception failure
    if(client_fd < 0) 
	    error("Failed to accept client");
    
    printf("Successfully accepted client\n");
    return client_fd;
}

/**
 * Sends a given message to the clients display
 *
 * @param client_fd The client's file descriptor
 * @param msg The specified messaged to send
 */
void send_to_client(int client_fd, const char *msg) 
{
    // send the message
    ssize_t bytes_sent = send(client_fd, msg, strlen(msg), 0);

    // check for failure
    if(bytes_sent < 0) {
        error("Failed to send message to client");
    }
}

/**
 * Recieve a message that was sent from the client
 *
 * @param client_fd The client's file descpritor
 * @param buffer The pointer to the buffer to contain the recieved message
 * @return The number of bytes recieved, -1 if failure occurred
 */
ssize_t receive_client_message(int client_fd, char *buffer) 
{
    // recieve the clients message and save it to the buffer
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_LEN, 0);

    // check for recieving failure
    if(bytes_received < 0) {
	    error("Failed to recieve message");
    }

    return bytes_received;
}

/**
 * Check to match the first word of any sentence to another given word
 *
 * @param str the given string
 * @param target the string to match
 * @return 0 if strings match, otherwise it is false
 */
int strings_match(char *str, char *target) {
    char str_cpy[BUFFER_LEN];

    int i;
    // Find the first word in input
    for (i = 0; str[i] != ' ' && str[i] != '\n'; i++)
        str_cpy[i] = tolower(str[i]);

    str_cpy[i] = '\0'; // terminate rest of string
    return strcmp(str_cpy, target); // Compare the first words
}

/**
 * Gets the filename and opens the file in the appropriate mode. 
 * Had to include multiple error message parameters to accurately follow given guildlines
 *
 * @param buffer The buffer containing the filename
 * @param client_fd The client's file descriptor
 * @param mode The given mode for opening the file
 * @param err_msg2 The first error message
 * @param err_msg2 The second error message
 * @return The opened file
 */
FILE *open_file(char buffer[], int client_fd, char mode, char *err_msg1, char *err_msg2) {
    char *filename = strchr(buffer, ' '); // get position of whitespace char
    
    // no whitespace = no filename
    if (filename == NULL) {
        send_to_client(client_fd, err_msg1);
        return NULL; // return a null file
    }

    filename++; // move filename pointer past the space
    filename[strcspn(filename, "\n")] = '\0'; // remove newline character from filename and replace with a null pointer
    FILE *file = fopen(filename, &mode); // open file

    // send error if the file isn't avalible
    if (file == NULL) 
        send_to_client(client_fd, err_msg2);

    return file;
}

/**
 * Sends the specified file contents to the client
 *
 * @param client_fd The clients file descriptor
 * @param buffer The buffer containing the filename
 */
void get_file(int client_fd, char buffer[])
{
    // handle GET request
    FILE *file = open_file(buffer, client_fd, 'r', "SERVER 500 Get Error\n", "SERVER 404 Not Found\n");
    
    // file couldn't open
    if (file == NULL)
        return; // go back to accepting new connections
    
    // file opened correctly
    send_to_client(client_fd, "SERVER 200 OK\n\n");

    // send file contents to client character by character
    int c;
    while ((c = fgetc(file)) != EOF) {
        send_to_client(client_fd, (char *)&c);
    }
    send_to_client(client_fd, "\n\n\n");

    fclose(file);
}

/**
 * Write the data that is recieved from the client into a specified file
 *
 * @param client_fd The clients file descriptor
 * @param buffer The buffer containing the filename
 */
void put_file(int client_fd, char buffer[])
{
    // Handle PUT request
    FILE *file = open_file(buffer, client_fd, 'w', "SERVER 501 Put Error\n", "SERVER 501 Put Error\n");

    // file couldn't open
    if (file == NULL) 
        return; // go back to accepting new connections

    // receive and write data to file until two consecutive empty lines
    int empty_lines = 0;
    while (1) {
        int bytes_received = receive_client_message(client_fd, buffer);

        // Check that the first character sent is a next line char,
        // and that the line only contains at most 2 characters total (\n and \0)
        if (buffer[0] == '\n' && bytes_received <= 2) 
            empty_lines++; 
        else 
            empty_lines = 0;

        if (empty_lines > 1) 
            break;
        
        fwrite(buffer, 1, bytes_received, file);
    }
    fclose(file);

    // file opened and closed correctly
    send_to_client(client_fd, "SERVER 200 OK\n\n");

}

/**
 * Entry point of the server program.
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments:
 *                  argv[0]: program name
 *                  argv[1]: port number
 * @return Returns 0 upon successful execution, -1 otherwise.
 */ 
int main(int argc, char *argv[])
{
    // SERVER SET UP:

    if (argc != 2) 
        return -1;  // incorrect amount of arguments

    // get port number
    int port_num = get_port_num(argv);
    if (port_num == -1)  
        return port_num; // port number is invalid

    int fd = create_socket(); // create TCP socket
    struct sockaddr_in addr = create_address(port_num); // create address
    bind_address(fd, addr); // bind address to socket
    listen_for_incoming_connection(fd); // listen for incomming connections
    

    // HANDLING CLIENT INPUT:

    while(1) {
        int client_fd = accept_client(fd); // get client file descriptor/accept client
        send_to_client(client_fd, "HELLO\n"); // send client a HELLO message
        
        // recieve message from the client
        char buffer[BUFFER_LEN];
        receive_client_message(client_fd, buffer);

        // CLIENT COMMANDS:
        // GET: display file contents
        if (strings_match(buffer, "get") == 0) 
            get_file(client_fd, buffer);
        
        // PUT: write into a file
        else if (strings_match(buffer, "put") == 0) 
            put_file(client_fd, buffer);
        
        // If not BYE: send an error
        else if (strings_match(buffer, "bye") != 0) 
            send_to_client(client_fd, "SERVER 502 Command Error\n");
        
        // close client file descriptor
        close(client_fd);
    }
    
    close(fd);
    return 0;
}