#include "System.h"
#include "controlpad.h"
#include "graphics.h"
#include "getvideoinfo.h"
#include "displayutils.h"
#include "timerutils.h"
#include "event.h"
#include "GestionAffichage.h"
#include "GestionSprites.h"

#define FPS_VIDEO 60
#define MAX_IMAGE 64

#ifdef SOUND_ENABLED

	#include "audio.h"
	#include "musicerror.h"
	#include "score.h"         
	#include "soundplayer.h"
	#include "soundfile.h"

	#define NUMBLOCKS_SND (4)
	#define BLOCKSIZE_SND (2048)
	#define BUFSIZE_SND (NUMBLOCKS_SND*BLOCKSIZE_SND)
	#define NUMBUFFS_SND  (2)

	#define NUMBLOCKS_MUS (16)
	#define BLOCKSIZE_MUS (2048)
	#define BUFSIZE_MUS (NUMBLOCKS_MUS*BLOCKSIZE_MUS)
	#define NUMBUFFS_MUS  (4)
	#define MAXAMPLITUDE (0x7FFF)
	
	#define MAX_SFX 16

	char *samplername;
	int32 RemainingCount = 99999;
	SPPlayer *player=NULL;
	SPSound *sound=NULL;
	Item samplerins=0;
	Item outputins=0;

	Err LoopDecisionFunction (SPAction *resultAction, int32 *remainingCountP, SPSound *sound, const char *markerName);
	Err SelectFixedSamplerForFile (char **resultInstrumentName, char *fileName);

	SoundFilePlayer *gfx_id[MAX_SFX];
	int32 gfx_data[MAX_SFX];

#endif

short sprites_img[MAX_IMAGE];
char sprites_state[MAX_IMAGE];
Sprite * sprites_[MAX_IMAGE];

short PAL_border_IMG;
Sprite * PAL_border[2];

char *sprites_img_directory[MAX_IMAGE];

#include "INPUT.h"
struct input BUTTON;

unsigned short done = 0;
char* game_name = "";

unsigned short PAL_Y = 24;
unsigned char isPAL = 0;

/*  
	3DO
	CONTROLLER KEY MAPPINGS
*/
#define Buttons_UP ControlUp
#define Buttons_LEFT ControlLeft
#define Buttons_RIGHT ControlRight
#define Buttons_DOWN ControlDown
#define Buttons_A ControlA
#define Buttons_B ControlB
#define Buttons_C ControlC
#define Buttons_D ControlLeftShift
#define Buttons_START ControlStart
#define Buttons_SELECT ControlRightShift
#define Buttons_QUIT 0


void Init_video()
{
	affichageInitialisation();
	InitializeControlPads();
	affichageCouleurUnie(0);
	imageFondChargement("System/donotremove.imag");
	affichageMiseAJour();
	affichageRendu();
	
	/* if 3DO is PAL */
	if (Screen_format() > 0)
	{
		isPAL = 1;
		PAL_border[0] = spriteChargement("System/donotremove_spr.cel", 0, 0, 0, 0, 0, FALSE, 0, 0, 0, 0);
		PAL_border[1] = spriteChargement("System/donotremove_spr.cel", 0, 0, 0, 0, 0, FALSE, 0, 0, 0, 0);
		PAL_border[1] = spriteCopie(PAL_border[0]);
		listeAffichageAjout((Node *) PAL_border[0], TRUE, 1, 1);	
		listeAffichageAjout((Node *) PAL_border[1], TRUE, 1, 1);	
	}
	else
	{
		PAL_Y = 0;
	}
	
}

void Load_Image(unsigned short a, char* directory)
{
	
	/* 
		The 3DO only accepts CEL format files.
		Sprites with Multiple frames must especially be
		converted to CEL ANIM format where Width and Height are definied at convertion time.
	*/
	
	sprites_[a] = spriteChargement(directory, 0, 0, 0, 0, 0, FALSE, 0, 0, 0, 0);
	sprites_img_directory[a] = directory;
	sprites_state[a] = 0;
}

void Copy_Image(unsigned short a, unsigned char i)
{
	/* sprites_[i] = spriteChargement(sprites_img_directory[a], 0, 0, 0, 0, 0, FALSE, 0, 0, 0, 0); */
	/*sprites_[a] = spriteCopie(sprites_[i]);*/
	/*sprites_[a] = spriteClone(sprites_[i]);*/
	Load_Image(i, sprites_img_directory[a]);
	sprites_state[i] = 0;
}

