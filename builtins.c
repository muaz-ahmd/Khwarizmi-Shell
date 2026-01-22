#include "shell.h"

int echo(char **args) {
    int first = 1;
    for (int i = 1; args[i] != NULL; i++) {
        if (!first) putchar(' ');
        first = 0;
        printf("%s", args[i]);
    }
    return 0;
}

int pwd() {
    char fullpath[MAX_LINE];
    if (getcwd(fullpath, sizeof(fullpath)) != NULL) {
        printf("%s\n", fullpath);
    } else {
        perror("getcwd() error");
        return 1;
    }
    return 0;
}

int cd(char *direct) {
    if (direct == NULL || strcmp(direct, "~") == 0) {
        direct = getenv("HOME");
        if (direct == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    if (chdir(direct) != 0) {
        printf("cd: %s: No such file or directory\n", direct);
        return 1;
    }
    return 0;
}
