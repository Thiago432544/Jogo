#include <stdio.h>
#include <SDL.h>
#include "./constantes.h"
#include <SDL_image.h>
#include <math.h>
#include <string.h>

int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int last_frame_time = FALSE;

char w_pressed = FALSE, a_pressed = FALSE, s_pressed = FALSE, d_pressed = FALSE, space_pressed = FALSE;
char up_pressed = FALSE, down_pressed = FALSE, right_pressed = FALSE, left_pressed = FALSE, c_pressed = FALSE;

SDL_Texture* background_textures[MAX_MAPAS];
int mapa_atual = 0;

int escolhaP1 = 0;
int escolhaP2 = 0;

#define MAX_FRAMES 10


typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UPLEFT,
    DIR_UPRIGHT,
    DIR_DOWNLEFT,
    DIR_DOWNRIGHT
} Direction;

typedef struct {
    SDL_Texture* frames[MAX_FRAMES];
    int frame_count;
    int current_frame;
    float frame_time;
    float time_accumulated;
} Animation;
struct projectile {
    float x;
    float y;
    float speed;
    Direction dir;
    int is_active;
    int from_square;
    SDL_Texture* texture;
    char personagem_disparador[50]; // Adicionar o nome do personagem que disparou
};

struct ball {
    float x;
    float y;
    float width;
    float height;
    SDL_Texture* texture;
    Animation anim_left;   // Left-facing animation
    Animation anim_right;  // Right-facing animation
    Direction last_dir;
    char personagem_path[50];
} ball;

struct square {
    float x;
    float y;
    float width;
    float height;
    SDL_Texture* texture;
    Animation anim_left;   // Left-facing animation
    Animation anim_right;  // Right-facing animation
    Direction last_dir;
    char personagem_path[50];
}square;

typedef struct {
    float x;
    float y;
    float width;
    float height;
    SDL_Rect rect;
}obstacle;


obstacle obstaculos[MAX_MAPAS][MAX_OBSTACULOS];
int num_obstaculos[MAX_MAPAS];


SDL_Texture* background_texture = NULL;

#define MAX_PROJECTILES 1000
struct projectile projectiles[MAX_PROJECTILES];

int ball_life = 10;
int square_life = 10;
//int mopa;

void init_animation(Animation* anim, float frame_time);
int load_animation_frames(Animation* anim, SDL_Renderer* renderer, const char* base_path, int num_frames);
void update_animation(Animation* anim, float delta_time);

int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Erro de inicializacao do SDL.\n");
        return FALSE;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Erro ao inicializar SDL_image: %s\n", IMG_GetError());
        return FALSE;
    }

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        fprintf(stderr, "Erro ao criar uma janela SDL.\n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Erro ao criar renderizador SDL.\n");
        return FALSE;
    }

    return TRUE;
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) game_is_running = FALSE;
            if (event.key.keysym.sym == SDLK_w) w_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_a) a_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_s) s_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_d) d_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_SPACE) space_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_UP) up_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_DOWN) down_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_RIGHT) right_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_LEFT) left_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_l) c_pressed = TRUE;
            if (event.key.keysym.sym == SDLK_TAB) { mapa_atual = (mapa_atual + 1) % MAX_MAPAS; } // Troca de mapa com TAB
            break;
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_w) w_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_a) a_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_s) s_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_d) d_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_SPACE) space_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_UP) up_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_DOWN) down_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_RIGHT) right_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_LEFT) left_pressed = FALSE;
            if (event.key.keysym.sym == SDLK_l) c_pressed = FALSE;
            break;
        }
    }
}
int check_collision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2);
}


