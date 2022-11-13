#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "status.h"

void init_stars(GameState *game)
{
    for (int i = 0; i < STAR_NUMBER; i++) {
        game->stars[i].base_x = WINDOW_WIDTH / 2 + rand() % 38400;
        game->stars[i].base_y = rand() % WINDOW_HEIGHT;
        game->stars[i].mode = rand() % WINDOW_HEIGHT;
        game->stars[i].phase = 2 * 3.14 *(rand() % 360)/360.0f;
    }
}

void load_game(GameState *game)
{
    SDL_Surface *surface = NULL;
    surface = IMG_Load("../resources/images/star.png");
    if (surface == NULL) {
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    game->star = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("../resources/images/man0.png");
    if (surface == NULL) {
        fprintf(stderr, "Cannot find man0.png\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    game->man_frames[0] = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("../resources/images/man1.png");
    if (surface == NULL) {
        fprintf(stderr, "Cannot find man1.png\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    game->man_frames[1] = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("../resources/images/brick.png");
    if (surface == NULL) {
        fprintf(stderr, "Cannot find brick.png\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    game->brick = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("../resources/images/fire.png");
    if (surface == NULL) {
        fprintf(stderr, "Cannot find fire.png\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    game->fire = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_FreeSurface(surface);

    game->font = TTF_OpenFont("../resources/fonts/NewYork.ttf", 48);
    if (game->font == NULL) {
        fprintf(stderr, "Cannot find font\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    game->bg_music = Mix_LoadWAV("../resources/sound/bg.wav");
    if (game->bg_music) {
        Mix_VolumeChunk(game->bg_music, 64);
    }
    game->jump_sound = Mix_LoadWAV("../resources/sound/jump.wav");
    game->land_sound = Mix_LoadWAV("../resources/sound/landing.wav");
    game->die_sound = Mix_LoadWAV("../resources/sound/die.wav");

    game->man.x = WINDOW_WIDTH / 4;
    game->man.y = WINDOW_HEIGHT / 2 - MAN_HEIGHT / 2;
    game->man.dx = 0;
    game->man.dy = 0;
    game->man.on_ledge = 0;
    game->man.is_dead = 0;
    game->man.anim_frame = 0;
    game->man.facing_left = 0;
    game->man.lives = 3;
    game->status_state = STATUS_STATE_LIVES;
    game->label = NULL;

    init_status_lives(game);

    game->scroll_x = 0;
    game->time = 0;
    game->death_countdown = -1;

    init_stars(game);

    // init ledges
    for (int i = 0; i < LEDGES_NUMBER - 20; i++) {
        game->ledges[i].w = 256;
        game->ledges[i].h = 64;
        game->ledges[i].x = i * 384;
        if (i == 0)
            game->ledges[i].y = 400;
        else
            game->ledges[i].y = 400 - rand() % 200;
    }
    
    for (int i = LEDGES_NUMBER - 20; i < LEDGES_NUMBER; i++) {
        game->ledges[i].w = 256;
        game->ledges[i].h = 64;
        game->ledges[i].x = 350 + rand() % 38400;
        if (i % 2 == 0)
            game->ledges[i].y = 200;
        else
            game->ledges[i].y = 350;
    }
}

int process_events(SDL_Window *window, GameState *game)
{
    SDL_Event e;
    int quit = 0;
    while (SDL_PollEvent(&e)) {
        switch (e.type){
            case SDL_MOUSEBUTTONDOWN:
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    case SDLK_f:
                        SDL_SetWindowFullscreen(window, SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                        break;
                    case SDLK_SPACE:    
                    case SDLK_UP:
                        if (game->man.on_ledge) {
                            game->man.dy = -8;
                            game->man.on_ledge = 0;
                            Mix_PlayChannel(-1, game->jump_sound, 0);
                        }
                        break;
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_SPACE]) {
        game->man.dy -= 0.2f;
    }
    // if (state[SDL_SCANCODE_DOWN]) {
    //     game->man.y += 10;
    // }
    // Walking
    if (state[SDL_SCANCODE_LEFT]) {
        game->man.dx -= 0.5;
        if (game->man.dx < -6) {
            game->man.dx = -6;
        }
        game->man.facing_left = 1;
    } else if (state[SDL_SCANCODE_RIGHT]) {
        game->man.dx += 0.5;
        if (game->man.dx > 6) {
            game->man.dx = 6;
        }
        game->man.facing_left = 0;
    } else {
        game->man.dx *= 0.8f;
        game->man.anim_frame = 0;
        if (fabsf(game->man.dx) < 0.1f) {
            game->man.dx = 0;
        }
    }

    return quit;
}

void process(GameState *game)
{
    game->time++;

    if (game->status_state == STATUS_STATE_LIVES) {
        if (game->time > 120) {
            shutdown_status_lives(game);
            game->status_state = STATUS_STATE_GAME;
            game->music_channel = Mix_PlayChannel(-1, game->bg_music, -1);
        }
    } else if (game->status_state == STATUS_STATE_GAME_OVER) {
        if (game->time > 190) {
            SDL_Quit();
            exit(0);
        }
    } else if (game->status_state == STATUS_STATE_WIN) {
        if (game->time > 190) {
            SDL_Quit();
            exit(0);
        }
    } else if (game->status_state == STATUS_STATE_GAME) {
        if (!game->man.is_dead) {
            game->man.x += game->man.dx;
            // if (game->man.x < -MAN_WIDTH) {
            //     game->man.x = WINDOW_WIDTH;
            // } else if (game->man.x > WINDOW_WIDTH) {
            //     game->man.x = 0;
            // }
            game->man.y += game->man.dy;
            if (game->man.y > WINDOW_HEIGHT) {
                game->man.is_dead = 1;
                Mix_PlayChannel(-1, game->die_sound, 0);
                Mix_HaltChannel(game->music_channel);
            }
            
            if (game->man.dx != 0 && game->man.on_ledge) {
                if (game->time % 10 == 0) {
                    game->man.anim_frame = !game->man.anim_frame;
                }
            }

            if (game->man.x > 38320) {
                init_game_win(game);
                game->time = 0;
                game->status_state = STATUS_STATE_WIN;
            }

            game->man.dy += GRAVITY;

            // Stars movement
            for (int i = 0; i < STAR_NUMBER; i++) {
                game->stars[i].x = game->stars[i].base_x;
                game->stars[i].y = game->stars[i].base_y;
                if (game->stars[i].mode == 0)
                    game->stars[i].x = game->stars[i].base_x + sinf(game->stars[i].phase + game->time * 0.06f) * 75;
                else
                    game->stars[i].y = game->stars[i].base_y + cosf(game->stars[i].phase + game->time * 0.06f) * 75;
            }

            game->scroll_x = -game->man.x + WINDOW_WIDTH / 2;
            if (game->scroll_x > 0) game->scroll_x = 0;
            if (game->scroll_x < -38000 + 320) game->scroll_x = -38000 + 320;
        }

        if (game->man.is_dead && game->death_countdown < 0) {
        game->death_countdown = 120;
    }

    if (game->death_countdown >= 0) {
        game->death_countdown--;
        if (game->death_countdown < 0) {
            game->man.lives--;
            if (game->man.lives >= 0) {
                init_status_lives(game);
                game->status_state = STATUS_STATE_LIVES;
                game->time = 0;
                game->man.is_dead = 0;
                game->man.x -= 100;
                game->man.y = rand() % 100 + 100;
                game->man.dx = 0;
                game->man.dy = 0;
                game->man.on_ledge = 0;
            } else {
                init_game_over(game);
                game->status_state = STATUS_STATE_GAME_OVER;
                game->time = 0;
            }
        }
    }
    }
}

int collide2d(float x1, float y1, float x2, float y2, float wt1, float ht1, float wt2, float ht2)
{
    return (!((x1 > (x2 + wt2)) || (x2 > (x1 + wt1)) || (y1 > (y2 + ht2)) || (y2 > (y1 + ht1))));
}

void detect_collision(GameState *game)
{
    for (int i = 0; i < STAR_NUMBER; i++) {
        if (collide2d(game->man.x, game->man.y, game->stars[i].x, game->stars[i].y, MAN_WIDTH, MAN_HEIGHT, STAR_WIDTH / 2, STAR_HEIGHT / 2)) {
            game->man.is_dead = 1;
            Mix_PlayChannel(-1, game->die_sound, 0);
            Mix_HaltChannel(game->music_channel);
            break;
        }
    }

    for (int i = 0; i < LEDGES_NUMBER; i++) {
        int mw = MAN_WIDTH, mh = MAN_HEIGHT;
        int mx = game->man.x, my = game->man.y;
        int bx = game->ledges[i].x, by = game->ledges[i].y, bw = game->ledges[i].w, bh = game->ledges[i].h;

        if (mx + mw/2 > bx && mx + mw/2 < bx + bw) {
            // are we bumping our head?
            if (my < by + bh && my > by && game->man.dy < 0) {
                game->man.y = by + bh;
                my = by + bh;
                game->man.dy = 0;
                game->man.on_ledge = 1;
            }
        }
        if (mx + mw > bx && mx < bx + bw) {
            // are we landing on the ledge
            if (my + mh > by && my < by && game->man.dy > 0) {
                game->man.y = by - mh;
                my = by - mh;
                game->man.dy = 0;
                if (game->man.on_ledge == 0) {
                    game->man.on_ledge = 1;
                    Mix_PlayChannel(-1, game->land_sound, 0);
                }
            }
        }

        if (my + mh > by && my < by + bh) {
            // rubbing arainst right edge
            if (mx < bx + bw && mx + mw > bx + bw && game->man.dx < 0) {
                game->man.x = bx + bw;
                mx = bx + bw;
                game->man.dx = 0;
            }
            // rubbing arainst left edge
            else if (mx + mw > bx && mx < bx && game->man.dx > 0) {
                game->man.x = bx - mw;
                mx = bx - mw;
                game->man.dx = 0;
            } 
        }
    }
}

void do_render(GameState *game)
{
    if (game->status_state == STATUS_STATE_LIVES) {
        draw_status_lives(game);
    }
    if (game->status_state == STATUS_STATE_GAME_OVER) {
        draw_status_lives(game);
    }
    if (game->status_state == STATUS_STATE_WIN) {
        draw_status_lives(game);
    }

    if (game->status_state == STATUS_STATE_GAME) {
        SDL_SetRenderDrawColor(game->renderer, 128, 128, 255, 255);
        SDL_RenderClear(game->renderer);

        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
        
        for (int i = 0; i < LEDGES_NUMBER; i++) {
            SDL_Rect ledgeRect = {game->scroll_x + game->ledges[i].x, game->ledges[i].y, game->ledges[i].w, game->ledges[i].h};
            SDL_RenderCopy(game->renderer, game->brick, NULL, &ledgeRect);
        }

        SDL_Rect rect = {game->scroll_x + game->man.x, game->man.y, MAN_WIDTH, MAN_HEIGHT};
        SDL_RenderCopyEx(game->renderer, game->man_frames[game->man.anim_frame], NULL,
            &rect, 0, NULL, game->man.facing_left == 1);

        if (game->man.is_dead) {
            SDL_Rect rect = {game->scroll_x + game->man.x - 24 + 32 - 10, game->man.y - 24 - 32, 64, 64};
            SDL_RenderCopyEx(game->renderer, game->fire, NULL, &rect, 0, NULL, game->time % 10 < 10);
        }

        for (int i = 0; i < STAR_NUMBER; i++) {
            SDL_Rect starRect = {game->scroll_x + game->stars[i].x, game->stars[i].y, MAN_WIDTH, MAN_HEIGHT};
            SDL_RenderCopy(game->renderer, game->star, NULL, &starRect);
        }
    }
    
    SDL_RenderPresent(game->renderer);
}

int game_run()
{
    GameState game;
    SDL_Window *window = NULL;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
    game.joystick = SDL_JoystickOpen(0);

    srand(time(NULL));
    
    window = SDL_CreateWindow("Game window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        SDL_WINDOW_RESIZABLE
        );
    game.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(game.renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    TTF_Init();

    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

    load_game(&game);   

    int quit = 0;
    while (!quit){
        quit = process_events(window, &game);
        
        process(&game);
        detect_collision(&game);

        do_render(&game);
    }

    Mix_FreeChunk(game.bg_music);
    Mix_FreeChunk(game.jump_sound);
    Mix_FreeChunk(game.land_sound);
    Mix_FreeChunk(game.die_sound);

    SDL_DestroyTexture(game.star);
    SDL_DestroyTexture(game.man_frames[0]);
    SDL_DestroyTexture(game.man_frames[1]);
    SDL_DestroyTexture(game.brick);
    SDL_DestroyTexture(game.fire);
    if (game.label != NULL) SDL_DestroyTexture(game.label);
    TTF_CloseFont(game.font);
    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(game.renderer);

    TTF_Quit();

    SDL_Quit();

    return EXIT_SUCCESS;
}
