#include "game.h"
#include "assets/bg.h"
#include "assets/colors.h"
#include "assets/combo1.h"
#include "assets/combo2.h"
#include "assets/combo3.h"
#include "assets/combo4.h"
#include "assets/drop.h"
#include "assets/title.h"

#include "audio.h"
#include "gba.h"
#include <stdio.h>

static u16 board[BOARD_HEIGHT + 1][BOARD_WIDTH + 2];

static u16 __attribute__((section(".ewram"))) chunks[CHUNKS_HEIGHT + 1][CHUNKS_WIDTH];

static enum state state = TITLE;
static int score;
static int clearFrame;
static int hasUpdated;
static int comboMult;
static int combo;
static int toClearX, toClearY;
static int fillNum;
static u16
    __attribute__((section(".ewram"))) visited[BOARD_HEIGHT][BOARD_WIDTH];

static int highscores[4];

static tetrimino pieceCurr;
static tetrimino pieceNext;

// extern void updateWorldASM();

static inline tetrimino newPiece(void) {
    return (tetrimino){
        .x = 24,
        .y = 0,
        .rotation = 0,
        .type = randint(0, 7),
        .color = randint(0, 4),
    };
}

void init(void) {
    volatile u32 zero = 0;

    // zero board
    DMA[3].src = &zero;
    DMA[3].dst = board;
    DMA[3].cnt = ((BOARD_WIDTH + 2) * (BOARD_HEIGHT + 1) / 2) | DMA_ON |
                 DMA_32 | DMA_SOURCE_FIXED;

    // fill borders
    for (int y = 0; y <= BOARD_HEIGHT; y++) {
        board[y][0] = 0xFFFF;
        board[y][BOARD_WIDTH + 1] = 0xFFFF;
    }
    for (int x = 1; x <= BOARD_WIDTH; x++) {
        board[BOARD_HEIGHT][x] = 0xFFFF;
    }

    // zero chunks
    DMA[3].dst = chunks;
    DMA[3].cnt = (((CHUNKS_WIDTH) * (CHUNKS_HEIGHT + 1))) | DMA_ON | DMA_32 |
                 DMA_SOURCE_FIXED;

    pieceCurr = newPiece();
    pieceNext = newPiece();

    score = 0;
    clearFrame = 12;
    hasUpdated = 1;
    comboMult = 1;
    combo = 0;
    fillNum = 1;

    // play audio
    reset_music();
    init_audio();
    loop_music();
}

void drawBoard(void) {
    drawImageDMA32(0, BOARD_X_OFFSET, BOARD_WIDTH + 2, BOARD_HEIGHT,
                   (const unsigned short *)board);
}

void drawPiece(void) {
    for (int i = 0; i < 4; i++) {
        const int *cell = PIECES[pieceCurr.type][pieceCurr.rotation][i];

        drawImageDMA(pieceCurr.y + cell[1],
                     BOARD_X_OFFSET + pieceCurr.x + cell[0] + 1, 8, 8,
                     TILES[pieceCurr.color]);
    }
}

void drawUI(void) {
    // next piece
    drawRectDMA32(NEXT_PIECE_OFFSET_Y, NEXT_PIECE_OFFSET_X, 32, 32, 0x1884);
    for (int i = 0; i < 4; i++) {
        const int *cell = PIECES[pieceNext.type][pieceNext.rotation][i];

        drawImageDMA(NEXT_PIECE_OFFSET_Y + cell[1],
                     NEXT_PIECE_OFFSET_X + cell[0], 8, 8,
                     TILES[pieceNext.color]);
    }
}

void drawScore(void) {
    char score_str[10];
    snprintf(score_str, 10, "%d", score);
    drawRectDMA32(SCORE_OFFSET_Y, SCORE_OFFSET_X, 50, 8, 0x1884);
    drawString(SCORE_OFFSET_Y, SCORE_OFFSET_X, score_str, COL_WHITE);
}

void drawChunks(void) {
    for (int y = 0; y < CHUNKS_HEIGHT; y++) {
        for (int x = 0; x < CHUNKS_WIDTH; x++) {
            if (chunks[y][x]) {
                videoBuffer[OFFSET(CHUNK_SIZE * y,
                                   CHUNK_SIZE * x + BOARD_X_OFFSET + 1,
                                   WIDTH)] = 0xFFF0;
            }
        }
    }
}

#define INSIDE(X, Y)                                                           \
    (((board[Y][X] & COL_MASK) == col) && (visited[Y][X - 1] != fillNum))

