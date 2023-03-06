// Mason Stott
// COSC 360; Dr. Huang
// Lab 8: jsh pt 3
// 11/8/21

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "fields.h"

// This function handles redirecting input and output to/from a file
int dup_stream(char *operator, char *fname)
{
  int stream, rv = 1;

  // Input from a file
  if (strcmp(operator, "<") == 0)
  {
    stream = open(fname, O_RDONLY);
    if (dup2(stream, 0) != 0) exit(1);
  }
  // Output to a file
  else if (strcmp(operator, ">") == 0)
  {
    stream = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dup2(stream, 1) != 1) exit(1);
  }
  // Output to end of file
  else if (strcmp(operator, ">>") == 0)
  {
    stream = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (dup2(stream, 1) != 1) exit(1);
  }
  // If we're not redirecting, just return 0
  else rv = 0;
  // Close the stream
  close(stream);
  // Return 1 if we redirected (rv is 1 by default)
  return rv;
}

// This function exists solely so my main() doesn't look so cluttered
void do_execvp(int index, IS is, int *status)
{
  int i, j;
  // Allocate the commands
  char **tasks = malloc(sizeof(char *) * is->NF + 1);

  j = 0;
  // Loop for redirecting i/o
  for (i = index; is->fields[i] != NULL; i++)
  {
    if (dup_stream(is->fields[i], is->fields[i + 1]) == 0)
    {
      tasks[j] = is->fields[i];
      j++;
    }
    else i++;
  }
  // Need to keep the output for wait()
  *status = execvp(is->fields[index], tasks);
  free(tasks);
  // Exit on error
  perror(is->fields[index]);
  exit(1);
}

int main(int argc, char **argv)
{
  char *prompt;
  int do_prompt = 1, end, fork_proc, curr_pipe[2], prev_pipe[2], num_pipes;
  int i = 0, j = 0, s;
  IS is = new_inputstruct(NULL);

  // Determine the input prompt
  if (argc == 2) 
  {
    if (strcmp(argv[1], "-") != 0) prompt = strdup(argv[1]);
    else do_prompt = 0;
  }
  else prompt = strdup("jsh1");

  // The fake shell
  while (1)
  {
    end = 0;
    // Display the prompt
    if (do_prompt) printf("%s: ", prompt);
    // End the loop if there is no more input
    if (get_line(is) == -1) break;
    // If no commands, just reprompt
    if (is->NF == 0) continue;
    // Exit command
    if (strcmp(is->fields[0], "exit") == 0) break;
    // Check for end of commands (&)
    if (strcmp(is->fields[is->NF - 1], "&") == 0) end = 1;

    // Get ready to store the command indices
    // By declaring here, we reset the array every loop
    int command_indices[10];
    // First command will start at 0
    command_indices[0] = 0;
    // Use j to keep track of the command indices
    j = 1;
    // Get the command indices by checking for |
    for (i = 0; i < is->NF; i++)
    {
      if (strcmp(is->fields[i], "|") == 0)
      {
        is->fields[i] = NULL;
        command_indices[j] = i + 1;
        j++;
      }
    }
    num_pipes = j - 1;

    // Set end of commands to NULL
    if (end == 1) is->fields[is->NF - 1] = NULL;
    else is->fields[is->NF] = NULL;

    // Loop through the commands and execute them
    for (i = 0; i < j; i++)
    {
      // Pipe for the next command
      if (i != num_pipes) pipe(curr_pipe);

      // Create the fork
      fork_proc = fork();

      if (fork_proc == 0)
      {
        if (i != 0)
        {
          // Pipe from the previous command and close
          if (dup2(prev_pipe[0], 0) != 0) { perror(is->fields[command_indices[i]]); exit(1); } 
          close(prev_pipe[1]);
          close(prev_pipe[0]);
        }
    
        if (i != num_pipes)
        {
          // Pipe to the next command and close
          close(curr_pipe[0]); 
          if (dup2(curr_pipe[1], 1) != 1) { perror(is->fields[command_indices[i]]); exit(1); }
          close(curr_pipe[1]);
        }

        // Call the exec handler
        do_execvp(command_indices[i], is, &s);
      }
      else 
      {
        // Close/store the pipes as necessary
        if (i != 0) { close(prev_pipe[1]); close(prev_pipe[0]); }
        if (i != num_pipes)
        {
          prev_pipe[0] = curr_pipe[0];
          prev_pipe[1] = curr_pipe[1];
        }
        // Wait if necessary
        if (end == 0) while(1) if (fork_proc == wait(&s)) break;
      }
    }
  }
  // Memory cleanup
  free(prompt);
  jettison_inputstruct(is);

  return 0;
}