void Put_image(unsigned short a, short x, short y)
{
	if (!sprites_state[a])
	{
		listeAffichageAjout((Node *) sprites_[a], TRUE, 1, 1);	
		sprites_state[a] = 1;
	}
	
	spriteModificationPosition(sprites_[a], x, y+PAL_Y, FALSE);
	
}

void Put_sprite(unsigned short a, short x, short y, unsigned short w, unsigned short h, unsigned char f)
{

	/* 
		The 3DO only accepts CEL format files.
		Sprites with Multiple frames must be especially
		converted to CEL ANIM format where Width
		and Height are definied at convertion time.
		w and h parameters are simply ignored here.
	*/
	
	if (sprites_state[a] == 0)
	{
		listeAffichageAjout((Node *) sprites_[a], TRUE, 1, 1);	
		sprites_state[a] = 1;
	}
	
	spriteModificationPosition(sprites_[a], x, y+PAL_Y, FALSE);
	spriteConfigurationFrame(sprites_[a], f);
}

void Faster_clearing(short x, short y, unsigned short w, unsigned short h)
{
	
}

void Clear_screen()
{

}

void Update_video()
{
	/* Display PAL borders */
	if (isPAL == 1)
	{
		spriteModificationPosition(PAL_border[0], 0,   0, FALSE);
		spriteModificationPosition(PAL_border[1], 0, 240, FALSE);
	}
	
	affichageMiseAJour();
	affichageRendu();
}

void Faster_update(short x, short y, short w, short h)
{
	Update_video();
}

void Sync_Refresh()
{

}

void Draw_Pixel(unsigned short x, unsigned short y, unsigned char R, unsigned char G, unsigned char B)
{
	fill_rectangle(x, y, 1, 1, R, G, B);
}

void Draw_Rect(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char R, unsigned char G, unsigned char B)
{
	fill_rectangle(x, y, width, height, R, G, B);
}

void Draw_Rect_noRGB(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned short pixel)
{
	fill_rectangle_norgb(x, y, width, height, pixel);
}

void Controls()
{
		uint32	gButtons;
		DoControlPad(1, &gButtons, (ControlUp | ControlDown | ControlLeft | ControlRight));
		
		BUTTON.UP = 0;	
		BUTTON.DOWN = 0;	
		BUTTON.LEFT = 0;	
		BUTTON.RIGHT = 0;	
		BUTTON.A = 0;	
		BUTTON.B = 0;	
		BUTTON.C = 0;	
		BUTTON.D = 0;	
		BUTTON.START = 0;	
		BUTTON.SELECT = 0;	
		
		if (gButtons & Buttons_UP)	
		{
			BUTTON.UP = 1;	
		}
		
		if (gButtons & Buttons_DOWN)	
		{
			BUTTON.DOWN = 1;	
		}
		
		if (gButtons & Buttons_LEFT)	
		{
			BUTTON.LEFT = 1;	
		}
		
		if (gButtons & Buttons_RIGHT)	
		{
			BUTTON.RIGHT = 1;	
		}
		
		if (gButtons & Buttons_A)	
		{
			BUTTON.A = 1;	
		}
		
		if (gButtons & Buttons_B)	
		{
			BUTTON.B = 1;	
		}
		
		if (gButtons & Buttons_C)	
		{
			BUTTON.C = 1;	
		}
		
		if (gButtons & Buttons_D)	
		{
			BUTTON.D = 1;	
		}
		
		if (gButtons & Buttons_START)	
		{
			BUTTON.START = 1;	
		}
			
		if (gButtons & Buttons_SELECT)	
		{
			BUTTON.SELECT = 1;	
		}
		
}

void Clear_Image(unsigned short a)
{
	/* 
	    The image is still in memory but it will not be shown.
		I need to find a proper way to delete images
		other than loading a new and smaller one
	*/
	listeAffichageSuppression((Node *) sprites_[a]);
}

void Clear_Images()
{
	unsigned short i;

	for (i=0;i<MAX_IMAGE+1;i++)
	{
		if (sprites_[i] != NULL)
		{
			listeAffichageSuppression((Node *) sprites_[i]);
		}
	}
}


