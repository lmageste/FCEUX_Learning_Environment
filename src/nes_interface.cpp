#include "nes_interface.hpp"
#include "drivers/sdl/config.h"
#include "drivers/sdl/sdl.h"
#include "driver.h"
#include "fceu.h"
#include "cheat.h"
#include "video.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <SDL/SDL.h>

// Global configuration info.
extern Config *g_config;
extern int noGui;
extern uint8_t *XBuf;

namespace nes {

// Return the RGB value of a given pixel we got from XBuf.
void NESInterface::getRGB(
    unsigned char pixel,
    unsigned char *red,
    unsigned char *green,
    unsigned char *blue) {
	FCEUD_GetPalette(pixel, red, green, blue);
}

class NESInterface::Impl {

    public:

        // create an NESInterface. This routine is not threadsafe!
        Impl(const std::string &rom_file);
        ~Impl();

        // Resets the game
        void reset_game();

        // Indicates if the game has ended
        bool gameOver();

        // Applies an action to the game and returns the reward. It is the user's responsibility
        // to check if the game has ended and reset when necessary - this method will keep pressing
        // buttons on the game over screen.
        int act(int action);

        // Returns the number of legal actions.
        int getNumLegalActions();

        // Returns the vector of legal actions.
        void getLegalActionSet(int legal_actions[]);

        // Minimum possible instantaneous reward.
        int minReward() const;

        // Maximum possible instantaneous reward.
        int maxReward() const;

        // The remaining number of lives.
        int lives() const;

        // Returns the frame number since the loading of the ROM
        int getFrameNumber() const;

        // Returns the frame number since the start of the current episode
        int getEpisodeFrameNumber() const;

        // Sets the episodic frame limit
        void setMaxNumFrames(int newMax);

        // Returns the current game screen
        void getScreen(unsigned char *screen, int screen_size);

        // Return screen height.
        const int getScreenHeight() const;

        // Return screen width.
        const int getScreenWidth() const;

        // Returns the current score.
        const int getCurrentScore() const;

        // Saves the state of the system
        void saveState();

        // Loads the state of the system
        bool loadState();

        // Gets current state as string
        std::string getSnapshot() const;

        // restores state from a string
        void restoreSnapshot(const std::string snapshot);

        // Get the RGB data from the raw screen.
        void fillRGBfromPalette(unsigned char *raw_screen, unsigned char *rgb_screen, int raw_screen_size);

	//new-created functions for multiple games
	int getGameState();
	int getLives();
	int getPoints();
	void initializeGamesData();

    private:

        int m_episode_score; // Score accumulated throughout the course of an episode
        bool m_display_active;    // Should the screen be displayed or not
        int m_max_num_frames;     // Maximum number of frames for each episode
        int nes_input; // Input to the emulator.
        int current_game_score;
        //int current_x;
        int remaining_lives;
        int game_state;
        int episode_frame_number;

        //TODO: initialize everything
        int curGame;
        int mappedActions[NUMBER_OF_ACTIONS] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 80, 
        	144, 96, 160, 17, 33, 65, 129, 81, 145, 97, 161, 18, 34,
        	66, 130, 82, 146, 98, 162, 3, 19, 35, 67, 131, 83, 147,
        	 99, 163};
        int allowedActions[NUMBER_OF_GAMES][NUMBER_OF_ACTIONS];
        std::map<std::string, int> allowedGames;
};


NESInterface::Impl::~Impl() {

	CloseGame();
	FCEUI_Kill();
	SDL_Quit();
}

bool NESInterface::Impl::loadState() {

	// TODO implement
	printf("loadState is not implemented...");
	//FCEUI_LoadState();
	return false;
}

bool NESInterface::Impl::gameOver() {

	// Update game state.
	game_state = NESInterface::Impl::getGameState();

	// Return true only if this byte is not 1.
	if (game_state == 1) return false;

	// Reset the score and position.
	//current_game_score = 0;
	//current_x = 0;
	return true;
}

