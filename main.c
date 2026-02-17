#include <handleapi.h>
#include <minwinbase.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include <unistd.h>
#include <windows.h>

// MAIN FUNCTIONS
void psh_run_loop(void);
char *psh_read_line(void); // TODO switch to a better implementation -> getline or something.
char **psh_read_args(char *line);
int psh_launch(int argc, char **args);
int psh_execute(int argc, char **args);

// OTHER FUNCTIONS
int count_args(char** args){
  int cnt = 0;
  while(args[cnt] != NULL)
    ++cnt;

  return cnt;
}


int main(int argc, char **args) {
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
    status = psh_execute(count_args(args), args);

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

#define PSH_CMD_BUFSIZE 1024
// used to launch a program, not a built-in!
int psh_launch(int argc, char **args){
  // execvp is unix only, so i'll use functions from windows.h
  // first I need to build a string separated by space using the args
  int bufsize = PSH_RL_BUFSIZE, cmd_size = 0;
  char *cmd = malloc(sizeof(char) * bufsize);

  // transform the arguments into a single string command 
  for(int i = 0; i < argc; ++i){
    cmd_size += strlen(args[i]);
    
    if(cmd_size >= bufsize){
      bufsize += PSH_CMD_BUFSIZE;
      cmd = realloc(cmd, sizeof(char) * bufsize);

      if(!cmd){
        fprintf(stderr, "psh: some allocation error occured\n");
        exit(EXIT_FAILURE);
      }
    }

    strcat(cmd, args[i]);
    cmd[cmd_size++] = ' ';
  }
  cmd[cmd_size] = '\0';

  //execute the command using CreateProcess
  //TODO understand this part better + comment it out
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if(!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
    fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
    exit(EXIT_FAILURE);
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  free(cmd);
  return 1;
}

int psh_execute(int argc, char **args){

  return 1;
}
