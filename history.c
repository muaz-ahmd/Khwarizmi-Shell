#include "shell.h"

char *history[MAX_HISTORY];
int history_count = 0;
int history_synced_count = 0;

void add_to_history(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
}

void load_history_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        add_to_history(line);
    }
    fclose(f);
    history_synced_count = history_count;
}

void save_history_to_file(const char *filename, int append_mode) {
    FILE *f = fopen(filename, append_mode ? "a" : "w");
    if (!f) return;
    int start = (append_mode == 1) ? history_synced_count : 0;
    for (int i = start; i < history_count; i++) {
        fprintf(f, "%s\n", history[i]);
    }
    fclose(f);
    history_synced_count = history_count;
}