void NESInterface::Impl::reset_game() {

	// Pretty simple...
	ResetNES();

	// Initialize the score,position, and frame counter.
	current_game_score = 0;
	//current_x = 0;
	episode_frame_number = 0;

	//printf("!!!Game Reseted: number %d\n", curGame);

	// Run a few frames first to get to the startup screen.
	for (int i = 0; i<60; i++) {
		NESInterface::Impl::act(ACT_NOOP);
	}

	// Hit the start button...
	for (int i = 0; i<5; i++) {
		if(curGame == DOUBLE_DRAGON)
			NESInterface::Impl::act(ACT_SELECT);
		if(curGame == MACH_RIDER)
			NESInterface::Impl::act(ACT_DOWN);
		else
			NESInterface::Impl::act(ACT_START);
	}

	if(curGame == LIFEFORCE){
		for (int i = 0; i < 100; ++i)
		{
			NESInterface::Impl::act(ACT_NOOP);
		}
	}

	if(curGame == CONTRA || curGame == DOUBLE_DRAGON || curGame == GALAGA ||
		curGame == GRADIUS || curGame == HUDSONS_ADVENTURE_ISLAND ||
		curGame == MACH_RIDER || curGame == RAMPAGE || curGame == PUNCH_OUT ||
		curGame == RAD_RACER || curGame == SPY_HUNTER){
		for (int i = 0; i<10; i++) {
			NESInterface::Impl::act(ACT_NOOP);
		}
		for (int i = 0; i<5; i++) {
			NESInterface::Impl::act(ACT_START);
		}
		if(curGame == RAD_RACER)
			for (int i = 0; i<10; i++) {
				NESInterface::Impl::act(ACT_NOOP);
			}
			for (int i = 0; i<5; i++) {
				NESInterface::Impl::act(ACT_START);
			}
			for (int i = 0; i<10; i++) {
				NESInterface::Impl::act(ACT_NOOP);
			}
			for (int i = 0; i<5; i++) {
				NESInterface::Impl::act(ACT_A);
			}
	}
}

void NESInterface::Impl::saveState() {

	// TODO implement
	printf("saveState NOT IMPLEMENTED.\n");
	//		FCEUD_SaveStateAs ();
}

std::string NESInterface::Impl::getSnapshot() const {

	// TODO implement
	printf("getSnapshot NOT IMPLEMENTED.\n");
	/*
    const ALEState* state = m_emu->environment->cloneState();
    std::string snapshot = state->getStateAsString();
    m_emu->environment->destroyState(state);
    return snapshot;
    */
	return "";
}

void NESInterface::Impl::restoreSnapshot(const std::string snapshot) {

	// TODO implement
	printf("restoreSnapshot NOT IMPLEMENTED.\n");
    //ALEState state(snapshot);
    //m_emu->environment->restoreState(state);
}

void NESInterface::Impl::getScreen(unsigned char *screen, int screen_size) {
        memcpy(screen, XBuf, screen_size);
}

const int NESInterface::Impl::getScreenHeight() const {
	return FSettings.LastSLine - FSettings.FirstSLine+1;
}

const int NESInterface::Impl::getScreenWidth() const {
	return NES_SCREEN_WIDTH;
}

void NESInterface::Impl::fillRGBfromPalette(unsigned char *raw_screen, unsigned char *rgb_screen, int raw_screen_size) {

        unsigned long i;
        for (i = 0; i<raw_screen_size; i++) {
                unsigned char r, g, b;
                NESInterface::getRGB(raw_screen[i], &r, &g, &b);

                // Man, this bastard took a long time to figure out!
                rgb_screen[3*i] = b;
                rgb_screen[(3*i)+1] = g;
                rgb_screen[(3*i)+2] = r;
        }
}

void NESInterface::Impl::setMaxNumFrames(int newMax) {
    m_max_num_frames = newMax;
}

int NESInterface::Impl::getEpisodeFrameNumber() const {
	return episode_frame_number;
}

int NESInterface::Impl::getFrameNumber() const {

	// TODO implement
	printf("getFrameNumber NOT IMPLEMENTED.\n");
    //return m_emu->environment->getFrameNumber();
	return 0;
}

int NESInterface::Impl::getNumLegalActions() {
	int count = 0;
	for(int i=0; i<NUMBER_OF_ACTIONS; i++){
		if(allowedActions[curGame][i]==1)
			count++;
	}

	return count;
}

void NESInterface::Impl::getLegalActionSet(int legal_actions[]) {
    int count =0;
	for(int i=0; i<NUMBER_OF_ACTIONS; i++){
		if(allowedActions[curGame][i]==1){
			legal_actions[count] = i;
			printf("action in slot %d is %d\n", count, i);
			count++;
		}
	}
}

int NESInterface::Impl::minReward() const {

	// TODO implement.
	printf("minReward NOT IMPLEMENTED.\n");
    //return m_rom_settings->minReward();
	return 0;
}

