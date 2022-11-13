
#ifndef SDL2_GAME
#define SDL2_GAME

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define STATUS_STATE_LIVES 0
#define STATUS_STATE_GAME 1
#define STATUS_STATE_GAME_OVER 2
#define STATUS_STATE_WIN 3

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define MAN_WIDTH 48
#define MAN_HEIGHT 48
#define STAR_WIDTH 64
#define STAR_HEIGHT 64
#define STAR_NUMBER 100
#define LEDGES_NUMBER 100
#define GRAVITY 0.35f

typedef struct {
    float x, y;
    float dx, dy;
    short lives;
    char *name;
    int on_ledge;
    int is_dead;
    int anim_frame, facing_left, slowing_down;
} Man;

typedef struct {
    int x, y;
    int base_x, base_y;
    int mode;
    float phase;
} Star;

typedef struct {
    int x, y, w, h;
} Ledge;

typedef struct {
    int time, death_countdown;
    int status_state;
    int scroll_x;

    // Players
    Man man;

    // Stars
    Star stars[STAR_NUMBER];

    Ledge ledges[LEDGES_NUMBER];

    // Images
    SDL_Joystick *joystick;
    SDL_Texture *star;
    SDL_Texture *man_frames[2];
    SDL_Texture *brick;
    SDL_Texture *fire;
    SDL_Texture *label;
    int labelw, labelh;

    // Fonts
    TTF_Font *font;

    // Sound
    Mix_Chunk *bg_music, *jump_sound, *land_sound, *die_sound;
    int music_channel;

    // Renderer
    SDL_Renderer *renderer;
} GameState;

int game_run();

#endif
