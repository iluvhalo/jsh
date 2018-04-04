#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fields.h>

int main (int argc, char **argv) {
  int i;
  int status;
  char *prompt;
  char **newarg;
  IS is;

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
  printf("%s", prompt);
  while (get_line(is) != -1) {
    // if calling exit, closes the shell
    if (strcmp(is->fields[0], "exit") == 0) exit(0);
    
    newarg = (char **) malloc(sizeof(char *) * (is->NF + 1));

    // fills the newarg array with the arguments to pass to exec
    for (i = 0; i < is->NF; i++) {
      newarg[i] = strdup(is->fields[i]);
//      printf("copying %s from is->fields into newarg\n", newarg[i]);
    }
    // NULL terminator for newarg array
    newarg[is->NF] = NULL;

    // prints newarg to make sure it filled correctly
    for (i = 0; i < is->NF; i++) {
//      printf("newarg[%d]: %s\n", i, newarg[i]);
    }

    if(fork() == 0) {
      i = execvp(newarg[0], newarg);
//      printf("exec failed\n");
      perror(newarg[0]);
      exit(1);
    } else {
      wait(&status);
    }



    free(newarg);
    printf("%s", prompt);
  }
  
  jettison_inputstruct(is);
  free(prompt);
  return 0;
}
