#include <stdio.h>
#include <stdlib.h>

void psh_run_loop(void);
char *psh_read_line(void);
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
  int c = getchar(); // current character

  if (!buffer) {
    fprintf(stderr, "psh: some alloation error occured.");
    exit(EXIT_FAILURE);
  }

  // the main idea is to read a char until we hit EOF or \n. If the bufsize is
  // exceeded, we double its size.
  while (c != EOF && c != '\n') {
    c = getchar();

    if (position >= bufsize) {
      bufsize += PSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);

      if (!buffer) {
        fprintf(stderr, "psh: some alloation error occured.");
        exit(EXIT_FAILURE);
      }
    }

    buffer[++position] = c;
  }
  
  buffer[++position] = '\0';
  return buffer; // no need to deallocate, that happens in the loop function
}
