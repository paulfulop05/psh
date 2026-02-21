#include <handleapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include <unistd.h>
#include <windows.h>

// MAIN FUNCTIONS
void psh_run_loop(void);
char *psh_read_line(void);
char **psh_read_args(char *line);
int psh_launch(char **args);
int psh_execute(char **args);

// OTHER FUNCTIONS
int count_args(char **args);
char *transform_command(char **args);
int builtins_count();

// BUILTINS
// return the exit code
// I need char **args for arguments
// => builtin function signature: int funcName(char**) -> therefore I can use function pointers
int psh_cd(char **args);
int psh_help(char **args);
int psh_exit(char **args);

// List of built-in commands followed by their corresponding functions
char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char**) = {psh_cd, psh_help, psh_exit};

int main() {
  // run config files (if any)...

  // run command loop...
  psh_run_loop();

  // perform any shutdown/cleanup...

  return 0;
}

// MAIN FUNCTIONS IMPLEMENTATION

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
    status = psh_execute(args);

    free(line);
    free(args);
  } while (status);
}

#define PSH_LINE_BUFSIZE 4096
// reads the content and returns it as a string
char *psh_read_line(void) {
  int bufsize = PSH_LINE_BUFSIZE;
  char *line = malloc(sizeof(char) * bufsize);

  if (!line) {
    fprintf(stderr, "psh: some allocation error occured\n");
    exit(EXIT_FAILURE);
  }

  if (fgets(line, bufsize, stdin) == NULL) {
    if (feof(stdin))
      exit(EXIT_SUCCESS); // we received an EOF
    else {
      perror("read_line");
      exit(EXIT_FAILURE);
    }
  }

  line[strcspn(line, "\n")] = '\0';
  return line;
}

#define PSH_TOKENS_BUFSIZE 64
#define PSH_TOKEN_DELIMITER " \t\r\n\a"
// tokenizes the input
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

// used to launch a program, not a built-in!
// execvp is unix only, so it's going to use functions from windows.h
int psh_launch(char **args) {
  char *cmd = transform_command(args);

  // execute the command using CreateProcess
  STARTUPINFO si = {0}; // use default startup behaviour
  PROCESS_INFORMATION pi;
  si.cb = sizeof(si);

  if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    printf("Failed to start process. Error code: %lu\n", GetLastError());
    return 0;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD exit_code; // saying DWORD ensures the same size is used everywhere, no
                   // matter what compiler is being used
  GetExitCodeProcess(pi.hProcess, &exit_code);
  printf("Process exited with code %lu\n", exit_code);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  free(cmd);
  return 1;
}

int psh_execute(char **args) {
  psh_launch(args);
  return 1;
}

// IMPLEMENTATION FOR BUILTIN FUNCTIONS

int psh_cd(char **agrs){
  return 1;
}

int psh_help(char **args){
  printf("Paul Fulop's shell\n");
  printf("Type program names followed by arguments, hit enter and enjoy\n");
  printf("The following comamnds are built-in:\n");

  int bt_count = builtins_count();
  for(int i = 0; i < bt_count; ++i){
    printf("%s\n", builtin_str[i]);
  }

  return 1;
}

int psh_exit(char **args){
  return 0; // this is exactly all I need here :))
}


// IMPLEMENTATION FOR OTHER FUNCTIONS
//counts the number of arguments
int count_args(char **args) {
  int cnt = 0;
  while (args[cnt] != NULL)
    ++cnt;

  return cnt;
}

#define PSH_CMD_BUFSIZE 1024
// transforms the list of arguments into a string
char *transform_command(char **args) {
  int bufsize = PSH_CMD_BUFSIZE, argc = count_args(args);
  size_t cmd_size = 0;
  char *cmd = malloc(sizeof(char) * bufsize);

  cmd[0] = '\0';
  for (int i = 0; i < argc; ++i) {
    size_t arglen = strlen(args[i]);
    if (cmd_size + arglen + 1 >= bufsize) {
      bufsize += PSH_CMD_BUFSIZE;
      cmd = realloc(cmd, sizeof(char) * bufsize);

      if (!cmd) {
        fprintf(stderr, "psh: some allocation error occured\n");
        exit(EXIT_FAILURE);
      }
    }

    strcpy(cmd + cmd_size, args[i]);
    cmd_size += arglen;
    cmd[cmd_size] = ' ';
    cmd[cmd_size + 1] = '\0';
    ++cmd_size; // incrementing because I also append a space at the end, the
                // terminator comes after
  }

  printf("%s\n", cmd); // debugging..
  return cmd;
}

// returns the amoutn of builtins I currently have
int builtins_count(){
  return sizeof(builtin_str) / sizeof(char *);
}
