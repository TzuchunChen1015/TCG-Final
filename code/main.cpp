#include <stdio.h>
#include <time.h>
#include <array>
#include <random>
#include <algorithm>
#include <string.h>
#include "board/board.hpp"

int main()
{
    // Disable buffering on stdout, making it unbuffered
    setvbuf(stdout, NULL, _IONBF, 0);
    // Disable buffering on stderr, making it unbuffered
    setvbuf(stderr, NULL, _IONBF, 0);

#define COMMAND_UPDATE 0
#define COMMAND_EXIT 1

    while (true)
    {
        double remain_time;
        int command, moving_color, dice;
        int piece_position[2][6];
        scanf("%d", &command);
        switch (command)
        {
        case COMMAND_UPDATE:
        {
            // scan input
            scanf("%d", &moving_color);
            scanf("%lf", &remain_time);
            for (int _color_ = 0; _color_ < 2; _color_++)
            {
                for (int piece_num = 0; piece_num < 6; piece_num++)
                {
                    scanf("%d", &piece_position[_color_][piece_num]);
                }
            }
            scanf("%d", &dice);
            // dump into board
            Board current_board;
            current_board.init_with_piecepos(piece_position, moving_color);
            current_board.dice = dice;
            current_board.print_board();
            int step_id = current_board.decide(remain_time);
            int step_start_position = current_board.moves[step_id][0], step_destination = current_board.moves[step_id][1];
            int moving_piece = current_board.board[step_start_position] - moving_color * PIECE_NUM;
            // reply steps
            fprintf(stderr, "====>%d %d\n", moving_piece, step_destination);
            break;
        }
        case COMMAND_EXIT:
        {
            printf("exit normally...\n");
            // you can add your exit handler here
            return 0;
        }
        default:
            return 0;
        }
    }
    return 0;
}
