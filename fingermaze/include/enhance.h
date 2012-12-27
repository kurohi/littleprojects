/*************************************************/
/*Universidade de Brasília - Unb                 */
/*Instituto de Ciências Exatas - IE              */
/*Departamento de Ciência da Computação - CIC    */
/*************************************************/

#ifndef __ENCHANCE_H__
#define __ENCHANCE_H__

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct imagem{
	int height;
	int width;
	unsigned char *data;
} imagem;

struct campo{
	double* pimg;
};

int pegar_direcao(struct imagem* fp, struct campo* ff, int block_size, int filter_size);
int imagem_get_frequency(struct imagem *fp, struct campo *direction, struct campo *frequency);
static void image_dilate(struct imagem *image);
static void image_erode(struct imagem *image);
int imagem_get_mask(struct imagem *fp, struct campo *direction,struct campo *frequency, struct imagem *mask);

static double enhance_gabor(double x, double y, double phi, double f, double r2);
int imagem_enhance_gabor(struct imagem *fp, struct campo *direction, struct campo *frequency, struct imagem *mask, double radius);

void imagem_binarize(struct imagem *fp, unsigned char limit);
void inverter_imagem(struct imagem* imagem);

void imagem_thin(struct imagem *fp);/**** Trocar essa função ****/
void imagem_bipixel(struct imagem *fp);

void enhance(imagem* img);

#endif

