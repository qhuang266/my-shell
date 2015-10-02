#ifndef COMMAND_H
#define COMMAND_H
#include <stddef.h>
#include <string.h>

#define MAX_ARG 50
#define MAX_LEN 1000

enum CMDTYPE
{
    REDIRECT_ERROR, // duplate input/output redirect
    COMMAND,
    REDIRECT_INPUT,
    REDIRECT_OUTPUT
};

class command
{
public:
	CMDTYPE type;
	command *next;
	command(CMDTYPE command_type) : type(command_type), next(NULL), arg_count(0) {};
	void setName(char *input_name);
	void setInputfile(char *filename);
	void setOutputfile(char *filename);
	void addArgument(char *arg);
    
    char *getName();
    char *getInputfile();
    char *getOutputfile();
    char **getArguments();
    char *getArguments_str();
    char *getCommand();

private:
	char *name;
	char *input_file;
	char *output_file;
	char *arguments[MAX_ARG];
	int arg_count;
};

#endif

