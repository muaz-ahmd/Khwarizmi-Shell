#include "shell.h"

int pathcmd(const char *cmdused, char *result, size_t size) {
    char *pathget = getenv("PATH");
    if (!pathget) return 0;
    char *path = strdup(pathget);
    char *dir = strtok(path, ":");
    while (dir) {
        snprintf(result, size, "%s/%s", dir, cmdused);
        if (access(result, X_OK) == 0) {
            free(path);
            return 1;
        }
        dir = strtok(NULL, ":");
    }
    free(path);
    return 0;
}

int parser(const char *cmd, char *argv[MAX_ARGS], redirection_t *redir) {
    static char buf[MAX_LINE];
    int argc = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;
    char *w = buf;
    char *arg_start = NULL;

    redir->stdout_file = NULL;
    redir->stdout_append = 0;
    redir->stderr_file = NULL;
    redir->stderr_append = 0;

    char *last_arg = NULL;
    int expecting_stdout = 0;
    int expecting_stderr = 0;

    for (int i = 0; cmd[i] != '\0' && cmd[i] != '\n'; i++) {
        char c = cmd[i];

        if (in_single_quote) {
            if (c == '\'') in_single_quote = 0;
            else {
                if (!arg_start) arg_start = w;
                *w++ = c;
            }
        } else if (in_double_quote) {
            if (c == '\\') {
                char next = cmd[i + 1];
                if (next == '"' || next == '\\' || next == '$' || next == '`') {
                    i++;
                    if (!arg_start) arg_start = w;
                    *w++ = cmd[i];
                } else {
                    if (!arg_start) arg_start = w;
                    *w++ = c;
                }
            } else if (c == '"') in_double_quote = 0;
            else {
                if (!arg_start) arg_start = w;
                *w++ = c;
            }
        } else {
            if (c == '\\') {
                i++;
                if (cmd[i] != '\0' && cmd[i] != '\n') {
                    if (!arg_start) arg_start = w;
                    *w++ = cmd[i];
                }
            } else if (c == '\'') {
                in_single_quote = 1;
                if (!arg_start) arg_start = w;
            } else if (c == '"') {
                in_double_quote = 1;
                if (!arg_start) arg_start = w;
            } else if (c == ' ') {
                if (arg_start) {
                    *w++ = '\0';
                    last_arg = arg_start;
                    arg_start = NULL;

                    if (expecting_stdout) {
                        redir->stdout_file = last_arg;
                        expecting_stdout = 0;
                    } else if (expecting_stderr) {
                        redir->stderr_file = last_arg;
                        expecting_stderr = 0;
                    } else if (strcmp(last_arg, ">") == 0 || strcmp(last_arg, "1>") == 0) {
                        expecting_stdout = 1;
                        redir->stdout_append = 0;
                    } else if (strcmp(last_arg, ">>") == 0 || strcmp(last_arg, "1>>") == 0) {
                        expecting_stdout = 1;
                        redir->stdout_append = 1;
                    } else if (strcmp(last_arg, "2>") == 0) {
                        expecting_stderr = 1;
                        redir->stderr_append = 0;
                    } else if (strcmp(last_arg, "2>>") == 0) {
                        expecting_stderr = 1;
                        redir->stderr_append = 1;
                    } else if (argc < MAX_ARGS - 1) {
                        argv[argc++] = last_arg;
                    }
                }
            } else {
                if (!arg_start) arg_start = w;
                *w++ = c;
            }
        }
    }

    if (arg_start) {
        *w++ = '\0';
        last_arg = arg_start;
        if (expecting_stdout) redir->stdout_file = last_arg;
        else if (expecting_stderr) redir->stderr_file = last_arg;
        else if (argc < MAX_ARGS - 1) argv[argc++] = last_arg;
    }
    argv[argc] = NULL;

    if (in_single_quote || in_double_quote) {
        fprintf(stderr, "syntax error: unterminated quote\n");
        return -1;
    }
    return argc;
}
