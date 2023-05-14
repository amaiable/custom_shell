#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void run_shell();
char* read_cmd();
char** parse(char* command);
bool execute(char** cmd_args);
bool shell_cd(char** args);
bool shell_ls(char** args);
bool shell_help(char** args);
bool shell_exit(char** args);
bool start_process(char** cmd_args);
bool shell_mkdir(char** args);

int main(int argc, char** argv) {
    
    run_shell();

    return 0;
}

void run_shell() {

    char* command;
    char** cmd_args;
    bool to_continue;

    while (true) {

        command = read_cmd();
        cmd_args = parse(command);
        to_continue = execute(cmd_args);

        if (!to_continue) {
            break;
        }
    }

    return;
}

char* read_cmd() {
    char* command = NULL;
    ssize_t command_size = 0;
    printf("> ");
    if (getline(&command, &command_size, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }
        else {
            perror("Readline failed");
            exit(EXIT_FAILURE);
        }
    }
    return command;
}

char** parse(char* command) {
    int arg_size = 1000;
    char** cmd_args = malloc(sizeof(char*) * arg_size);
    int args_helper = 0;
    char* arg;

    char* token = strtok(command, " \t\r\n\a");

    while (token != NULL) {
        if (args_helper >= arg_size) {
            arg_size *= 2;
            cmd_args = realloc(cmd_args, sizeof(char*) * arg_size);
        }

        cmd_args[args_helper++] = token;

        token = strtok(NULL, " \t\r\n\a");
    }

    cmd_args[args_helper] = NULL;
    return cmd_args;
}

bool execute(char** cmd_args) {
    if (cmd_args[0] == NULL) {
        return true;
    }

    if (strcmp(cmd_args[0], "cd") == 0) {
        return shell_cd(cmd_args);
    }
    else if (strcmp(cmd_args[0], "ls") == 0) {
        return shell_ls(cmd_args);
    }
    else if (strcmp(cmd_args[0], "mkdir") == 0) {
        return shell_mkdir(cmd_args);
    }

    return start_process(cmd_args);
}

bool start_process(char** cmd_args) {
    pid_t pid, wpid;
    int to_continue;

    pid = fork();
    if (pid == 0) {
        perror("pid == 0");
    }
    else if (pid < 0) {
        perror("pid < 0");
    }
    else {
        while (true) {
            wpid = waitpid(pid, &to_continue, WUNTRACED);
            if (WIFEXITED(to_continue) || WIFSIGNALED(to_continue)) {
                break;
            }
        }
    }
    return true;
}

bool shell_cd(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "No directory to travel to provided");
    }
    else {
        int change_res = chdir(args[1]);
        if (change_res != 0) {
            perror("cd Argument error");
        }
    }
    return true;
}

bool shell_ls(char** args) {
    char* dir = ".";
    struct dirent* d;
    DIR* dh = opendir(dir);
    while ((d = readdir(dh)) != NULL) {
        printf("%s ", d->d_name);
    }
    printf("\n");
    return true;
}

bool shell_mkdir(char** args) {
    char* dir = ".";
    char* new_dir = args[1];
    struct dirent* d;
    DIR* dh = opendir(dir);
    while ((d = readdir(dh)) != NULL) {
        if (d->d_name == new_dir) {
            return true;
        }
    }
    return mkdir(args[1], 0777) == 0;
}
