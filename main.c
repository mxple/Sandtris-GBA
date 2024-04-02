#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "gba.h"

#include "bg.h"

int dbgs = 0, dbge = 0;

int main(void) {
    // Manipulate REG_DISPCNT here to set Mode 3. //
    REG_DISPCNT = BG2_ENABLE | MODE3;

    while (1) {
        // input polling
        key_poll();

        // game logic
        run();
        //
        // char s[12];
        // sprintf(s, "%d.%d", dbgs, dbge);
        // undrawImageDMA(8, 0, 56, 8, bg);
        // drawString(8, 0, s, COL_CYAN);
    }

    return 0;
}
