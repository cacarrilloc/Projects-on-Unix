/**************************************************************************
 **
 ** Author:      Carlos Carrillo-Calderon
 ** Date:        07/19/16
 ** Filename:    carrilca.adventure.c
 **
 ** Description: This program first creates a series of files that hold
 **              descriptions of rooms and how the rooms are connected.
 **              Then the program offers to the player an interface for
 **              playing the game using those generated rooms. The player
 **              begins in the “starting room” and wins the game
 **              automatically upon entering the “ending room”. Finally,
 **              the program exits and displays the path taken by the player.
 **
 ** Input:       from the keyboard: type char*
 **              from files:        type char*
 **
 ** Output:      to the console and files : type char*, int
 **              to files:                  type char*, int
 **
 *************************************************************************/

// Define Libraries
#define _GNU_SOURCE
#include <dirent.h>    // for defining typedef
#include <assert.h>    // for making assertions
#include <stdbool.h>   // for bool performance
#include <time.h>      // for random
#include <errno.h>     // for errno
#include <unistd.h>    // for getpid
#include <sys/types.h> // for pid_t
#include <stdio.h>     // for fgets, fopen, fclose, fseek
#include <stdlib.h>    // for rand and srand
#include <sys/stat.h>  // for stat
#include <string.h>    // for strcpy, strcat
#include <time.h>      // for time
#include <fcntl.h>     // for open

// Define Global Constants
#define MIN_CONX 3
#define MAX_CONX 6
#define NUM_ROOMS 7
#define EXISTING_ROOMS 10
#define BUFFER_LENGTH 12
#define BUFFER_SIZE 512  // Used for the reading operation.


// This array contains all the rooms possible used in the game.
const char *roomPool[EXISTING_ROOMS] = {"Library", "Cafeteria", "Hall", "Kitchen", "Bathroom", "Basement", "Laundry", "Closet", "Pantry", "Garage"};

// Create enum type for writing room information into the files created.
enum roomType {START_ROOM, MID_ROOM, END_ROOM};

// Struct to write into the files.
struct room {
        enum roomType type;
        const char *name;
        int conexCapacity;   // Maximum connections assigned to a room
        int conexNumber;     // Used for assigning connections to room
        struct room *connections[NUM_ROOMS];
};

// Struct to read files.
struct readRoom {
        char type[BUFFER_LENGTH];
        char name[BUFFER_LENGTH];
        int conexCapacity2;          // Maximum connections assigned to a room
        int conexCounter2;           // Used for assigning connections to room
        char connections[MAX_CONX][BUFFER_LENGTH];
};
// Create array for reading the rooms from the files created.
const char *roomType[] = {"START_ROOM", "MID_ROOM", "END_ROOM"};

// Arrays of structs for storing the rooms for the game.
struct room roomContainer[NUM_ROOMS];     //used for the writing process
struct readRoom readContainer[NUM_ROOMS]; //used for the reading process

// Functions Prototypes
void readRoomFiles();
struct room *roomGeneration();
void actualGame(struct readRoom readContainer[NUM_ROOMS]);
void writeRoomFiles(struct room roomContainer[NUM_ROOMS]);
bool connectRooms(int room1, int room2, struct room roomContainer[NUM_ROOMS]);

// Define a couple of global variables to make life easier. ;-)
char visitedRooms[100][12];  // stores player's path (holds up to 100 moves)
int currentRoom;             // used as struct array index.


