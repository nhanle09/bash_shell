/*
    Name:   Nhan Le
    ID:     1000589528
*/

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line
#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports five arguments
#define HIST_SIZE 15			// History only display last 15 commands


static void sig_handler(int sig)
{
	if (sig == SIGTSTP)
	{
		return;
	}
}

int main(int argc, char *argv[])
{

	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

	// Store command history
	char *cmd_history[HIST_SIZE];
    memset(cmd_history, '\0', sizeof(cmd_history));
	int cmd_counter = 0;
	int cmd_counter_reset = 0;
	int pid_counter_reset = 0;

	// Store PID history
	int pid_history[HIST_SIZE];
	int pid_counter = 0;



	// Blocking Ctrl+C
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset( &sigmask, SIGINT);
	if (sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0)
	{
		perror("sigprocmask ");
	}

	// Handling Ctrl + Z
	struct sigaction sigact;
	memset(&sigact, '\0', sizeof(sigact));
	sigact.sa_handler = &sig_handler;
	if (sigaction(SIGTSTP, &sigact, NULL) == 0)
	{
		if (kill(pid_history[pid_counter], SIGTSTP) < 0)
		{
		}
	}

	while( 1 )
  	{
		int i = 0;
		// Paths of executables to be used later in execl
		char *path[50];
    	path[0] = "./";
    	path[1] = "/usr/local/bin/";
    	path[2] = "/usr/bin/";
    	path[3] = "/bin/";



    	// Print out the msh prompt
    	printf ("msh> ");

	    // Read the command from the commandline.  The
	    // maximum command that will be read is MAX_COMMAND_SIZE
	    // This while command will wait here until the user
	    // inputs something since fgets returns NULL when there
	    // is no input
	    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

	    /* Parse input */
	    char *token[MAX_NUM_ARGUMENTS];

	    int   token_count = 0;

		// Handle !n history if any
	    if (cmd_str[0] == '!')
		{
			char *str_remain;
			// Separate ! from history # value to obtain just the command
			int hist_id = strtol(strtok(cmd_str, "!"), &str_remain, 10);
			if ((hist_id >= HIST_SIZE) || (hist_id >= cmd_counter && cmd_counter_reset == 0))
			{
				printf("Command not in history\n");
				*cmd_str = '\0';
			}
			else
			{
				cmd_str = strndup(cmd_history[hist_id], MAX_COMMAND_SIZE);
			}
		}

	    // Pointer to point to the token
	    // parsed by strsep
	    char *arg_ptr;
	    char *working_str  = strdup( cmd_str );

	    // we are going to move the working_str pointer so
	    // keep track of its original value so we can deallocate
	    // the correct amount at the end
	    char *working_root = working_str;

	    // Tokenize the input strings with whitespace used as the delimiter
	    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
	              (token_count<MAX_NUM_ARGUMENTS))
	    {
	      	token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
		    if( strlen( token[token_count] ) == 0 )
		    {
		    	token[token_count] = NULL;
			}
		    token_count++;
	    }


	    // Record entered command into a "string" array
		cmd_history[cmd_counter++] = strndup(cmd_str, MAX_COMMAND_SIZE);


	    // Does nothing on blank input
		if (token[0] == NULL)
		{	}
	    // Exit msh on "quit" or "exit"
	    else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
	    {
	        free( working_root);
	        exit(0);
	    }
		// Show command history
	    else if (strcmp(token[0], "history") == 0)
	    {
			int high = 0;
			// Takes care of accounting problem when counter goes over 15
			// but still display previous 15 commands
			if (cmd_counter_reset == 0)
			{
				high = cmd_counter;
			}
			else
			{
				high = HIST_SIZE;
			}
	        for (i = 0; i < high; ++i)
	        {
	            printf("%d. %s", i, cmd_history[i]);
	        }
	    }
		// Show PIDs history
	    else if (strcmp(token[0], "listpids") == 0)
	    {
			int high = 0;
			// Takes care of accounting problem when counter goes over 15
			// but still display previous 15 PIDs
			if (pid_counter_reset == 0)
			{
				high = pid_counter;
			}
			else
			{
				high = HIST_SIZE;
			}
	        for (i = 0; i < high; ++i)
	        {
	            printf("%d. %d\n", i, pid_history[i]);
	        }
	    }
		// Using chdir() to change directory
		else if (strcmp(token[0], "cd") == 0)
		{
			if (chdir(token[1]) < 0)
			{
				perror("chdir: ");
			}
		}
		// Use bg to resume background process
		else if (strcmp(token[0], "bg") == 0)
		{
			if (kill(pid_history[pid_counter - 1], SIGCONT) < 0)
			{
				perror("kill: ");
			}
		}
		// Fork() and execute command
	    else
	    {
			pid_t pid_fork = fork();
			int status;
			// Store child pid into array
			if (pid_fork > 0)
			{
				pid_history[pid_counter++] = pid_fork;
			}
	        // Execute input command
			// Check if all paths are valid before error out
			if (pid_fork == 0)
			{
				char *filepath = strdup(path[0]);
				strcat(filepath, token[0]);
				if (execl(filepath, token[0], token[1], token[2], token[3],
					token[4], token[5], token[6], token[7], token[8], token[9], token[10],NULL) < 0)
				{
					filepath = strdup(path[1]);
					strcat(filepath, token[0]);
					if (execl(filepath, token[0], token[1], token[2], token[3],
						token[4], token[5], token[6], token[7], token[8], token[9], token[10],NULL) < 0)
					{
						filepath = strdup(path[2]);
						strcat(filepath, token[0]);
						if (execl(filepath, token[0], token[1], token[2], token[3],
							token[4], token[5], token[6], token[7], token[8], token[9],token[10], NULL) < 0)
						{
							filepath = strdup(path[3]);
							strcat(filepath, token[0]);
							if (execl(filepath, token[0], token[1], token[2], token[3],
								token[4], token[5], token[6], token[7], token[8], token[9], token[10],NULL) < 0)
							{
								perror(token[0]);
							}
						}
					}
				}
	            continue;
			}
			waitpid(pid_fork, &status, 0);
	    }
		// Takes care of accounting problem when counter goes over 15
		if (cmd_counter >= HIST_SIZE)
		{
			cmd_counter = 0;
			cmd_counter_reset++;
		}
		if (pid_counter >= HIST_SIZE)
		{
			pid_counter = 0;
			pid_counter_reset++;
		}
	    free( working_root );
	  }
	  return 0;
}
