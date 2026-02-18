#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <string.h>
#include <synchapi.h>
#include <unistd.h>
#include <windows.h>
#include <stdlib.h>

// MAIN FUNCTIONS
void psh_run_loop(void);
char *psh_read_line(
    void); // TODO switch to a better implementation -> getline or something.
char **psh_read_args(char *line);
int psh_launch(int argc, char **args);
int psh_execute(int argc, char **args);

// OTHER FUNCTIONS
int count_args(char **args) {
  int cnt = 0;
  while (args[cnt] != NULL)
    ++cnt;

  return cnt;
}

int main() {
  // run config files (if any)...

  // run command loop...
  psh_run_loop();

  // perform any shutdown/cleanup...

  return 0;
}

// basically the main function -> loop, parse, execute
void psh_run_loop(void) {
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

#define PSH_LINE_BUFSIZE 4096
// reads the content and returns it as a string
char *psh_read_line(void) {
  int bufsize = PSH_LINE_BUFSIZE;
  char *line = malloc(sizeof(char) * bufsize);
  
  if(!line){
    fprintf(stderr, "psh: some allocation error occured\n");
    exit(EXIT_FAILURE);
  }
  
  if(fgets(line, bufsize, stdin) == NULL){
    if(feof(stdin))
      exit(EXIT_SUCCESS); // we received an EOF
    else{
      perror("read_line");
      exit(EXIT_FAILURE);
    }
  }

  line[strcspn(line, "\n")] = '\0';
  return line;
}

#define PSH_TOKENS_BUFSIZE 64
#define PSH_TOKEN_DELIMITER " \t\r\n\a"
// tokenizes the input according to a delimiter (a space in my case, for simplicity)
char **psh_read_args(char *line) {
  int bufsize = PSH_TOKENS_BUFSIZE, position = 0;
  char **tokens = malloc(sizeof(char *) * bufsize);
  char *token = strtok(line, PSH_TOKEN_DELIMITER);

  if (!tokens) {
    fprintf(stderr, "psh: some allocation error occured\n");
    exit(EXIT_FAILURE);
  }

  while (token != NULL) {
    tokens[position++] = token;

    if (position >= bufsize) // we have more than 64 args by any chance
    {
      bufsize += PSH_TOKENS_BUFSIZE;
      tokens = realloc(tokens, sizeof(char *) * bufsize);

      if (!tokens) {
        fprintf(stderr, "psh: some allocation error occured\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, PSH_TOKEN_DELIMITER);
  }

  tokens[position] = NULL;
  return tokens;
}

#define PSH_CMD_BUFSIZE 1024
// used to launch a program, not a built-in!
int psh_launch(int argc, char **args) {
  // execvp is unix only, so i'll use functions from windows.h
  // first I need to build a string separated by space using the args
  int bufsize = PSH_CMD_BUFSIZE;
  size_t cmd_size = 0;
  char *cmd = malloc(sizeof(char) * bufsize);
  cmd[0] = '\0';

  // transform the arguments into a single string command
  // TODO make a separate function for this
  for (int i = 0; i < argc; ++i) {
    cmd_size += strlen(args[i]) + 1;
    if (cmd_size + 1 >= bufsize) {
      bufsize += PSH_CMD_BUFSIZE;
      cmd = realloc(cmd, sizeof(char) * bufsize);

      if (!cmd) {
        fprintf(stderr, "psh: some allocation error occured\n");
        exit(EXIT_FAILURE);
      }
    }

    strcat(cmd, args[i]); // TODO find a better way to do this
    strcat(cmd, " ");
  }
  cmd[cmd_size] = '\0';
  printf("%s\n", cmd); //debugging..

  // execute the command using CreateProcess
  // TODO understand this part
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi;
  si.cb = sizeof(si);

  if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    printf("Failed to start process. Error: %lu\n", GetLastError());
    return 0;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD exit_code;
  GetExitCodeProcess(pi.hProcess, &exit_code);
  printf("Process exited with code %lu\n", exit_code);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  free(cmd);
  return 1;
}

int psh_execute(int argc, char **args) {
  psh_launch(argc, args);
  return 1;
}
