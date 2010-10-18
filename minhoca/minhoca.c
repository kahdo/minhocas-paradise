#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define SETVIDEOMODEFLAGS (SDL_SWSURFACE|SDL_FULLSCREEN)

#define SNAKEWIDTH 10
#define SNAKEHEIGHT 10

#define SNAKECOLORR 0
#define SNAKECOLORG 255
#define SNAKECOLORB 0

#include "basicdefs.h"
#include "ai.h"
#include "fonte.h"
#include "snake.h"

/* brain */
minhocabrain brain;

/* option vars */
int fps = 60;
int mspf = 0; 
Uint32 before, after;

int xres = 320; // -x
int yres = 240; // -y
int bpp = 8; // -b
int surfaceparams = SDL_SWSURFACE; // -f

void parse_cmd(int argc,char* argv)
{
	int opt;
	while ( (opt=getopt(argc, (char * const*)argv, "x:y:b:f")) != -1) {
		switch(opt) {
			case 'x':
				xres = atoi(optarg);
				break;
			case 'y':
				yres = atoi(optarg);
				break;
			case 'b':
				bpp = atoi(optarg);
				break;
			case 'f':
				surfaceparams |= SDL_FULLSCREEN;
				break;
			default: // _?????
				printf("Minhoca's Paradise:\n-x n\tX res\n-y n\tY res\n-b n\tBPP\n-f\tFULLSCREEN\n");
				exit(1);
		}
	}
	
}
		
int video_init()
{
	int retval = 0;
	if ((retval = SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO )) == -1) {
		printf("ERROR: SDL_Init(): %s\n",SDL_GetError());
		exit(1);
	} else { /* continue init */
	}
	
}



