//
//  simpsh.c
//  Assignment3OS
//
//  Created by Sammeh Tillman on 4/5/15.
//  Copyright (c) 2015 weatherfall.org. All rights reserved.
//  This program replicates functionality of a shell.
//
#define MAX_BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
int backgrounding;

//this function is to count of the amount of length of the string.
int lengthString(char *string) {
    int counter = 0;//the counter to be used for...counting.
    while (*string != 0) {//while the string position is not null.
        counter++;//count the counter
        string++;//keep moving down the string
    }
    return counter;//return the count
}
//this function counts up the length of the array.
int lengthOfArray(char **args){
    int counter =0;
    int total = 0;
    while(1){
        if(args[counter]){
            total++;
            counter++;
        }
        else{
            break;
        }
    }
    return total;
}
//this reads in commands or input from the commandline.
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
//this parses the lines of all input and delimits.
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
//this copys all of the doubled arrays
char **copyDouble(char **source){
    int length = lengthOfArray(source),i;
    char **newArray = (char **)malloc((length+1) * sizeof(char*));
    newArray[length] = NULL;
    int counter = 0;
    for (i = 0; i < length;i++){
        if(strcmp(source[i], ">")){
            if(strcmp(source[i], "<")){
                if(strcmp(source[i], "|")){
                    newArray[counter++] = strdup(source[i]);
                }
            }
        }
    }
    return newArray;
}
//this function finds all characters that would be normally inputted.
char *findCharacter(char **args){
    int length = lengthOfArray(args),i;
    char *newArray = (char*)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    for(i = 0; i < length; i++){
        if((!strcmp(args[i], "|"))||(!strcmp(args[i], "<"))||(!strcmp(args[i], ">")) || (!strcmp(args[i], "&")) ){
            strcat(newArray,strcat(args[i]," "));
        }
    }
    return newArray;
}
//this is the regular program launcher
int launchProgram(char **commands,char **args,short int background){
    pid_t pid;
    int child_status;
    pid = fork();
    if(pid == 0){
        printf("kid is about to execute.");
        char **newArgs = parse_line(args[0], 1);//this creates the arguments
        
        int length = lengthOfArray(newArgs);
        if(!strcmp(newArgs[length-1],"&")){
            newArgs[length-1] = NULL;
        }
        if(execv(commands[0],newArgs)){
            perror("It's no good.");
            exit(EXIT_FAILURE);
        }
    }
    else{
        if(!backgrounding){
            waitpid(pid,&child_status,0);
        }
    }
    return 1;
}
//this launch redirects all inputs depending on the signal given.
int launchRedirect(char **commands,char **args,short int background,short int redirectSignal){
    int fork_rtn;
    int child_status,input_fd,output_fd;
    background = backgrounding;
    switch (redirectSignal) {
        case 1: // input redirect only.
            if ((fork_rtn = fork())) {
                /* This is the parent. */
                if(!background){
                    wait(&child_status);
                }
            }
            else{
                //this opens file specified.
                if ((input_fd = open(commands[1], O_RDONLY)) >= 0) {
                    close(0);
                    dup2(input_fd, 0);
                }
                else{
                    fprintf(stderr,
                            "%s: unable to input redirect.'.\n", args[0]);
                    exit(0);
                }
                
                char **newArgs = parse_line(args[0], 1);
                int length = lengthOfArray(newArgs);
                if(!strcmp(newArgs[length-1],"&")){
                    newArgs[length-1] = NULL;
                }
                if(execv(commands[0],newArgs)){
                    perror("It's no good.");
                    exit(0);
                }
            }
            break;
        case 2: // output found.
            
            if ((fork_rtn = fork())) {
                /* This is the parent. */
                if(!background){
                    wait(&child_status);
                }
            }
            else{
                //creates the output file.
                if((output_fd = creat(commands[2], S_IRUSR| S_IWUSR))>=0){
                    close(1);
                    dup2(output_fd, 1);
                    fchmod(output_fd, S_IRUSR| S_IWUSR| S_IRGRP);
                }
                else{
                    fprintf(stderr, "Unable to create the file.\n");
                    exit(2);
                }
                char **newArgs = parse_line(args[0], 1);
                int length = lengthOfArray(newArgs);
                if(!strcmp(newArgs[length-1],"&")){
                    newArgs[length-1] = NULL;
                }
                if(execv(commands[0],newArgs)){
                    perror("It's no good.");
                    exit(0);
                }
            }
            break;
        case 3://both input and output redirection.
            if ((fork_rtn = fork())) {
                /* This is the parent. */
                if(!background){
                    wait(&child_status);
                }
            }
            else{
                if ((input_fd = open(commands[1], O_RDONLY)) >= 0) {
                    close(0);
                    dup2(input_fd, 0);
                }
                else{
                    //perror(args[1]);
                    fprintf(stderr,
                            "%s: I can't!.\n", args[0]);
                    exit(0);
                }
                if((output_fd = creat(commands[2], S_IRUSR| S_IWUSR))>=0){
                    close(1);
                    dup2(output_fd, 1);
                    fchmod(output_fd, S_IRUSR| S_IWUSR| S_IRGRP);
                }
                else{
                    fprintf(stderr, "Unable to create the file.\n");
                    exit(2);
                }
                char **newArgs = parse_line(args[0], 1);
                int length = lengthOfArray(newArgs);
                if(!strcmp(newArgs[length-1],"&")){
                    newArgs[length-1] = NULL;
                }
                if(execv(commands[0],newArgs)){
                    perror("It's no good.");
                    exit(0);
                }
            }
            break;
        default:
            break;
    }
    return 1;
}
void endProgram(){
    printf("\nGood bye!");
    exit(0);
}
//this function launches the pipes and redirection
int launchPipes(char **command, char **args,int pipeCount,int redirectSignal){
    int tube[2*pipeCount],fork_rtn,i,child_status;
    /*
     Create all pipes at once.
     */
    for(i = 0; i < pipeCount; i++){
        if (pipe(tube+i*2)) {//create the pipes in order
            fprintf(stderr, "%s: Unable to create pipe.\n", args[0]);
            exit(1);
        }
    }
    int commandNum = 0,input_fd,tempLength;
    int j;
    tempLength = lengthOfArray(args);
    char *argv[3];
    /*
     Using the created pipes, fork the children and move along the pipeline that
     connects them.
     */
    int length = pipeCount+1;
    for(i = 0; i < length;i++){
        fork_rtn = fork();
        argv[0] = args[i];//
        argv[1] = args[i+1];
        if(fork_rtn == 0){
            if(i == 0){//only for instances of an input redirection.
                if(redirectSignal == 1 || redirectSignal == 3){
                    if ((input_fd = open(command[1], O_RDONLY)) >= 0) {
                        close(0);
                        dup2(input_fd, 0);//change your input.
                    }
                }
            }
            //duplicate standard input routine; if it isnt a redirect.
            if(i!=0){
                dup2(tube[commandNum-2], 0);
            }
            //duplicate standard out routine
            if(i<length-1){
              dup2(tube[commandNum+1], 1);
            }
            for(j = 0; j < 2*pipeCount;j++) {//close all pipes.
                close(tube[j]);
            }
                char **newArgs = parse_line(args[i], 1);
                int length = lengthOfArray(newArgs);
                if(!strcmp(newArgs[length-1],"&")){
                    newArgs[length-1] = NULL;
                }
                if(execv(command[i],newArgs)){
                    perror("It's no good.");
                    exit(0);
                }
        }
        else if(fork_rtn < 0){
            perror("Broke.");
            exit(EXIT_FAILURE);
        }
        commandNum+=2;
        
    }
    for(i = 0; i < 2 * pipeCount; i++ ){
        close(tube[i]);
    }
    if(!backgrounding)//determines backgrounding or not
    for(i = 0; i < pipeCount+1;i++){
        wait(&child_status);
    }
    
    return 1;
    
}
//this serves as a launcher for the main launch and redirecting--it coordinates
//all of the situations.
int chainExecution(char **commands,char **args,char **commandType){
    int length = lengthOfArray(commandType),i;
    int rFlag =0,iFlag=0,backgrounded=0;
    if(commandType[0]){
        for(i = 0; i < length;i++){
            if(!strcmp(commandType[i], "<")){
                rFlag = 1;
            }
            if(!strcmp(commandType[i], ">")){
                iFlag = 1;
            }
        }
        /*
         assumes no pipes are present.
         */
        if(rFlag && !iFlag){
            launchRedirect(commands,args, backgrounded, 1);
        }else if (!rFlag && iFlag){
            launchRedirect(commands,args, backgrounded, 2);
        }
        else if(iFlag && rFlag){
            launchRedirect(commands,args, backgrounded, 3);
        }
        else{
            launchProgram(commands,args, backgrounded);
        }
    }
    else{
        launchProgram(commands,args, backgrounded);
    }
    return 1;
}

