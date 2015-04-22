/* Example program demonstrating how to print information about a file or
 directory.
 */

#define MAX_BUFFER_SIZE 1024
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

/* Simple example contains only the main function that takes a file
 (or directory) name from the command line and prints out various
 statistics about it.
 */
char **parse_line(char *theLine,int mode){
    
    int bufferSize = MAX_BUFFER_SIZE,position = 0;
    char **commands = malloc(bufferSize * sizeof(char*));
    char *singleCommand;
    if(mode==1){
        singleCommand = strtok(theLine," ");
        while(singleCommand){
            commands[position++] = singleCommand;
            singleCommand = strtok(NULL," ");
        }
    }
    else{
        singleCommand = strtok(theLine,"|<>");
        while(singleCommand){
            commands[position++] = singleCommand;
            singleCommand = strtok(NULL,"|<>");
        }
    }
    commands[position] = '\0';
    return commands;
    
}
char *readInCommand(){
    int position = 0;
    char *inputLine = (char *)malloc(sizeof(char) * MAX_BUFFER_SIZE);
    int c;
    while (1){
        c = getchar();
        if(c == EOF){
            printf("Goodbye!");
            exit(0);
        }
        if(c == '\n'){
            inputLine[position] = '\0';
            return inputLine;
        }
        inputLine[position++] = c;
    }
}
int main(int argc, char* argv[]) {
    struct stat fd_stat;
    
    char *theInput;
    
    while(1){
        printf("%% ");
        theInput = readInCommand();
        char **commands = parse_line(theInput, 0);
        
    }
    
    return 0;
}