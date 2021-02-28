#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXLINE 1024
#define clear() printf("\033[H\033[J")
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef enum Bool {true = 1, false = 0} bool;

void printShell();
bool input(char*);
int parseInput(char*, char**, char**);
int parsePipe(char*, char**);
void parseWords(char*, char**);
bool defCommand(char*, char**);
void run(char**);
void runWithPipe(char**, char**);

int main(int argc, char* argv[], char* envp[])
{
  clear();
  char inputLine[MAXLINE];
  char *parsedArgs[MAXLINE], *parsedPipes[MAXLINE];
  int type = 0;
  for(int i=0; i<1;)
  {
    printShell();
    if(input(inputLine))
    {
      type = parseInput(inputLine, parsedArgs, parsedPipes);
      if(type==1)
      {
        run(parsedArgs);
      }
      if(type==2)
      {
        runWithPipe(parsedArgs, parsedPipes);
      }
    }
  }
  return 0;
}

void printShell()
{
  char* username = getenv("USER");
  char pwd[100];
  getcwd(pwd, sizeof(pwd));
  printf(ANSI_COLOR_RED"%s@shell"ANSI_COLOR_RESET ANSI_COLOR_GREEN":"ANSI_COLOR_RESET ANSI_COLOR_BLUE"~%s"ANSI_COLOR_RESET, username, pwd);
}

bool input(char* str)
{
  char* buf;
  buf = readline("# ");
  if(strlen(buf)!=0)
  {
    add_history(buf);
    strcpy(str, buf);
    free(buf);
    return true;
  }
  else
  {
    return false;
  }
}

int parseInput(char* line, char** parsedArgs, char** parsedPipes)
{
  char* pipes[MAXLINE];
  int piped = parsePipe(line, pipes);
  if(piped>1)
  {
    parseWords(pipes[0], parsedArgs);
    parseWords(pipes[1], parsedPipes);
  }
  else
  {
    parseWords(line, parsedArgs);
  }
  if(defCommand(line, parsedArgs))
  {
    return 0;
  }
  return piped;
}

int parsePipe(char* line, char** cmd)
{
  int pipes = 0;
  for(; pipes<=1; ++pipes)
  {
    cmd[pipes] = strsep(&line, "|");
    if(!cmd[pipes])
    {
      break;
    }
    else if(!strlen(cmd[pipes]))
    {
      --pipes;
    }
  }
  return pipes;
}

void parseWords(char* line, char** words)
{
  for(int i=0; i<MAXLINE; ++i)
  {
    words[i] = strsep(&line, " ");
    if(!words[i])
    {
      break;
    }
    else if(!strlen(words[i]))
    {
      --i;
    }
  }
}

bool defCommand(char* line, char** parsedArgs)
{
  int i, choice = 0;
  char* list[4];
  char dir[MAXLINE] = {'\0'};
  list[0] = "exit";
  list[1] = "cd";
  list[2] = "help";
  list[3] = "clear";
  for(int i=0; i<4; ++i)
  {
    if(strcmp(parsedArgs[0], list[i])==0)
    {
      choice = i + 1;
      break;
    }
  }
  switch(choice)
  {
    case 1:
      printf("\nGoodbye :)\n");
      exit(0);
    case 2:
      strcpy(dir, parsedArgs[1]);
      for(int i=2; parsedArgs[i]!=NULL; ++i)
      {
        strcat(dir, " ");
        strcat(dir, parsedArgs[i]);
      }
      chdir(dir);
      return true;
    case 3:
      printf("version 2.0.0\nCreated by Blaze_Phoenix\n");
      return true;
    case 4:
      clear();
      return true;
    default:
      break;
  }
  return false;
}

void run(char** parsedArgs)
{
  if(!fork())
  {
    if(execvp(parsedArgs[0], parsedArgs)<0)
    {
      printf("Command not found\n");
    }
    exit(0);
  }
  else
  {
    wait(NULL);
  }
}

void runWithPipe(char** cmd1, char** cmd2)
{
  if(!fork())
  {
    int pipes[2];
    pipe(pipes);
    if(!fork())
    {
      close(1);
      dup(pipes[1]);
      close(pipes[0]);
      if(execvp(cmd1[0], cmd1)<0)
      {
        printf("Command not found\n");
      }
    }
    else
    {
      close(0);
      dup(pipes[0]);
      close(pipes[1]);
      if(execvp(cmd2[0], cmd2)<0)
      {
        printf("Command not found\n");
      }
    }
  }
  else
  {
    wait(NULL);
  }
}