#ifdef SOUND_ENABLED

	void Init_sound()
	{
		OpenAudioFolio();
	}
	
	void Clean_Music()
	{
		if (player)
		{
			spStop (player);
			spDeletePlayer (player);
		}
		
		if (outputins)
		{
			UnloadInstrument (outputins);
		}
		
		if (samplerins)
		{
			UnloadInstrument (samplerins);
		}
	}

	void Load_Music(char* filename)
	{
		short Result;
		
		SelectFixedSamplerForFile (&samplername, filename);
		samplerins = LoadInstrument (samplername, 0, 100);
		outputins = LoadInstrument ("directout.dsp", 0, 100);
					
		/* try mono connection */
		
		ConnectInstruments (samplerins, "Output", outputins, "InputLeft");
		Result = ConnectInstruments (samplerins, "Output", outputins, "InputRight");

		/* if that failed, try stereo connection */
		
		if (Result < 0) 
		{
			Result = ConnectInstruments (samplerins, "LeftOutput", outputins, "InputLeft");
			Result = ConnectInstruments (samplerins, "RightOutput", outputins, "InputRight");
		}

		/* start output instrument */
		
		StartInstrument (outputins, NULL);
		
		/* create player */
		
		spCreatePlayer (&player, samplerins, NUMBUFFS_MUS, BUFSIZE_MUS, NULL);
		spAddSoundFile (&sound, player, filename);
		spLoopSound (sound);
		spSetMarkerDecisionFunction (sound, SP_MARKER_NAME_END, (SPDecisionFunction)LoopDecisionFunction,&RemainingCount);
		spStartReading (sound, SP_MARKER_NAME_BEGIN);
		spStartPlayingVA (player, AF_TAG_AMPLITUDE, MAXAMPLITUDE,TAG_END);
	}

	void Play_Music()
	{
		static unsigned char wait_sound = 0;

		if (wait_sound > 160) 
		{
			spService (player, WaitSignal (spGetPlayerSignalMask (player)));
			wait_sound = 0;
		}	
		wait_sound = wait_sound + 1;
	}

	void Load_SFX(unsigned char i, char* directory)
	{
		if (gfx_id[i])
		{
			StopSoundFile (gfx_id[i]);
			UnloadSoundFile(gfx_id[i]);
			DeleteSoundFilePlayer(gfx_id[i]);
		}
		gfx_id[i] = CreateSoundFilePlayer ( NUMBLOCKS_SND, BUFSIZE_SND, NULL );
		gfx_data[i] = LoadSoundFile( gfx_id[i], directory );
	}

	void Play_SFX(unsigned char i)
	{
		gfx_data[i] = StartSoundFile( gfx_id[i], MAXAMPLITUDE );
	}

	void Unload_SFX()
	{
		unsigned char a;

		for (a=0;a<MAX_SFX;a++)
		{
			if (gfx_id[a])
			{
				StopSoundFile (gfx_id[a]);
				UnloadSoundFile(gfx_id[a]);
				DeleteSoundFilePlayer(gfx_id[a]);
			}
		}
	}

	Err LoopDecisionFunction (SPAction *resultAction, int32 *remainingCountP, SPSound *sound, const char *markerName)
	{
		(*remainingCountP)--;

		if (*remainingCountP <= 0)
		{
			return spSetStopAction (resultAction);
		}
		else
		{
			return 0;
		}
	}

	Err SelectFixedSamplerForFile (char **resultSamplerName, char *fileName)
	{
		char *InstrumentName = NULL;
		Item TempSample = 0;
		short Result = 0;

		if (!resultSamplerName) return ML_ERR_BADPTR;

		TempSample = ScanSample (fileName, 0);

		InstrumentName = SelectSamplePlayer (TempSample, FALSE);
		if (InstrumentName == NULL)
		{
			goto error;
		}

		error:
		UnloadSample (TempSample);
		*resultSamplerName = InstrumentName;
		return Result;
	}


#else
		void Init_sound()
		{
		}
		
		void Clean_Music()
		{
		}

		void Load_Music(const char* directory)
		{
		}

		void Play_Music(char loop)
		{
		}

		void Load_SFX(unsigned char i, const char* directory)
		{
		}

		void Play_SFX(unsigned char i)
		{
		}

		void Unload_SFX()
		{
		}
#endif


void Clearing()
{
	Clear_Images();
	Clean_Music();
	Unload_SFX();
}
