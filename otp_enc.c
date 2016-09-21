/***************************************************************************
 ** Author:      Carlos Carrillo-Calderon                                  *
 ** Date:        08/07/16                                                  *
 ** Filename:    otp_enc.c                                                 *
 **                                                                        *
 ** Description: This program connects to otp_enc_d, and asks it to        *
 **              perform a one-time pad style encryption. By itself,       *
 **              otp_enc doesn’t do the encryption. When otp_enc receives  *
 **              the ciphertext, it outputs it to stdout. If otp_enc       *
 **              receives key or plaintext files with bad characters in    *
 **              them, or the key file is shorter than the plaintext, it   *
 **              exits with an error, and set the exit value to 1.         *
 **              If otp_enc cannot find the port given, it reports this    *
 **              error to the screen with the bad port, and set the exit   *
 **              value to 2. Otherwise, on successfully running, otp_enc   *
 **              sets the exit value to 0. otp_enc is NOT able to connect  *
 **              to otp_dec_d.                                             *
 **************************************************************************/

#include <arpa/inet.h>
#include <fcntl.h>     
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

#define BUFFERSIZE 100000

// Main body (CODE TAKEN FROM SERVER)
int main(int argc, char *argv[]) {

    // Declare variables.
    int writer, n;
    int value = 1;
    int key_length;
    int returnedData;
    int input_length;
    int i, file_opener;
    int sockfd, portno;
    char tempBuffer[1];
    char keyBuffer[BUFFERSIZE];
    char textBuffer[BUFFERSIZE];
    char valiBuffer[BUFFERSIZE];
    struct hostent *server;
    struct sockaddr_in serv_addr;
    
    /*******************************************************
    * OPEN AND READ INPUT ARGUMENTS/FILES.                 *
    *******************************************************/
    // Check if there are enough arguments.
    if (argc < 4) {
        printf("Usage: otp_enc plaintext key port\n");
        exit(1);
    }
    // Interpret argument content as an integer to get the port number.
    portno = atoi(argv[3]);
    
    // Open the argument/file that should contain the plaintext file.
    file_opener = open(argv[1], O_RDONLY);

    // Check for opening errors.
    if (file_opener < 0) {
        printf("Error: cannot open plaintext file %s\n", argv[1]);
        exit(1);
    }
    // Read the content of the plaintext file and place
    // it into the buffer and also get its length.
    input_length = read(file_opener, textBuffer, BUFFERSIZE);
   
    // Clear temporary buffer.
    bzero(tempBuffer, sizeof(tempBuffer));

    // Validate the contents inside the plaintext file.
    while (read(file_opener, tempBuffer, 1) != 0) {
        if (tempBuffer[0]!=' '&&(tempBuffer[0]<'A'||tempBuffer[0]>'Z')) {
            if (tempBuffer[0] != '\n') {
                fprintf(stderr,"%s contains invalid characters\n", argv[1]);
                exit(EXIT_FAILURE);
            }
        }
    }
    // Close plaintext file.
    close(file_opener);

    // Open the argument/file that should contain the key file.
    file_opener = open(argv[2], O_RDONLY);
    
    // Check for opening errors.
    if (file_opener < 0) {
        printf("Error: cannot open key file %s\n", argv[2]);
        exit(1);
    }
    // Read the content of the key file and place
    // it into the buffer and also get its length.
    key_length = read(file_opener, keyBuffer, BUFFERSIZE);
 
    // Clear temporary buffer.
    bzero(tempBuffer, sizeof(tempBuffer));

    // Validate the contents inside the key file.
    while (read(file_opener, tempBuffer, 1) != 0) {
        if (tempBuffer[0]!=' '&&(tempBuffer[0]<'A'||tempBuffer[0]>'Z')) {
            if (tempBuffer[0] != '\n') {
                fprintf(stderr,"%s contains invalid characters\n", argv[1]);
                exit(EXIT_FAILURE);
            }
        }
    }
    // Close key file.
    close(file_opener);

    // Check if key file is as long as the plaintext file.
    if (key_length < input_length) {
        printf("Error: key '%s' is too short\n", argv[2]);
    }
    
    /*********************************************************
    * SOCKET SETTINGS.                                       *
    *********************************************************/
    // Create TPC socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Error checking.
    if (sockfd < 0) {
        printf("Error: could not contact otp_enc_d on port %d\n", portno);
        exit(2);
    }
    // Set host name.
    server = gethostbyname("localhost");
    
    // Error checking.
    if (server == NULL) {
        printf("Error: could not connect to otp_enc_d\n");
        exit(2);
    }
    // Set SO_REUSEADDR on socket to be reused. It allows other sockets
    // to bind() to this port, unless there is an active listening
    // socket bound to the port already.
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
 
    // Clear the server address to which we want to connect.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    // Set the address family and the symbolic constant AF_INET.
    serv_addr.sin_family = AF_INET;
    
    // Copy length bytes from h_addr to s_addr.
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    // Converts port number from host byte order to network byte order.
    serv_addr.sin_port = htons(portno);
    
    
    /*********************************************************
    * SENDING DATA TO otp_enc_d (Plaintext and key)          *
    *********************************************************/
    // Connect the socket to the server address and do error checking.
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: could not connect to otp_enc_d on port %d\n", portno);
        exit(2);
    }
    // Make sure otp_enc is NOT able to connect to otp_dec_d.
    char auth[]="enc_bs";
    
    // Write process name to host.
    write(sockfd, auth, sizeof(auth));
    
    // Read from host.
    read(sockfd, valiBuffer, sizeof(valiBuffer));
    
    // Validate answer.
    if (strcmp(valiBuffer, "enc_d_bs") != 0) {
        fprintf(stderr,"unable to contact otp_enc_d on given port\n");
        exit(2);
    }
    // Write plaintext into the socket for encryption.
    writer = write(sockfd, textBuffer, input_length-1);
    
    // Error checking.
    if (writer < input_length - 1) {
        printf("Error: could not send plaintext to otp_enc_d on port %d\n", portno);
        exit(2);
    }
    // Clear temporary buffer for server acknowledgement process.
    memset(tempBuffer, 0, 1);
    
    // Get acknowledgement from server.
    returnedData = read(sockfd, tempBuffer, 1);
    
    // Check error in server connection.
    if (returnedData < 0) {
       printf("Error receiving acknowledgement from otp_enc_d\n");
       exit(2);
    }
    // Write key into the socket for encryption.
    writer = write(sockfd, keyBuffer, key_length-1);
    
    // Error checking.
    if (writer < key_length-1) {
        printf("Error: could not send key to otp_enc_d on port %d\n", portno);
        exit(2);
    }
    
    /*********************************************************
    * RECEIVE ENCRYPTED DATA FROM otp_enc_d.                 *
    *********************************************************/
    // Clear buffer.
    memset(textBuffer, 0, BUFFERSIZE);

    // Read encrypted content and place it into textBuffer.
    do {
        returnedData = read(sockfd, textBuffer, input_length-1);
    } while (returnedData > 0);

    // Error checking.
    if (returnedData < 0) {
       printf("Error receiving ciphertext from otp_enc_d\n");
       exit(2);
    }

    /*********************************************************
    * OUTPUT ENCRYPTED DATA                                  *
    *********************************************************/
    // Print encrypted content to console.
    for (i = 0; i < input_length-1; i++) {
        printf("%c", textBuffer[i]);
    }
    printf("\n");

    // close socket
    close(sockfd);

    return 0;
}
