#ifndef TETRIMINOS_H
#define TETRIMINOS_H

#define TETR_O 0
#define TETR_I 1
#define TETR_L 2
#define TETR_J 3
#define TETR_S 4
#define TETR_Z 5
#define TETR_T 6

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

typedef struct tetrimino {
    int x, y;
    int color; // or tile
    int type;
    int rotation;
} tetrimino;

extern const int PIECES[7][4][4][2];
extern const int LR_BOUNDS[3][4][2];

#endif