/**************************************************
* actualGame: Function to perform the actual game.
***************************************************/
void actualGame(struct readRoom readContainer[NUM_ROOMS]) {

        // Declare variables.
        bool matched;
        char input[BUFFER_SIZE];
        bool missionAccomplished = false;
        int stepsNum = 0;
        int i, j;
    
        // This loop performs the actual game until the END_ROOM is found.
        do {
            // First, check if the player reached the END_ROOM.
            if (strcmp(readContainer[currentRoom].type, roomType[2]) == 0){
            
                missionAccomplished = true;
                continue; // exit the loop.
            }
            // Display the very first part of the user interface.
            printf("\nCURRENT LOCATION: %s\n", readContainer[currentRoom].name);
            printf("POSSIBLE CONNECTIONS: ");

            // Display all the connections attached to the current room.
            for (i = 0; i < readContainer[currentRoom].conexCapacity2; i++){
            
                // Print period right after the last connection in the list.
                if (i == readContainer[currentRoom].conexCapacity2 - 1) {
                    printf("%s.", readContainer[currentRoom].connections[i]);
                    
                } else {
                    printf("%s, ", readContainer[currentRoom].connections[i]);
                }
            }
            // Prompt user to enter the input room
            printf("\nWHERE TO? >");

            // Get user input
            fgets(input, BUFFER_SIZE, stdin);

            // Parse input and make sure only the input text is taken.
            strtok(input, "\n");

            // Now compare user's input against all the connections
            // attached to the current room.
            matched = false;
            for(i = 0; i < readContainer[currentRoom].conexCapacity2; i++){
            
                // Check if the user input matches any of the room connections.
                if(strcmp(input, readContainer[currentRoom].connections[i]) == 0){
                   // Set variable to tell the program there's a match.
                   matched = true;
                   stepsNum++; //increment the step counter.
                    
                   // Add the matched room to the array that keeps
                   // a record of the the rooms visited.
                   strcpy(visitedRooms[stepsNum], readContainer[currentRoom].connections[i]);
               
                   // Now find the room just visited within the array
                   // that keeps the 7 rooms created for the game and set
                   // that room as the next current room to be displayed.
                   for(j = 0; j < NUM_ROOMS; j++) {
                       if(strcmp(readContainer[currentRoom].connections[i], readContainer[j].name) == 0){
                          currentRoom = j; //// Set the current room index.
                          break;
                        }
                   }
                   // If match found, break from outer loop early.
                   break;
                }
            }
            // Continue and prompt again if it's a valid input.
            if (matched == true){
                continue;
            }
            // Print error if input doesn't match with a valid room option
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");

        } while(missionAccomplished == false); // repeat until win condition achieved
    
        // Display winning message when user reaches End Room.
        printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
        printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepsNum);

        // Display the visited rooms in the right order.
        for (i = 0; i <= stepsNum ; i++) {
            printf("%s\n", visitedRooms[i]);
        }
        printf("\n");
}

/***********************************************************
* roomGeneration: Function to generate the 7 rooms required.
************************************************************/
struct room *roomGeneration() {

        // Set variables
        int i, j;
        int key = 0;
        int roomCounter = 0;
        int randomIndex = 0;
    
        //Fill up room container with 7 random rooms
        for(i=0; i<NUM_ROOMS; i++){
        
            // Initialize the number of connections to 0.
            roomContainer[i].conexNumber = 0;
    
            // Since the program must have  at least 3 and and at most 6
            // outgoing connections, the connection capacity is set to
            // a random number between 3 and 6.
            int conexCapacity2 = rand() % (MAX_CONX - MIN_CONX + 1) + MIN_CONX;
            roomContainer[i].conexCapacity = conexCapacity2;
    
            //Loop to avoid repeated rooms
            do{
                // Generate index for random rooms
                randomIndex = rand() % NUM_ROOMS;
                key = 1; // The loop is always open, unless...
        
                for(j= 0; j<roomCounter; j++) {
                    // Check for duplicate names.
                    if(strcmp(roomContainer[j].name, roomPool[randomIndex])==0){
                        key = 0; // Close the loop since there're duplicates
                    }
                }
            }while(key == 0);
        
            // Copy random room value into the room container.
            roomContainer[i].name = roomPool[randomIndex];
            roomCounter++;

            // Initially set all the rooms to MID_ROOM
            roomContainer[i].type = MID_ROOM;
        }

        // Now randomly connect the rooms to each other
        // using connectRoom function.
        for (i = 0; i < NUM_ROOMS; i++) {
            // for each connection,
            for (j = 0; j < roomContainer[i].conexCapacity; j++) {
                // generate a random room
                int random_room = rand() % NUM_ROOMS;
                    
                // Use function to connect the rooms.
                // Randomize again if connection fails.
                if(!connectRooms(i, random_room, roomContainer)) {
                    random_room = rand() % NUM_ROOMS;
                }else{
                    connectRooms(i, random_room, roomContainer);
                }
            }
        }
        // Select the first room in the array as the start room
        // and the last room in the array as the end room.
        roomContainer[0].type = START_ROOM;
        roomContainer[NUM_ROOMS - 1].type = END_ROOM;
        return roomContainer;
}