void fill(int x, int y, const int col) {
    // new stack
    static int stack[BOARD_HEIGHT][2];
    int sz = 0;

    // add x,y to stack
    stack[sz][0] = x;
    stack[sz][1] = y;
    ++sz;

    // while stack is not empty
    while (sz) {
        // pop into lx, yy
        x = stack[sz - 1][0];
        y = stack[sz - 1][1];
        --sz;
        int lx = x;

        while (INSIDE(lx - 1, y)) {
            visited[y][lx - 2] = fillNum;
            --lx;
        }
        while (INSIDE(x, y)) {
            visited[y][x - 1] = fillNum;
            ++x;
        }

        int a = 0, b = 0;
        for (int i = lx - 1; i <= x; i++) {
            if (!INSIDE(i, y - 1)) {
                a = 0;
            } else if (!a) {
                stack[sz][0] = i;
                stack[sz][1] = y - 1;
                ++sz;
                a = 1;
            }
            if (!INSIDE(i, y + 1)) {
                b = 0;
            } else if (!b) {
                stack[sz][0] = i;
                stack[sz][1] = y + 1;
                ++sz;
                b = 1;
            }
        }
    }
}

int contour(const int x, const int y, const int col, const int targ, int dir) {
    static const int __attribute__((section(".vram"))) offset[4][3][2] = {
        {{-1, -1}, {0, -1}, {1, -1}},
        {{1, -1}, {1, 0}, {1, 1}},
        {{1, 1}, {0, 1}, {-1, 1}},
        {{-1, 1}, {-1, 0}, {-1, -1}},
    };
    int px = x;
    int py = y;

    int cx, cy;
    do {
        int rots = 0;
        for (; rots < 3; ++rots) {
            if (px == targ) {
                toClearX = x, toClearY = y;
                return 1;
            }
            cx = px + offset[dir][0][0];
            cy = py + offset[dir][0][1];
            if ((board[cy][cx] & COL_MASK) == col) {
                px = cx, py = cy;
                dir = (dir + 3) % 4;
                break;
            }
            cx = px + offset[dir][1][0];
            cy = py + offset[dir][1][1];
            if ((board[cy][cx] & COL_MASK) == col) {
                px = cx, py = cy;
                break;
            }
            cx = px + offset[dir][2][0];
            cy = py + offset[dir][2][1];
            if ((board[cy][cx] & COL_MASK) == col) {
                px = cx, py = cy;
                break;
            }
            dir = (dir + 1) % 4;
        }
        if (rots == 3)
            break;
    } while (!(px == x && py == y));
    return 0;
}

int contourL(void) {
    ++fillNum;
    int regions = 0;
    for (int y = 1; y < BOARD_HEIGHT; y++) {
        int col = board[y][1] & COL_MASK;
        if (col == 0 || col == (board[y - 1][1] & COL_MASK))
            continue;

        ++fillNum;
        if (contour(1, y, col, BOARD_WIDTH, 1)) {
            return 1;
        }

        ++regions;
        if (regions == 2)
            break;
    }
    return 0;
}

int contourR(void) {
    ++fillNum;
    int regions = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        int col = board[y][BOARD_WIDTH] & COL_MASK;
        if (col == 0 || col == (board[y + 1][BOARD_WIDTH] & COL_MASK))
            continue;

        ++fillNum;
        if (contour(BOARD_WIDTH, y, col, 1, 3)) {
            return 1;
        }

        ++regions;
        if (regions == 2)
            break;
    }
    return 0;
}

static inline void __attribute__((target("arm"))) updateWorld(void) {
    for (int y = CHUNKS_HEIGHT - 1; y > 1; y--) {
        for (int x = 0; x < CHUNKS_WIDTH; x++) {
            if (!chunks[y][x])
                continue;

            int isActive = 0;
            int zz = 4;
            for (int yy = (y + 1) * CHUNK_SIZE - 1; yy >= y * CHUNK_SIZE;
                 yy--) {
                for (int xx = x * CHUNK_SIZE + 1; xx <= (x + 1) * CHUNK_SIZE;
                     xx++) {
                    if (!board[yy][xx])
                        continue;

                    // fall
                    if (!board[yy + 1][xx]) {
                        isActive = 1;
                        if (--zz == 0) {
                            zz = 4;
                            continue;
                        }
                        board[yy + 1][xx] = board[yy][xx];
                        board[yy][xx] = 0;
                    } else if (!board[yy + 1][xx - 1]) {
                        isActive = 1;
                        if (--zz == 0) {
                            zz = 4;
                            continue;
                        }
                        board[yy + 1][xx - 1] = board[yy][xx];
                        board[yy][xx] = 0;
                    } else if (!board[yy + 1][xx + 1]) {
                        isActive = 1;
                        if (--zz == 0) {
                            zz = 4;
                            continue;
                        }
                        board[yy + 1][xx + 1] = board[yy][xx];
                        board[yy][xx] = 0;
                    }
                }
            }

            chunks[y][x] = isActive;
            if (isActive) {
                chunks[y - 1][x] = 1;
                chunks[y][x - 1] = 1;
                chunks[y][x + 1] = 1;
                chunks[y + 1][x] = 1;
            } else {
                chunks[y][x] = 0;
            }
        }
    }
}