int NESInterface::Impl::maxReward() const {

	// TODO implement.
	printf("maxReward NOT IMPLEMENTED.\n");
    //return m_rom_settings->maxReward();
	return 0;
}

int NESInterface::Impl::lives() const {
	return remaining_lives;
}

const int NESInterface::Impl::getCurrentScore() const {
	return current_game_score;
}

int NESInterface::Impl::getLives(){
	int lives = 0; //ok all

	switch(curGame){
		case BALLOON_FIGHT:
			lives = FCEU_CheatGetByte(0x0041);
		break;
		case BREAKTHRU:
			lives = FCEU_CheatGetByte(0x00b2);
		break;
		case BUMP_N_JUMP:
			lives = FCEU_CheatGetByte(0x004d);
		break;
		case CONTRA:
			lives = FCEU_CheatGetByte(0x0032);
		break;
		case DOUBLE_DRAGON:
			lives = FCEU_CheatGetByte(0x0043);
		break;
		case GALAGA:
			lives = FCEU_CheatGetByte(0x0485);
		break;
		case GRADIUS:
			lives = FCEU_CheatGetByte(0x0020);
		break;
		case GUNSMOKE:
			lives = FCEU_CheatGetByte(0x007a);
		break;
		case HUDSONS_ADVENTURE_ISLAND:
			lives = FCEU_CheatGetByte(0x003f);
		break;
		case JOUST:
			lives = FCEU_CheatGetByte(0x00e9);
		break;
		case KUNGFU:
			lives = FCEU_CheatGetByte(0x005c);
		break;
		case LIFEFORCE:
			lives = FCEU_CheatGetByte(0x0034);
		break;
		case MACH_RIDER:
			//no lives
			lives = 0;
		break;
		case PUNCH_OUT:
			//no lives
			lives = 0;
		break;
		case RAD_RACER:
			//no lives
			lives = 0;
		break;
		case RAMPAGE:
			//no lives
			lives = 0;
		break;
		case SPY_HUNTER:
			//no lives
			lives = 0;
		break;
	}

	//printf("vidas valem: %d\n", lives);

	return lives;
}

//if gamestate equals 1: game not on gameover

int NESInterface::Impl::getGameState(){
	int gameState=0; //ok all

	switch(curGame){
		case BALLOON_FIGHT:
			gameState = FCEU_CheatGetByte(0x0041);
			gameState = (gameState == 255)?0:1;
		break;
		case BREAKTHRU:
			gameState = FCEU_CheatGetByte(0x00b2);
			gameState = (gameState == 0)?0:1;
		break;
		case BUMP_N_JUMP:
			gameState = FCEU_CheatGetByte(0x004d);
			gameState = (gameState == 0)?0:1;
		break;
		case CONTRA:
			gameState = FCEU_CheatGetByte(0x0038);
			gameState = (gameState == 1)?0:1;
		break;
		case DOUBLE_DRAGON:
			gameState = FCEU_CheatGetByte(0x0043);
			gameState = (gameState == 255)?0:1;
		break;
		case GALAGA:
			gameState = FCEU_CheatGetByte(0x0671);
			gameState = (gameState == 254)?0:1;
		break;
		case GRADIUS: 
			gameState = FCEU_CheatGetByte(0x0020);
			gameState = (gameState == 255)?0:1;
		break;
		case GUNSMOKE:
			gameState = FCEU_CheatGetByte(0x06a1);
			gameState = (gameState == 49)?0:1;
		break;
		case HUDSONS_ADVENTURE_ISLAND:
			gameState =  FCEU_CheatGetByte(0x0048);
			gameState = (gameState == FCEU_CheatGetByte(0x003f) && gameState == 1)?0:1;
		break;
		case JOUST:
			gameState = FCEU_CheatGetByte(0x00e4);
			gameState = (gameState == 1)?0:1;
		break;
		case KUNGFU:
			gameState = FCEU_CheatGetByte(0x005c);
			gameState = (gameState == 0)?0:1;
		break;
		case LIFEFORCE:
			gameState = FCEU_CheatGetByte(0x0034);
			gameState = (gameState == 0)?0:1;
		break;
		case MACH_RIDER:
			gameState = FCEU_CheatGetByte(0x0023);
			gameState = (gameState == 0)?0:1;
		break;
		case PUNCH_OUT:
			gameState = FCEU_CheatGetByte(0x0173);
			gameState = (gameState == 3)?0:1;
		break;
		case RAD_RACER:
			gameState = FCEU_CheatGetByte(0x00ba);
			gameState = (gameState == 115)?0:1;
		break;
		case RAMPAGE:
			gameState = FCEU_CheatGetByte(0x0101);
			gameState = (gameState == 2)?0:1;
		break;
		case SPY_HUNTER:
			gameState = FCEU_CheatGetByte(0x00f0);
			gameState = (gameState == 0)?0:1;
		break;
	}

	//printf("gamestate vale %d\n", gameState);

	return gameState;
}

