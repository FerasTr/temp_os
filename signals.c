#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/fcntl.h>

pid_t pid_index[5];
int global_index = 0;
pid_t ppid;

void sigCatcher(int signo)
{
  printf("PID %d caught one\n", getpid());

  kill(pid_index[global_index], SIGINT);
  global_index++;

  exit(0);
}

void BurnThemAll(int signo)
{
  for (int i = 0; i < 5; i++)
  {
    kill(pid_index[i], SIGTERM);
    printf("Process %d is dead\n", pid_index[i]);
    sleep(1);
  }
}

int main()
{
  ppid = getpid();
  for (int i = 0; i < 5; i++)
  {
    pid_t pid = fork();
    // If failed to create a new process.
    if (pid < 0)
    {
      // Print message to the error stream.
      fprintf(stderr, "Failed at creating a new process.");
      return 1;
    }
    /* Parent */
    else if (pid > 0)
    {
      signal(SIGINT, BurnThemAll);
      pid_index[i] = pid;
    }
    /* Child */
    else
    {
      signal(SIGINT, sigCatcher);
      printf("PID %d ready\n", getpid());
      pause();
    }
  }
  sleep(1);

  kill(pid_index[global_index], SIGINT);
  pause();
  exit(0);
}