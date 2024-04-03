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

        // if (key_hit(BUTTON_A))
        //     // stop_music();
        //     play_sfx(combo1, combo1_bytes);

        // game logic
        run();

        // dbgs = TIMER3_REGDATA;
        // char s[16];
        // sprintf(s, "%d.%d", dbgs, dbge);
        // undrawImageDMA(8, 0, 56, 8, bg);
        // drawString(8, 0, s, COL_CYAN);
    }

    return 0;
}
