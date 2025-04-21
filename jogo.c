#include <stdio.h>
#include <SDL.h>
#include "./constantes.h"
#include <SDL_image.h>
#include <math.h>

int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int last_frame_time = FALSE;

char w_pressed = FALSE, a_pressed = FALSE, s_pressed = FALSE, d_pressed = FALSE, space_pressed = FALSE;
char up_pressed = FALSE, down_pressed = FALSE, right_pressed = FALSE, left_pressed = FALSE, c_pressed = FALSE;

SDL_Texture* background_textures[MAX_MAPAS];
int mapa_atual = 0;


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

struct projectile {
    float x;
    float y;
    float speed;
    Direction dir;
    int is_active;
    int from_square;
    SDL_Texture* texture;
}projectile;

struct ball {
    float x;
    float y;
    float width;
    float height;
    SDL_Texture* texture;
    Direction last_dir;
} ball;

struct square {
    float x;
    float y;
    float width;
    float height;
    SDL_Texture* texture;
    Direction last_dir;
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
    SDL_Surface* tmpSurface = IMG_Load("assets/mario.png");
    if (tmpSurface) {
        ball.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
    }
    // Estrutura do quadrado
    square.x = WINDOW_WIDTH / 1.47;
    square.y = WINDOW_HEIGHT / 2.16;
    square.width = 75;
    square.height = 75;
    square.last_dir = DIR_LEFT;

    tmpSurface = IMG_Load("assets/meneghetti.png");
    if (tmpSurface) {
        square.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
    }


    // Inicializa projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].is_active = FALSE;
        projectiles[i].speed = 800.0f;
        projectiles[i].texture = NULL;

        // Cria superfície simples para o projétil 
        tmpSurface = IMG_Load("assets/c.png");
        if (tmpSurface) {
            projectiles[i].texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
            SDL_FreeSurface(tmpSurface);
        }
    }
}
void fire_projectile(int from_square) {
    Direction dir;
    float x, y;

    if (from_square) {
        if (square.last_dir == DIR_NONE) {
            return;
        }
        dir = square.last_dir;
        x = square.x + square.width / 2 - 5;
        y = square.y + square.height / 2 - 5;
    }
    else {
        if (ball.last_dir == DIR_NONE) {
            return;
        }
        dir = ball.last_dir;
        x = ball.x + ball.width / 2 - 5;
        y = ball.y + ball.height / 2 - 5;
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].is_active) {
            projectiles[i].x = x;
            projectiles[i].y = y;
            projectiles[i].dir = dir;
            projectiles[i].is_active = TRUE;
            projectiles[i].from_square = from_square;
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
    if (ball.texture) {
        SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &ball_rect);
    }
    // Renderiza square
    SDL_Rect square_rect = { (int)square.x, (int)square.y, (int)square.width, (int)square.height };
    if (square.texture) {
        SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &square_rect);
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