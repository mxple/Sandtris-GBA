#ifndef GAME_H
#define GAME_H

#include "gba.h"
#include "tetriminos.h"

#define BOARD_WIDTH 80
#define BOARD_HEIGHT 160
#define BOARD_X_OFFSET 40

#define CHUNK_SIZE 4
#define CHUNKS_WIDTH 20
#define CHUNKS_HEIGHT 40

#define NEXT_PIECE_OFFSET_X 132
#define NEXT_PIECE_OFFSET_Y 30
#define SCORE_OFFSET_X 128
#define SCORE_OFFSET_Y 94

#define COL_MASK 0x7
#define STP_MASK 0x8000

enum state {
    TITLE,
    START,
    FALL,
    FLOOD,
    SPAWN,
    SPAWN2,
    CLEAR1,
    CLEAR2,
    CLEAR3,
    GAMEOVER
};

void restart(void);
void drawBoard(void);
void drawPiece(void);
void drawChunks(void);
void drawUI(void);

void run(void);

extern int dbgs;
extern int dbge;

#endif