int NESInterface::Impl::getPoints(){
	int points = 0;
	switch(curGame){
		case BALLOON_FIGHT: //ok
			points = (FCEU_CheatGetByte(0x0007) * 100000) +
				(FCEU_CheatGetByte(0x0006) * 10000) +
				(FCEU_CheatGetByte(0x0005) * 1000) +
				(FCEU_CheatGetByte(0x0004) * 100) +
				(FCEU_CheatGetByte(0x0003) * 10) +
				(FCEU_CheatGetByte(0x0002) * 1);
		break;
		case BREAKTHRU: //ok
			points = (FCEU_CheatGetByte(0x0091) * 100000) +
				(FCEU_CheatGetByte(0x0092) * 10000) +
				(FCEU_CheatGetByte(0x0093) * 1000) +
				(FCEU_CheatGetByte(0x0094) * 100) +
				(FCEU_CheatGetByte(0x0095) * 10) +
				(FCEU_CheatGetByte(0x0096) * 1);
		break;
		case BUMP_N_JUMP: //ok
			points = (FCEU_CheatGetByte(0x0053) * 100000) +
				(FCEU_CheatGetByte(0x0052) * 10000) +
				(FCEU_CheatGetByte(0x0051) * 1000) +
				(FCEU_CheatGetByte(0x0050) * 100) +
				(FCEU_CheatGetByte(0x004f) * 10) +
				(FCEU_CheatGetByte(0x004e) * 1);
		break;
		case CONTRA: //ok (with 10 mult factor so that each kill will be worth 10 points)
			points = 10*((FCEU_CheatGetByte(0x07e5) << 6) +
				(FCEU_CheatGetByte(0x07e4) << 4) +
				(FCEU_CheatGetByte(0x07e3) << 2) +
				(FCEU_CheatGetByte(0x07e2)));
		break;
		case DOUBLE_DRAGON: //ok
			points = (FCEU_CheatGetByte(0x0040) * 1600) +
				(FCEU_CheatGetByte(0x0042) * 160) +
				(FCEU_CheatGetByte(0x0041) );
			points=points*50/32;
		break;
		case GALAGA: //ok
			points = (FCEU_CheatGetByte(0x00e0) * 1000000) +
				(FCEU_CheatGetByte(0x00e1) * 100000) +
				(FCEU_CheatGetByte(0x00e2) * 10000) +
				(FCEU_CheatGetByte(0x00e3) * 1000) +
				(FCEU_CheatGetByte(0x00e4) * 100) +
				(FCEU_CheatGetByte(0x00e5) * 10) +
				(FCEU_CheatGetByte(0x00e6) * 1);
		break;
		case GRADIUS: //ok
			points = (FCEU_CheatGetByte(0x07e6)/16 * 100000) +
				(FCEU_CheatGetByte(0x07e6)%16 * 10000) +
				(FCEU_CheatGetByte(0x07e5)/16 * 1000) +
				(FCEU_CheatGetByte(0x07e5)%16 * 100) +
				(FCEU_CheatGetByte(0x07e4)/16 * 10) +
				(FCEU_CheatGetByte(0x07e4)%16 * 1);
		break;
		case GUNSMOKE: //ok
			points = ((FCEU_CheatGetByte(0x06f2)-88) * 100000) +
				((FCEU_CheatGetByte(0x06f4)-88) * 10000) +
				((FCEU_CheatGetByte(0x06f6)-88) * 1000) +
				((FCEU_CheatGetByte(0x06f8)-88) * 100) +
				((FCEU_CheatGetByte(0x06fa)-88) * 10) +
				((FCEU_CheatGetByte(0x06fc)-88) * 1);
		break;
		case HUDSONS_ADVENTURE_ISLAND: //ok
			points = ((FCEU_CheatGetByte(0x069b)==255?0:(FCEU_CheatGetByte(0x069b)-245)) * 10000000) +
				((FCEU_CheatGetByte(0x069c)==255?0:(FCEU_CheatGetByte(0x069c)-245)) * 1000000) +
				((FCEU_CheatGetByte(0x069d)==255?0:(FCEU_CheatGetByte(0x069d)-245)) * 100000) +
				((FCEU_CheatGetByte(0x069e)==255?0:(FCEU_CheatGetByte(0x069e)-245)) * 10000) +
				((FCEU_CheatGetByte(0x069f)==255?0:(FCEU_CheatGetByte(0x069f)-245)) * 1000) +
				((FCEU_CheatGetByte(0x06a0)==255?0:(FCEU_CheatGetByte(0x06a0)-245)) * 100) +
				((FCEU_CheatGetByte(0x06a1)==255?0:(FCEU_CheatGetByte(0x06a1)-245)) * 10) +
				((FCEU_CheatGetByte(0x06a2)==255?0:(FCEU_CheatGetByte(0x06a2)-245))* 1);
		break;
		case JOUST: //ok
			points = (FCEU_CheatGetByte(0x00ed)/16 * 100000) +
				(FCEU_CheatGetByte(0x00ed)%16 * 10000) +
				(FCEU_CheatGetByte(0x00ec)/16 * 1000) +
				(FCEU_CheatGetByte(0x00ec)%16 * 100) +
				(FCEU_CheatGetByte(0x00eb)/16 * 10) +
				(FCEU_CheatGetByte(0x00eb)%16 * 1);
		break;
		case KUNGFU: //ok
			points = 100*(FCEU_CheatGetByte(0x0531) * 1000000) +
				(FCEU_CheatGetByte(0x0532) * 10000) +
				(FCEU_CheatGetByte(0x0533) * 100) +
				(FCEU_CheatGetByte(0x0534) * 1);
		break;
		case LIFEFORCE: //ok
			points = (FCEU_CheatGetByte(0x07e6)/16 * 100000) +
				(FCEU_CheatGetByte(0x07e6)%16 * 10000) +
				(FCEU_CheatGetByte(0x07e5)/16 * 1000) +
				(FCEU_CheatGetByte(0x07e5)%16 * 100) +
				(FCEU_CheatGetByte(0x07e4)/16 * 10) +
				(FCEU_CheatGetByte(0x07e4)%16 * 1);
		break;
		case MACH_RIDER: //ok with 100 mult factor
			points = 100*((FCEU_CheatGetByte(0x0000) * 10000) +
				(FCEU_CheatGetByte(0x0001) * 1000) +
				(FCEU_CheatGetByte(0x0002) * 100) +
				(FCEU_CheatGetByte(0x0003) * 10) +
				(FCEU_CheatGetByte(0x0004) * 1));
		
		break;
		case PUNCH_OUT: //ok
			points = (FCEU_CheatGetByte(0x03e8) * 100000) +
				(FCEU_CheatGetByte(0x03e9) * 10000) +
				(FCEU_CheatGetByte(0x03ea) * 1000) +
				(FCEU_CheatGetByte(0x03eb) * 100) +
				(FCEU_CheatGetByte(0x03ec) * 10) +
				(FCEU_CheatGetByte(0x03ed) * 1);
		break;
		case RAD_RACER: //ok
			points = ((FCEU_CheatGetByte(0x0563)-48) * 10000) +
				((FCEU_CheatGetByte(0x0564)-48) * 1000) +
				((FCEU_CheatGetByte(0x0565)-48) * 100) +
				((FCEU_CheatGetByte(0x0566)-48) * 10) +
				((FCEU_CheatGetByte(0x0567)-48) * 1);
		break;
		case RAMPAGE: //ok
			points = ((FCEU_CheatGetByte(0x0119)==255)?0:(FCEU_CheatGetByte(0x0119)) * 10000000) +
				((FCEU_CheatGetByte(0x011a)==255)?0:(FCEU_CheatGetByte(0x011a)) * 1000000) +
				((FCEU_CheatGetByte(0x011b)==255)?0:(FCEU_CheatGetByte(0x011b)) * 100000) +
				((FCEU_CheatGetByte(0x011c)==255)?0:(FCEU_CheatGetByte(0x011c)) * 10000) +
				((FCEU_CheatGetByte(0x011d)==255)?0:(FCEU_CheatGetByte(0x011d)) * 1000) +
				((FCEU_CheatGetByte(0x011e)==255)?0:(FCEU_CheatGetByte(0x011e)) * 100) +
				((FCEU_CheatGetByte(0x011f)==255)?0:(FCEU_CheatGetByte(0x011f)) * 10) +
				((FCEU_CheatGetByte(0x0120)==255)?0:(FCEU_CheatGetByte(0x0120)) * 1);
		break;
		case SPY_HUNTER: //ok
			points = (FCEU_CheatGetByte(0x0123) * 100000) +
				(FCEU_CheatGetByte(0x0124) * 10000) +
				(FCEU_CheatGetByte(0x0125) * 1000) +
				(FCEU_CheatGetByte(0x0126) * 100) +
				(FCEU_CheatGetByte(0x0127) * 10) +
				(FCEU_CheatGetByte(0x0128) * 1);
		break;
	}
	//printf("points so far: %d\n", points);
	return points;
}

