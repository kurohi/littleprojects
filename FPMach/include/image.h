#ifndef _IMAGE_H_
#define _IMAGE_H_

//Bibliotecas padrão
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Bilbioteca para usar leitor de impressões digitais
#include <libfprint/fprint.h>

//Estrutura das imagens usadas
typedef struct image{
	int height;	//Altura
	int width;	//Largura
	unsigned char *data;	//Vetor contendo os pixels
}image;

//defines variados
#define	SUCCESS		1
#define	FAILURE		0

/*
---get_image---
Inicia o hardware de aquisição e pega uma imagem da impressão digital e apenas uma
Entrada:
  void
Saida: 
  -Um ponteiro para a imagem adiquirida
*/
image* get_image();

/*
---free_image---
Retira a imagem da memória
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
Saida: void
*/
void free_image(image* img);

/*
---save_image---
Salva a imagem em arquivo
Entrada:
  -image *img: Estrutura contendo a imagem da impressão digital
  -char *filename: Nome do arquivo da imagem
Saida: void
*/
void save_image(image* img, char* filename);

/*
---loadPGM---
Carrega uma imagem no formato PGM
Entrada:
  -char *filename: Endereçod o arquivo a ser aberto
Saida: 
  -Ponteiro para a imagem carregada
*/
image * loadPGM(char *filename);

/*
---clone_image---
Cria uma imagem nova como cópia de uma pré existente.
Deve-se usar um ponteiro desalocado para receber a imagem nova, pois essa função já aloca a memória necessaria.
Entrada:
   -image *img: Imagem a ser copiada
Saida:
   -image *: ponteiro para a imagem nova
*/
image *clone_image(image *img);

#endif
