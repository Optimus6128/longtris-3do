/*   COPYRIGHT (C) 2014-2015 GAMEBLABLA   Licensed under the Apache License, Version 2.0 (the "License");   you may not use this file except in compliance with the License.   You may obtain a copy of the License at       http://www.apache.org/licenses/LICENSE-2.0   Unless required by applicable law or agreed to in writing, software   distributed under the License is distributed on an "AS IS" BASIS,   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   See the License for the specific language governing permissions and   limitations under the License.*/#define FPS_VIDEO 60#define MAX_IMAGE 512#if defined(DEBUG) || defined(DEBUG_CRAZY)	#include <stdio.h>#endif#include <SDL/SDL.h>#ifdef SOUND_ENABLED	#include <SDL/SDL_mixer.h>	#define MAX_SFX 32	Mix_Music* music;	Mix_Chunk* gfx_id[MAX_SFX];#endif#ifdef IMAGE_CODEC_ENABLED	#include <SDL/SDL_image.h>#endif#include "INPUT.h"struct input BUTTON;const float real_FPS = 1000/FPS_VIDEO;unsigned short done = 0;char* game_name = 0;SDL_Surface *sprites_img[MAX_IMAGE];SDL_Surface *screen;unsigned short sprites_img_tocopy[MAX_IMAGE];#ifdef JOYSTICK	SDL_Joystick *joy;#endif#ifdef SCALING#include <SDL/SDL_rotozoom.h>SDL_Surface *real_screen;SDL_Rect screen_position;short scale_w, scale_h, scale_w_total, scale_h_total;#endif#define Buttons_UP SDLK_UP#define Buttons_LEFT SDLK_LEFT#define Buttons_RIGHT SDLK_RIGHT#define Buttons_DOWN SDLK_DOWN#define Buttons_A SDLK_x#define Buttons_B SDLK_c#define Buttons_C SDLK_v#define Buttons_D SDLK_b#define Buttons_START SDLK_SPACE#define Buttons_SELECT SDLK_BACKSPACE#define Buttons_QUIT SDLK_ESCAPE#define Joypad_A 0 #define Joypad_B 1#define Joypad_C 2#define Joypad_D 3#define Joypad_START 7#define Joypad_SELECT 5 void msleep(unsigned char milisec){	#ifdef UNIX		struct timespec req={0};		time_t sec=(unsigned short)(milisec/1000);		milisec=milisec-(sec*1000);		req.tv_sec=sec;		req.tv_nsec=milisec*1000000L;		while(nanosleep(&req,&req)==-1)		continue;	#else		SDL_Delay(milisec);	#endif}void Init_video(){	#ifdef JOYSTICK		SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);		SDL_JoystickEventState(SDL_ENABLE);	#else		SDL_Init( SDL_INIT_VIDEO );	#endif		SDL_WM_SetCaption(game_name, NULL );	SDL_ShowCursor(0);	#ifdef SCALING	const SDL_VideoInfo* info = SDL_GetVideoInfo();	scale_w = info->current_w / 320;	scale_h = info->current_h / 240;  	scale_w_total = scale_w * 320;	scale_h_total = scale_h * 240;		screen_position.x = (info->current_w - scale_w_total)/2;	screen_position.y = (info->current_h - scale_h_total)/2;		real_screen = SDL_SetVideoMode(info->current_w, info->current_h, info->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_NOFRAME);	screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240, info->vfmt->BitsPerPixel, 0,0,0,0);#else	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);#endif}void Load_Image(unsigned short a, const char* directory){		SDL_Surface *tmp;	#ifdef DEBUG		fprintf(stderr, "Clearing surface for image %d...\n", a);	#endif	if (sprites_img[a] != NULL)	{		SDL_FreeSurface(sprites_img[a]);	}	#ifdef DEBUG		fprintf(stderr, "Trying to load image %d (%s)...\n", a, directory);	#endif	#ifdef IMAGE_CODEC_ENABLED		tmp = IMG_Load(directory);	#else		tmp = SDL_LoadBMP(directory);	#endif	#ifdef DEBUG		fprintf(stderr, "Loading image %d (%s) was successful\n", a, directory);	#endif	#ifdef DEBUG		fprintf(stderr, "Trying to set color for transparency on image %d\n", a);	#endif	SDL_SetColorKey(tmp, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(tmp->format, 255, 0, 255));	sprites_img[a] = SDL_DisplayFormat(tmp);	SDL_FreeSurface(tmp);	#ifdef DEBUG		fprintf(stderr, "It seemed to work.\n");	#endif	sprites_img_tocopy[a] = 0;}void Copy_Image(unsigned short a, unsigned short i){	#ifdef DEBUG		fprintf(stderr, "Transfering the data of id %d to id %d.\n", a, i);	#endif	sprites_img_tocopy[i] = a;}void Put_image(unsigned short a, short x, short y){	SDL_Rect position;	position.x = x;	position.y = y;	#ifdef DEBUG_CRAZY		fprintf(stderr, "Put image %d on screen and update its position\n X: %d \n Y: %d\n", a, x ,y);	#endif	if (sprites_img_tocopy[a] > 0)	{		SDL_BlitSurface(sprites_img[sprites_img_tocopy[a]], NULL, screen, &position);	}	else	{		SDL_BlitSurface(sprites_img[a], NULL, screen, &position);	}}void Put_sprite(unsigned short a, short x, short y, unsigned short w, unsigned short h, unsigned char f){	SDL_Rect position;	position.x = x;	position.y = y;	SDL_Rect frame;	frame.x = f*w;	frame.y = 0;	frame.w = w;	frame.h = h;	#ifdef DEBUG_CRAZY		fprintf(stderr, "Put sprite %d on screen and update its position\n X: %d \n Y: %d\n Frame: %d\n", a, x ,y, f);	#endif	if (sprites_img_tocopy[a] > 0)	{		SDL_BlitSurface(sprites_img[sprites_img_tocopy[a]], &frame, screen, &position);	}	else	{		SDL_BlitSurface(sprites_img[a], &frame, screen, &position);	}}void Clear_screen(){	#ifdef DEBUG_CRAZY		fprintf(stderr, "Screen was cleared.\n");	#endif    SDL_FillRect(screen, 0, 0);}void Faster_clearing(short x, short y, unsigned short w, unsigned short h){	SDL_Rect position;	position.x = x;	position.y = y;	position.w = w;	position.h = h;	#ifdef DEBUG_CRAZY		fprintf(stderr, "Screen was cleared.\n");	#endif    SDL_FillRect(screen, &position, 0);}void Update_video(){#ifdef SCALING		SDL_Surface* doble;		doble = zoomSurface(screen,scale_w,scale_h,0);		SDL_BlitSurface(doble,NULL,real_screen,&screen_position);	#ifdef UNCAPPED		SDL_Flip(real_screen);	#else		Uint32 start;		start = SDL_GetTicks();		SDL_Flip(real_screen);		if(real_FPS > SDL_GetTicks()-start) msleep(real_FPS-(SDL_GetTicks()-start));	#endif		SDL_FreeSurface(doble);#else	#ifdef UNCAPPED		SDL_Flip(screen);	#else		Uint32 start;		start = SDL_GetTicks();		SDL_Flip(screen);		if(real_FPS > SDL_GetTicks()-start) msleep(real_FPS-(SDL_GetTicks()-start));	#endif#endif}void Faster_update(short x, short y, short w, short h){#ifdef SCALING		SDL_Surface* doble;		doble = zoomSurface(screen,scale_w,scale_h,0);		SDL_BlitSurface(doble,NULL,real_screen,&screen_position);	#ifdef UNCAPPED		SDL_UpdateRect(real_screen, x, y, w, h);	#else		Uint32 start;		start = SDL_GetTicks();		SDL_UpdateRect(real_screen, x, y, w, h);		if(real_FPS > SDL_GetTicks()-start) msleep(real_FPS-(SDL_GetTicks()-start));	#endif		SDL_FreeSurface(doble);#else	#ifdef UNCAPPED		SDL_UpdateRect(screen, x, y, w, h);	#else		Uint32 start;		start = SDL_GetTicks();		SDL_UpdateRect(screen, x, y, w, h);		if(real_FPS > SDL_GetTicks()-start) msleep(real_FPS-(SDL_GetTicks()-start));	#endif#endif}void Sync_refresh(){	#ifndef UNCAPPED		Uint32 start;		start = SDL_GetTicks();		if(real_FPS > SDL_GetTicks()-start) msleep(real_FPS-(SDL_GetTicks()-start));	#endif}void Draw_Pixel(unsigned short x, unsigned short y, unsigned char R, unsigned char G, unsigned char B){	SDL_LockSurface(screen);		int color_draw;	SDL_Rect scr_draw;	scr_draw.x = x;	scr_draw.y = y;	scr_draw.w = 1;	scr_draw.h = 1;		color_draw = SDL_MapRGB(screen->format, R, G, B);		SDL_FillRect(screen, &scr_draw, color_draw);		SDL_UnlockSurface(screen);}void Draw_Rect(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char R, unsigned char G, unsigned char B){	SDL_LockSurface(screen);		int color_draw;	SDL_Rect scr_draw;	scr_draw.x = x;	scr_draw.y = y;	scr_draw.w = width;	scr_draw.h = height;		color_draw = SDL_MapRGB(screen->format, R, G, B);		SDL_FillRect(screen, &scr_draw, color_draw);		SDL_UnlockSurface(screen);}void Draw_Rect_noRGB(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned short pixel){	SDL_LockSurface(screen);		SDL_Rect scr_draw;	scr_draw.x = x;	scr_draw.y = y;	scr_draw.w = width;	scr_draw.h = height;		SDL_FillRect(screen, &scr_draw, pixel);		SDL_UnlockSurface(screen);}void Controls(){		Uint8 *keystate = SDL_GetKeyState(NULL);		unsigned char i;		unsigned char joy_b[8];		short x_joy = 0, y_joy = 0;    		BUTTON.UP = 0;		BUTTON.DOWN = 0;		BUTTON.LEFT = 0;		BUTTON.RIGHT = 0;		BUTTON.A = 0;		BUTTON.B = 0;		BUTTON.C = 0;		BUTTON.D = 0;		BUTTON.START = 0;		BUTTON.SELECT = 0;		BUTTON.QUIT = 0;				for (i=0;i<8;i++)		{			joy_b[i] = 0;		}				#ifdef JOYSTICK						if (SDL_NumJoysticks() > 0)			{				joy = SDL_JoystickOpen(0);			}        			for(i=0;i<8;i++)			{				joy_b[i] = 0;			}						for(i=0;i<8;i++)			{				if (SDL_JoystickGetButton(joy, i))				{					joy_b[i] = 1;				}			}						x_joy = SDL_JoystickGetAxis(joy, 0);			y_joy = SDL_JoystickGetAxis(joy, 1);						SDL_JoystickUpdate();		#endif        if (keystate[Buttons_UP] || y_joy < -5000)        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "UP IS PRESSED\n");			#endif			BUTTON.UP = 1;        }        if (keystate[Buttons_DOWN] || y_joy > 5000)        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "DOWN IS PRESSED\n");			#endif			BUTTON.DOWN = 1;        }        if (keystate[Buttons_LEFT] || x_joy < -5000)        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "LEFT IS PRESSED\n");			#endif			BUTTON.LEFT = 1;        }        if (keystate[Buttons_RIGHT] || x_joy > 5000)        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "RIGHT IS PRESSED\n");			#endif			BUTTON.RIGHT = 1;        }        if (keystate[Buttons_A] || joy_b[Joypad_A])        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "A IS PRESSED\n");			#endif			BUTTON.A = 1;        }        if (keystate[Buttons_B] || joy_b[Joypad_B])        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "B IS PRESSED\n");			#endif			BUTTON.B = 1;        }        if (keystate[Buttons_C] || joy_b[Joypad_C])        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "C IS PRESSED\n");			#endif			BUTTON.C = 1;        }        if (keystate[Buttons_D] || joy_b[Joypad_D])        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "D IS PRESSED\n");			#endif			BUTTON.D = 1;        }        if (keystate[Buttons_START] || joy_b[Joypad_START])        {			#ifdef DEBUG				fprintf(stderr, "START IS PRESSED\n");			#endif			BUTTON.START = 1;        }        if (keystate[Buttons_SELECT] || joy_b[Joypad_SELECT])        {			#ifdef DEBUG_CRAZY				fprintf(stderr, "SELECT IS PRESSED\n");			#endif			BUTTON.SELECT = 1;        }        if (keystate[Buttons_QUIT])        {			#ifdef DEBUG				fprintf(stderr, "QUIT BUTTON IS PRESSED\n");			#endif			BUTTON.QUIT = 1;        }        SDL_Event event;        while (SDL_PollEvent(&event))        {            switch (event.type)            {                case SDL_QUIT:                {                    BUTTON.QUIT = 1;                    break;                }            }        }}void Clear_Image(unsigned short a){	if (sprites_img[a] != NULL)	{		SDL_FreeSurface(sprites_img[a]);	}	sprites_img_tocopy[a] = 0;}void Clear_Images(){	short i;	for (i=0;i<MAX_IMAGE+1;i++)	{		if (sprites_img[i] != NULL)		{			SDL_FreeSurface(sprites_img[i]);		}		sprites_img_tocopy[i] = 0;	}}#ifdef SOUND_ENABLED		void Init_sound()		{			#ifdef DEBUG				fprintf(stderr, "Init sound system\n");			#endif				Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024);				Mix_AllocateChannels(MAX_SFX);		}				void Clean_Music()		{			#ifdef DEBUG				fprintf(stderr, "Stop and clear music from memory\n");			#endif			if (music)			{				Mix_HaltMusic();				Mix_FreeMusic(music);			}		}		void Load_Music(const char* directory)		{			#ifdef DEBUG				fprintf(stderr, "Load music in memory\n");			#endif			Clean_Music();			music = Mix_LoadMUS(directory);		}		void Play_Music(char loop)		{			if (loop == 1)			{				#ifdef DEBUG					fprintf(stderr, "Playing music in loop\n");				#endif				Mix_PlayMusic(music, -1);			}			else			{				#ifdef DEBUG					fprintf(stderr, "Playing music\n");				#endif				Mix_PlayMusic(music, 0);			}		}		void Load_SFX(unsigned char i, const char* directory)		{			#ifdef DEBUG				fprintf(stderr, "Load sound effect %d (%s) in memory\n", i, directory);			#endif			if (gfx_id[i])			{				Mix_FreeChunk(gfx_id[i]);				gfx_id[i] = NULL;			}			gfx_id[i] = Mix_LoadWAV(directory);		}		void Play_SFX(unsigned char i)		{			#ifdef DEBUG				fprintf(stderr, "Play sound effect %d loaded in memory\n", i);			#endif			Mix_PlayChannel(-1, gfx_id[i], 0) ;		}		void Unload_SFX()		{			short i;			#ifdef DEBUG				fprintf(stderr, "Free sound effect from memory\n");			#endif			for (i=0;i<MAX_SFX;i++)			{				if (gfx_id[i])				{					Mix_FreeChunk(gfx_id[i]);					gfx_id[i] = NULL;				}			}		}#else		void Init_sound()		{		}				void Clean_Music()		{		}		void Load_Music(const char* directory)		{		}		void Play_Music(char loop)		{		}		void Load_SFX(unsigned char i, const char* directory)		{		}		void Play_SFX(unsigned char i)		{		}		void Unload_SFX()		{		}#endifvoid Clearing(){	#ifdef SOUND_ENABLED		Clean_Music();	#endif	#ifdef DEBUG		fprintf(stderr, "Clean surface and free SDL from memory\n");	#endif	Clear_Images();	SDL_FreeSurface(screen);		#ifdef SOUND_ENABLED		Unload_SFX();		Mix_CloseAudio();	#endif	   	SDL_Quit();}