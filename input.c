#include "shell.h"

struct termios orig_termios;
const char *cmdlist[] = {"echo", "exit", "type", "pwd", "cd", "history"};
int builtin_count = sizeof(cmdlist) / sizeof(cmdlist[0]);

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void find_completions(const char *prefix, char ***matches, int *match_count) {
    *matches = malloc(100 * sizeof(char *));
    *match_count = 0;
    size_t prefix_len = strlen(prefix);

    for (int i = 0; i < builtin_count; i++) {
        if (strncmp(prefix, cmdlist[i], prefix_len) == 0) {
            (*matches)[(*match_count)++] = strdup(cmdlist[i]);
        }
    }

    char *path_env = getenv("PATH");
    if (path_env) {
        char *path = strdup(path_env);
        char *dir_path = strtok(path, ":");
        while (dir_path) {
            DIR *d = opendir(dir_path);
            if (d) {
                struct dirent *dir;
                while ((dir = readdir(d)) != NULL) {
                    if (strncmp(prefix, dir->d_name, prefix_len) == 0) {
                        int exists = 0;
                        for (int k = 0; k < *match_count; k++) {
                            if (strcmp((*matches)[k], dir->d_name) == 0) { exists = 1; break; }
                        }
                        if (!exists) (*matches)[(*match_count)++] = strdup(dir->d_name);
                    }
                }
                closedir(d);
            }
            dir_path = strtok(NULL, ":");
        }
        free(path);
    }
    qsort(*matches, *match_count, sizeof(char *), compare_strings);
}

static void get_lcp(char **matches, int count, char *lcp) {
    if (count == 0) { lcp[0] = '\0'; return; }
    strcpy(lcp, matches[0]);
    for (int i = 1; i < count; i++) {
        int j = 0;
        while (lcp[j] && matches[i][j] && lcp[j] == matches[i][j]) j++;
        lcp[j] = '\0';
    }
}

int read_line(char *buffer, size_t size) {
    if (!isatty(STDIN_FILENO)) {
        if (!fgets(buffer, size, stdin)) return 0;
        buffer[strcspn(buffer, "\n")] = 0;
        return 1;
    }
    enable_raw_mode();
    size_t pos = 0;
    int tabs_pressed = 0;
    int current_history_index = history_count;
    char saved_current_line[MAX_LINE] = "";

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;
        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0'; printf("\n");
            disable_raw_mode(); return 1;
        } else if (c == '\x1b') {
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;
            if (seq[0] == '[') {
                if (seq[1] == 'A') { // Up
                    if (current_history_index > 0) {
                        if (current_history_index == history_count) { buffer[pos] = '\0'; strcpy(saved_current_line, buffer); }
                        current_history_index--;
                        while (pos > 0) { printf("\b \b"); pos--; }
                        strcpy(buffer, history[current_history_index]);
                        pos = strlen(buffer); printf("%s", buffer);
                    }
                } else if (seq[1] == 'B') { // Down
                    if (current_history_index < history_count) {
                        current_history_index++;
                        while (pos > 0) { printf("\b \b"); pos--; }
                        if (current_history_index == history_count) strcpy(buffer, saved_current_line);
                        else strcpy(buffer, history[current_history_index]);
                        pos = strlen(buffer); printf("%s", buffer);
                    }
                }
            }
        } else if (c == '\t') {
            buffer[pos] = '\0'; char **matches; int match_count;
            find_completions(buffer, &matches, &match_count);
            if (match_count > 0) {
                char lcp[MAX_LINE]; get_lcp(matches, match_count, lcp);
                size_t lcp_len = strlen(lcp);
                if (lcp_len > pos) {
                    for (size_t i = pos; i < lcp_len; i++) { printf("%c", lcp[i]); buffer[pos++] = lcp[i]; }
                    tabs_pressed = 0;
                    if (match_count == 1) { printf(" "); buffer[pos++] = ' '; }
                } else {
                    if (match_count == 1) { printf(" "); buffer[pos++] = ' '; tabs_pressed = 0; }
                    else {
                        tabs_pressed++;
                        if (tabs_pressed == 1) printf("\a");
                        else if (tabs_pressed == 2) {
                            printf("\n");
                            for (int i = 0; i < match_count; i++) printf("%s%s", matches[i], (i == match_count - 1) ? "" : "  ");
                            printf("\n$ %s", buffer);
                        }
                    }
                }
            } else printf("\a");
            for (int i = 0; i < match_count; i++) free(matches[i]);
            free(matches);
        } else if (c == 127 || c == 8) {
            if (pos > 0) { pos--; printf("\b \b"); }
        } else if (!iscntrl(c)) {
            if (pos < size - 1) { buffer[pos++] = c; printf("%c", c); }
        }
    }
    disable_raw_mode();
    return pos > 0 ? 1 : 0;
}
