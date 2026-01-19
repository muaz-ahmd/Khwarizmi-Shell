#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>

int pathcmd(const char *cmdused, char *result, size_t size);
int pwd();
int cd(char *direct);

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  char cmd[1024],cmdlist[][50]={"echo","exit","type","pwd","cd"};
  int builtin_count = sizeof(cmdlist) / sizeof(cmdlist[0]);
  int _isbuiltin=0,_ispathh=0;
  while (1) {                                                 //prompt prefix
    if (isatty(STDIN_FILENO)) {
        printf("$ ");
        fflush(stdout);
    }
    fflush(stdout);

    if (!fgets(cmd, sizeof(cmd), stdin)) {
    break;
    }

    /* STRIP NEWLINE FIRST */
    cmd[strcspn(cmd, "\n")] = '\0';

    /* THEN tokenize */
    char *g[10];
    int argcnt = 0;

    char *token = strtok(cmd, " ");
    while (token && argcnt < 9) {
        g[argcnt++] = token;
        token = strtok(NULL, " ");
    }
    g[argcnt] = NULL;
    // argv MUST end with NULL

    if(!strcmp(cmd, "exit")){                   //Command: exit
      break;
    }else if (strcmp(g[0], "echo") == 0) {      //Command: echo
        for (int i = 1; g[i] != NULL; i++) {
            printf("%s", g[i]);
            if (g[i + 1] != NULL) {
                printf(" ");
            }
        }
        printf("\n");
    }else if (strcmp(g[0], "type") == 0) {      //Command: type
          if (g[1] == NULL) {
              printf("type: missing argument\n");
              continue;
          }

          char *arg = g[1];
          char fullpath[1024];
          int is_builtin = 0;

          for (int i = 0; i < builtin_count; i++) {
              if (strcmp(arg, cmdlist[i]) == 0) {
                  is_builtin = 1;
                  break;
              }
          }

          if (is_builtin) {
              printf("%s is a shell builtin\n", arg);
          }
          else if (pathcmd(arg, fullpath, sizeof(fullpath))) {
              printf("%s is %s\n", arg, fullpath);
          }
          else {
              printf("%s: not found\n", arg);
          }
      }else if (strcmp(g[0],"pwd")==0){            //Command: pwd
        pwd();
      }else if (strcmp(g[0],"cd")==0){
        cd(g[1]);
      }
      else {
        char fullpath[1024];

        if (strchr(g[0], '/') && access(g[0], X_OK) == 0) {
            strcpy(fullpath, g[0]);
        }
        else if (pathcmd(g[0], fullpath, sizeof(fullpath))) {
            // found in PATH
        }
        else {
            printf("%s: command not found\n", g[0]);
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execv(fullpath, g);
            perror("execv");
            exit(1);
        }
        else if (pid > 0) {
            wait(NULL);
        }
        else {
            perror("fork");
        }
    }

  }

  return 0;
}

int pathcmd(const char *cmdused, char *result, size_t size){
  char *pathget=getenv("PATH");
  char *path=strdup(pathget);
  char *dir=strtok(path,":");
  while(dir){
    snprintf(result, size, "%s/%s", dir, cmdused);
    char fullpath[1024];
    snprintf(fullpath,sizeof(fullpath),"%s/%s",dir,cmdused);
    if(access(fullpath, X_OK)==0){
      free(path);
      return 1;
    }
    dir=strtok(NULL,":");
  }
  free(path);
  return 0;
}

int pwd(){
    char fullpath[1024];
    char *pointer;
    pointer=getcwd(fullpath, sizeof(fullpath));
    if(pointer!=0){
        printf("%s\n", fullpath);
    }else{
    perror("cwd() error:");
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

