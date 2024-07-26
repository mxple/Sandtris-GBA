#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "assets/bg.h"

#include "audio.h"
#include "game.h"
#include "gba.h"

#include "assets/combo1.h"

int dbgs = 0, dbge = 0;

int main(void) {
    // Manipulate REG_DISPCNT here to set Mode 3. //
    REG_DISPCNT = BG2_ENABLE | MODE3;


    while (1) {
        // input polling
        key_poll();

        // game logic
        run();

        char s[16];
        int p = dbge < dbgs ? 228 + dbge - dbgs : dbge - dbgs;
        sprintf(s, "%d", p);
        undrawImageDMA(8, 0, 56, 8, bg);
        drawString(8, 0, s, COL_CYAN);
    }

    return 0;
}