int NESInterface::Impl::act(int action) {

	//printf("acted %d\n", action);

	// Calculate lives.
	remaining_lives = NESInterface::Impl::getLives();
	//remaining_lives = FCEU_CheatGetByte(0x075a);

	// Update game state.
	game_state = NESInterface::Impl::getGameState();
	//game_state = FCEU_CheatGetByte(0x0770);

	//TODO: check for invalid input
	nes_input = mappedActions[action];

	//printf("action: %d\n", action);
	//printf("nes input: %d\n", nes_input);

	uint8 *gfx;
	int32 *sound;
	int32 ssize;
	static int fskipc = 0;

	// Main loop.
	episode_frame_number++;
	FCEUI_Emulate(&gfx, &sound, &ssize, fskipc);
	FCEUD_Update(gfx, sound, ssize);

	// Get score...
	int new_score =  NESInterface::Impl::getPoints();
	/*
	(FCEU_CheatGetByte(0x07dd) * 1000000) +
			(FCEU_CheatGetByte(0x07de) * 100000) +
			(FCEU_CheatGetByte(0x07df) * 10000) +
			(FCEU_CheatGetByte(0x07e0) * 1000) +
			(FCEU_CheatGetByte(0x07e1) * 100) +
			(FCEU_CheatGetByte(0x07e2) * 10);
	*/

	/*
	// Calculate the change in x (this is the x position on the screen, not in the level).
	int new_x = FCEU_CheatGetByte(0x0086);
	int deltaX = new_x - current_x;
	deltaX = deltaX * 5;

	// Handle resets of level, etc.
	if (abs(deltaX) > MAX_ALLOWED_X_CHANGE) {
		deltaX = 0;
		current_x = 0;
	} 
        current_x = new_x;
	*/

	// Calculate the reward based on score.
	int reward = new_score - current_game_score;

	// Handle negative scores.
	/*if (reward < 0) {
			reward = 0;
	}*/
	current_game_score = new_score;

	// Add reward based on position.
        // Oh wow - now sure we want to do this :(
	//reward = reward + deltaX;

	return reward;
}

