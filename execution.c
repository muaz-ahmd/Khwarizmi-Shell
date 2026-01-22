#include "shell.h"

int execute_command(char **g, redirection_t *redir) {
    if (g[0] == NULL) return 0;
    int status = 0;

    int original_stdout = -1, original_stderr = -1;
    if (redir->stdout_file) {
        original_stdout = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT | (redir->stdout_append ? O_APPEND : O_TRUNC);
        int fd = open(redir->stdout_file, flags, 0644);
        if (fd < 0) { perror("open stdout"); return 1; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (redir->stderr_file) {
        original_stderr = dup(STDERR_FILENO);
        int flags = O_WRONLY | O_CREAT | (redir->stderr_append ? O_APPEND : O_TRUNC);
        int fd = open(redir->stderr_file, flags, 0644);
        if (fd < 0) { perror("open stderr"); return 1; }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    if (strcmp(g[0], "exit") == 0) {
        if (g[1]) last_exit_status = atoi(g[1]);
        exit_shell = 1;
        return last_exit_status;
    } else if (strcmp(g[0], "echo") == 0) {
        status = echo(g); printf("\n");
    } else if (strcmp(g[0], "type") == 0) {
        if (g[1] == NULL) { printf("type: missing argument\n"); status = 1; }
        else {
            char fullpath[MAX_LINE];
            int is_builtin = 0;
            for (int i = 0; i < builtin_count; i++) {
                if (strcmp(g[1], cmdlist[i]) == 0) { is_builtin = 1; break; }
            }
            if (is_builtin) printf("%s is a shell builtin\n", g[1]);
            else if (pathcmd(g[1], fullpath, sizeof(fullpath))) printf("%s is %s\n", g[1], fullpath);
            else { printf("%s: not found\n", g[1]); status = 1; }
        }
    } else if (strcmp(g[0], "pwd") == 0) {
        status = pwd();
    } else if (strcmp(g[0], "cd") == 0) {
        status = cd(g[1]);
    } else if (strcmp(g[0], "history") == 0) {
        if (g[1] && (strcmp(g[1], "-r") == 0 || strcmp(g[1], "-w") == 0 || strcmp(g[1], "-a") == 0)) {
            if (g[2]) {
                if (strcmp(g[1], "-r") == 0) load_history_from_file(g[2]);
                else if (strcmp(g[1], "-w") == 0) save_history_to_file(g[2], 0);
                else if (strcmp(g[1], "-a") == 0) save_history_to_file(g[2], 1);
            } else status = 1;
        } else {
            int limit = history_count;
            if (g[1]) { limit = atoi(g[1]); if (limit > history_count) limit = history_count; }
            for (int i = history_count - limit; i < history_count; i++) printf("%5d  %s\n", i + 1, history[i]);
        }
    } else {
        char fullpath[MAX_LINE];
        if (strchr(g[0], '/') && access(g[0], X_OK) == 0) strcpy(fullpath, g[0]);
        else if (pathcmd(g[0], fullpath, sizeof(fullpath))) { /* found */ }
        else { fprintf(stderr, "%s: command not found\n", g[0]); return 127; }
        
        pid_t pid = fork();
        if (pid == 0) {
            execv(fullpath, g);
            perror("execv");
            exit(1);
        } else {
            int wait_status;
            waitpid(pid, &wait_status, 0);
            if (WIFEXITED(wait_status)) status = WEXITSTATUS(wait_status);
            else status = 1;
        }
    }

    if (original_stdout != -1) { dup2(original_stdout, STDOUT_FILENO); close(original_stdout); }
    if (original_stderr != -1) { dup2(original_stderr, STDERR_FILENO); close(original_stderr); }
    return status;
}
