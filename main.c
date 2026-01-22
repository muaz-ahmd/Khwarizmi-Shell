#include "shell.h"

int last_exit_status = 0;
int interactive = 0;
int exit_shell = 0;

int process_line(char *line) {
    if (strlen(line) == 0) return last_exit_status;
    if (interactive) add_to_history(line);

    char *commands[MAX_ARGS];
    int cmd_count = 0;
    char *start = line;
    int in_s_quote = 0, in_d_quote = 0;
    for (int i = 0; line[i]; i++) {
        if (line[i] == '\'' && !in_d_quote) in_s_quote = !in_s_quote;
        else if (line[i] == '"' && !in_s_quote) in_d_quote = !in_d_quote;
        else if (line[i] == '|' && !in_s_quote && !in_d_quote) {
            line[i] = '\0';
            commands[cmd_count++] = start;
            start = &line[i+1];
        }
    }
    commands[cmd_count++] = start;

    if (cmd_count == 1) {
        char *g[MAX_ARGS]; redirection_t redir;
        int argc_p = parser(commands[0], g, &redir);
        if (argc_p <= 0 && redir.stdout_file == NULL && redir.stderr_file == NULL) return last_exit_status;
        last_exit_status = execute_command(g, &redir);
    } else {
        int pipes[MAX_PIPES][2];
        for (int i = 0; i < cmd_count - 1; i++) { if (pipe(pipes[i]) < 0) { perror("pipe"); return 1; } }
        for (int i = 0; i < cmd_count; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
                if (i < cmd_count - 1) dup2(pipes[i][1], STDOUT_FILENO);
                for (int j = 0; j < cmd_count - 1; j++) { close(pipes[j][0]); close(pipes[j][1]); }
                char *g[MAX_ARGS]; redirection_t redir;
                parser(commands[i], g, &redir);
                exit(execute_command(g, &redir));
            }
        }
        for (int i = 0; i < cmd_count - 1; i++) { close(pipes[i][0]); close(pipes[i][1]); }
        int status;
        for (int i = 0; i < cmd_count; i++) {
            int wait_status;
            wait(&wait_status);
            if (i == cmd_count - 1) {
                if (WIFEXITED(wait_status)) last_exit_status = WEXITSTATUS(wait_status);
                else last_exit_status = 1;
            }
        }
    }
    return last_exit_status;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    char cmd[MAX_LINE];
    char *histfile = getenv("HISTFILE");
    FILE *input = stdin;
    interactive = isatty(STDIN_FILENO);

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) { perror("fopen"); return 1; }
        interactive = 0;
    }

    if (interactive && histfile) load_history_from_file(histfile);

    while (!exit_shell) {
        if (interactive) { printf("$ "); fflush(stdout); }
        
        if (interactive) {
            if (!read_line(cmd, sizeof(cmd))) break;
        } else {
            if (fgets(cmd, sizeof(cmd), input) == NULL) break;
            cmd[strcspn(cmd, "\n")] = 0;
            // Handle shebang on first line
            static int first_line = 1;
            if (first_line) {
                first_line = 0;
                if (strncmp(cmd, "#!", 2) == 0) continue;
            }
        }

        process_line(cmd);
    }

    if (interactive && histfile) save_history_to_file(histfile, 1);
    if (input != stdin) fclose(input);
    return last_exit_status;
}