//main function where all the magic happens.
int main(int argc, const char * argv[]) {
    // insert code here...
    char *theInput;
    signal(SIGQUIT,endProgram);
    int interactive = 0;
    if(argv[1]){
        const char *capture = argv[1];
        if(!strcmp(capture, "-i")){
            interactive = 1;
        }
    }
    while(1) {
        if(interactive)printf("%% ");
        backgrounding = 0;
        theInput = readInCommand();
        //this block of code deals with parsing the input.
        char *tempInput = strdup(theInput); //input to hold the copy
        char **commands = parse_line(theInput,0);// commands
        char **backupCommands = copyDouble(commands);//copy the double
        char **specialCount = parse_line(tempInput, 1);//this special count
        //holds the input that has been parsed.
        int length = lengthOfArray(specialCount),i,pipeCount=0,iFlag=0,oFlag=0;
        int secondLength = lengthOfArray(backupCommands);
        
        int checkIn=0,checkOut = 0,checPipe = 0;
        for(i = 0; i < length;i++){
            //check for the following symbols.
            if(!strcmp(specialCount[i], "|")){
                pipeCount+=1;
            }
            if(!strcmp(specialCount[i], "&")){
                specialCount[i] = "";
                backgrounding = 1;
            }
            if(!strcmp(specialCount[i], "<")){
                iFlag = 1;
            }
            if(!strcmp(specialCount[i], ">")){
                oFlag = 1;
            }
        }
        //this holds the new set of characters that have been found in the
        //commandline.
        char *miscCommands = findCharacter(specialCount);
        int stringLen = lengthString(miscCommands);
        
        for (i=0; i <stringLen;i++) {
            if(miscCommands[i] == '<'){
                checkIn = 1;
            }
            if(miscCommands[i] == '>'){
                checkOut = 1;
            }
            if(miscCommands[i] == '|'){
                checPipe = 1;
            }
        }
        //final commands that will hold the last section of code
        char **finalCommands = parse_line(miscCommands, 1);
        char **passingArgs = (char**)malloc(sizeof(char **)*MAX_BUFFER_SIZE);
        int commandNum = 0;
        for(i = 0; i < secondLength;i++){
            char *tempString = backupCommands[i];
            char **tempHolder = parse_line(strdup(tempString), 1);
            if((checkIn == 1 || checkOut == 1) && checPipe){
                if(i == 1){
                    continue;
                }
                passingArgs[commandNum++] = strdup(tempHolder[0]);
            }
            else{
                passingArgs[commandNum++] = strdup(tempHolder[0]);
            }
        }
        
        if(!commands[0]){
            continue;
        }
        if(!strcmp(commands[0],"end")){
            printf("\nGoodbye!");
            break;
        }
        //this checks for pipes.
        if(!pipeCount){
            chainExecution(passingArgs,commands,finalCommands);
        }
        else{
            //this checks for redirectSignals
            int redirectSignal =0;
            if(iFlag && oFlag){
                redirectSignal = 3;
            }
            else if(!iFlag && oFlag){
                redirectSignal = 2;
            }
            else if(iFlag && !oFlag){
                redirectSignal = 1;
            }
            launchPipes(passingArgs, commands,pipeCount,redirectSignal);
        }
    }
    return 0;
}
