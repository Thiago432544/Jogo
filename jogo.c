#include <stdio.h>
#include <SDL.h>
#include "./constantes.h"
#include <SDL_image.h>

int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int last_frame_time = 0;

char w_pressed = FALSE, a_pressed = FALSE, s_pressed = FALSE, d_pressed = FALSE, space_pressed = FALSE;
char up_pressed = FALSE, down_pressed = FALSE, right_pressed = FALSE, left_pressed = FALSE;

typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

struct projectile {
    float x;
    float y;
    float speed;
    Direction dir;
    int is_active;
    SDL_Texture* texture;
};

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

#define MAX_PROJECTILES 1000
struct projectile projectiles[MAX_PROJECTILES];

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
            break;
        }
    }
}

void setup() {
    // Estrutura da bola
    ball.x = 200;
    ball.y = 200;
    ball.width = 120;
    ball.height = 120;
    ball.last_dir = DIR_RIGHT; // Direção inicial padrão

    // Carrega textura da bola
    SDL_Surface* tmpSurface = IMG_Load("mario.png");
    if (tmpSurface) {
        ball.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
    }
    // Estrutura do quadrado
    square.x = 100;
    square.y = 100;
    square.width = 60;
    square.height = 60;
    square.last_dir = DIR_LEFT;
    square.texture = NULL;
    
    SDL_Surface* squareSurface = SDL_CreateRGBSurface(0, square.width, square.height, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (squareSurface) {
        SDL_FillRect(squareSurface, NULL, SDL_MapRGB(squareSurface->format, 160, 200, 0));  
        square.texture = SDL_CreateTextureFromSurface(renderer, squareSurface);
        SDL_FreeSurface(squareSurface);
    }


    // Inicializa projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].is_active = FALSE;
        projectiles[i].speed = 800.0f;
        projectiles[i].texture = NULL;

        // Cria superfície simples para o projétil 
        tmpSurface = SDL_CreateRGBSurface(0, 10, 10, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        if (tmpSurface) {
            SDL_FillRect(tmpSurface, NULL, SDL_MapRGB(tmpSurface->format, 0, 100, 255));
            projectiles[i].texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
            SDL_FreeSurface(tmpSurface);
        }
    }
}

void fire_projectile() {
    if (ball.last_dir == DIR_NONE) return;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].is_active) {
            projectiles[i].x = ball.x + ball.width / 2 - 5;
            projectiles[i].y = ball.y + ball.height / 2 - 5;
            projectiles[i].dir = ball.last_dir;
            projectiles[i].is_active = TRUE;
            break;
        }
    }
}

void update() {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    // Movimento da bola
    if (a_pressed) {
        ball.x -= 500 * delta_time;
        ball.last_dir = DIR_LEFT;
    }
    if (d_pressed) {
        ball.x += 500 * delta_time;
        ball.last_dir = DIR_RIGHT;
    }
    if (w_pressed) {
        ball.y -= 500 * delta_time;
        ball.last_dir = DIR_UP;
    }
    if (s_pressed) {
        ball.y += 500 * delta_time;
        ball.last_dir = DIR_DOWN;
    }
    //Movimento do quadrado
    if (left_pressed) {
        square.x -= 500 * delta_time;
        square.last_dir = DIR_LEFT;
    }
    if (right_pressed) {
        square.x += 500 * delta_time;
        square.last_dir = DIR_RIGHT;
    }
    if (up_pressed) {
        square.y -= 500 * delta_time;
        square.last_dir = DIR_UP;
    }
    if (down_pressed) {
        square.y += 500 * delta_time;
        square.last_dir = DIR_DOWN;
    }

    // Disparo de projéteis
    if (space_pressed) {
        fire_projectile();
        space_pressed = FALSE;
    }

    // Movimento dos projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].is_active) {
            switch (projectiles[i].dir) {
            case DIR_UP:    projectiles[i].y -= projectiles[i].speed * delta_time; break;
            case DIR_DOWN:  projectiles[i].y += projectiles[i].speed * delta_time; break;
            case DIR_LEFT:  projectiles[i].x -= projectiles[i].speed * delta_time; break;
            case DIR_RIGHT: projectiles[i].x += projectiles[i].speed * delta_time; break;
            default: break;
            }

            // Remove projéteis que saíram da tela
            if (projectiles[i].x < -10 || projectiles[i].x > WINDOW_WIDTH ||
                projectiles[i].y < -10 || projectiles[i].y > WINDOW_HEIGHT) {
                projectiles[i].is_active = FALSE;
            }
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Renderiza a bola
    SDL_Rect ball_rect = { (int)ball.x, (int)ball.y, (int)ball.width, (int)ball.height };
    if (ball.texture) {
        SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &ball_rect);
    }
    // Renderiza o quadrado
    SDL_Rect square_rect = { (int)square.x, (int)square.y, (int)square.width, (int)square.height };
    if (ball.texture) {
        SDL_RenderCopy(renderer, square.texture, NULL, &square_rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &square_rect);
    }
    // Renderiza projéteis
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].is_active) {
            SDL_Rect proj_rect = { (int)projectiles[i].x, (int)projectiles[i].y, 10, 10 };
            if (projectiles[i].texture) {
                SDL_RenderCopy(renderer, projectiles[i].texture, NULL, &proj_rect);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 70, 0, 255);
                SDL_RenderFillRect(renderer, &proj_rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void destroy_window() {
    if (ball.texture) {
        SDL_DestroyTexture(ball.texture);
    }
    if (square.texture) {
        SDL_DestroyTexture(square.texture);
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
        
        game_is_running = initialize_window();
        setup();

        while (game_is_running) {
            process_input();
            update();
            render();
        }

        destroy_window();
        return 0;
}