int updatePiece(void) {
    int type = pieceCurr.type;
    if (type > 1)
        type = 2;

    // rotate the current piece if necessary
    if (key_hit(BUTTON_UP)) {
        pieceCurr.rotation = (pieceCurr.rotation + 1) % 4;
    }

    // move the current piece left or right if necessary
    if (key_is_down(BUTTON_LEFT)) {
        --pieceCurr.x;
    } else if (key_is_down(BUTTON_RIGHT)) {
        ++pieceCurr.x;
    }

    // check for wall collision
    if (pieceCurr.x < LR_BOUNDS[type][pieceCurr.rotation][0])
        pieceCurr.x = LR_BOUNDS[type][pieceCurr.rotation][0];
    else if (pieceCurr.x > LR_BOUNDS[type][pieceCurr.rotation][1])
        pieceCurr.x = LR_BOUNDS[type][pieceCurr.rotation][1];

    // move the current piece down 1 cell
    ++pieceCurr.y;
    if (key_is_down(BUTTON_DOWN))
        ++pieceCurr.y;

    // check for collision with ground or sand
    // if (pieceCurr.y < 0) return 0;
    for (int i = 0; i < 4; i++) {
        const int *cell = PIECES[pieceCurr.type][pieceCurr.rotation][i];
        // check if in bounds
        int x = pieceCurr.x + cell[0];
        int y = pieceCurr.y + cell[1] + 8;
        for (int offset = 0; offset < 8; offset++) {
            // check if hit sand
            if (board[y][x + offset + 1]) {
                play_sfx(drop, drop_bytes);
                return 1;
            }
        }
    }

    --comboMult;

    return 0;
}

void restart(void) {
    reset_music();
    state = TITLE;
}