void fire_projectile(int from_square) {
    Direction dir;
    float x, y;
    const char* personagem;

    if (from_square) {
        if (square.last_dir == DIR_NONE) {
            return;
        }
        dir = square.last_dir;
        x = square.x + square.width / 2 - 5;
        y = square.y + square.height / 2 - 5;
        personagem = PERSONAGENS[escolhaP2 - 1]; // Personagem do square
    }
    else {
        if (ball.last_dir == DIR_NONE) {
            return;
        }
        dir = ball.last_dir;
        x = ball.x + ball.width / 2 - 5;
        y = ball.y + ball.height / 2 - 5;
        personagem = PERSONAGENS[escolhaP1 - 1]; // Personagem do ball
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].is_active) {
            projectiles[i].x = x;
            projectiles[i].y = y;
            projectiles[i].dir = dir;
            projectiles[i].is_active = TRUE;
            projectiles[i].from_square = from_square;

            // Armazena o nome do personagem disparador
            strcpy_s(projectiles[i].personagem_disparador, sizeof(projectiles[i].personagem_disparador), personagem);

            // Carrega a textura apropriada apenas quando um projétil é disparado
            SDL_Surface* tmpSurface = NULL;

            if (strcmp(personagem, "python") == 0) {
                tmpSurface = IMG_Load("assets/python_proj.png");  // Projétil especial para Python
            }
            else {
                tmpSurface = IMG_Load("assets/c.png");  // Projétil padrão para outros personagens
            }

            if (tmpSurface) {
                // Se já existir uma textura, destruí-la para evitar vazamento de memória
                if (projectiles[i].texture) {
                    SDL_DestroyTexture(projectiles[i].texture);
                }

                projectiles[i].texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
                SDL_FreeSurface(tmpSurface);
            }

            break;
        }
    }
}
int colidiu_com_obstaculos(SDL_Rect personagem, obstacle* obstacles, int num_obstacles) {
    for (int i = 0; i < num_obstacles; i++) {
        if (SDL_HasIntersection(&personagem, &obstacles[i].rect)) {
            return 1;
        }
    }
    return 0;
}