int main(int argc, char *argv)
{
	// Calculate desired FPS
	mspf = 100000/fps;

	// get params
	parse_cmd(argc, argv);

	/* Field's variables */
	int fieldx = (xres/SNAKEWIDTH);
	int fieldy = (yres/SNAKEHEIGHT);

	/* Food variables */
	posxy food;
	int hasfood = 0;
	
	/* Snake's variables */
	snake thesnake;
	
	/* SDL/Drawing Support variables */
	SDL_Surface *screen;
	SDL_Event event;
	SDL_Rect headdraw;

	/* Control/Game variables */
	long unsigned int score = 0;
	int roda = 1;
	int checkgo = 0;
	int direcao = RIGHT;
	int chdir = 0;

	// ---------------------

	// Initialize random number generator with time.
	srand((unsigned int)time(NULL));


	// Create one snake
	initsnake(&thesnake, fieldx/2, fieldy/2, 5);

	// Calculate Snake's Initial Position

	// Init SDL
	video_init();

	// Init screen
	screen = SDL_SetVideoMode( xres, yres, bpp , surfaceparams);

	SDL_Rect dst;
	dst.x = 0;
	dst.y = 0;
	SDL_Surface *number;
	number = plotlongnumber(score);
/* Main Loop*/
while(roda == 1) {
	before = SDL_GetTicks()*100;

	// Score
	SDL_BlitSurface(number, NULL, screen, &dst);
	SDL_FreeSurface(number);
	number = plotlongnumber(score);

	/* Read the Keyboard*/
	while(SDL_PollEvent(&event) && !chdir) {
		switch(event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q) roda = 0;
				if (event.key.keysym.sym == SDLK_LEFT && direcao != RIGHT) { direcao = LEFT; chdir = 1;}
				if (event.key.keysym.sym == SDLK_RIGHT && direcao != LEFT) { direcao = RIGHT; chdir = 1;}
				if (event.key.keysym.sym == SDLK_UP && direcao != DOWN) { direcao = UP; chdir = 1;}
				if (event.key.keysym.sym == SDLK_DOWN && direcao != UP) { direcao = DOWN; chdir = 1;}
			break;
		}
	}

	/* ai test */
	direcao = ai_run(&brain, snakehead(&thesnake), &food, hasfood, direcao);

	/* Erase TAIL */

	headdraw.x = (snaketail(&thesnake)->x)*SNAKEWIDTH;
	headdraw.y = (snaketail(&thesnake)->y)*SNAKEHEIGHT;
	headdraw.w = SNAKEWIDTH;
	headdraw.h = SNAKEHEIGHT;
	SDL_FillRect(screen, &headdraw, SDL_MapRGB(screen->format, 0, 0, 0));

	/* Calculate and print HEAD */
	thesnake.hpos = (thesnake.hpos+(thesnake.size-1))%thesnake.size;

	switch(direcao) {
		case UP:
			snakehead(&thesnake)->x = snakeneck(&thesnake)->x;
			snakehead(&thesnake)->y = (snakeneck(&thesnake)->y + (fieldy-1))%fieldy;
			break;
		case DOWN:
			snakehead(&thesnake)->x = snakeneck(&thesnake)->x;
			snakehead(&thesnake)->y = (snakeneck(&thesnake)->y + 1)%fieldy;
			break;
		case LEFT:
			snakehead(&thesnake)->x = (snakeneck(&thesnake)->x + (fieldx-1))%fieldx;
			snakehead(&thesnake)->y = snakeneck(&thesnake)->y;
			break;
		case RIGHT:
			snakehead(&thesnake)->x = (snakeneck(&thesnake)->x + 1)%fieldx;
			snakehead(&thesnake)->y = snakeneck(&thesnake)->y;
			break;
	}
	
	/* Detect game over */
	
	for ( checkgo = 3; checkgo <= (thesnake.size-1) ; checkgo++) {
		if ((snakehead(&thesnake)->x == (thesnake.data+((thesnake.hpos+checkgo)%thesnake.size))->x) &&
			(snakehead(&thesnake)->y == (thesnake.data+((thesnake.hpos+checkgo)%thesnake.size))->y)) roda = 0;
	}

	// create new food that doesn't overlaps with snake!
	if(!hasfood) {
			int hit = 0;
			
			food.x = rand()%fieldx;
			food.y = rand()%fieldy;
			//food.y = 10;
			
			for ( checkgo = 0; checkgo <= (thesnake.size-1) ; checkgo++) {
				if ((food.x == (thesnake.data+((thesnake.hpos+checkgo)%thesnake.size))->x) &&
				(food.y == (thesnake.data+((thesnake.hpos+checkgo)%thesnake.size))->y)) 
				hit = 1;
			}

			if (!hit) 
				hasfood=1;
	}
	
	// food on screen? draw.
	if(hasfood) {	
		headdraw.x = food.x*SNAKEWIDTH;
		headdraw.y = food.y*SNAKEHEIGHT;
		headdraw.w = SNAKEWIDTH;
		headdraw.h = SNAKEHEIGHT;
		SDL_FillRect(screen, &headdraw, SDL_MapRGB(screen->format, 255, 255, 0));
	}

	// Food eaten?
	if( (snakehead(&thesnake)->x ==	food.x) && (snakehead(&thesnake)->y == food.y) ) {
		hasfood = 0;
		score += 10;
		growsnake(&thesnake, 2);
	}
	
	// Draw HEAD
	headdraw.x = (snakehead(&thesnake)->x)*SNAKEWIDTH;
	headdraw.y = (snakehead(&thesnake)->y)*SNAKEHEIGHT;
	headdraw.w = SNAKEWIDTH;
	headdraw.h = SNAKEHEIGHT;
	SDL_FillRect(screen, &headdraw, SDL_MapRGB(screen->format, SNAKECOLORR, SNAKECOLORG, SNAKECOLORB));

	/* Re-enable changing directions */
	chdir = 0;

	
	// Flip screen
	SDL_Flip(screen);

	// Limit framerate
	after = mspf - (SDL_GetTicks()*100 - before);
	if ((long int)after>0) {
//		printf("%d\n",after/100);
		SDL_Delay((long int)(after/100));
	}

	}

}