void run(void) {
    // local vars
    int cleared = 0;

    // handle input
    if (key_hit(BUTTON_SELECT))
        restart();

    if (key_hit(BUTTON_START))
        state = START;

    switch (state) {
    case TITLE:
        if (key_hit(BUTTON_START))
            state = START;

        waitForVBlank();
        drawFullScreenImageDMA(title);

        break;
    case START:
        init();
        state = FALL;

        waitForVBlank();

        drawFullScreenImageDMA(bg);
        drawUI();
        drawScore();

        // sync
        while (SCANLINECOUNTER > 175)
            ;
        while (SCANLINECOUNTER < 175)
            ;

        break;

    case FALL:
        hasUpdated = 1;
        if (updatePiece()) {
            state = SPAWN;
            break;
        }

        updateWorld();

        while (SCANLINECOUNTER > 205)
            ;
        state = FLOOD;

        while (SCANLINECOUNTER < 145)
            ;

        drawBoard();
        drawPiece();
        // drawChunks();

        break;
    case FLOOD:
        if (updatePiece()) {
            state = SPAWN;
            break;
        }
        int a = SCANLINECOUNTER;

        if (contourL() || contourR()) {
            state = CLEAR1;
        } else {
            state = FALL;
        }

        while (SCANLINECOUNTER > a)
            ;

        while (SCANLINECOUNTER < 145)
            ;

        dbgs = SCANLINECOUNTER;
        drawBoard();
        drawPiece();
        dbge = SCANLINECOUNTER;

        break;

    case SPAWN:
        if (!hasUpdated) {
            state = GAMEOVER;
            break;
        }
        hasUpdated = 0;
        combo = 0;

        // blit current piece to board
        for (int i = 0; i < 4; i++) {
            const int *cell = PIECES[pieceCurr.type][pieceCurr.rotation][i];
            for (int y = 0; y < 8; y++) {
                if (pieceCurr.y + cell[1] + y >= BOARD_HEIGHT ||
                    pieceCurr.y + cell[1] + y < 0)
                    break;
                DMA[3].src = &TILES[pieceCurr.color][8 * y];
                DMA[3].dst = &board[pieceCurr.y + cell[1] + y]
                                   [pieceCurr.x + cell[0] + 1];
                DMA[3].cnt = 8 | DMA_ON | DMA_16;
            }
            int cx = ((pieceCurr.x + cell[0]) / CHUNK_SIZE);
            int cy = ((pieceCurr.y + cell[1]) / CHUNK_SIZE);
            chunks[cy][cx] = 1;
            chunks[cy][cx + 1] = 1;
            chunks[cy][cx + 2] = 1;
            chunks[cy + 1][cx] = 1;
            chunks[cy + 1][cx + 1] = 1;
            chunks[cy + 1][cx + 2] = 1;
            chunks[cy + 2][cx] = 1;
            chunks[cy + 2][cx + 1] = 1;
            chunks[cy + 2][cx + 2] = 1;
        }

        state = SPAWN2;

        while (SCANLINECOUNTER >= 145)
            ;
        while (SCANLINECOUNTER < 145)
            ;

        const int yOffset = 1;
        drawImageDMA32(yOffset, BOARD_X_OFFSET, BOARD_WIDTH + 2,
                       BOARD_HEIGHT - yOffset, (const unsigned short *)board);

        for (int i = 0; i < 4; i++) {
            const int *cell = PIECES[pieceCurr.type][pieceCurr.rotation][i];
            for (int y = 0; y < 8; y++) {
                if (pieceCurr.y + cell[1] + y + yOffset >= HEIGHT ||
                    pieceCurr.y + cell[1] + y + yOffset < 0)
                    break;
                drawRectDMA(pieceCurr.y + cell[1] + y + yOffset,
                            BOARD_X_OFFSET + pieceCurr.x + cell[0] + 1, 8, 1,
                            0xFFFF);
            }
        }

        pieceCurr = pieceNext;
        pieceNext = newPiece();

        if (key_is_down(BUTTON_LEFT))
            pieceCurr.x -= 8;
        if (key_is_down(BUTTON_RIGHT))
            pieceCurr.y -= 8;

        drawPiece();
        drawUI();

        break;
    case SPAWN2:
        state = FLOOD;

        // stall 2 frames
        int i = 3;
        while (i--) {
            while (SCANLINECOUNTER >= 145)
                ;
            while (SCANLINECOUNTER < 145)
                ;
        }

        drawBoard();
        drawPiece();

        break;
    // dont care to time clears, just sync at end
    case CLEAR1:
        fill(toClearX, toClearY, board[toClearY][toClearX] & COL_MASK);

        // color visited white
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (visited[y][x] == fillNum) {
                    board[y][x + 1] = 0xFFFF;
                    ++cleared;
                }
            }
        }
        cleared *= (cleared >> 8) ? (cleared >> 8) : 1;

        if (comboMult < 0)
            comboMult = 0;
        comboMult += 220;
        ++combo;

        score += combo * (comboMult / 220) * cleared;

        if (combo > 3) {
            play_sfx(combo4, combo4_bytes);
        } else if (combo > 2) {
            play_sfx(combo3, combo3_bytes);
        } else if (combo > 1) {
            play_sfx(combo2, combo2_bytes);
        } else {
            play_sfx(combo1, combo1_bytes);
        }
        state = CLEAR2;

        waitForVBlank();
        drawBoard();
        drawPiece();
        drawScore();

        break;
    case CLEAR2:
        --clearFrame;
        if (clearFrame) {
            state = CLEAR2;
        } else {
            state = CLEAR3;
            clearFrame = 12;
        }

        if (clearFrame == 8) {
            // color visited black
            for (int y = 0; y < BOARD_HEIGHT; y++) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    if (visited[y][x] == fillNum) {
                        board[y][x + 1] = 0;
                    }
                }
            }
        }
        if (clearFrame == 4) {
            // color visited white
            for (int y = 0; y < BOARD_HEIGHT; y++) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    if (visited[y][x] == fillNum) {
                        board[y][x + 1] = 0xFFFF;
                    }
                }
            }
        }

        waitForVBlank();
        drawBoard();
        drawPiece();

        break;
    case CLEAR3:
        // color visited black
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (visited[y][x] == fillNum) {
                    board[y][x + 1] = 0;
                }
            }
        }
        volatile u32 one = 1;

        DMA[3].src = &one;
        DMA[3].dst = chunks;
        DMA[3].cnt = (((CHUNKS_WIDTH) * (CHUNKS_HEIGHT + 1))) | DMA_ON |
                     DMA_32 | DMA_SOURCE_FIXED;

        state = FALL;

        while (SCANLINECOUNTER >= 145)
            ;
        while (SCANLINECOUNTER < 145)
            ;

        drawBoard();
        drawPiece();

        break;

    case GAMEOVER:
        stop_music();

        for (unsigned int i = 0; i < sizeof(highscores) / sizeof(int); i++) {
            if (score < highscores[i])
                continue;
            int tmp = highscores[i];
            highscores[i] = score;
            score = tmp;
        }

        if (key_hit(BUTTON_START))
            state = START;

        break;
    }
}
