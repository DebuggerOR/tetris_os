
// ori fogler
// 318732484


#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#define QUIT_KEY 'q'
#define RIGHT_KEY 'd'
#define LEFT_KEY 'a'
#define DOWN_KEY 's'
#define TURN_KEY 'w'

#define BUFFER 1
#define BLOCK_SIZE 3
#define PLAYBOARD_SIZE 20

#define FRAME '*'
#define BLOCK '-'
#define EMPTY ' '
#define BOARD_LIM 3
#define X_START 9


typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    char entries[PLAYBOARD_SIZE][PLAYBOARD_SIZE];
} PlayBoard;

typedef struct {
    int most_right;
    int most_left;
    int most_down;
    int num_points;
    Point *points;
} Block;


PlayBoard *play_board;
Block *block;
int quit = 0;


void free_block(Block *block) {
    // free if not null
    if (block != NULL && block->points != NULL) {
        free(block->points);
        free(block);
    }
}


void free_board(PlayBoard *board) {
    // free if not null
    if (board != NULL) {
        free(board);
    }
}


void clear_block(PlayBoard *board, Block *block) {
    int i;
    for (i = 0; i < block->num_points; ++i) {
        Point p = block->points[i];
        board->entries[p.y][p.x] = EMPTY;
    }
}


void draw_board(PlayBoard *board) {
    system("clear");
    printf("Tetris Game\n");

    // display board
    int y, x;
    for (y = 0; y < PLAYBOARD_SIZE; ++y) {
        for (x = 0; x < PLAYBOARD_SIZE; ++x) {
            printf("%c", board->entries[y][x]);
        }
        printf("\n");
    }

    printf("%c - End Game, %c - Right, %c - Left, %c - Down, %c - Flip\n",
           QUIT_KEY, RIGHT_KEY, LEFT_KEY, DOWN_KEY, TURN_KEY);
}


void add_block(PlayBoard *board, Block *block) {
    int i;
    for (i = 0; i < block->num_points; ++i) {
        board->entries[block->points[i].y][block->points[i].x] = BLOCK;
    }
}


void copy_block(Block *dest, Block *src) {
    dest->most_down = src->most_down;
    dest->most_right = src->most_right;
    dest->most_left = src->most_left;
    dest->num_points = src->num_points;

    int i;
    for (i = 0; i < src->num_points; ++i) {
        dest->points[i].y = src->points[i].y;
        dest->points[i].x = src->points[i].x;
    }
}


void right_block(PlayBoard *board, Block *block) {
    // if pass right border
    if (block->most_right > PLAYBOARD_SIZE - BOARD_LIM - 1) {
        return;
    }

    int i;
    for (i = 0; i < block->num_points; ++i) {
        Point p = block->points[i];
        board->entries[p.y][p.x] = EMPTY;
        ++(block->points[i].x);
    }

    ++(block->most_right);
    ++(block->most_left);
    add_block(board, block);
}


void left_block(PlayBoard *board, Block *block) {
    // if pass left border
    if (block->most_left < BOARD_LIM) {
        return;
    }

    int i;
    for (i = 0; i < block->num_points; ++i) {
        Point p = block->points[i];
        board->entries[p.y][p.x] = EMPTY;
        --(block->points[i].x);
    }

    --(block->most_right);
    --(block->most_left);
    add_block(board, block);
}


void down_block(PlayBoard *board, Block *block) {
    // if pass down border
    if (block->most_right > PLAYBOARD_SIZE - BOARD_LIM) {
        return;
    }

    int i;
    for (i = 0; i < block->num_points; ++i) {
        Point p = block->points[i];
        board->entries[p.y][p.x] = EMPTY;
        ++(block->points[i].y);
    }

    ++(block->most_down);
    add_block(board, block);
}


void flip_block(PlayBoard *board, Block *block) {
    Block temp_block;
    temp_block.points = malloc(sizeof(Point) * block->num_points);
    copy_block(&temp_block, block);
    Point center = temp_block.points[temp_block.num_points / 2];

    int x_center = center.x;
    int y_center = center.y;

    int most_right = x_center;
    int most_left = x_center;
    int most_down = y_center;

    int diff;

    int i;
    for (i = 0; i < temp_block.num_points; i++) {
        Point p = temp_block.points[i];
        if (p.x != x_center && p.y == y_center) {
            diff = x_center - p.x;
            temp_block.points[i].x = x_center;
            temp_block.points[i].y = y_center + diff;

            if (temp_block.points[i].y > most_down) {
                most_down = temp_block.points[i].y;
            }
            if (temp_block.points[i].x < most_left) {
                most_left = temp_block.points[i].x;
            }
            if (temp_block.points[i].x > most_right) {
                most_right = temp_block.points[i].x;
            }
        } else if (p.y != y_center && p.x == x_center) {
            diff = y_center - p.y;
            temp_block.points[i].x = x_center + diff;
            temp_block.points[i].y = y_center;

            if (temp_block.points[i].x > most_right) {
                most_right = temp_block.points[i].x;
            }
            if (temp_block.points[i].x < most_left) {
                most_left = temp_block.points[i].x;
            }
            if (temp_block.points[i].y > most_down) {
                most_down = temp_block.points[i].y;
            }
        }
    }
    if (most_down < PLAYBOARD_SIZE - BOARD_LIM &&
        most_right <= PLAYBOARD_SIZE - BOARD_LIM && most_left >= BOARD_LIM) {
        clear_block(board, block);
        copy_block(block, &temp_block);
        block->most_right = most_right;
        block->most_left = most_left;
        block->most_down = most_down;
        add_block(board, block);
    }

    // free temp block
    free(temp_block.points);
}