void update() {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    // Movimento da bola (ball) com checagem de obstáculos
    float next_ball_x = ball.x;
    float next_ball_y = ball.y;

    if (a_pressed) {
        next_ball_x -= 500 * delta_time;
        ball.last_dir = DIR_LEFT;
    }
    if (d_pressed) {
        next_ball_x += 500 * delta_time;
        ball.last_dir = DIR_RIGHT;
    }
    if (w_pressed) {
        next_ball_y -= 500 * delta_time;
        ball.last_dir = DIR_UP;
    }
    if (s_pressed) {
        next_ball_y += 500 * delta_time;
        ball.last_dir = DIR_DOWN;
    }
    if (a_pressed && w_pressed) {
        ball.last_dir = DIR_UPLEFT;
    }
    if (a_pressed && s_pressed) {
        ball.last_dir = DIR_DOWNLEFT;
    }
    if (d_pressed && w_pressed) {
        ball.last_dir = DIR_UPRIGHT;
    }
    if (d_pressed && s_pressed) {
        ball.last_dir = DIR_DOWNRIGHT;
    }

    // Verifica colisão da bola com obstáculos
    SDL_Rect next_ball_rect = { (int)next_ball_x, (int)next_ball_y, (int)ball.width, (int)ball.height };
    int ball_collision = colidiu_com_obstaculos(next_ball_rect, obstaculos[mapa_atual], num_obstaculos[mapa_atual]);

    if (!ball_collision) {
        ball.x = next_ball_x;
        ball.y = next_ball_y;
    }

    // Movimento do quadrado (square) com checagem de obstáculos
    float next_square_x = square.x;
    float next_square_y = square.y;

    if (left_pressed) {
        next_square_x -= 500 * delta_time;
        square.last_dir = DIR_LEFT;
    }
    if (right_pressed) {
        next_square_x += 500 * delta_time;
        square.last_dir = DIR_RIGHT;
    }
    if (up_pressed) {
        next_square_y -= 500 * delta_time;
        square.last_dir = DIR_UP;
    }
    if (down_pressed) {
        next_square_y += 500 * delta_time;
        square.last_dir = DIR_DOWN;
    }
    if (left_pressed && up_pressed) {
        square.last_dir = DIR_UPLEFT;
    }
    if (left_pressed && down_pressed) {
        square.last_dir = DIR_DOWNLEFT;
    }
    if (up_pressed && right_pressed) {
        square.last_dir = DIR_UPRIGHT;
    }
    if (right_pressed && down_pressed) {
        square.last_dir = DIR_DOWNRIGHT;
    }

    // Update animations
    if (ball.anim_left.frame_count > 0 || ball.anim_right.frame_count > 0) {
        // Include vertical movement (w_pressed, s_pressed)
        if (a_pressed || w_pressed || s_pressed) {  // Update for left orientation
            update_animation(&ball.anim_left, delta_time);
        }
        if (d_pressed || w_pressed || s_pressed) {  // Update for right orientation
            update_animation(&ball.anim_right, delta_time);
        }
    }

    if (square.anim_left.frame_count > 0 || square.anim_right.frame_count > 0) {
        // Include vertical movement (up_pressed, down_pressed)
        if (left_pressed || up_pressed || down_pressed) {  // Update for left orientation
            update_animation(&square.anim_left, delta_time);
        }
        if (right_pressed || up_pressed || down_pressed) {  // Update for right orientation
            update_animation(&square.anim_right, delta_time);
        }
    }

    // Verifica colisão do quadrado com obstáculos
    SDL_Rect next_square_rect = { (int)next_square_x, (int)next_square_y, (int)square.width, (int)square.height };
    int square_collision = colidiu_com_obstaculos(next_square_rect, obstaculos[mapa_atual], num_obstaculos[mapa_atual]);

    if (!square_collision) {
        square.x = next_square_x;
        square.y = next_square_y;
    }

    // Disparo de projéteis
    if (space_pressed) {
        fire_projectile(FALSE);
        space_pressed = FALSE;
    }
    if (c_pressed) {
        fire_projectile(TRUE);
        c_pressed = FALSE;
    }

    // Verifica colisão com obstáculos
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].is_active) {
            SDL_Rect proj_rect = {
                (int)projectiles[i].x,
                (int)projectiles[i].y,
                15, 15
            };

            int obstacle_collision = 0;
            for (int j = 0; j < num_obstaculos[mapa_atual]; j++) {
                if (SDL_HasIntersection(&proj_rect, &obstaculos[mapa_atual][j].rect)) {
                    obstacle_collision = 1;
                    break;
                }
            }

            if (obstacle_collision) {
                projectiles[i].is_active = FALSE;
                continue; // Pula para o próximo projétil
            }
        }
    }
    // Movimento dos projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].is_active) {
            if (!projectiles[i].from_square &&
                check_collision(projectiles[i].x, projectiles[i].y, 15, 15,
                    square.x, square.y, square.width, square.height)) {
                printf("Square atingido!\n");
                projectiles[i].is_active = FALSE;
                square_life--;
            }
            if (projectiles[i].from_square &&
                check_collision(projectiles[i].x, projectiles[i].y, 15, 15,
                    ball.x, ball.y, ball.width, ball.height)) {
                printf("Ball atingido!\n");
                projectiles[i].is_active = FALSE;
                ball_life--;
            }
            if (ball_life == 0) {
                printf("Vitoria de square!\n");
                game_is_running = FALSE;
            }
            if (square_life == 0) {
                printf("Vitoria de ball!\n");
                game_is_running = FALSE;
            }

            switch (projectiles[i].dir) {
            case DIR_UP: projectiles[i].y -= projectiles[i].speed * delta_time; break;
            case DIR_DOWN: projectiles[i].y += projectiles[i].speed * delta_time; break;
            case DIR_LEFT: projectiles[i].x -= projectiles[i].speed * delta_time; break;
            case DIR_RIGHT: projectiles[i].x += projectiles[i].speed * delta_time; break;
            case DIR_UPLEFT: projectiles[i].x -= projectiles[i].speed * delta_time; projectiles[i].y -= projectiles[i].speed * delta_time; break;
            case DIR_UPRIGHT: projectiles[i].x += projectiles[i].speed * delta_time; projectiles[i].y -= projectiles[i].speed * delta_time; break;
            case DIR_DOWNLEFT: projectiles[i].x -= projectiles[i].speed * delta_time; projectiles[i].y += projectiles[i].speed * delta_time; break;
            case DIR_DOWNRIGHT: projectiles[i].x += projectiles[i].speed * delta_time; projectiles[i].y += projectiles[i].speed * delta_time; break;
            default: break;
            }

            if (projectiles[i].x < -10 || projectiles[i].x > WINDOW_WIDTH ||
                projectiles[i].y < -10 || projectiles[i].y > WINDOW_HEIGHT) {
                projectiles[i].is_active = FALSE;
            }
        }
    }

    if (check_collision(ball.x, ball.y, ball.width, ball.height,
        square.x, square.y, square.width, square.height)) {
        printf("COLISÃO DETECTADA ENTRE BALL E SQUARE!\n");
    }

    // Limitar posição de ball
    if (ball.x < 0) ball.x = 0;
    if (ball.y < 0) ball.y = 0;
    if (ball.x + ball.width > WINDOW_WIDTH) ball.x = WINDOW_WIDTH - ball.width;
    if (ball.y + ball.height > WINDOW_HEIGHT) ball.y = WINDOW_HEIGHT - ball.height;

    // Limitar posição de square
    if (square.x < 0) square.x = 0;
    if (square.y < 0) square.y = 0;
    if (square.x + square.width > WINDOW_WIDTH) square.x = WINDOW_WIDTH - square.width;
    if (square.y + square.height > WINDOW_HEIGHT) square.y = WINDOW_HEIGHT - square.height;
}

