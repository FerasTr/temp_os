#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include "md5.h"

using std::cout;
using std::endl;
using std::string;

// Boolean flag for checking if the string is encrypted.
bool is_enc_string = false;
// The encryped string is inserted into a global string.
char answ[33];

// Signal handler, used to check if the child encrypted the string.
void sig_handler(int signo)
{
    // Check if the string is encrypted (naive check).
    if (strlen(answ) == 32)
    {
        is_enc_string = true;
    }
}

int main(int argc, char *argv[])
{
    // to_child_fd is used to send from the parent to the child.
    // to_child_fd[0] read end.
    // to_parent_fd[1] write end.
    int to_child_fd[2];
    // to_parent_fd is used to send from the child to the parent.
    // to_parent_fd[0] read end.
    // to_parent_fd[1] write end.
    int to_parent_fd[2];
    // pid of the child process.
    pid_t pid;
    // Create an unidirectional data channel for both ends.
    if (pipe(to_child_fd) == -1 || pipe(to_parent_fd) == -1)
    {
        // Print message to the error stream.
        fprintf(stderr, "Failed at creating pipes.");
        return 1;
    }
    // String to encrypt (user's input).
    char input_str[20];
    // insert the string into the buffer.
    printf("plain text: ");
    scanf("%s", input_str);
    // Create new process.
    pid = fork();
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
        // Assing a new handler function to the SIGINT signal.
        signal(SIGINT, sig_handler);
        // Close reading end.
        close(to_child_fd[0]);
        // Write the string to the write end.
        write(to_child_fd[1], input_str, strlen(input_str) + 1);
        // Close writing end (EOF).
        close(to_child_fd[1]);
        // Close writing end.
        close(to_parent_fd[1]);
        // Read the encrypted string into the global string.
        read(to_parent_fd[0], answ, 33);
        // Close the reading end.
        close(to_parent_fd[0]);
        // Check if the string is encrypted.
        if (is_enc_string)
        {
            printf("encrypted by process %d : %s\n", pid, answ);
            // Kill the child process.
            kill(pid, SIGTERM);
        }
    }
    /* Child */
    else
    {
        // Close reading end.
        close(to_child_fd[1]);
        // Buffer used to store the string that was sent from the parent process.
        char rcved_string[100];
        // Read the sent string into the allocated buffer.
        read(to_child_fd[0], rcved_string, 100);
        // Close reading end.
        close(to_child_fd[0]);
        // Encrypt the string using md5.
        string ans = md5(rcved_string);
        // Close the reading end.
        close(to_parent_fd[0]);
        // Turn the string into a char array and write it to the pipe.
        write(to_parent_fd[1], ans.c_str(), ans.length() + 1);
        // Close the writing end.
        close(to_parent_fd[1]);
        // Send a signal indicating that the task is finished.
        kill(getppid(), SIGINT);
    }

    return 0;
}
//md5 of 'grape': b781cbb29054db12f88f08c6e161c199
