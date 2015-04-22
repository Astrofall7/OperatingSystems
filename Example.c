//
//  Example.c
//  Assignment4OS
//
//  Created by Sammeh Tillman on 4/22/15.
//  Copyright (c) 2015 Sammeh Tillman. All rights reserved.
//

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


/* Simple example contains only the main function that takes a file
 (or directory) name from the command line and prints out various
 statistics about it.
 */

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
    
    /* Must be called with a file or directory name. */
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s file_or_dir_name\n", argv[0]);
        exit(1);
    }
    
    /* Get stat of the file/directory given on the command line. */
    if (stat(argv[1], &fd_stat)) {
        fprintf(stderr, "%s: Unable to stat '%s'\n", argv[0], argv[1]);
        exit(2);
    }
    
    /* Print various statistics for the file/directory. */
    printf("Statistics for the %s '%s':\n",
           S_ISDIR(fd_stat.st_mode) ? "directory" : "file",
           argv[1]);
    printf("\tOwning user ID: %d\n", fd_stat.st_uid);
    printf("\tOwning group ID: %d\n", fd_stat.st_gid);
    printf("\tLast access time: %ld\n", fd_stat.st_atime);
    printf("\tLast modification time: %ld\n", fd_stat.st_mtime);
    printf("\tSize: %lld\n", fd_stat
           .st_size);
    printf("\tBlock size used: %d\n", fd_stat.st_blksize);
    printf("\tNumber of blocks used: %lld\n", fd_stat.st_blocks);
    
    /* If it's a directory, then print the files/directories below it. */
    if (S_ISDIR(fd_stat.st_mode)) {
        DIR *fd;
        struct dirent *de;
        
        if ((fd = opendir(argv[1])) == NULL) {
            fprintf(stderr, "%s: Unable to open '%s'\n",
                    argv[0], argv[1]);
            exit(3);
        }
        
        printf("\tSub-directories and files are:\n");
        while ((de = readdir(fd))) {
            printf("\t\t%s\n", de->d_name);
        }
        closedir(fd);
    }
    
    return 0;
}