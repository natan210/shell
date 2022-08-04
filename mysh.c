#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define DONE "exit"
#define misd "Redirection misdirected\n"
#define ICMD "Usage: mysh [batch-file]\n"
#define COF "Error: Cannot open file "
#define job ": Command not found.\n"

char** splitLine(char *line);
void mode(char* fn, int type);
int execute(char **args);


int main(int argc, char* argv[]) {
    if (argc > 2){
        write(STDERR_FILENO, ICMD, strlen(ICMD));
        exit(1);
    }
    int type = 0;
    char *fn;
    if (argc < 2) {
//interactive
    }
    else {
        type = 1;
        fn = argv[1]; //batch
    }
    mode(fn, type);
    exit(0);
}



char** splitLine(char *line) {
    int max = 100;
    const char s[10] = " \n\t\r\v\f ";
    char *token = strtok(line, s);
    char **ret = malloc(max*sizeof(char*));
    int i = 0;

    while (token != NULL) {
        ret[i] = token;
        i++;
        if (i >=(max-1)) { 
           break;
        }
        token = strtok(NULL, s);
    }
    ret[i] = NULL;
    return ret;
}
void mode(char* fn, int type) {
    FILE *fp;
    if (type) {
        fp = fopen(fn, "r");
        if (fp == NULL) {

            write(STDERR_FILENO, ("Error: Cannot open file "), strlen(COF));
            write(STDERR_FILENO, (fn), strlen(fn));
            write(STDERR_FILENO, ".\n", 2);

            exit(1);
        }
    }
    else {
       
        fp = stdin;
    }

    char line[512];
    while (1) {
        if (type == 0){
            printf("mysh> ");
            fflush(stdout);
        }
        char* check = fgets(line, sizeof(line), fp);
        if (check == NULL){
            if (type) {
                fclose(fp);
            }
            exit(0);
        }
        if (type){
            printf("%s", line);
            fflush(stdout);
        }
        char **args = splitLine(line);
        fflush(stdout);
        if (args[0] != NULL){
            if (!strncmp(DONE, args[0], 4)){
                exit(0);
            }
            else {
                execute(args);
            }
        }



        free(args);
    }
}

int execute(char **args) {
    int check = 1;
    if (args[0] == NULL) {
        return 1;
    }
    if (!strncmp(DONE, args[0], 4)) {
        exit(0);
    }
    int rc = fork();
    if (rc == 0) {
        if (args[0] == NULL) {
            return 1;
        }
        if (!strncmp(DONE, args[0], 4)) {
            exit(0);
        }
     char* fn;
     int m;
     int signal = 0;
        for (int i = 1; args[i] != NULL; i++){
            for (m = 0; m < strlen(args[i]); m++){
                if (args[i][m] == '>'){
                    signal = 1;
                    break;
                }
            }
            if (signal){
                char temp[m+1];
                if (m != 0){
                    strncpy(temp, args[i], m);
                    temp[m] = '\0';
                    (args[i])+=m;
                }
                
                if (strlen(args[i]) > 1){
                    if (args[i][1] == '>'){
                        fprintf(stderr, "Redirection misformatted.\n");
                        fflush(stderr);
                        exit(1);
                    }
                    else {
                        args[i]++;
                        fn = args[i];
                        if (args[i+1] != NULL){
                            fprintf(stderr, "Redirection misformatted.\n");
                            fflush(stderr);
                            exit(1);
                        }
                    }
                }
                else{
                    if (args[i+1] == NULL) {
                        fprintf(stderr, "Redirection misformatted.\n");
                        fflush(stderr);
                        exit(1);
                    }
                    if (args[i+1][0] == '>'){
                        fprintf(stderr, "Redirection misformatted.\n");
                        fflush(stderr);
                        exit(1);
                    }
                    
                    fn = args[i+1];
                    if (args[i+2] != NULL){
                        fprintf(stderr, "Redirection misformatted.\n");
                        fflush(stderr);
                        exit(1);
                    }
                    
                }
                if (m == 0){
                args[i] = NULL;
                }
                else {
                    args[i] = temp;
                }
                args[i+1] = NULL;
                close(STDOUT_FILENO);
                int fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0666);

                if (fd == -1){
                    fprintf(stderr, "Cannot write to file %s\n", fn);
                    fflush(stderr);
                    exit(1);
                }
                check = 0;
                execv(args[0], args);
                write(STDERR_FILENO, args[0], strlen(args[0]));
                write(STDERR_FILENO, job, strlen(job));
                _exit(1);


        }
        
    }
            
        
        

    
        if (check){
            

            execv(args[0], args);
            write(STDERR_FILENO, args[0], strlen(args[0]));
            write(STDERR_FILENO, job, strlen(job));
            _exit(1);

        }
    }      
    else if (rc > 0) {
        waitpid(rc, NULL , 0);
    }
    else {

    }
    

    return 0;
}
