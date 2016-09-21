/***************************************************************************
 ** Author:      Carlos Carrillo-Calderon                                  *
 ** Date:        08/07/16                                                  *
 ** Filename:    otp_enc_d.c                                               *
 **                                                                        *
 ** Description: This program runs in the background as a daemon. Its      *
 **              function is to perform the actual encoding. This program  *
 **              listens on a particular assigned port. Thus, when it's    *
 **              first ran, it receives plaintext and a key via that port  *
 **              when a connection to it is made. It then write back the   *
 **              ciphertext to the process that it is connected to via the *
 **              same port. Note that the key passed in must be at least   *
 **              as big as the plaintext. This program output an error if  *
 **              the program cannot be run due to a network error, such    *
 **              as the ports being unavailable.                           *
 **************************************************************************/

#include <fcntl.h>     
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFERSIZE 100000

// Main body (CODE TAKEN FROM CLIENT)
int main(int argc, char *argv[]) {
   
    // Declare variables.
    pid_t pid;
    int writer;
    int status;			 // exit status of child processes.
    int keyChar;
    int inputChar;
    int value = 1;
    int cipherText;
    int numChild=0;		 // number of child processes.
    int plaintext_length;
    socklen_t clilen;
    int i, n, key_length;
    int sockfd, newsockfd, portno;
    char textBuffer[BUFFERSIZE];
    char keyBuffer[BUFFERSIZE];
    char tempBuffer[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    
    // Validate number of user arg.
    if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	else if(argc > 2){
		fprintf(stderr,"ERROR, too many arguments provided\n");
		exit(1);
	}
    // Interpret argument content as an integer to get the port number.
    portno = atoi(argv[1]);

    /*********************************************************
    * SOCKET SETTINGS                                        *
    *********************************************************/
    // Create TPC socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Error checking.
    if (sockfd < 0) {
        printf("Error: opt_enc_d could not create socket\n");
        exit(1);
    }
    // Set SO_REUSEADDR on socket to be reused. It allows other sockets
    // to bind() to this port, unless there is an active listening
    // socket bound to the port already.
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
    
    // Set IP address to zero.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    // Set the address family and symbolic constant AF_INET.
    serv_addr.sin_family = AF_INET;
    
    // Set IP address of the host symbolic constant INADDR_ANY.
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Converts port number from host byte order to network byte order.
    serv_addr.sin_port = htons(portno);

    // Bind socket to port number.
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: otp_enc_d unable to bind socket to port %d\n", portno);
        exit(2);
    }
    // Set up to five socket connections and perform error checking.
    if (listen(sockfd, 5) == -1) {
        printf("Error: otp_enc_d unable to listen on port %d\n", portno);
        exit(2);
    }
    
    /*********************************************************
    * LOOP TO SET ALL POSSIBLE CONNECTIONS.                  *
    *********************************************************/
    while (1) {
        // Stores the address size of the client.
        // This is needed for the accept system call.
        clilen = sizeof(cli_addr);

        // Check for completion of child processes.
		for(i=0; i < numChild; i++) {
			if(waitpid(-1, &status, WNOHANG) == -1){
				perror("wait failed");}
			if(WIFEXITED(status)){
				numChild -= 1;}
		}
        // Extract the first connection on the queue of pending
        // connections, create a new socket with the same socket
        // type protocol and address family as the specified socket,
        // and allocate a new file descriptor for that socket.
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        // Error checking.
        if (newsockfd < 0) {
            printf("Error: opt_enc_d unable to accept connection\n");
            continue;
        }
        // Start Fork process.
        pid = fork();

        // Error checking.
        if (pid < 0) {
            perror("ERROR(otp_enc_d): error on fork\n");
            exit(1);
        }
        // Child Process
        if (pid == 0) {
        
            // Set textBuffer to zero.
            memset(textBuffer, 0, BUFFERSIZE);
    
            // Receive authentication message and reply.
            read(newsockfd, textBuffer, sizeof(textBuffer)-1);
            
            // Validate connection and write error back to client.
            if (strcmp(textBuffer, "enc_bs") != 0) {
                char response[]  = "invalid";
                write(newsockfd, response, sizeof(response));
                _Exit(2);
            }
            // Write confirmation back to client.
            else {
                char response[] = "enc_d_bs";
                write(newsockfd, response, sizeof(response));
            }
            // Set textBuffer to zero again.
            bzero(textBuffer, sizeof(textBuffer));
		
            // Read the content of the plaintext file sent by otp_enc
            // and place it into the buffer and also get its length.
            plaintext_length  = read(newsockfd, textBuffer, BUFFERSIZE);
            
            // Error checking.
            if (plaintext_length  < 0) {
                printf("Error: otp_end_d could not read plaintext on port %d\n", portno);
                exit(2);
            }
            // Validate the contents inside the plaintext file.
            for (i = 0; i < plaintext_length ; i++) {
                if ((int)textBuffer[i]>'Z'||((int)textBuffer[i]<'A'&&(int)textBuffer[i]!=' ')) {
                    printf("ERROR(otp_enc_d): plaintext5 contains bad characters!!\n");
                    exit(EXIT_FAILURE);
                }
            }
            // Write/sent acknowledgement message to the client.
            writer = write(newsockfd, "!", 1);
            
            // Check if the message was sent to the client.
            if (writer < 0) {
                printf("ERROR(otp_enc_d): failure in sending acknowledgement to client\n");
                exit(2);
            }
            // Clear buffer to hold the key file.
            memset(keyBuffer, 0, BUFFERSIZE);

            // Read the content of the key file sent by otp_enc
            // and place it into the buffer and also get its length.
            key_length = read(newsockfd, keyBuffer, BUFFERSIZE);
            
            // Error checking.
            if (key_length < 0) {
                printf("Error: otp_end_d could not read key on port %d\n", portno);
                exit(2);
            }
            // Validate the contents inside the key file.
            for (i = 0; i < key_length; i++) {
                if ((int)keyBuffer[i]>'Z'||((int)keyBuffer[i]<'A'&&(int)keyBuffer[i]!=' ')) {
                    printf("ERROR(otp_enc_d): key contains bad characters\n");
                    exit(EXIT_FAILURE);
                }
            }
            // Check if the key is as long as plaintext file.
            if (key_length < plaintext_length ) {
                printf("ERROR(otp_enc_d): key is too short\n");
                exit(1);
            }
            
            /**********************************************
            * PERFORM ENCRYPTION PROCESS.                 *
            **********************************************/
            for (i = 0; i < plaintext_length ; i++) {
                // Spaces are replaced/marked with "@" so they can
                // be put back in their place after encryption.
                if (textBuffer[i] == ' ') {
                    textBuffer[i] = '@';
                }
                if (keyBuffer[i] == ' ') {
                    keyBuffer[i] = '@';
                }
                // Typecast to integer for ASCII processing.
                inputChar = (int) textBuffer[i];
                keyChar = (int) keyBuffer[i];

                // Transform plaintext into ASCII code range.
                inputChar = inputChar - 64;
                keyChar = keyChar - 64;

                // Perform sum and mod 27 operation to encrypt plaintext.
                cipherText = (inputChar + keyChar) % 27;

                // Set cipher output to capital letters.
                cipherText = cipherText + 64;

                // Typecast character back to char and store
                // them in the temporary buffer.
                tempBuffer[i] = (char) cipherText + 0;

                // Set spaces back in their place after encryption.
                if (tempBuffer[i] == '@') {
                    tempBuffer[i] = ' ';
                }
            }
            // Write the encrypted text into the new socket.
            writer = write(newsockfd, tempBuffer, plaintext_length);
            
            // Check for writing errors.
            if (writer < plaintext_length) {
                printf("otp_enc_d error writing to socket\n");
                exit(2);
            }
            // Close the sockets previously created.
            close(newsockfd);
            close(sockfd);
            exit(0);
        }
        //Parent process.
        else {
            numChild += 1;		// Increment number of child processes.
			close(newsockfd);	// Close new socket.
        }
    }
    return 0;
}
