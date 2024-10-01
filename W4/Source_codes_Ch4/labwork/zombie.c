#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // Create a child process
    pid_t pid = fork();

    // Error handling for fork failure
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    // Child process (pid == 0)
    else if (pid == 0) {
        printf("This is the child process. My PI5D is %d.\n", getpid());
    }

    // Parent process (pid > 0)
    else {
        printf("This is the parent process. My child's PID is %d.\n", pid);
        printf("My PID is %d.\n", getpid());
        wait(NULL);
        printf("Completed retrieving Zombie Process\n");
        sleep(30);
        printf("Done sleeping bruh\n");       
    }

    return 0;
}