void render() {

    SDL_RenderClear(renderer);

    // Renderiza os mapas
    if (background_textures[mapa_atual]) {
        SDL_RenderCopy(renderer, background_textures[mapa_atual], NULL, NULL);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 122, 122, 122, 0);
        SDL_RenderClear(renderer);
    }
    for (int i = 0; i < num_obstaculos[mapa_atual]; i++) {
        SDL_Rect obs_rect = {
            (int)obstaculos[mapa_atual][i].x,
            (int)obstaculos[mapa_atual][i].y,
            (int)obstaculos[mapa_atual][i].width,
            (int)obstaculos[mapa_atual][i].height
        };
        SDL_SetRenderDrawColor(renderer, 122, 122, 122, 255); // cor dos obstaculos
        SDL_RenderFillRect(renderer, &obs_rect);
    }

    // Renderiza ball
    SDL_Rect ball_rect = { (int)ball.x, (int)ball.y, (int)ball.width, (int)ball.height };
    if (strcmp(PERSONAGENS[escolhaP1 - 1], "meneghetti") == 0 || strcmp(PERSONAGENS[escolhaP1 - 1], "miranha") == 0 || strcmp(PERSONAGENS[escolhaP1 - 1], "mario") == 0) {
        if (ball.last_dir == DIR_LEFT || ball.last_dir == DIR_UPLEFT || ball.last_dir == DIR_DOWNLEFT) {
            if (ball.anim_left.frame_count > 0 && (a_pressed || w_pressed || s_pressed)) { 
                // Use animation if moving 
                SDL_RenderCopy(renderer, ball.anim_left.frames[ball.anim_left.current_frame], NULL, &ball_rect);
            }
            else if (ball.anim_left.frame_count > 0) {
                // Use static image if not moving
                SDL_RenderCopy(renderer, ball.anim_left.frames[0], NULL, &ball_rect);
            }
            else {
                SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
            }
        }
        else if (ball.last_dir == DIR_RIGHT || ball.last_dir == DIR_UPRIGHT || ball.last_dir == DIR_DOWNRIGHT ||
            ball.last_dir == DIR_UP || ball.last_dir == DIR_DOWN) {
            if (ball.anim_right.frame_count > 0 && (d_pressed || w_pressed || s_pressed)) {
                // Use animation if moving
                SDL_RenderCopy(renderer, ball.anim_right.frames[ball.anim_right.current_frame], NULL, &ball_rect);
            }
            else if (ball.anim_right.frame_count > 0) {
                // Use static image if not moving
                SDL_RenderCopy(renderer, ball.anim_right.frames[0], NULL, &ball_rect);
            }
            else {
                SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
            }
        }
        else {
            // Use default texture for other directions
            SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
        }
    }
    else {
        // For non-Meneghetti characters, use default texture
        SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
    }

    // Renderiza square
    SDL_Rect square_rect = { (int)square.x, (int)square.y, (int)square.width, (int)square.height };
    if (strcmp(PERSONAGENS[escolhaP2 - 1], "meneghetti") == 0 || strcmp(PERSONAGENS[escolhaP2 - 1], "miranha") == 0 || strcmp(PERSONAGENS[escolhaP2 - 1], "mario") == 0) {
        if (square.last_dir == DIR_LEFT || square.last_dir == DIR_UPLEFT || square.last_dir == DIR_DOWNLEFT) {
            if (square.anim_left.frame_count > 0 && (left_pressed || up_pressed || down_pressed)) {  // Include vertical movement
                // Use animation if moving
                SDL_RenderCopy(renderer, square.anim_left.frames[square.anim_left.current_frame], NULL, &square_rect);
            }
            else if (square.anim_left.frame_count > 0) {
                // Use static image if not moving
                SDL_RenderCopy(renderer, square.anim_left.frames[0], NULL, &square_rect);
            }
            else {
                SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
            }
        }
        else if (square.last_dir == DIR_RIGHT || square.last_dir == DIR_UPRIGHT || square.last_dir == DIR_DOWNRIGHT ||
            square.last_dir == DIR_UP || square.last_dir == DIR_DOWN) {  // Added DIR_UP and DIR_DOWN
            if (square.anim_right.frame_count > 0 && (right_pressed || up_pressed || down_pressed)) {  // Include vertical movement
                // Use animation if moving
                SDL_RenderCopy(renderer, square.anim_right.frames[square.anim_right.current_frame], NULL, &square_rect);
            }
            else if (square.anim_right.frame_count > 0) {
                // Use static image if not moving
                SDL_RenderCopy(renderer, square.anim_right.frames[0], NULL, &square_rect);
            }
            else {
                SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
            }
        }
        else {
            // Use default texture for other directions
            SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
        }
    }
    else {
        // For non-Meneghetti characters, use default texture
        SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
    }
    // Renderiza projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].is_active) {
            SDL_Rect proj_rect = { (int)projectiles[i].x, (int)projectiles[i].y, 15, 15 };
            if (projectiles[i].texture) {
                SDL_RenderCopy(renderer, projectiles[i].texture, NULL, &proj_rect);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 70, 0, 255);
                SDL_RenderFillRect(renderer, &proj_rect);
            }
        }
    }

    int max_bar_width = 100;
    int bar_height = 25;

    // Barra de vida de ball
    SDL_Rect ball_bg = { 105, 20, max_bar_width * 3.32, bar_height }; // fundo da barra (vermelho)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // vermelho
    SDL_RenderFillRect(renderer, &ball_bg);

    SDL_Rect ball_hp = { 105, 20, max_bar_width * ball_life / 3, bar_height }; // barra de vida (verde)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // verde
    SDL_RenderFillRect(renderer, &ball_hp);

    // Barra de vida de square
    SDL_Rect square_bg = { WINDOW_WIDTH - max_bar_width - 340, 20, max_bar_width * 3.32, bar_height };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // vermelho
    SDL_RenderFillRect(renderer, &square_bg);

    SDL_Rect square_hp = { WINDOW_WIDTH - max_bar_width - 340, 20, max_bar_width * square_life / 3, bar_height };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &square_hp);

    SDL_RenderPresent(renderer);
}

