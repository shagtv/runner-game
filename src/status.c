#include <stdio.h>
#include "game.h"

void init_status_lives(GameState *game)
{
    char str[128] = "";
    sprintf(str, "x %d", game->man.lives);

    SDL_Color white = {0, 0, 0, 255};
    SDL_Surface *tmp = TTF_RenderText_Blended(game->font, str, white);
    game->label = SDL_CreateTextureFromSurface(game->renderer, tmp);
    game->labelw = tmp->w;
    game->labelh = tmp->h;
    SDL_FreeSurface(tmp);
}

void init_game_over(GameState *game)
{
    char str[128] = "Game over";

    SDL_Color white = {0, 0, 0, 255};
    SDL_Surface *tmp = TTF_RenderText_Blended(game->font, str, white);
    game->label = SDL_CreateTextureFromSurface(game->renderer, tmp);
    game->labelw = tmp->w;
    game->labelh = tmp->h;
    SDL_FreeSurface(tmp);
}

void init_game_win(GameState *game)
{
    char str[128] = "You win";

    SDL_Color white = {0, 0, 0, 255};
    SDL_Surface *tmp = TTF_RenderText_Blended(game->font, str, white);
    game->label = SDL_CreateTextureFromSurface(game->renderer, tmp);
    game->labelw = tmp->w;
    game->labelh = tmp->h;
    SDL_FreeSurface(tmp);
}

void draw_status_lives(GameState *game)
{
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderClear(game->renderer);

    SDL_Rect rect = {WINDOW_WIDTH / 2 - MAN_WIDTH - game->labelw / 2, WINDOW_HEIGHT / 2 - game->labelh / 2, MAN_WIDTH, MAN_HEIGHT};
    SDL_RenderCopyEx(game->renderer, game->man_frames[0], NULL, &rect, 0, NULL, 0);

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_Rect text_rect = {WINDOW_WIDTH / 2 - game->labelw / 2, WINDOW_HEIGHT / 2 - game->labelh / 2, game->labelw, game->labelh};
    SDL_RenderCopy(game->renderer, game->label, NULL, &text_rect);
}

void shutdown_status_lives(GameState *game)
{
    if (game->label != NULL) {
        SDL_DestroyTexture(game->label);
        game->label = NULL;
    }
}
