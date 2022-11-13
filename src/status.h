#ifndef SDL2_GAME_STATUS
#define SDL2_GAME_STATUS

void init_game_over(GameState *game);
void init_game_win(GameState *game);
void init_status_lives(GameState *game);
void draw_status_lives(GameState *game);
void shutdown_status_lives(GameState *game);

#endif
