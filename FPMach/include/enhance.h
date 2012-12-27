#ifndef _ENHANCE_H_
#define _ENHANCE_H_
//-----------------------Importando Bibliotecas-------------------------
//Biblioteca de imagem
#include <image.h>
//Biblioteca matemática, para as funções sin, cos, pow e etc
#include <math.h>
//Biblioteca para manipulação de vetores de bytes(strings)
#include <string.h>


//caso use open-cv para a visualização dos dados
#ifdef USEOPENCV
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

//---------------------Constantes pré-definidas-------------------------
#define GAUSSIGMA	1.4		//Limiar para a função gaussiana
#define GAUSSSIZE	5		//Tamanho da matriz gaussiana
#define PI 		3.14159265	//PI
#define SEGLIM		1.0		//Limiar para a segmentação da imagem
#define FREQMIN		0.08		//Frequencia minima, caso contrário copia a do viziho
#define FREQMAX		0.24		//Frequencia maxima, caso contrário copia a do vizinho
#define FREQLIM		65		//Limite para considerar os picos nos pixels da janela
#define GABSIGMA	3.5		//Filtro de Gabor: Parametro sigma
#define GABGAMMA	1.0		//Filtro de Gabor: Parametro gamma
#define GABPHASE	0.0		//Filtro de Gabor: Parametro deslocamento de fase
#define GABWIDTH	9
#define GABHEIGHT	9
#define BINLIMIAR	600		//Limite para a binarização da imagem
#define	LINE		255
#define	BACK		0
#define	SMOOTH_WIN	3

#define TEST_IMAGE	50		//porcentagem de branco aceitavel, menos que isso a imagem está falha

//---------------------Prototipo de funções-----------------------------
#ifdef USEOPENCV
/*
---show---
Mostra a imagem ou parte dela
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
  -int x: Ponto no eixo X onde começa o frame da imagem
  -int y: Ponto no eixo Y onde começa o frame da imagem
  -int jx: Ponto no eixo X onde termina o frame da imagem
  -int jy: Ponto no eixo Y onde termina o frame da imagem
  -char *winname: Nome da janela (já criada) do highgui onde mostrar a imagem
Saida: void

Exemplo: Para mostrar a imagem img inteira na janela com o nome "Teste"
show(img,0,0,img->width,img->height,"Teste");
*/
void show(image *img, int x, int y, int jx, int jy, char *winname);

/*
---showgraph---
Cria um gráfico em 2D do vetor dado com tamanho estipulado
Entrada:
  -unsigned char *graph: Vetor contendo os valores no qual o indice é a variavel independente
  -int size: Tamanho desse vetor
  -char *winname: Nome da janela (já criada) do highgui onde mostrar a imagem
Saida: void
*/
void showgraph(unsigned char *graph, int size, char *winame);

/*
---showdirec---
Mostra a matriz direcional por cima de uma imagem, A IMAGEM É ALTERADA ENTÃO USE UMA CÓPIA
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
  -int win_x: Tamanho da janela usada na estimação das direções, eixo X
  -int win_y: Tamanho da janela usada na estimação das direções, eixo Y
  -double *direc: Matriz direcional já estimada
  -char *winname: Nome da janela (já criada) do highgui onde mostrar a imagem
Saida: void
*/
void showdirec(image *img, int win_x, int win_y, double *direc, char *winname);
#endif

/*
---segmentation---
Separa o fundo da imagem, usa a varância dos pixels para isso
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital, entrada
  -image *dst: Estrutura de imagem para colocar a saida
  -int win_x: Tamanho da janela, eixo X
  -int win_y: Tamanho da janela, eixo Y
Saida: void
*/
void segmentation(image *img, image *dst, int win_x, int win_y);

/*
---equalization---
Melhora o contraste usando equalização de histograma, A IMAGEM É ALTERADA
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
Saida: void
*/
void equalization(image *img);

/*
---direction---
Preence a matriz direcional com a estimação da direção de cada bloco de imagem com tamanho win_x*win_y. Usa filtro de Sobel
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
  -double *direcs: Vetor (já alocado) para conter a matriz direcional
  -int win_x: Tamanho da janela, eixo X
  -int win_y: Tamanho da janela, eixo Y
Saida: void
*/
void direction(image *img, double *direcs,int win_x, int win_y);

/*
---makeGaussianMatrix---
Cria a matriz Gaussiana para uma futura convolução
Entrada:
  -double *matrix: Vetor já alocado para a matriz
  -int width: Tamanho da matriz, eixo X
  -int height: Tamanho da matriz, eixo Y
Saida:void
*/
void makeGaussianMatrix(double *matrix, int width, int height);

/*
---gaussian---
Passa o filtro gaussiano em uma matriz
Entrada:
  -double *matrix: Vetor já alocado para a matriz
  -int width: Largura da matriz para convolução
  -int height: Altura da matriz para convolução
Saida:void
*/
void gaussian(double *matrix, int width, int height);

/*
---frequency---
Estima a frequência de linhas na janela de tamanho win_x*win_y.
Funciona percorrendo a janela no sentido ortogonal a direção predominando e usa aos picos de valores dos pixels para determininar sua frequência
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
  -double *direc: Matriz direcional
  -double *freq: Vetor (já alocado) que guardará as frequências
  -int win_x: Tamanho da janela, eixo X
  -int win_y: Tamanho da janela, eixo Y
Saida: void
*/
void frequency(image *img, double *direc, double *freq, int win_x, int win_y);

/*
---make_gabor_Matrix---
Cria uma matriz para a convolução do filtro de Gabor
Entrada:
  -double direc: Valor da orientação estimada essa janela
  -double freq: Valor da frequência estimada dessa janela
  -int width: Largura da matriz para convolução
  -int height: Altura da matriz para convolução
  -double *gMatrix: Vetor (já alocado) para conter a matriz de convolução
Saida: void
*/
void make_gabor_Matrix(double direc, double freq, int width, int height, double *gMatrix);

/*
---gabor_filter---
Aplica o Filtro de Gabor
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital (entrada)
  -image *dst: Estrutura de imagem para colocar a saida
  -double *direcs: Matriz direcional
  -double *freqs: Matriz contendo os valores de frequência estimados
  -int win_x: Tamanho da janela, eixo X
  -int win_y: Tamanho da janela, eixo Y
Saida: void
*/
void gabor_filter(image *img, image *dst, double *direcs, double *freqs, int win_x, int win_y);

/*
---enhance---
Faz todo o pré-processamento da imagem utilizando as funções já definidas na ordem certa. ALTERA A IMAGEM
Entrada:
  -image *img: Imagem de entrada que será alterada
  -int win_x: Tamanho da janela, eixo X
  -int win_y: Tamanho da janela, eixo Y
Saida: void
*/

/*
---thinning---
Afina as linhas utilizando o algoritmo de Zhang e Suen
Entrada:
   -image *img: Imagem para aplicar o afinamento(está será editada então use uma copia)
Saida: Void
*/
void thinning(image *img);

/*
---enhance---
Aplica todo o pre-processamento na imagem deixando-a pronta para a extração das minucias
Entrada:
   -image *img: Imagem para o processamento(a imagem será editada então use uma copia)
   -int win_x: tamanho da Janela(usada em diversos processos) no eixo X
   -int win_y: tamanho da Janela no eixo y
Saida: int
   Retorna FAILURE se a imagem for predominantemente preta, ou seja ela ter vindo corrompida
*/
int enhance(image *img, int win_x, int win_y);

#endif