NESInterface::Impl::Impl(const std::string &rom_file) :
    m_episode_score(0),
    m_display_active(false),
	nes_input(0),
	current_game_score(0),
	//current_x(0),
	remaining_lives(0),
	game_state(0),
	episode_frame_number(0)
{
	NESInterface::Impl::initializeGamesData();

	//TODO: error handling
	//		check name of file, probably ".zip" added, or maybe even fullpath

	printf("roms name is %s\n", rom_file.substr(rom_file.find_last_of("/")+1).c_str());

	//trims the file path until to store just the name of the rom, our identifier
	curGame = allowedGames[rom_file.substr(rom_file.find_last_of("/")+1)];
	printf("curgame is %d\n", curGame);

	// Initialize some configuration variables.
	static int inited = 0;
	noGui = 1;

	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
	}
	SDL_GL_LoadLibrary(0);

	// Initialize the configuration system
	g_config = InitConfig();

	if(!g_config) {
		SDL_Quit();
		printf("Could not initialize configuration.\n");
	}

	int error = FCEUI_Initialize();
	if(error != 1) {
		SDL_Quit();
		printf("Error initializing FCEUI.\n");
	}

	// Specify some arguments - may want to do this dynamically at some point...
	int argc = 2;
	char** argv = new char*[argc];
        for (int i=0; i < argc; i++) {
                argv[i] = new char[200+rom_file.length()];
        }
        strcpy(argv[0], "./fceux");
        strcpy(argv[1], rom_file.c_str());

    // Parse the args.
	int romIndex = g_config->parse(argc, argv);
	if(romIndex < 1) {
		printf("ERROR: romIndex less than 1.\n");
	}

	// Now set some configuration options.
	int ret = g_config->setOption("SDL.NoConfig", 1);
	if (ret != 0) {
		printf("Error setting --no-config option.");
	}

	// Update input devices.
	UpdateInput(g_config);

    // Update the emu core with the config options.
	UpdateEMUCore(g_config);

	// load the specified game.
	error = LoadGame(argv[romIndex]);

	// If something went wrong, kill the driver stuff.
	if(error != 1) {
		if(inited&2)
			KillJoysticks();
		if(inited&4)
			KillVideo();
		if(inited&1)
			KillSound();
		inited=0;
		printf("Error loading ROM.\n");
		SDL_Quit();
	}

	// Set the emulation speed.
	FCEUD_SetEmulationSpeed(EMUSPEED_FASTEST);

	// Set the emulator to read from nes_input instead of the Gamepad :)
	FCEUI_SetInput(0, (ESI) SI_GAMEPAD, &nes_input, 0);
	FCEUI_SetInput(1, (ESI) SI_GAMEPAD, &nes_input, 0);
}


