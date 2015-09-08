#include <stdint.h>
#include <r0ketlib/idle.h>
#include <r0ketlib/keyin.h>
#include <r0ketlib/print.h>
#include <rad1olib/setup.h>
#include <rad1olib/audio.h>
#include <math.h>
#include <r0ketlib/itoa.h>


#define M_PI (3.14159265358979323846264338327950f)


#define MAX_SAMPLE_VAL 585  // No clipping!
#define AUDIO_BUFSIZE 1000
#define AUDIO_RATE    32000
#define RITIMER_SPEED   204000000
static uint16_t audio_buf[AUDIO_BUFSIZE];

static int tone_volume = 5;
static int tone_frequency = 600;
static float tone_angle = 0.0; // To calculate the sine wave
static float tone_angle_increment = 2.0*M_PI*600/AUDIO_RATE; // increase angle by this much

static void update_volume();
static void update_frequency();
static void push_next_value();




static void tone_begin()
{
	cpu_clock_set(204); // Turbo mode
	delayms(500);
	audio_init(audio_buf, AUDIO_BUFSIZE, AUDIO_RATE, RITIMER_SPEED);
	lcdClear();
	update_volume();
	update_frequency();
	for(int i=0; i<(AUDIO_BUFSIZE/2); i++)
	{
		push_next_value();
	}
	audio_play();
}

static void tone_end()
{
	audio_stop();
	cpu_clock_set(102);
	delayms(500);
}

static void push_next_value()
{
	float s = sin(tone_angle) + 1.0; // shift value so it is always positive (because we're such optimists).
	s = s * MAX_SAMPLE_VAL;
	uint16_t smpl = ((uint16_t)s)/2;
	audio_push(smpl);
	tone_angle = tone_angle + tone_angle_increment;
	if(tone_angle > 2.0*M_PI)
	{
		tone_angle = tone_angle - 2.0*M_PI;
	}
}

static void update_volume()
{
	audio_set_volume(tone_volume);
	lcdSetCrsr(0,10);
	lcdPrint("vol=");
	lcdPrint(IntToStr(tone_volume,2,F_LONG));
	lcdPrint("  ");
	lcdDisplay();
}

static void increase_volume()
{
	if(tone_volume < 9)
		tone_volume++;
	update_volume();
}
static void decrease_volume()
{
	if(tone_volume > 0)
		tone_volume--;
	update_volume();

}

static void update_frequency()
{
	tone_angle_increment = 2.0*M_PI*tone_frequency/AUDIO_RATE; // increase angle by this much
	lcdSetCrsr(0,0);
	lcdPrint("f=");
	lcdPrint(IntToStr(tone_frequency,4,F_LONG));
	lcdPrint("Hz ");
	lcdDisplay();
}
static void decrease_frequency()
{
	if(tone_frequency > 100)
		tone_frequency -=  50;
	update_frequency();
}
static void increase_frequency()
{
	if(tone_frequency < 3000)
		tone_frequency += 50;
	update_frequency();
}

//# MENU tone
void tone_show()
{
	uint8_t old_btn, new_btn=BTN_NONE, pressed_btn;
	bool running = true;
	tone_begin();

	while(running)
	{
		old_btn = new_btn;
		new_btn = getInputRaw();
		pressed_btn = new_btn & ~old_btn;

		if(pressed_btn & BTN_UP)
		{
			increase_volume();
		}
		if(pressed_btn & BTN_DOWN)
		{
			decrease_volume();
		}
		if(pressed_btn & BTN_LEFT)
		{
			decrease_frequency();
		}
		if(pressed_btn & BTN_RIGHT)
		{
			increase_frequency();
		}
		if(pressed_btn & BTN_ENTER)
		{
			running = false;
		}
		while(audio_fill() < (AUDIO_BUFSIZE/2))
		{
			push_next_value();
		}
	}
	tone_end();
}
