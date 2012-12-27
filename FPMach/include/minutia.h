#ifndef _MINUTIA_H_
#define _MINUTIA_H_

//inclusao das bibliotecas necessarias
#include <image.h>
#include <math.h>

#ifdef USEOPENCV
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

//Restrição no espaço de procura(em pixels)
#define	LIM_X	40
#define	LIM_Y	40

//Restrição para a proximidade de minucias
#define RANGE	20

//Maximo para considerar uma minucia de borda
#define	BORDER_MAX	50

//Tipos de minucias
#define	ENDLINE		0
#define	BIFURCATION	1

//Estrutura da minucia
typedef struct minutia{
	int x,y;
	double angle;
	long distance;
	char type;
	int mincount;
	struct minutia *next;
}minutia;

#ifdef USEOPENCV
#define PI 		3.14159265	//PI
/*
---showMinutia---
Mostra a imagem da impresão digitao contendo as minucias e a direção para qual apontam
Entrada:
   -image* img: Imagem onde serão colocadas as minucias
   -minutia *root: lista de minucias
   -int win_x: Tamanho da janela que limita a seta da direção, eixo X
   -int win_y: Mesma coisa, eixo Y
Saida: Void
*/
void showMinutia(image *img, minutia *root,int win_x, int win_y);
#endif

/*
---findMinutia---
Acha todas as minucias na imagem e retorna uma lista com elas. A função usa o algorítimo do CrossingNumbers, contando o numero
de transições de preto(fundo) para branco(linha)
Entrada:
   -image *img: Imagem depois do pre-processamento
   -double *direcs: Matriz direcional que foi conseguida no processo de pre-processamento
   -int win_x: Tamanho da janela usada para pegar as direções, para pegar o endereço na memoria(eixo X)
   -int win_y: Tamanho da janela (eixo Y)
Saida:
   -minutia *: ponteiro para a lista de minucias
*/
minutia *findMinutia(image *img, double *direcs, int win_x, int win_y);

/*
---allocMinutia---
Aloca e coloca na lista um elemento da lista de minucias, necessita de todos os dados já em maos.
Entrada:
   -minutia *root: ponteiro para a raiz da lista
   -int x: coordenada X da minucia
   -int y: coordenada Y da minucia
   -double angle: direção da linha em que a minucia se encontra
   -char type: tipo da minucia
Saida:
   -minutia *: ponteiro para a raiz, para o caso de ser NULL
*/
minutia *allocMinutia(minutia *root, int x, int y, double angle, char type);

/*
---freeMinutia---
Libera a lista de minucias da memoria
Entrada:
   -minutia *root: ponteiro para a raiz da lista
Saida: Void
*/
void freeMinutiaList(minutia *root);

/*
---deleteMinutia---
Libera apenas uma minucia
Entrada:
   -minutia *root: ponteiro para a raiz da lista
   -minutia *minu: ponteiro para a minucia a ser deletada
Saida:
   -minutia *:para o caso de deletar a raiz
*/
minutia *deleteMinutia(minutia *root, minutia *minu);

/*
---markLine---
Marca uma linha e susas bifurcações com um determinado valor para o pos processamento.
Usa recursividade para as bifurcações.
Entrada:
   -image *img: Pedaço da imagem copiado para editar
   -unsigned char value: Valor a ser colocado nos pixels
   -int x: Posição atual para a avaliação no eixo X(necessario para a recurcividade)
   -int y: Posição no eixo Y
   -int win_x: Tamanho desse pedaço de imagem(eixoX)
   -int win_y: Tamanho desse pedaço de imagem(eixoY)
Saida: Void
*/
void markLine(image *img, unsigned char value, int x, int y, int win_x, int win_y);

/*
---posprocessing---
Remove as minucias falsas decorrentes do processo de pre-processamento
As entradas nesse arquivo serão:
(x,y)(angulo)(distancia)(tipo)\n
Entrada:
   -image *img: imagem já afinada que servira como base
   -minutia *root: ponteiro para a raiz da lista de minucias
   -int win_x: tamanho da janela limite para o pos-processamento(eixo X)
   -int win_y: tamanho da janela limite para o pos-processamento(eixo Y)
Saida: 
   minutia *: vetor com as minucias sem as falsas
*/
minutia *posprocessing(image *img, minutia *root, int win_x, int win_y);


/*
---cloneMinutiaList---
Clona uma lista de minucias para outra variavel. As duas listas estarão em partes diferentes da memória
então a exlusão de uma não acarretará na exclusão da outra.
Entrada:
   -minutia *template: Lista a ser copiada
   -minutia *last: ponteiro para a lista antiga, para evitar ficar alocando e desalocando memoria
Saida: minutia *
   Ponteiro que receberá a nova lista
*/
minutia *cloneMinutiaList(minutia *temp, minutia *last);

/*
---removeBorderminutia---
Remove as minucias que aparecem nas bordas da impressão digital. Normalemnte causadas na fronteira da
impressão digital com o fundo. Ela o faz seguindo a direção da linha em que a impressão digital se encntra
Até achar ou o final da imagem, ou outra linha, ou ter andado o suficiente para dizer que não encontrará mais nada.
Entrada:
   -image *img: Imagem da impressão digital
   -minutia *root: lista de minucias da impressão digital
Saida: minutia *
   Ponteiro para a nova lista de minucias, é preciso retornar isso porque a minucia root pode ser eliminada eventualmente
*/
minutia *removeBorderMinutia(image *img, minutia *root);

#endif