int should_remove_block(Block *block) {
    if (block->most_down > PLAYBOARD_SIZE - BOARD_LIM) {
        return 1;
    }
    return 0;
}


void handle_signal(int sig) {
    if (should_remove_block(block)) {
        return;
    }

    if (sig == SIGUSR2) {
        char buffer[BUFFER];
        read(0, buffer, sizeof(buffer));
        char pushed = buffer[0];
        switch (pushed) {
            case RIGHT_KEY:
                right_block(play_board, block);
                break;
            case LEFT_KEY:
                left_block(play_board, block);
                break;
            case DOWN_KEY:
                down_block(play_board, block);
                break;
            case TURN_KEY:
                flip_block(play_board, block);
                break;
            case QUIT_KEY:
                quit = 1;
                break;
            default:
                break;
        }
        draw_board(play_board);
    }
}


void handle_alarm(int sig) {
    if (sig == SIGALRM && !should_remove_block(block)) {
        down_block(play_board, block);
        draw_board(play_board);
    }
    alarm(1);
}


PlayBoard *create_empty_board() {
    PlayBoard *new_board = malloc(sizeof(PlayBoard));
    int i, j;
    for (i = 0; i < PLAYBOARD_SIZE - 1; ++i) {
        for (j = 1; j < PLAYBOARD_SIZE - 1; ++j) {
            new_board->entries[i][j] = EMPTY;
        }
    }

    for (i = 0; i < PLAYBOARD_SIZE; ++i) {
        new_board->entries[PLAYBOARD_SIZE - 1][i] = FRAME;
    }

    for (j = 0; j < PLAYBOARD_SIZE - 1; ++j) {
        new_board->entries[j][0] = FRAME;
        new_board->entries[j][PLAYBOARD_SIZE - 1] = FRAME;
    }

    return new_board;
}


Block *create_first_block(int num_points) {
    Block *first_block = malloc(sizeof(block));
    first_block->points = malloc(sizeof(Point) * num_points);
    first_block->num_points = num_points;

    int i;
    for (i = 0; i < num_points; ++i) {
        first_block->points[i].y = 0;
        first_block->points[i].x = X_START + i;
    }

    first_block->most_down = 0;
    first_block->most_left = X_START;
    first_block->most_right = X_START + num_points - 1;

    return first_block;
}


int main() {
    play_board = create_empty_board();
    block = create_first_block(BLOCK_SIZE);
    draw_board(play_board);
    add_block(play_board, block);

    // alarm signal
    struct sigaction act1;
    sigset_t block_mask1;
    sigaddset(&block_mask1, SIGUSR2);
    act1.sa_handler = handle_alarm;
    act1.sa_mask = block_mask1;
    act1.sa_flags = 0;
    sigaction(SIGALRM, &act1, NULL);

    // sigusr signal
    struct sigaction act2;
    sigset_t block_mask2;
    sigaddset(&block_mask2, SIGALRM);
    act2.sa_handler = handle_signal;
    act2.sa_mask = block_mask2;
    act2.sa_flags = 0;
    sigaction(SIGUSR2, &act2, NULL);

    sigset_t both_signals;
    sigaddset(&both_signals, SIGUSR2);
    sigaddset(&both_signals, SIGALRM);

    sigset_t pending;

    alarm(1);
    while (quit == 0) {
        if (should_remove_block(block)) {
            sigprocmask(SIG_BLOCK, &both_signals, NULL);
            clear_block(play_board, block);
            free_block(block);
            block = create_first_block(BLOCK_SIZE);
            add_block(play_board, block);
            draw_board(play_board);
            sigprocmask(SIG_UNBLOCK, &both_signals, NULL);
        }

        sigpending(&pending);
        if (sigismember(&pending, SIGALRM) == 1) {
            raise(SIGALRM);
        }
        if (sigismember(&pending, SIGUSR2)) {
            raise(SIGUSR2);
        }

        pause();
    }

    // end stuff
    system("clear");
    printf("\n\thope u enjoyed!\n\t");
    free_block(block);
    free_board(play_board);

    return 0;
}
