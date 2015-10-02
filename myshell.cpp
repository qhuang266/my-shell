/**
 * myshell 
 * @author: QianHuang
 * about input command:
 * 1. no backtick `
 * 2. no append redirect >> <<
 * 3. no 
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "command.h"

#define ARGV_MAX 100
#define MAX_LEN 256

enum STATE
{
	ONE_SPACE,
	INPUT_REDIRECT,
	OUTPUT_REDIRECT,
	NORMAL,
	PIPE
};


// print current path
void printPath()
{
	char current_path[MAX_LEN];
	char user[MAX_LEN];
	char hostname[MAX_LEN];
	getcwd(current_path, MAX_LEN);
	getlogin_r(user, MAX_LEN);
	gethostname(hostname, MAX_LEN);
	printf("%s@%s %s$ ", user, hostname, current_path);
}

// format the input command， delete or add some space, make it easier to be tokenized
void format(char *input)
{
	char tmp[MAX_LEN];
	strcpy(tmp, input);
	int new_ptr = 0, tmp_ptr = 0;
	STATE status = NORMAL;
	while (tmp[tmp_ptr] != '\0')
	{
		switch(status)
		{
			case NORMAL:
			{
				if (tmp[tmp_ptr] == ' ')
					status = ONE_SPACE;
				else if (tmp[tmp_ptr] == '<')
				{
					input[new_ptr++] = ' ';
					status = INPUT_REDIRECT;
				}
				else if (tmp[tmp_ptr] == '>')
				{
					input[new_ptr++] = ' ';
					status = OUTPUT_REDIRECT;
				}
				else if (tmp[tmp_ptr] == '|')
				{
					input[new_ptr++] = ' ';
					status = PIPE;
				}
				input[new_ptr] = tmp[tmp_ptr];
				++tmp_ptr;
				++new_ptr;
				break;
			}
			case ONE_SPACE:
			{
				if (tmp[tmp_ptr] == ' ')
					++tmp_ptr;
				else
				{
					if (tmp[tmp_ptr] == '>')
					{
						if (new_ptr > 2 && (input[new_ptr - 2] == '1' || input[new_ptr - 2] == '2' || input[new_ptr - 2] == '0') && input[new_ptr - 3] == ' ')
							new_ptr -= 1;
						status = OUTPUT_REDIRECT;
					}
					else if (tmp[tmp_ptr] == '<')
					{
						if (new_ptr > 2 && (input[new_ptr - 2] == '1' || input[new_ptr - 2] == '2' || input[new_ptr - 2] == '0') && input[new_ptr - 3] == ' ')
							new_ptr -= 1;
						status = INPUT_REDIRECT;
					}
					else if (tmp[tmp_ptr] == '|')
					{
						status = PIPE;
					}
					else
						status = NORMAL;
					input[new_ptr] = tmp[tmp_ptr];
					++tmp_ptr;
					++new_ptr;
				} 
				break;
			}
			case PIPE:
			{
				if (tmp[tmp_ptr] == ' ')
					++tmp_ptr;
				else
				{
					input[new_ptr++] = ' ';
					input[new_ptr] = tmp[tmp_ptr];
					++tmp_ptr;
					++new_ptr;
				}
				break;
			}
			case OUTPUT_REDIRECT:
			case INPUT_REDIRECT:
			{
				if (tmp[tmp_ptr] == ' ')
					++tmp_ptr;
				else
					status = NORMAL;
				break;
			}
		}
	}
	input[new_ptr] = '\0';
}

void deleteCommandList(command *head)
{
	command *ptr = head->next;
	while (ptr)
	{
		head->next = ptr->next;
		delete ptr;
		ptr = head->next;
	}
	delete head;
}


command *parse(char *input)
{
	command *dummy = new command(COMMAND);
	command *tail = dummy;
	command *node_pointer = nullptr;
	char *token = strtok(input, " ");
	while (token != nullptr)
	{
		// redirect first
		if (token[0] == '<' || token[0] == '>' || ((token[0] == '0' || token[0] == '1' || token[0] == '2') && (token[1] == '<' || token[1] == '>')))
		{
			// check whether there is duplicate redirect already
			if (dummy->next)
			{
				command *ptr = dummy->next;
				while (ptr && (ptr->type == REDIRECT_INPUT || ptr->type == REDIRECT_OUTPUT))
				{
					if ((ptr->type == REDIRECT_INPUT && token[0] == '<') || (ptr->type == REDIRECT_OUTPUT && token[0] == '>'))
					{ 
						deleteCommandList(dummy);
						return new command(REDIRECT_ERROR);
					}
				}
			}
			// check follow with filename
			if (strlen(token) == 1) 
			{
				deleteCommandList(dummy);
				return new command(REDIRECT_ERROR);
			}
			else
			{
				if (node_pointer != nullptr)
				{
					tail->next = node_pointer;
					tail = tail->next;
					node_pointer = nullptr;
				}
				if (token[0] == '>')
				{
					++token;
					command *new_node = new command(REDIRECT_OUTPUT);
					new_node->setOutputfile(token);
					new_node->next = dummy->next;
					dummy->next = new_node;
				}
				else if (token[0] == '<')
				{
					++token;
					command *new_node = new command(REDIRECT_INPUT);
					new_node->setInputfile(token);
					new_node->next = dummy->next;
					dummy->next = new_node;
				}
				else if (token[1] == '>')
				{
					char *output_file = token + 2;
					token[1] = '\0';
					command *new_node = new command(REDIRECT_INPUT);
					new_node->setInputfile(token);
					new_node->next = dummy->next;
					dummy->next = new_node;
					new_node = new command(REDIRECT_OUTPUT);
					new_node->setOutputfile(output_file);
					new_node->next = dummy->next;
					dummy->next = new_node;
				}
				else
				{
					char *input_file = token + 2;
					token[1] = '\0';
					command *new_node = new command(REDIRECT_OUTPUT);
					new_node->setInputfile(token);
					new_node->next = dummy->next;
					dummy->next = new_node;
					new_node = new command(REDIRECT_INPUT);
					new_node->setOutputfile(input_file);
					new_node->next = dummy->next;
					dummy->next = new_node;
				}
			}
		}

		else if (token[0] == '|')
		{
			if (node_pointer != nullptr)
			{
                node_pointer->addArgument("\0");
				tail->next = node_pointer;
				tail = tail->next;
				node_pointer = nullptr;
			}
		}

		else
		{
			if (node_pointer == nullptr)
			{
				node_pointer = new command(COMMAND);
				node_pointer->setName(token);
			}
			else
			{
				node_pointer->addArgument(token);
			}
		}
		token = strtok(nullptr, " ");
	}
    if (node_pointer != nullptr)
    {
        node_pointer->addArgument("\0");
        tail->next = node_pointer;
    }
	return dummy->next;
}

void executeInputRedirect(command *ptr)
{
	int fd = open(ptr->getInputfile(), O_RDONLY, 0);
	dup2(fd, 0);
	close(fd);
}

void executeOutputRedirect(command *ptr)
{
	int fd = creat(ptr->getOutputfile(), 0644);
	dup2(fd, 1);
	close(fd);
}

bool isBuildin(char *command_name)
{
    if (strcmp(command_name, "cd") == 0 )
        return true;
    // ...
    else
        return false;
}

void executeCd(command *ptr)
{
    int result = chdir(ptr->getArguments_str());
    if (result < 0)
    {
    	perror("can't do cd command");
    }
}

void executeBuildin(command *ptr)
{
    if (strcmp(ptr->getName(), "cd") == 0)
        executeCd(ptr);
}

void execute(command *ptr)
{
    pid_t pid = fork();
    int status;
    if (pid < 0)
    {
    	perror("error to fork a new process");
    	_exit(1);
    }
    else if (pid == 0)
    {
    	if (execvp(ptr->getName(), ptr->getArguments()) < 0)
    	{
    		perror("something wrong when execute process");
    		_exit(1);
    	}
    }
    else
    {
    	while (wait(&status) != pid)
    		;
    }
}


int main(int argc, char const *argv[])
{
	char *input_line = nullptr;
	char *arg_vector[ARGV_MAX];
	size_t line_length = 0;
	while (true)
	{
		printPath();
		getline(&input_line, &line_length, stdin);
		format(input_line);
		command *cmd_list = parse(input_line);
		
		if (cmd_list == nullptr) // empty input
			continue;
		else if (strcmp(cmd_list->getName(), "exit") == 0) // exit
			return 0;
		else
        {
            command *ptr = cmd_list;
            while (ptr != nullptr)
            {
                if (ptr->type == REDIRECT_ERROR)
                {
                    printf("duplicate redirect error!\n");
                    break;
                }
                else if (ptr->type == REDIRECT_INPUT)
                    executeInputRedirect(ptr);
                else if (ptr->type == REDIRECT_OUTPUT)
                	executeOutputRedirect(ptr);
                else if (isBuildin(ptr->getName()))
                    executeBuildin(ptr);
                else
                    execute(ptr);
                ptr = ptr->next;
            }
        }
	}
	return 0;
}
