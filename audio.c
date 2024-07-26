#include "audio.h"
#include "gba.h"

#include "assets/typeA.h"

static const unsigned short ticks_per_sample = 16777216 / 16834;

void __attribute__ ((target("arm"))) play_typeA(void) {
    DMA[DMA_CHANNEL_1].cnt = 0;
    DMA[DMA_CHANNEL_1].src = typeA;
    DMA[DMA_CHANNEL_1].dst = SOUND_A_FIFO_BUFFER;
    DMA[DMA_CHANNEL_1].cnt = DMA_ON | DMA_DESTINATION_FIXED | DMA_32 | DMA_REPEAT | DMA_AT_REFRESH;

    REG_IF = IRQ_TIMER2;
}

void __attribute__ ((target("arm"))) stop_sfx(void) {
    REG_SOUNDCNT_H &= ~(SOUND_B_RIGHT_CHANNEL | SOUND_B_LEFT_CHANNEL);
    DMA[DMA_CHANNEL_2].cnt = 0;
    REG_IF = IRQ_TIMER3;

    TIMER3_REGCNT  = 0;
}

void __attribute__ ((target("arm"))) interrupt(void) {
	if (REG_IF & IRQ_TIMER3) {
		stop_sfx();
	}
	if (REG_IF & IRQ_TIMER2) {
		play_typeA();
	}
}

void init_audio(void) {
    REG_SOUNDCNT_X = SOUND_MASTER_ENABLE;
    TIMER0_REGDATA = 65536 - ticks_per_sample;
    TIMER0_REGCNT = TIMER_ENABLE | TIMER_FREQ_1;
	REG_ISR_MAIN = interrupt;  // tell the GBA where my isr is
}

void reset_music(void) {
	REG_SOUNDCNT_X = 0;
	REG_SOUNDCNT_H = 0;
	TIMER0_REGCNT = 0;
	TIMER1_REGCNT = 0;
	TIMER2_REGCNT = 0;
	TIMER3_REGCNT = 0;

    DMA[DMA_CHANNEL_1].cnt = 0;
	REG_IE = 0;
	REG_IME = 0;
}

void loop_music(void) {
    REG_SOUNDCNT_H |= SOUND_A_RIGHT_CHANNEL | SOUND_A_LEFT_CHANNEL | SOUND_A_FIFO_RESET;
    DMA[DMA_CHANNEL_1].src = typeA;
    DMA[DMA_CHANNEL_1].dst = SOUND_A_FIFO_BUFFER;
    DMA[DMA_CHANNEL_1].cnt = DMA_ON | DMA_DESTINATION_FIXED | DMA_32 | DMA_REPEAT | DMA_AT_REFRESH;

    TIMER1_REGDATA = 0;
    TIMER1_REGCNT  = TIMER_ENABLE | TIMER_CASCADE;

    TIMER2_REGDATA = 65536 - 42;
    TIMER2_REGCNT = TIMER_ENABLE | TIMER_CASCADE | TIMER_INTERRUPT;

	REG_IE |= IRQ_TIMER2;       // Tell the GBA to catch Timer1
	REG_IME = 1;                // Tell the GBA to enable interrupts;
}

void stop_music(void) {
    REG_SOUNDCNT_H &= ~(SOUND_A_RIGHT_CHANNEL | SOUND_A_LEFT_CHANNEL);
    DMA[DMA_CHANNEL_1].cnt = 0;
}


void play_sfx(const char* sfx, int bytes) {
    REG_SOUNDCNT_H |= SOUND_B_RIGHT_CHANNEL | SOUND_B_LEFT_CHANNEL | SOUND_B_FIFO_RESET;
    DMA[DMA_CHANNEL_2].cnt = 0;
    DMA[DMA_CHANNEL_2].src = sfx;
    DMA[DMA_CHANNEL_2].dst = SOUND_B_FIFO_BUFFER;
    DMA[DMA_CHANNEL_2].cnt = DMA_ON | DMA_DESTINATION_FIXED | DMA_32 | DMA_REPEAT | DMA_AT_REFRESH;

	TIMER3_REGCNT = 0;
    TIMER3_REGDATA = 65536 - bytes + 1000;
    TIMER3_REGCNT  = TIMER_ENABLE | TIMER_FREQ_1024 | TIMER_INTERRUPT;

	REG_IE |= IRQ_TIMER3;
	REG_IME = 1;
}



