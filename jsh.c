#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jval.h>
#include <jrb.h>
#include <fields.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main (int argc, char **argv) {
  int i, j, k;
  int status;
  int amp;
  int pid;
  int size;
  int fd;
  int newargSize;
  char *prompt;
  char **newarg;
  IS is;
  JRB cList;

  // checks for correct usage
  if (argc > 2) {
    printf("usage: ./jsh [optional command prompt]\n");
    exit(1);
  }

  // assigns the prompt given the command line arguments
  // ./jsh             --> 'jsh: '
  // ./jsh [anything]  --> '[anything]'
  // ./jsh -           --> '' <-- (no prompt)
  if (argc == 2) {
    if (strcmp(argv[1], "-") == 0) {
      prompt = malloc(sizeof(char) * 1);
      strcpy(prompt, "\0");
    } else {
      prompt = malloc(sizeof(char) * strlen(argv[1]));
      strcpy(prompt, argv[1]);
    }
  } else {
    prompt = malloc(sizeof(char) * strlen("jsh: "));
    strcpy(prompt, "jsh: ");
  }

  //  printf("The prompt is -- '%s'\n", prompt);
  // this is the main loop of the shell
  is = new_inputstruct(NULL);
  cList = make_jrb();
beginningOfLoop:
  printf("%s", prompt);
  while (get_line(is) != -1) {
    if (is->NF == 0) goto beginningOfLoop;
    // if calling exit, closes the shell
    // also added q, just for a faster way for when I'm testing
    if ((strcmp(is->fields[0], "exit") == 0) || (strcmp(is->fields[0], "q") == 0)) exit(0);

    // checks if there is a '&' at the end of the command
    amp = 0;
    if (strcmp(is->fields[is->NF - 1], "&") == 0) {
      //      printf("there is an &\n");
      amp = 1;
    }
    
    //printf("is->NF: %d\n", is->NF);
/*
    // allocates the coreect amount of args for newargs
    if (amp) {
      newarg = (char **) malloc(sizeof(char *) * is->NF);
    } else {
      newarg = (char **) malloc(sizeof(char *) * (is->NF + 1));
    }
*/
    // 
    for (j = 0; j < is->NF; j++) {
      if ((strcmp(is->fields[j], ">") == 0) || (strcmp(is->fields[j], "<") == 0) || (strcmp(is->fields[j], ">>") == 0) || (strcmp(is->fields[j], "&") == 0)) break;
    }
    newargSize = j;

    newarg = (char **) malloc(sizeof(char *) * (newargSize+1));
    
    for (i = 0; i < newargSize; i++) {
      newarg[i] = strdup(is->fields[i]);
    }
    newarg[i] = NULL;

    /*
    // fills the newarg array with the arguments to pass to exec
    if  (!amp) {
      for (i = 0; i < is->NF; i++) {
        if (strcmp(is->fields[i], ">") == 0) break;
        if (strcmp(is->fields[i], "<") == 0) break;
        if (strcmp(is->fields[i], ">>") == 0) break;
        newarg[i] = strdup(is->fields[i]);
      }
      // NULL terminator for newarg array
      newarg[i] = NULL;
    } else {
      for (i = 0; i < (is->NF - 1); i++) {
        if (strcmp(is->fields[i], ">") == 0) break;
        if (strcmp(is->fields[i], "<") == 0) break;
        if (strcmp(is->fields[i], ">>") == 0) break;
        newarg[i] = strdup(is->fields[i]);
      }
      newarg[i] = NULL;
    }*/

    // prints newarg to make sure it filled correctly
    for (i = 0; i < newargSize+1; i++) {
      //printf("newarg[%d]: %s  ", i, newarg[i]);
    }
    //printf("\n");

    // fork and exec
    pid = fork();
    if(pid == 0) {

      // find when the file redirection commands start
      j = 0;
      for (i = 0; i < is->NF; i++) {
        if ((strcmp(is->fields[i], ">") == 0) || (strcmp(is->fields[i], "<") == 0) || (strcmp(is->fields[i], ">>") == 0)) {
          j = i;
          break;
        }
      }
      j = 0;

//      printf("starting file redirection\n");
//      printf("is->text1: %s", is->text1);
      // set up file redirection
      while (is->fields[j] != NULL) {
//        printf("is->fields[%d]: %s\n", j, is->fields[j]);
        if (strcmp(is->fields[j], "<") == 0) {
//          printf("redirect %s to stdin\n", is->fields[j+1]);
          fd = open(is->fields[j+1], O_RDONLY);
          if (fd < 0) {
            perror(is->fields[j+1]);
            exit(1);
          }
          if (dup2(fd, 0) != 0) {
            perror("jsh: dup2(fd, 0)");
            exit(1);
          }

          j++;
          close(fd);
        }
        if (strcmp(is->fields[j], ">") == 0) {
//          printf("redirect stdout to %s\n", is->fields[j+1]);
          fd = open(is->fields[j+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
          if (fd < 0) {
            perror(is->fields[j+1]);
            exit(1);
          }
          if (dup2(fd, 1) != 1) {
            perror("jsh: dup2(fd, 1)");
            exit(1);
          }

          j++;
          close(fd);
        }
        if (strcmp(is->fields[j], ">>") == 0) {
//          printf("appending to %s\n", is->fields[j+1]);
          fd = open(is->fields[j+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
          if (fd < 0) {
            perror(is->fields[j+1]);
            exit(1);
          }
          if (dup2(fd, 1) != 1) {
            perror("jsh: dup2(fd, 1)");
            exit(1);
          }

          j++;
          close(fd);
        }
        j++;
      }
//      printf("finished file redirection\n");
      jrb_insert_int(cList, pid, new_jval_i(pid));
      i = execvp(newarg[0], newarg);
      perror(newarg[0]);
      exit(1);
    } else {
      if (!amp) {
        do {
          i = wait(&status);
          if (i != pid) {
            if (!jrb_empty(cList)) jrb_delete_node(jrb_find_int(cList, i));
          }
        } while (i != pid);
      }
    }
    
    free(newarg);
    printf("%s", prompt);
  }

  jettison_inputstruct(is);
  free(prompt);
  return 0;
}
