/* 
   Jonas da Silva
   119289334
   jdasilva
*/
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "command.h"
#include "executor.h"

/*struct tree {
  enum { NONE = 0, AND, OR, SEMI, PIPE, SUBSHELL } conjunction;
  struct tree *left, *right;
  char **argv;
  char *input;
  char *output;
  }


static void print_tree(struct tree *t);*/

int execute_aux(struct tree *t, int parent_input_fd, int parent_output_fd){
  int pipe_fd[2], status, fd;
  pid_t pid, pid2;

  if(!t){
    return 0;
  }

  if(t->conjunction == NONE){
    if(strcmp(t->argv[0], "exit") == 0){
      exit(0);
    }
    else if(strcmp(t->argv[0], "cd") == 0){
      if(t->argv[1] == NULL){
        chdir(getenv("HOME"));
      }
      else{
        chdir(t->argv[1]);
      }
    }
    else{
      pid = fork();
      if(pid < 0){
        err(EX_OSERR, "fork error");
      }

      if(!pid){
        if(t->input){
          if ((fd = open(t->input, O_RDONLY)) < 0) {
            err(EX_OSERR, "File opening failed");
          } 
          if (dup2(fd, STDIN_FILENO) < 0) { /* Now stdin is associated with file */
            err(EX_OSERR, "dup2 (read) failed");
          }
          if (close(fd) < 0) { /* We need it otherwise resource leak */
          err(EX_OSERR, "close error");
          }
        }

        if(t->output){
          if ((fd = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
              err(EX_OSERR, "File opening (write) failed");
          }
          if (dup2(fd, STDOUT_FILENO) < 0) { /* Now stdout is associated with file */
              err(EX_OSERR, "dup2 (write) failed");
          } 
          if (close(fd) < 0) { /* We need it otherwise resource leak */
              err(EX_OSERR, "close error");
            }
        }

        execvp(t->argv[0], (char * const *)t->argv);
        /*printf("Failed to execute %s\n", t->argv[0]);*/
        fflush(stdout);
        exit(EX_OSERR);
      }
      else{
        wait(&status);
        if((WIFEXITED(status)) && WEXITSTATUS(status) == 0) {
	      return 0; 
	      } 
        else {
	        return 1;
	      }
      }
    }
  }
  else if(t->conjunction == SUBSHELL){
    pid = fork();
    if(!pid){
      execute_aux(t->left, parent_input_fd, parent_output_fd);
      exit(0);
    }
    else{
      wait(NULL);
    }
  }
  else if(t->conjunction == AND){
    if(execute_aux(t->left, parent_input_fd, parent_output_fd) || 
    execute_aux(t->right, parent_input_fd, parent_output_fd)) {
      return 1;
    }
  }
  else{
    if(t->left->output){
      printf("Ambiguous output redirect.\n");
    }
    
    else if(t->right->input){
      printf("Ambiguous input redirect.\n");
    }
    else{
      if(pipe(pipe_fd) < 0){
        err(EX_OSERR, "pipe error");
      }
      pid = fork();
      if(!pid){
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]); 
        execute_aux(t->left, parent_input_fd, parent_output_fd);
        exit(0);
      }
      else{
        pid2 = fork();
        if(!pid2){
          close(pipe_fd[1]);
          dup2(pipe_fd[0], STDIN_FILENO);
          close(pipe_fd[0]);
          execute_aux(t->right, parent_input_fd, parent_output_fd);
          exit(0);
        }
        else{
          close(pipe_fd[0]);
          close(pipe_fd[1]);
          wait(NULL);
          wait(NULL);
        }
      }
    }
  }
  return 0;
}



int execute(struct tree *t) {

  /*print_tree(t);*/ 

  return execute_aux(t, STDIN_FILENO, STDOUT_FILENO);
}

/*static void print_tree(struct tree *t) {
  if (t != NULL) {
    print_tree(t->left);

    if (t->conjunction == NONE) {
      printf("NONE: %s, ", t->argv[0]);
    } else {
      printf("%s, ", conj[t->conjunction]);
    }
    printf("IR: %s, ", t->input);
    printf("OR: %s\n", t->output);

    print_tree(t->right);
  }
}*/

