#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <ctype.h>

/* --- Constants --- */
#define MAX_HISTORY 500
#define MAX_LINE 1024
#define MAX_ARGS 10
#define MAX_PIPES 9

/* --- Types --- */
typedef struct {
    char *stdout_file;
    int stdout_append;
    char *stderr_file;
    int stderr_append;
} redirection_t;

/* --- Builtin Commands --- */
extern const char *cmdlist[];
extern int builtin_count;

int echo(char **args);
int pwd();
int cd(char *direct);

/* --- Parser --- */
int parser(const char *cmd, char *argv[MAX_ARGS], redirection_t *redir);
int pathcmd(const char *cmdused, char *result, size_t size);

/* --- History & Input --- */
extern char *history[MAX_HISTORY];
extern int history_count;
extern int history_synced_count;

void add_to_history(const char *cmd);
void load_history_from_file(const char *filename);
void save_history_to_file(const char *filename, int append_mode);
int read_line(char *buffer, size_t size);

/* --- Terminal Mode --- */
void enable_raw_mode();
void disable_raw_mode();

/* --- Execution --- */
extern int last_exit_status;
extern int interactive;
extern int exit_shell;
int execute_command(char **g, redirection_t *redir);
int process_line(char *line);

#endif
