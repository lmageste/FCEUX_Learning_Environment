#ifndef __NES_INTERFACE_HPP__
#define __NES_INTERFACE_HPP__

#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <SDL/SDL.h>

namespace nes {

// NES screen width.
#define NES_SCREEN_WIDTH 256

// Number of normal game actions we want to test.
//#define NUM_NES_LEGAL_ACTIONS 15

// The amount of change in the position above which we disregard.
//#define MAX_ALLOWED_X_CHANGE 100

// Define possible actions
#define NUMBER_OF_ACTIONS   38 //until and excluding reset

#define ACT_NOOP            0  // 0
#define ACT_A               1  // 1
#define ACT_B	            2  // 2
#define ACT_SELECT          3  // probably 4
#define ACT_START           4  // 8
#define ACT_UP              5  // 16
#define ACT_DOWN            6  // 32
#define ACT_LEFT            7  // 64
#define ACT_RIGHT           8  // 128
#define ACT_UP_LEFT         9  // 80
#define ACT_UP_RIGHT        10 // 144
#define ACT_DOWN_LEFT       11 // 96
#define ACT_DOWN_RIGHT      12 // 160
#define ACT_A_UP            13 // 17
#define ACT_A_DOWN          14 // 33
#define ACT_A_LEFT          15 // 65
#define ACT_A_RIGHT         16 // 129
#define ACT_A_UP_LEFT       17 // 81
#define ACT_A_UP_RIGHT      18 // 145
#define ACT_A_DOWN_LEFT     19 // 97
#define ACT_A_DOWN_RIGHT    20 // 161
#define ACT_B_UP            21 // 18
#define ACT_B_DOWN          22 // 34
#define ACT_B_LEFT          23 // 66
#define ACT_B_RIGHT         24 // 130
#define ACT_B_UP_LEFT       25 // 82
#define ACT_B_UP_RIGHT      26 // 146
#define ACT_B_DOWN_LEFT     27 // 98
#define ACT_B_DOWN_RIGHT    28 // 162
#define ACT_A_B             29 // 3
#define ACT_A_B_UP          30 // 19
#define ACT_A_B_DOWN        31 // 35
#define ACT_A_B_LEFT        32 // 67
#define ACT_A_B_RIGHT       33 // 132
#define ACT_A_B_UP_LEFT     34 // 83
#define ACT_A_B_UP_RIGHT    35 // 147
#define ACT_A_B_DOWN_LEFT   36 // 99
#define ACT_A_B_DOWN_RIGHT  37 // 163
#define ACT_RESET           38 // note: we use SYSTEM_RESET instead to reset the environment.
#define ACT_UNDEFINED       39
#define ACT_RANDOM          40

/*
//value to send to NES input for each action:
int mappedActions[38] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 80, 144, 96, 160,
 17, 33, 65, 129, 81, 145, 97, 161, 18, 34, 66, 130, 82, 146, 98, 162, 3
 19, 35, 67, 132, 83, 147, 99, 163};

// allowed actions for each game:
std::vector <int> <std::vector<int>> allowedActions;
// set game based on title:
std::map<std::string, int> allowedGames;
*/

#define NUMBER_OF_GAMES             17

#define BALLOON_FIGHT               0
#define BREAKTHRU                   1
#define BUMP_N_JUMP                 2
#define CONTRA                      3
#define DOUBLE_DRAGON               4
#define GALAGA                      5
#define GRADIUS                     6
#define GUNSMOKE                    7
#define HUDSONS_ADVENTURE_ISLAND    8
#define JOUST                       9
#define KUNGFU                      10
#define LIFEFORCE                   11
#define MACH_RIDER                  12
#define PUNCH_OUT                   13
#define RAD_RACER                   14
#define RAMPAGE                     15
#define SPY_HUNTER                  16


typedef unsigned char byte_t;
typedef unsigned char pixel_t;

// This class provides a simplified interface to NES.
class NESInterface {

    public:

        /** create a NESInterface. This routine is not threadsafe!
            One also has the option of creating a single NES session
            that will randomly (uniform) alternate between a number of
            different ROM files. The syntax is:  
                <rom path>+<rom path>+... */
        NESInterface(const std::string &rom_file);
        
        /** Unload the emulator. */
        ~NESInterface();

        /** Resets the game. */
        void resetGame();

        /** Indicates if the game has ended. */
        bool gameOver();

        /** Applies an action to the game and returns the reward. It is the user's responsibility
            to check if the game has ended and reset when necessary - this method will keep pressing
            buttons on the game over screen. */
        int act(int action);

        /** Returns the number of legal actions. */
        int getNumLegalActions();

        /** Returns the vector of legal actions. */
        void getLegalActionSet(int legal_actions[]);

        /** Returns the frame number since the loading of the ROM. */
        int getFrameNumber() const;

        /** Set the frame limit for each episode. '0' means no limit. */
        void setMaxNumFrames(int newMax);

        /** Minimum possible instantaneous reward. */
        int minReward() const;

        /** Maximum possible instantaneous reward. */
        int maxReward() const;

        /** The remaining number of lives. */
        int lives() const;

        /** Returns the frame number since the start of the current episode. */
        int getEpisodeFrameNumber() const;

        /** Returns a handle to the current game screen. */
        void getScreen(unsigned char *screen, int screen_size);

        /** Returns width and height. */
        const int getScreenHeight() const;
        const int getScreenWidth() const;

        /** Returns the score. */
        const int getCurrentScore() const;

        /** Saves the state of the emulator system, overwriting any 
            previously saved state. */
        void saveState();

        /** Restores a previously saved state of the emulator system,
            returns false if no such state exists (and makes no changes
            to the emulator system). */
        bool loadState();

        /** Gets a state as a string. */
        std::string getSnapshot() const;

        /** Sets the state from a string*/
        void restoreSnapshot(const std::string snapshot);
        
        /** Converts a pixel to its RGB value. */
        static void getRGB(
            unsigned char pixel, 
            unsigned char *red, 
            unsigned char *green, 
            unsigned char *blue
        );

        /** Get the full RGB screen from the raw pixel data. */
        void fillRGBfromPalette(unsigned char *raw_screen, unsigned char *rgb_screen, int raw_screen_size);

    private:

        /** Copying is explicitly disallowed. */
        NESInterface(const NESInterface &);

        /** Assignment is explicitly disallowed. */
        NESInterface &operator=(const NESInterface &);

        class Impl;
        Impl *m_pimpl;
};

} // namespace nes

#endif // __NES_INTERFACE_HPP__





