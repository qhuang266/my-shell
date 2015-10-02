#include "command.h"

void command::setName(char *input_name)
{
	name = input_name;
}
void command::setInputfile(char *filename)
{
	input_file = filename;
}
void command::setOutputfile(char *filename)
{
	output_file = filename;
}
void command::addArgument(char *arg)
{
	arguments[arg_count++] = arg;
}

char *command::getName()
{
    return name;
}

char *command::getInputfile()
{
    return input_file;
}

char *command::getOutputfile()
{
    return output_file;
}

char **command::getArguments()
{
    return arguments;
}

char *command::getArguments_str()
{
    char *arguments_str = new char[MAX_LEN];
    int str_ptr = 0;
    for (int i = 0; i < arg_count; ++i)
    {
        int arg_ptr = 0;
        while (arguments[i][arg_ptr] != '\0')
        {
            arguments_str[str_ptr] = arguments[i][arg_ptr];
            ++str_ptr;
            ++arg_ptr;
        }
    }
    arguments_str[str_ptr] = '\0';
    return arguments_str;
}

char *command::getCommand()
{
    char *command_str = new char[MAX_LEN];
    strcpy(command_str, name);
    char *ptr = command_str;
    while (*ptr != '\0')
        ++ptr;
 //   strcpy(ptr, getArguments());
    return command_str;
}