/*****************************************************************
* connectRooms: Function to Connect two rooms. Returns true 
* if the rooms are connected or false if the rooms are already
* connected or the maximum number of connections has been reached.
******************************************************************/
bool connectRooms(int room1, int room2, struct room roomContainer[NUM_ROOMS]) {

        // Create 2 pointer variables to access to the room connections
        struct room *tempRoom1 = &roomContainer[room1];
        struct room *tempRoom2 = &roomContainer[room2];
        int i;
    
        // Terminate if the maximum number of connections is reached.
        if (tempRoom1->conexNumber == MAX_CONX) {
                return true;
        }
        // Finish if the rooms are already connected to themselves.
        if (room1 == room2) {
                return false;
        }
        // Check if each room is connected to the current room.
        for (i = 0; i < roomContainer[room1].conexNumber; i++) {
            if (roomContainer[room1].connections[i] == &roomContainer[room2] &&
                    roomContainer[room1].connections[i] != NULL) {
                return false;
            }
        }
        // Don't make a connection if the maximum number of connections
        // has been reached in either of the rooms.
        if(tempRoom1->conexNumber >= MAX_CONX||tempRoom2->conexNumber >= MAX_CONX) {
                return false;
        }
        assert(tempRoom1 != NULL);
        assert(tempRoom2 != NULL);
    
        // Actual connection between rooms
        tempRoom1->connections[tempRoom1->conexNumber] = tempRoom2;
        tempRoom2->connections[tempRoom2->conexNumber] = tempRoom1;
    
        // Update the number of connections
        tempRoom1->conexNumber++;
        tempRoom2->conexNumber++;
    
        // Assert in case something went wrong.
        assert(tempRoom1->connections[tempRoom1->conexNumber-1] != NULL);
        assert(tempRoom2->connections[tempRoom2->conexNumber-1] != NULL);
        return true;
}

/*********************************************************************
* makeDirectoryName: Function to create and return the directory name.
*********************************************************************/
char *makeDirectoryName() {

        // Get the current process id.
        pid_t processID = getpid();

        char *dir_name = malloc(30 * sizeof(char));
        assert(dir_name != NULL);
        char pidStr[15];

        // create directory called <username>.rooms.<process id>
        // prepare pathname string to create directory
        strcpy(dir_name, "./carrilca.rooms.");
        sprintf(pidStr, "%d", processID);
        strcat(dir_name, pidStr);

        // Per: http://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c check if directory already exists.
        struct stat st = {0};
        if (stat(dir_name, &st) == -1) {
            // if not, create it
            int retVal = mkdir(dir_name, 0777);

            // check return value to see if the operation was successful.
            if (retVal == -1) {
                // if not, send message to standard error
                fprintf(stderr, "Unable to create directory.\n");
                perror("Error: in main");
                exit(1);
            }
        }
        return dir_name;
}

/************************************************************************
* writeRoomFiles: Function to Create room files and write info into them.
*************************************************************************/
void writeRoomFiles(struct room roomsArray[NUM_ROOMS]) {

        int i, j;
        // Call makeDirectoryName to get
        char *dir_name = makeDirectoryName();
    
        // Set permissions for the directory
        mkdir(dir_name, 0777);
    
        // Make dir_name the current working directory
        chdir(dir_name);
    
        // Make a file for each room.
        for (i = 0; i < NUM_ROOMS; i++) {
            
            // Open the file
            FILE *outputFile = fopen(roomsArray[i].name, "w");

            // Write the room name
            fprintf(outputFile, "ROOM NAME: %s\n", roomsArray[i].name);

            // Write each connection onto the file
            for (j = 0; j < roomsArray[i].conexNumber; j++) {
                // Create pointer variables to access to the room connections
                struct room *connect1;
                struct room *connect2 = &roomsArray[i];
                // Access to the actual connection
                connect1 = connect2->connections[j];
                // Print connection name into the file
                fprintf(outputFile, "CONNECTION %d: %s\n", j+1, connect1->name);
            }
            
            // Now write the room type into the file
            if(roomsArray[i].type == START_ROOM){
                 fprintf(outputFile, "ROOM TYPE: START_ROOM\n");
                 fprintf(outputFile, '\0');
                    
            }else if(roomsArray[i].type == END_ROOM){
                 fprintf(outputFile, "ROOM TYPE: END_ROOM\n");
                 fprintf(outputFile, '\0');
                
            }else{
                 fprintf(outputFile, "ROOM TYPE: MID_ROOM\n");
                 fprintf(outputFile, '\0');
            }
            
            // Close file
            fclose(outputFile);
        }
    
        // Return to the original directory.
        chdir("..");
        // Free directory name
        free(dir_name);
}

