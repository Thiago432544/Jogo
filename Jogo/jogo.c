#include<stdio.h>
#include<SDL.h>
#include "./constantes.h"
#include<SDL_image.h>

int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time = 0;

SDL_Texture* texture;

char w_pressed = FALSE, a_pressed = FALSE, s_pressed = FALSE, d_pressed = FALSE;

struct ball {
	float x;
	float y;
	float width;
	float height;
	SDL_Texture* texture;
} ball;

int initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Erro de inicializacao do SDL.\n");
		return FALSE;
	}
	SDL_Window* window = SDL_CreateWindow(
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
	SDL_PollEvent(&event);

	switch (event.type) {
	case SDL_QUIT:
		game_is_running = FALSE;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			game_is_running = FALSE;
		if (event.key.keysym.sym == SDLK_ESCAPE) game_is_running = FALSE;
		if (event.key.keysym.sym == SDLK_w) w_pressed = TRUE;
		if (event.key.keysym.sym == SDLK_a) a_pressed = TRUE;
		if (event.key.keysym.sym == SDLK_s) s_pressed = TRUE;
		if (event.key.keysym.sym == SDLK_d) d_pressed = TRUE;
		break;
	case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_w) w_pressed = FALSE;
		if (event.key.keysym.sym == SDLK_a) a_pressed = FALSE;
		if (event.key.keysym.sym == SDLK_s) s_pressed = FALSE;
		if (event.key.keysym.sym == SDLK_d) d_pressed = FALSE;

	}
}

void setup() {
	ball.x = 50;
	ball.y = 50;
	ball.width = 30;
	ball.height = 30;
	
	SDL_Surface* tmpSurface = IMG_Load("mario.png"); // Substitua pelo seu arquivo de textura
	if (!tmpSurface) {
		printf("Erro ao carregar imagem da bola: %s\n", IMG_GetError());
		
	}
	else {
		ball.texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
		SDL_FreeSurface(tmpSurface);
		if (!ball.texture) {
			printf("Erro ao criar textura da bola: %s\n", SDL_GetError());
		}
	}
}

void update() {
	//TODO: Perder algum tempo / Dormir ate a gente chegar no tempo do frame q a gente quer
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}
	//para conseguir um fator do tempo de delta convetido para segundos pra ser usado nos objetos
	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

	last_frame_time = SDL_GetTicks();
	if (a_pressed == TRUE) {
		ball.x -= 400 * delta_time;
	}
	if (d_pressed == TRUE) {
		ball.x += 400 * delta_time;
	}
	if (w_pressed == TRUE) {
		ball.y -= 400 * delta_time;
	}
	if (s_pressed == TRUE) {
		ball.y += 400 * delta_time;
	}
}

void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	//TODO: Aqui vamos começar a desenhar objetos do jogo



	SDL_Rect ball_rect = {
		(int)ball.x,
		(int)ball.y,
		(int)ball.width,
		(int)ball.height
	};
	if (ball.texture) {
		SDL_RenderCopy(renderer, ball.texture, NULL, &ball_rect);
	}
	else {
		SDL_SetRenderDrawColor(renderer, 255, 70, 0, 255);
		SDL_RenderFillRect(renderer, &ball_rect);
	}

	SDL_RenderPresent(renderer);

}

void destroy_window() {

	if (ball.texture) {
		SDL_DestroyTexture(ball.texture);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
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