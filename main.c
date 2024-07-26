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
        // __asm(
        //    ".global dbgs\n\t"           
        //     "LDR R2, =dbgs\n\t"                                                     
        //     "LDR R2, [R2, #0]\n\t"                                                     
        //     "ADD R2, R2, #4\n\t"
        //     "STR R2, =dbgs\n\t"
        // );
        
        // input polling
        key_poll();

        // if (key_hit(BUTTON_A))
        //     // stop_music();
        //     play_sfx(combo1, combo1_bytes);

        // game logic
        run();

        // char s[16];
        // sprintf(s, "%d.%d", dbgs, dbge);
        // undrawImageDMA(8, 0, 56, 8, bg);
        // drawString(8, 0, s, COL_CYAN);
    }

    return 0;
}