/**********************************************************
* writeRoomFiles: Function to read the files for the game.
**********************************************************/
void readRoomFiles() {

        // Declare variables.
        int i, j;
        char roomFile[40];
        char* token;
        char dirName[30];
        char readBuffer[BUFFER_SIZE];  // buffer to keep data read.
        bool connectionFlag = true;
        char pidStr[10];
        int fileOpener;
        int roomIndex = 0;
        ssize_t roomRead[NUM_ROOMS];   // Arrays to store data read.
        pid_t processID = getpid();    // Get the current process id.
    
    
        // Get the directory-name string
        strcpy(dirName, "./carrilca.rooms.");
        sprintf(pidStr, "%d", processID);
        strcat(dirName, pidStr);

        // This loop creates a file name for each of the 10 existing
        // rooms available for the game. If there's a file with the
        // room name just created, its contents are read and stored.
        // Otherwise, the loop is exited and start the same process
        // with the next room from the roomPool array.
        for (i = 0; i < EXISTING_ROOMS; i++) {
            // Create file name for roomPool[i]
            strcpy(roomFile, dirName);
            strcat(roomFile, "/");
            strcat(roomFile, roomPool[i]);

            // Attempt to open the roomFile corresponding to roomPool[i]
            fileOpener = open(roomFile, O_RDONLY);

            // Exit the loop if the file is not found and check
            // if the next file in the roomPool array exist.
            if (fileOpener < 0){
                continue;
            }
            // Read the content of the file just found and keep it
            // in the array created for that purpose.
            roomRead[roomIndex] = read(fileOpener, readBuffer, BUFFER_SIZE);

            // Now start parsing the content read and stored in readBuffer
            // in order to get the name of the room.
            token = strtok(readBuffer, " ");
            token = strtok(NULL, " ");
            token = strtok(NULL, "\n");

            // Copy the room name obtained into the struct array
            // designated for this purpose.
            strcpy(readContainer[roomIndex].name, token);

            // Initialize all the connection counters and flag.
            readContainer[roomIndex].conexCapacity2 = 0;
            readContainer[roomIndex].conexCounter2 = 0;
            connectionFlag = true;
            j = 0;
            
            // Loop to read all the connections of the room just
            // stored in the buffer.
            do{
                // Keep parsing after the room name was obtained.
                token = strtok(NULL, " ");

                // Make sure the data/string read comes after the
                // CONNECTION string. If it's not the case, exit
                // the do-while loop by changing connectionFlag.
                if (strcmp(token, "CONNECTION") != 0) {
                    connectionFlag = false;
                    continue;
                }
                // Otherwise, keep parsing data.
                token = strtok(NULL, " ");
                token = strtok(NULL, "\n");

                // Copy the connection name obtained into the struct array
                // designated for this purpose.
                strcpy(readContainer[roomIndex].connections[j], token);

                // Update the number of connections assigned to current room.
                readContainer[roomIndex].conexCapacity2++;

                // increment the counter to get the next connection.
                j++;

            } while (connectionFlag == true);
        
            // Now parse data to get the room type
            token = strtok(NULL, " ");
            token = strtok(NULL, "\n");
        
            // Copy the connection name obtained into the struct array
            // designated for this purpose.
            strcpy(readContainer[roomIndex].type, token);

            // Set the START_ROOM as the current room so the function
            // that performs the actual game will be correctly started.
            if (strcmp(readContainer[roomIndex].type, roomType[0]) == 0) {
                currentRoom = roomIndex;
                strcpy(visitedRooms[0], readContainer[currentRoom].name);
            }
            // increment index for the struct array.
            roomIndex++;
    }
}

/**********************************************************
* Main Function: Function to execute the entire program.
**********************************************************/
int main() {

        // Seed the random number generator.
        srand(time(0));
    
        // Generate the 7 rooms for the maze.
        roomGeneration();
    
        // Create room files and write info into them
        writeRoomFiles(roomContainer);
    
        // Read files and get information for the game.
        readRoomFiles();
    
        // Perform the actual game!
        actualGame(readContainer);

        return 0;
}






