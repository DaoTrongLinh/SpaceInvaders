#ifndef _DEFS__H
#define _DEFS__H

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700
#define WINDOW_TITLE "Hello World!"

#define BACKGROUND_IMG "sky.jpg"

const char*  CHARACTER_SPRITE_FILE = "bird.png";
const int CHARACTER_CLIPS[][4] = {
    {0, 0, 182, 168},
    {181, 0, 182, 168},
    {364, 0, 182, 168},
    {547, 0, 182, 168},
    {728, 0, 182, 168},

    {0, 170, 182, 168},
    {181, 170, 182, 168},
    {364, 170, 182, 168},
    {547, 170, 182, 168},
    {728, 170, 182, 168},

    {0, 340, 182, 168},
    {181, 340, 182, 168},
    {364, 340, 182, 168},
    {547, 340, 182, 168},
};
const int CHARACTER_FRAMES = sizeof(CHARACTER_CLIPS)/sizeof(int)/4;

#endif
