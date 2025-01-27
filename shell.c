#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h> 
#include<sys/wait.h>
#include<fcntl.h>

#define LSH_RS_BUFSIZE 1024;

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int builtinShellCmd() {
    return sizeof(builtin_str)/sizeof(char *);
}

char *readShell(void) {
    size_t size = LSH_RS_BUFSIZE;
    char *buffer = malloc(sizeof(char)*size);
    ssize_t length;

    if(!buffer) {
        fprintf(stderr, "Shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    length = getline(&buffer, &size, stdin);

    if(length == -1) {
        //EOF or error
        free(buffer);
        return NULL;
    }

    if(buffer[length-1] == '\n') {
        buffer[length-1] = '\0';
    }

    return buffer;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " "

char **parseLine(char* line) {
    int bufsize = LSH_TOK_BUFSIZE;
    int pos = 0;
    char **tokens = malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens) {
        fprintf(stderr, "Shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL) {
        tokens[pos] = token;
        pos++;

        if(pos >=  bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize*sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "Shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

void io_redirection(char **args) {
    for(int i=0; args[i]!=NULL; i++) {
        if(strcmp(args[i], ">") == 0) {
            int file_descriptor = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(file_descriptor == -1) {
                perror("Shell");        
                exit(EXIT_FAILURE);
            }
            dup2(file_descriptor, STDOUT_FILENO);
            close(file_descriptor);
            args[i] = NULL;
        } else if(strcmp(args[i], "<") == 0) {
            int file_descriptor = open(args[i+1], O_RDONLY);
            if(file_descriptor == -1) {
                perror("Shell");
                exit(EXIT_FAILURE);
            }
            dup2(file_descriptor, STDIN_FILENO);
            close(file_descriptor);
            args[i] = NULL;
        }
    }
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        //Child
        io_redirection(args);
        if(execvp(args[0], args) == -1) {
            perror("Shell");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("Shell");
    } else {
        //parent waiting for child
        // printf("%d", pid);
        do {
            wpid = waitpid(pid, &status, 0);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int shellExecute(char **args) {
    if(args[0] == NULL) {
        return 1;
    }

    for(int i=0; i<builtinShellCmd(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            switch (i) {
                case 0:
                    if (args[1] == NULL) {
                        fprintf(stderr, "Shell: expected argument to \"cd\"\n");
                    } else {
                    if (chdir(args[1]) != 0) {
                    perror("Shell");
                    }

                    return 1;
                }

                case 1:
                    printf("SHELL\n");
                    printf("Type program names and arguments, and hit enter.\n");
                    printf("Built in commands:\n");

                    for (int i = 0; i < builtinShellCmd(); i++) {
                        printf("  %s\n", builtin_str[i]);
                    }

                    return 1;

                case 2:
                    return 0;

                default:
                    fprintf(stderr, "Shell: unknown built-in command\n");
                    return 1;
            }
        }
    }
    //for non built in commands
    return shell_launch(args);
}


int main(int argc, char **argv) {
    char *line;
    char **args;
    int status;

    do {
        printf("Kushal's Custom Shell> ");
        line = readShell();
        args = parseLine(line);
        status = shellExecute(args);
        // printf("You have enterred: \n");
        // for(int i=0; args[i]!=NULL; i++) {
        //     printf("Token %d: %s\n", i, args[i]);
        // }65

        free(line);
        free(args);
    } while(status);


    return EXIT_SUCCESS;
}