void NESInterface::Impl::initializeGamesData(){

	allowedGames["balloon.zip"] = BALLOON_FIGHT;
	allowedGames["breakthru.zip"] = BREAKTHRU;
	allowedGames["bumpnjump.zip"] = BUMP_N_JUMP;
	allowedGames["contra.zip"] = CONTRA;
	allowedGames["doubledragon.zip"] = DOUBLE_DRAGON;
	allowedGames["galaga.zip"] = GALAGA;
	allowedGames["gradius.zip"] = GRADIUS;
	allowedGames["gunsmoke.zip"] = GUNSMOKE;
	allowedGames["hudsons.zip"] = HUDSONS_ADVENTURE_ISLAND;
	allowedGames["joust.zip"] = JOUST;
	allowedGames["kungfu.zip"] = KUNGFU;
	allowedGames["lifeforce.zip"] = LIFEFORCE;
	allowedGames["machrider.zip"] = MACH_RIDER;
	allowedGames["punchout.zip"] = PUNCH_OUT;
	allowedGames["radracer.zip"] = RAD_RACER;
	allowedGames["rampage.zip"] = RAMPAGE;
	allowedGames["spyhunter.zip"] = SPY_HUNTER;


	for(int i=0; i<NUMBER_OF_GAMES; i++){
		for(int j=0; j<NUMBER_OF_ACTIONS; j++){
			switch(i){
				case BALLOON_FIGHT:
					if(	j == ACT_NOOP 		||
					    j == ACT_LEFT 		||
					    j == ACT_RIGHT 		||
					    j == ACT_B 			||
					    j == ACT_B_LEFT 	||
					    j == ACT_B_RIGHT 	||
					    j == ACT_A 			||
					    j == ACT_A_LEFT 	||
					    j == ACT_A_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case BREAKTHRU:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_UP ||
					    j == ACT_A_DOWN
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;

				break;
				case BUMP_N_JUMP:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_DOWN ||
					    j == ACT_A_UP ||
					    j == ACT_A_UP_RIGHT ||
					    j == ACT_A_UP_LEFT ||
					    j == ACT_A_DOWN_RIGHT ||
					    j == ACT_A_DOWN_LEFT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case CONTRA:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case DOUBLE_DRAGON:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_A ||
					    j == ACT_A_B ||
					    j == ACT_A_B_LEFT ||
					    j == ACT_A_B_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case GALAGA:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case GRADIUS: //NEEDS TO BE CHECKED AGAIN
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_DOWN ||
					    j == ACT_A_UP ||
					    j == ACT_A_UP_RIGHT ||
					    j == ACT_A_UP_LEFT ||
					    j == ACT_A_DOWN_RIGHT ||
					    j == ACT_A_DOWN_LEFT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;

				break;
				case GUNSMOKE:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_DOWN ||
					    j == ACT_A_UP ||
					    j == ACT_A_UP_RIGHT ||
					    j == ACT_A_UP_LEFT ||
					    j == ACT_A_DOWN_RIGHT ||
					    j == ACT_A_DOWN_LEFT ||
					    j == ACT_A_B ||
					    j == ACT_A_B_LEFT ||
					    j == ACT_A_B_RIGHT ||
					    j == ACT_A_B_DOWN ||
					    j == ACT_A_B_UP ||
					    j == ACT_A_B_DOWN_RIGHT ||
					    j == ACT_A_B_DOWN_LEFT ||
					    j == ACT_A_B_UP_RIGHT ||
					    j == ACT_A_B_UP_LEFT ||
					    j == ACT_SELECT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case HUDSONS_ADVENTURE_ISLAND:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				break;
				case JOUST:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
					
				break;
				case KUNGFU:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_B ||
					    j == ACT_A
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
					
				break;
				case LIFEFORCE:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_DOWN ||
					    j == ACT_A_UP ||
					    j == ACT_A_UP_RIGHT ||
					    j == ACT_A_UP_LEFT ||
					    j == ACT_A_DOWN_RIGHT ||
					    j == ACT_A_DOWN_LEFT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				
				break;
				case MACH_RIDER:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
				
				break;
				case PUNCH_OUT:
					if(	//j == ACT_NOOP || not necessary since we have start, lesser complexity
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_DOWN ||
					    j == ACT_B ||
					    j == ACT_B_UP ||
					    j == ACT_A ||
					    j == ACT_A_UP ||
					    j == ACT_START //to proceed after round intermission
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
					
				break;
				case RAD_RACER:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_UP
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
					
				break;
				case RAMPAGE:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_B ||
					    j == ACT_A ||
					    j == ACT_A_LEFT ||
					    j == ACT_A_RIGHT ||
					    j == ACT_A_UP ||
					    j == ACT_A_DOWN
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
			
				break;
				case SPY_HUNTER:
					if(	j == ACT_NOOP ||
					    j == ACT_LEFT ||
					    j == ACT_RIGHT ||
					    j == ACT_UP ||
					    j == ACT_DOWN ||
					    j == ACT_DOWN_RIGHT ||
					    j == ACT_DOWN_LEFT ||
					    j == ACT_UP_LEFT ||
					    j == ACT_UP_RIGHT ||
					    j == ACT_B ||
					    j == ACT_B_LEFT ||
					    j == ACT_B_RIGHT ||
					    j == ACT_B_UP ||
					    j == ACT_B_DOWN ||
					    j == ACT_B_DOWN_RIGHT ||
					    j == ACT_B_DOWN_LEFT ||
					    j == ACT_B_UP_RIGHT ||
					    j == ACT_B_UP_LEFT
					)
					    allowedActions[i][j] = 1;
					else
						allowedActions[i][j] = 0;
					
				break;
			}
		}
	}

}

/* --------------------------------------------------------------------------------------------------*/

/* begin PIMPL wrapper */

bool NESInterface::loadState() {
    return m_pimpl->loadState();
}

bool NESInterface::gameOver() {
    return m_pimpl->gameOver();
}

void NESInterface::resetGame() {
    m_pimpl->reset_game();
}

void NESInterface::saveState() {
    m_pimpl->saveState();
}

std::string NESInterface::getSnapshot() const {
    return m_pimpl->getSnapshot();
}

void NESInterface::restoreSnapshot(const std::string snapshot) {
    m_pimpl->restoreSnapshot(snapshot);
}

void NESInterface::getScreen(unsigned char *screen, int screen_size) {
         m_pimpl->getScreen(screen, screen_size);
}

const int NESInterface::getScreenHeight() const {
	return m_pimpl->getScreenHeight();
}

const int NESInterface::getScreenWidth() const {
	return m_pimpl->getScreenWidth();
}

void NESInterface::fillRGBfromPalette(unsigned char *raw_screen, unsigned char *rgb_screen, int raw_screen_size) {
        m_pimpl->fillRGBfromPalette(raw_screen, rgb_screen, raw_screen_size);
}

void NESInterface::setMaxNumFrames(int newMax) {
    m_pimpl->setMaxNumFrames(newMax);
}

int NESInterface::getEpisodeFrameNumber() const {
    return m_pimpl->getEpisodeFrameNumber();
}

int NESInterface::getFrameNumber() const {
    return m_pimpl->getFrameNumber();
}

int NESInterface::getNumLegalActions() {
	return m_pimpl->getNumLegalActions();
}

void NESInterface::getLegalActionSet(int legal_actions[]) {
        m_pimpl->getLegalActionSet(legal_actions);
}

int NESInterface::minReward() const {
    return m_pimpl->minReward();
}

int NESInterface::maxReward() const {
    return m_pimpl->maxReward();
}

int NESInterface::lives() const {
    return m_pimpl->lives();
}

const int NESInterface::getCurrentScore() const {
	return m_pimpl->getCurrentScore();
}

int NESInterface::act(int action) {
    return m_pimpl->act(action);
}

NESInterface::NESInterface(const std::string &rom_file) :
    m_pimpl(new NESInterface::Impl(rom_file)) {

}

NESInterface::~NESInterface() {
    delete m_pimpl;
}

} // namespace nes

