#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fields.h>

int main (int argc, char **argv) {
  char *prompt;

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

  printf("The prompt is -- '%s'\n", prompt);

  

  free(prompt);
  return 0;
}
