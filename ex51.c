
// ori fogler
// 318732484


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>


#define LEFT_KEY 'a'
#define RIGHT_KEY 'd'
#define DOWN_KEY 's'
#define QUIT_KEY 'q'
#define TURN_KEY 'w'

#define DRAW_FILE "./draw.out"
#define ERROR_MSG "Error in system call.\n"
#define ERROR_EXIT_STAT 1


int is_key(char k) {
    // case quit, left, right, turn or down
    if (k == QUIT_KEY || k == RIGHT_KEY || k == LEFT_KEY
        || k == DOWN_KEY || k == TURN_KEY) {
        return 1;
    }

    // else, not a key game
    return 0;
}


char get_char() {
    char buffer = 0;
    struct termios old = {0};

    if (tcgetattr(0, &old) < 0) {
        perror(ERROR_MSG);
    }

    old.c_lflag &= ~ECHO;
    old.c_lflag &= ~ICANON;

    old.c_cc[VTIME] = 0;
    old.c_cc[VMIN] = 1;

    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror(ERROR_MSG);
    }
    if (read(0, &buffer, 1) < 0) {
        perror(ERROR_MSG);
    }

    old.c_lflag |= ECHO;
    old.c_lflag |= ICANON;

    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror(ERROR_MSG);
    }

    return (buffer);
}


void update_new_key(int child_pid, int write_fd, char k) {
    kill(child_pid, SIGUSR2);
    char buffer[] = {k};
    write(write_fd, buffer, sizeof(buffer));
}


char handle_keys(int child_pid, int write_fd) {
    char key = 0;
    while((key = get_char()) != QUIT_KEY){
        if (!is_key(key)){
            continue;
        }

        update_new_key(child_pid, write_fd, key);
    }

    update_new_key(child_pid, write_fd, QUIT_KEY);
}


int main() {
    // use pipe for connection
    int fd[2];
    pipe(fd);

    // create child
    int pid = fork();

    // case error
    if (pid < 0) {
        write(2, ERROR_MSG, strlen(ERROR_MSG));
        exit(ERROR_EXIT_STAT);

        // case child
    } else if (pid == 0) {
        dup2(fd[0], 0);
        close(fd[1]);
        execlp(DRAW_FILE, DRAW_FILE, NULL);

        // case parent
    } else {
        close(fd[0]);
        handle_keys(pid, fd[1]);
    }

    return 0;
}