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


/**
 * @brief This program is a unix shell that is supposed to mimic the functions of the base linux shell, with two changes. One is that it can read input from
 * a file instead of from the command line itself. The second is that it can redirect the output of the commands(either from command line or from console) to 
 * a seperate output file. 
 * @author Nathaniel Israel
 * 
 */
*/

/**
 * @brief This function starts the shell, and figures out what mode it is 
 * 
 * @param argc number of inputs(if it is batch, then 2, interactive is 1)
 * @param argv the program name and the name of the file to write it to(in interactive)
 */
int main(int argc, char* argv[]) {
    if (argc > 2){
        write(STDERR_FILENO, ICMD, strlen(ICMD));
        exit(1);
    }
    int type = 0;
    char *fn; //fn= file name
    if (argc < 2) {
//interactive
//if it is interactive mode, then the program will continually take input from the user. If it is in batch mode, the program will read input from an input file that is given. 
    }
    else {
        type = 1;
        fn = argv[1]; //batch
    }
    mode(fn, type);
    exit(0);
}


/**
 * @brief this function breaks down the string of whatever is given, so linux can understand it. 
 * 
 * @param line 
 * @return char** 
 */
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
    ret[i] = NULL; //sets the last character to null
    return ret;
}

/**
 * @brief This function sets the file pointer to either the input file or stdin, and then takes in and executes the command. 
 * 
 * mode 1 is batch, 0 is interactive
 * @param fn 
 * @param type 
 */
void mode(char* fn, int type) {
    FILE *fp;
    if (type) { //batch mode
        fp = fopen(fn, "r");
        if (fp == NULL) {

            write(STDERR_FILENO, ("Error: Cannot open file "), strlen(COF));
            write(STDERR_FILENO, (fn), strlen(fn));
            write(STDERR_FILENO, ".\n", 2);

            exit(1);
        }
    }
    else {
       // reading from prompt
        fp = stdin;
    }

    char line[512];
    while (1) {
        if (type == 0){ //assuming it is in interactive mode
            printf("mysh> ");
            fflush(stdout);
        }
        char* check = fgets(line, sizeof(line), fp);
        if (check == NULL){ //if there's no more input. 
            if (type) {
                fclose(fp);  //closes the file
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

/**
 * @brief This function starts a new process. The user can decide to redirect the output to a file by adding a > after the command. The shell exits 
 * when it sees either then end of a command line or the string "exit"
 * 
 * @param args that user is inputing, received from function mode()
 */
int execute(char **args) {
    int check = 1; //makes sure it doesn't execute twice
    if (args[0] == NULL) {
        return 1;
    }
    if (!strncmp(DONE, args[0], 4)) {
        exit(0);
    }
    int rc = fork();
    if (rc == 0) { //child process
        if (args[0] == NULL) {
            return 1;
        }
        if (!strncmp(DONE, args[0], 4)) {
            exit(0);
        }
     char* fn;
     int m;
     int signal = 0; //signal is checking if there is the command to redirect
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
                //formatting the command so it can be executed
                if (m == 0){
                args[i] = NULL;
                }
                else {
                    args[i] = temp;
                }
                args[i+1] = NULL;
                close(STDOUT_FILENO);
                int fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0666); //writes output to file instead of stdout. 

                if (fd == -1){
                    fprintf(stderr, "Cannot write to file %s\n", fn);
                    fflush(stderr);
                    exit(1);
                }
                check = 0;
                execv(args[0], args);
                write(STDERR_FILENO, args[0], strlen(args[0])); //if there's an error
                write(STDERR_FILENO, job, strlen(job)); //if there's an error
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
        waitpid(rc, NULL , 0); //waiting for child process to finish
    }
    else {

    }
    

    return 0;
}
