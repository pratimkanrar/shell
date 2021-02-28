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

typedef enum Bool {true = 1, false = 0} bool;

void printShell();
bool input(char*);
int parseInput(char*, char**, char**);
int parsePipe(char*, char**);
void parseWords(char*, char**);
bool defCommand(char**);
void run(char**, char**);
void runWithPipe(char**, char**, char**);

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
        run(parsedArgs, envp);
      }
      if(type==2)
      {
        runWithPipe(parsedArgs, parsedPipes, envp);
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
  printf("%s@shell:~%s", username, pwd);
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
  if(defCommand(parsedArgs))
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

bool defCommand(char** parsedArgs)
{
  int i, choice = 0;
  char* list[4];
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
        chdir(parsedArgs[1]);
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

void run(char** parsedArgs, char** envp)
{
  if(fork()==0)
  {
    if(execve(parsedArgs[0], parsedArgs, envp)<0)
    {
      char pref[50] = {'/', 'b', 'i', 'n', '/'};
      strcat(pref, parsedArgs[0]);
      if(execve(pref, parsedArgs, envp)<0)
      {
        printf("Command not found\n");
      }
    }
    exit(0);
  }
  else
  {
    wait(NULL);
  }
}

void runWithPipe(char** cmd1, char** cmd2, char** envp)
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
      if(execve(cmd1[0], cmd1, envp)<0)
      {
        char pref[50] = {'/', 'b', 'i', 'n', '/'};
        strcat(pref, cmd1[0]);
        if(execve(pref, cmd1, envp)<0)
        {
          printf("Command not found\n");
        }
      }
    }
    else
    {
      close(0);
      dup(pipes[0]);
      close(pipes[1]);
      if(execve(cmd2[0], cmd2, envp)<0)
      {
        char pref[50] = {'/', 'b', 'i', 'n', '/'};
        strcat(pref, cmd2[0]);
        if(execve(pref, cmd2, envp)<0)
        {
          printf("Command not found\n");
        }
      }
    }
  }
  else
  {
    wait(NULL);
  }
}