void destroy_window() {
    // Free ball animations
    for (int i = 0; i < ball.anim_left.frame_count; i++) {
        if (ball.anim_left.frames[i]) {
            SDL_DestroyTexture(ball.anim_left.frames[i]);
        }
    }
    for (int i = 0; i < ball.anim_right.frame_count; i++) {
        if (ball.anim_right.frames[i]) {
            SDL_DestroyTexture(ball.anim_right.frames[i]);
        }
    }

    // Free square animations
    for (int i = 0; i < square.anim_left.frame_count; i++) {
        if (square.anim_left.frames[i]) {
            SDL_DestroyTexture(square.anim_left.frames[i]);
        }
    }
    for (int i = 0; i < square.anim_right.frame_count; i++) {
        if (square.anim_right.frames[i]) {
            SDL_DestroyTexture(square.anim_right.frames[i]);
        }
    }

    if (ball.texture) {
        SDL_DestroyTexture(ball.texture);
    }

    if (square.texture) {
        SDL_DestroyTexture(square.texture);
    }

    for (int i = 0; i < MAX_MAPAS; i++) {
        if (background_textures[i]) {
            SDL_DestroyTexture(background_textures[i]);
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].texture) {
            SDL_DestroyTexture(projectiles[i].texture);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void mostrarMenuPersonagens(const char* jogador) {
    printf("\n===== %s, selecione seu personagem =====\n", jogador);
    for (int i = 0; i < NUM_PERSONAGENS; i++) {
        printf("%d - %s\n", i + 1, PERSONAGENS[i]);
    }
}

void selecionarPersonagens() {
    // Player 1
    printf("\n===== Player 1 (Ball), selecione seu personagem =====\n");
    for (int i = 0; i < NUM_PERSONAGENS; i++) {  // Fixed .0 to 0
        printf("%d - %s\n", i + 1, PERSONAGENS[i]);
    }

    while (escolhaP1 < 1 || escolhaP1 > NUM_PERSONAGENS) {
        printf("Digite sua escolha (1-%d): ", NUM_PERSONAGENS);
        scanf_s("%d", &escolhaP1);
        if (escolhaP1 < 1 || escolhaP1 > NUM_PERSONAGENS) {
            printf("Opcao invalida! Tente novamente.\n");
        }
    }

    // Player 2
    printf("\n===== Player 2 (Square), selecione seu personagem =====\n");
    for (int i = 0; i < NUM_PERSONAGENS; i++) {
        printf("%d - %s\n", i + 1, PERSONAGENS[i]);
    }

    while (escolhaP2 < 1 || escolhaP2 > NUM_PERSONAGENS) {
        printf("Digite sua escolha (1-%d): ", NUM_PERSONAGENS);
        scanf_s("%d", &escolhaP2);
        if (escolhaP2 < 1 || escolhaP2 > NUM_PERSONAGENS) {
            printf("Opcao invalida! Tente novamente.\n");
        }
    }

    // Construir caminhos das texturas
    snprintf(ball.personagem_path, sizeof(ball.personagem_path), "assets/%s.png", PERSONAGENS[escolhaP1 - 1]);
    snprintf(square.personagem_path, sizeof(square.personagem_path), "assets/%s.png", PERSONAGENS[escolhaP2 - 1]);
}

void init_animation(Animation* anim, float frame_time) {
    anim->frame_count = 0;
    anim->current_frame = 0;
    anim->frame_time = frame_time;
    anim->time_accumulated = 0;

    for (int i = 0; i < MAX_FRAMES; i++) {
        anim->frames[i] = NULL;
    }
}

int load_animation_frames(Animation* anim, SDL_Renderer* renderer, const char* base_path, int num_frames) {
    char path[100];

    for (int i = 0; i < num_frames && i < MAX_FRAMES; i++) {
        sprintf_s(path, sizeof(path), "%s_%d.png", base_path, i + 1);

        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            printf("Failed to load animation frame: %s\n", path);
            return 0;
        }

        anim->frames[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!anim->frames[i]) {
            printf("Failed to create texture from surface: %s\n", path);
            return 0;
        }

        anim->frame_count++;
    }

    return anim->frame_count > 0;
}

void update_animation(Animation* anim, float delta_time) {
    if (anim->frame_count <= 1) return;

    anim->time_accumulated += delta_time;

    if (anim->time_accumulated >= anim->frame_time) {
        anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
        anim->time_accumulated = 0;
    }
}

void setup() {

    const char* background_paths[MAX_MAPAS] = {
    "assets/background.png",
    "assets/background2.png",
    "assets/background3.png"
    };

    for (int i = 0; i < MAX_MAPAS; i++) {
        SDL_Surface* bg_surface = IMG_Load(background_paths[i]);
        if (bg_surface) {
            background_textures[i] = SDL_CreateTextureFromSurface(renderer, bg_surface);
            SDL_FreeSurface(bg_surface);
        }
        else {
            printf("Erro carregando background %d: %s\n", i, IMG_GetError());
        }
    }

    // Obstáculos para o mapa 1
    num_obstaculos[1] = 3;
    obstaculos[1][0] = (obstacle){ 650, 760, 550, 300 };
    obstaculos[1][1] = (obstacle){ 650, 200, 100, 500 };
    // obstaculos[1][2] = (obstacle){ 350, 450, 120, 60 };

     // Obstáculos para o mapa 2
    num_obstaculos[2] = 4;
    obstaculos[2][0] = (obstacle){ 650, 700, 500, 100 };
    obstaculos[2][1] = (obstacle){ 650, 300, 100, 500 };
    obstaculos[2][2] = (obstacle){ 100, 700, 150, 300 };
    obstaculos[2][3] = (obstacle){ 400, 600, 60, 180 };

    for (int i = 0; i < MAX_MAPAS; i++) {
        for (int j = 0; j < num_obstaculos[i]; j++) {
            obstaculos[i][j].rect.x = (int)obstaculos[i][j].x;
            obstaculos[i][j].rect.y = (int)obstaculos[i][j].y;
            obstaculos[i][j].rect.w = (int)obstaculos[i][j].width;
            obstaculos[i][j].rect.h = (int)obstaculos[i][j].height;
        }
    }
    // Estrutura da bola
    ball.x = WINDOW_WIDTH / 4;
    ball.y = WINDOW_HEIGHT / 2.16;
    ball.width = 75;
    ball.height = 75;
    ball.last_dir = DIR_RIGHT; // Direção inicial padrão

    // Carrega textura da bola
    SDL_Surface* tmpSurface = IMG_Load(ball.personagem_path);
    if (tmpSurface) {
        ball.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
    }
    else {
        printf("Erro ao carregar textura do Player 1 (%s): %s\n", ball.personagem_path, IMG_GetError());
        // Carrega uma textura padrão se necessário
        tmpSurface = IMG_Load("assets/padrao.png");  // Adicione uma fallback texture
        if (tmpSurface) {
            ball.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
            SDL_FreeSurface(tmpSurface);
        }
    }

    // Initialize animations for ball
    init_animation(&ball.anim_left, 0.1f);  // 0.1 seconds per frame
    init_animation(&ball.anim_right, 0.1f);

    // Load animation frames if meneghetti is selected
    if (strcmp(PERSONAGENS[escolhaP1 - 1], "meneghetti") == 0) {
        load_animation_frames(&ball.anim_left, renderer, "assets/meneghetti_left", 4);  // Assuming 4 frames
        load_animation_frames(&ball.anim_right, renderer, "assets/meneghetti_right", 4);
    }
    // Load animation frames if miranha is selected (for player 1)
    if (strcmp(PERSONAGENS[escolhaP1 - 1], "miranha") == 0) {
        load_animation_frames(&ball.anim_left, renderer, "assets/miranha_left", 4);  // Supondo 4 frames
        load_animation_frames(&ball.anim_right, renderer, "assets/miranha_right", 4);
    }
    // Load animation frames if mario is selected (for player 1)
    if (strcmp(PERSONAGENS[escolhaP1 - 1], "mario") == 0) {
        load_animation_frames(&ball.anim_left, renderer, "assets/mario_left", 4);  // Supondo 4 frames
        load_animation_frames(&ball.anim_right, renderer, "assets/mario_right", 4);
    }

    // Estrutura do quadrado
    square.x = WINDOW_WIDTH / 1.47;
    square.y = WINDOW_HEIGHT / 2.16;
    square.width = 75;
    square.height = 75;
    square.last_dir = DIR_LEFT;

    tmpSurface = IMG_Load(square.personagem_path);
    if (tmpSurface) {
        square.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
    }
    else {
        printf("Erro ao carregar textura do Player 2 (%s): %s\n", square.personagem_path, IMG_GetError());
        // Carrega uma textura padrão se necessário
        tmpSurface = IMG_Load("assets/padrao.png");  // Mesma fallback
        if (tmpSurface) {
            square.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
            SDL_FreeSurface(tmpSurface);
        }
    }

    // Initialize animations for square
    init_animation(&square.anim_left, 0.1f);
    init_animation(&square.anim_right, 0.1f);

    // Load animation frames if meneghetti is selected
    if (strcmp(PERSONAGENS[escolhaP2 - 1], "meneghetti") == 0) {
        load_animation_frames(&square.anim_left, renderer, "assets/meneghetti_left", 4);
        load_animation_frames(&square.anim_right, renderer, "assets/meneghetti_right", 4);
    }
    // Load animation frames if miranha is selected (for player 2)
    if (strcmp(PERSONAGENS[escolhaP2 - 1], "miranha") == 0) {
        load_animation_frames(&square.anim_left, renderer, "assets/miranha_left", 4);
        load_animation_frames(&square.anim_right, renderer, "assets/miranha_right", 4);
    }
    // Load animation frames if mario is selected (for player 2)
    if (strcmp(PERSONAGENS[escolhaP2 - 1], "mario") == 0) {
        load_animation_frames(&square.anim_left, renderer, "assets/mario_left", 4);
        load_animation_frames(&square.anim_right, renderer, "assets/mario_right", 4);
    }

    // Inicializa projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].is_active = FALSE;
        projectiles[i].speed = 800.0f;
        projectiles[i].texture = NULL;
        projectiles[i].personagem_disparador[0] = '\0'; // Inicializa como string vazia
    }
}

int main() {
    printf("===== Selecione o plano de fundo =====\n");
    printf("1 - Padrao \n");
    printf("2 - Mata da Ufpb \n");
    printf("3 - Uma Vez Flamengo... \n");

    int escolha = 0;
    while (escolha < 1 || escolha > 3) {
        printf("Digite sua escolha (1-3): ");
        scanf_s("%d", &escolha);
        if (escolha < 1 || escolha > 3) {
            printf("Opcao invalida! Tente novamente.\n");
        }
    }


    mapa_atual = escolha - 1;
    selecionarPersonagens();
    game_is_running = initialize_window();
    setup();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    int mopa;
    scanf_s("%d", &mopa);
    return 0;
}