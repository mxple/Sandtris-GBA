#ifndef AUDIO_H
#define AUDIO_H

#include "gba.h"

#define REG_SOUNDCNT_X (*(volatile unsigned short *)0x4000084)

#define SOUND_MASTER_ENABLE (1 << 7)

#define REG_SOUNDCNT_H (*(volatile unsigned short *)0x4000082)

#define SOUND_A_RIGHT_CHANNEL (1 << 8)
#define SOUND_A_LEFT_CHANNEL (1 << 9)
#define SOUND_A_FIFO_RESET (1 << 11)
#define SOUND_B_RIGHT_CHANNEL (1 << 12)
#define SOUND_B_LEFT_CHANNEL (1 << 13)
#define SOUND_B_FIFO_RESET (1 << 15)

#define SOUND_A_FIFO_BUFFER (volatile unsigned char *)0x40000A0
#define SOUND_B_FIFO_BUFFER (volatile unsigned char *)0x40000A4

#define TIMER0_REGDATA (*(volatile unsigned short *)0x4000100)
#define TIMER0_REGCNT (*(volatile unsigned short *)0x4000102)

#define TIMER1_REGDATA (*(volatile unsigned short *)0x4000104)
#define TIMER1_REGCNT (*(volatile unsigned short *)0x4000106)

#define TIMER2_REGDATA (*(volatile unsigned short *)0x4000108)
#define TIMER2_REGCNT (*(volatile unsigned short *)0x400010a)

#define TIMER3_REGDATA (*(volatile unsigned short *)0x400010c)
#define TIMER3_REGCNT (*(volatile unsigned short *)0x400010e)

#define TIMER_FREQ_1 0x0
#define TIMER_FREQ_64 0x1
#define TIMER_FREQ_256 0x2
#define TIMER_FREQ_1024 0x3
#define TIMER_ENABLE 0x80
#define TIMER_CASCADE 0x4
#define TIMER_INTERRUPT 0x40

typedef void (*fnptr)(void);

#define REG_IE (*(volatile unsigned short *)0x4000200)
#define REG_IF (*(volatile unsigned short *)0x4000202)
#define REG_IME (*(volatile unsigned short *)0x4000208)
#define REG_ISR_MAIN *(fnptr*)(0x03007FFC)

#define IRQ_TIMER1 0x0010
#define IRQ_TIMER2 0x0020
#define IRQ_TIMER3 0x0040

void init_audio(void);
void reset_music(void);
void play_typeA(void);

void loop_music(void);

void stop_music(void);

void play_sfx(const char* sfx, int bytes);

#endif
