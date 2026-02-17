#include <stdio.h>

int main(){
  printf("Hello world!");

  //run command loop...
  printf("Hello world!");
  
  //perform any shutdown/cleanup...
  
  return 0;
  }

// basically the main function -> loop, interpret commands etc
void psh_run_loop(void){

  // BASIC LOOP OF A SHELL:
  // 1. read -> read command from the standard input
  // 2. parse -> separate command string into a program and args
  // 3. execute -> run the parsed command
  
  char *line; // the string interpretation of the program
  char **args; // arguments of the program
  int status; // status = 0 => exit
  
  
  //more convenient because it executes once before checking the status value
  do{
    printf("> ");
    line = psh_read_line();
    args = psh_read_args(line);
    status = psh_execute(args);

    free(line);
    free(args);
  }
  while(status);

  return;
}
