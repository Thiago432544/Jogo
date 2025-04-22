#define FALSE 0
#define TRUE 1

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


#define FPS 300
#define FRAME_TARGET_TIME (1000/FPS)

#define MAX_MAPAS 3
#define MAX_OBSTACULOS 100

#define NUM_PERSONAGENS 5

//aqui é o nome dos arquivos dos personagens (sem botar ".png")
static const char* PERSONAGENS[NUM_PERSONAGENS] = {
    "mario",
    "meneghetti",
    "miranha",
    "tale",
    "python"
};

// aqui é tamanho máximo do caminho do personagem
#define TAM_MAX_PATH_PERSONAGEM 50