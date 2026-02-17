#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void psh_run_loop(void);
char *psh_read_line(
    void); // TODO switch to a better implementation -> getline or something.
char **psh_read_args(char *line);
int psh_execute(char **args);

int main(int argc, char **argv) {
  // load config files, if any.
  // ...

  // run command loop...
  psh_run_loop();

  // perform any shutdown/cleanup...

  return 0;
}

// basically the main function -> loop, interpret commands etc
void psh_run_loop(void) {

  // BASIC LOOP OF A SHELL:
  // 1. read -> read command from the standard input
  // 2. parse -> separate command string into a program and args
  // 3. execute -> run the parsed command

  char *line;  // the string interpretation of the program
  char **args; // arguments of the program
  int status;  // status = 0 => exit

  // more convenient because it executes once before checking the status value
  do {
    printf("> ");
    line = psh_read_line();
    args = psh_read_args(line);
    status = psh_execute(args);

    free(line);
    free(args);
  } while (status);
}

#define PSH_RL_BUFSIZE 1024
// reads the content and returns it as a string
char *psh_read_line(void) {
  int bufsize = PSH_RL_BUFSIZE;
  int position = -1;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c = getchar(); // current character (using int because I need to check EOF
                     // as well, which is an integer)

  if (!buffer) {
    fprintf(stderr, "psh: some alloation error occured.\n");
    exit(EXIT_FAILURE);
  }

  // the main idea is to read a char until we hit EOF or \n. If the bufsize is
  // exceeded, we double its size.
  while (c != EOF && c != '\n') {
    c = getchar();

    if (position >= bufsize) {
      bufsize += PSH_RL_BUFSIZE;
      buffer = realloc(buffer, sizeof(char) * bufsize);

      if (!buffer) {
        fprintf(stderr, "psh: some alloation error occured.\n");
        exit(EXIT_FAILURE);
      }
    }

    buffer[++position] = c;
  }

  buffer[++position] = '\0';
  return buffer; // no need to deallocate, that happens in the loop function
}

#define PSH_TOKENS_BUFSIZE 64
#define PSH_TOKEN_DELIMITER " \t\r\n\a"
// tokenizes the input according to a delimiter (a space in my case, for simplicity)
char **psh_read_args(char *line) {
  int bufsize = PSH_TOKENS_BUFSIZE, position = -1;
  char **tokens = malloc(sizeof(char *) * bufsize);
  char *token = strtok(line, PSH_TOKEN_DELIMITER);

  if (!tokens) {
    fprintf(stderr, "psh: some allocation error occured\n");
    exit(EXIT_FAILURE);
  }

  while (token != NULL) {
    tokens[++position] = token;

    if (position >= bufsize) // we have more than 64 args by any chance
    {
      bufsize += PSH_RL_BUFSIZE;
      tokens = realloc(tokens, sizeof(char*) * bufsize);

      if (!tokens) {
        fprintf(stderr, "psh: some allocation error occured\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, PSH_TOKEN_DELIMITER);
  }

  tokens[++position] = NULL;
  return tokens;
}
