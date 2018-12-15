// to use the shell you have to compile it like $ gcc shell_name.c -L/usr/include -lreadline -o shell_name
// and also you need to get the readline lib using $ brew install readline or $ apt-get install ibreadline-dev

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
  
#define MAX_LETTERS 1000
#define MAX_COMMANDS 100

// just a simple ascii clear used for a clean console :)
#define clear() printf("\033[H\033[J")
  
int shell_input(char* str) 
{ 
    char* buffer; 
  
    buffer = readline("\n$> "); 
    if (strlen(buffer) != 0) { 
        add_history(buffer);
        strcpy(str, buffer);
        return 0; 
    } else { 
        return 1; 
    } 
} 
  
void shell_exec(char** parsed) 
{ 
    pid_t threadid = fork();  
  
    if (threadid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (threadid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else { 
        wait(NULL);  
        return; 
    } 
} 
  
// help panel
void shell_help() 
{ 
    puts("\nType program names and arguments, and hit enter."
        "\nThe following are built in\n"
        "\n> UNIX general commands"
        "\n> help"
        "\n> version"
        "\n> cd"
        "\n> ls"
        "\n> exit"
        "\n> pipes"); 
  
    return; 
} 
  
// simple function to ease the declaration of new functions
int shell_cmd_handler(char** parsed) 
{ 
    int cmds = 5, i, arg_switcher = 0; 
    char* custom_funcs[cmds]; 
    char* user; 
  
    custom_funcs[0] = "exit"; 
    custom_funcs[1] = "cd"; 
    custom_funcs[2] = "help"; 
    custom_funcs[3] = "hello"; 
    custom_funcs[4] = "version";
  
    for (i = 0; i < cmds; i++) { 
        if (strcmp(parsed[0], custom_funcs[i]) == 0) { 
            arg_switcher = i + 1; 
            break; 
        } 
    } 
  
    switch (arg_switcher) { 
    case 1: 
        printf("\nGoodbye\n"); 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        shell_help(); 
        return 1; 
    case 4: 
        user = getenv("USER"); 
        printf("\nHello %s.\nStop playing arround! ""\nUse help to know more..\n", 
            user); 
        return 1;
    case 5:
        printf("\nv 1.0\n");
        return 1; 
    default: 
        break; 
    } 
  
    return 0; 
} 
  
// function for finding pipe 
int shell_pipe_parser(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return 1; 
    } 
} 
  
// the pipe function
void shell_exec_pipe(char** parsed, char** parsed_pipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t thread1, thread2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    thread1 = fork(); 
    if (thread1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (thread1 == 0) { 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        thread2 = fork(); 
  
        if (thread2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        if (thread2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsed_pipe[0], parsed_pipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 
  
// function for parsing command words 
void shell_space_parser(char* str, char** parsed) 
{ 
    int i; 
  
    for (i = 0; i < MAX_COMMANDS; i++) { 
        parsed[i] = strsep(&str, " "); //space as separator
  
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  
int shell_string_processor(char* str, char** parsed, char** parsed_pipe) 
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = shell_pipe_parser(str, strpiped); 
  
    if (piped) { 
        shell_space_parser(strpiped[0], parsed); 
        shell_space_parser(strpiped[1], parsed_pipe); 
  
    } else { 
  
        shell_space_parser(str, parsed); 
    } 
  
    if (shell_cmd_handler(parsed)) 
        return 0; 
    else
        return 1 + piped; 
} 
  
int main() 
{ 
    char string_input[MAX_LETTERS], *parsed_args[MAX_COMMANDS]; 
    char* parsed_args_pipe[MAX_COMMANDS]; 
    int exec_flag = 0; 
    static int first_time = 1;
        if(first_time){
            clear();
            printf("Use help!\n");
            first_time = 0;
        }
    while (1) {
        // take input 
        if (shell_input(string_input)) 
            continue; 
        // process 
        exec_flag = shell_string_processor(string_input, 
        parsed_args, parsed_args_pipe); 
  
        // execute 
        if (exec_flag == 1) 
            shell_exec(parsed_args); 
  
        if (exec_flag == 2) 
            shell_exec_pipe(parsed_args, parsed_args_pipe); 
    } 
    return 0; 
} 