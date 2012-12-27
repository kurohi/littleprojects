/*
* Universidade de brasília - UnB
* Instituto de Ciências Exatas - IE
* Departamento de Ciência da Computação - CiC
* 
* Disciplina: Programação avançada
* Professor: Jan Correia
* 
* Aluno: Igor de Carvalho Coelho
* Matricula: 06/20068
*
* Programa principal
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define USE_FPRINT

#ifdef USE_FPRINT
#include "imagem.h"
#endif

#include "enhance.h"

#define REGULADOR 20 //serve para limitar a altura das paredes, quanto maior o valor menor a parede
#define BLIMIT 120


//função para substituir texto
void substitui(char *orig, char *proc, char *subs){
	char temp[strlen(orig)*2];
	char *pt;
	pt = strstr(orig,proc);
	strncpy(temp,orig,(pt-orig));
	temp[pt-orig] = '\0';
	strcat(temp,subs);
	strcat(temp,(char*)(int)pt+(int)strlen(proc));
	strcpy(orig,temp);
}

//cria uma peça do labirinto e já salva no arquivo
void makeMaze(imagem *img, int x1, int z1, char *grid, FILE *arq){
	char string[1000];
	sprintf(string, "%d", x1);
	substitui(grid,"tra_x",string);
	substitui(grid,"tra_y","-10");
	sprintf(string, "%d", z1);
	substitui(grid,"tra_z",string);
	sprintf(string,"%.2f", (float)img->width/(float)2);
	sprintf(string, "%d", img->width);
	substitui(grid, "DIM_X", string);
	sprintf(string, "%d", img->height);
	substitui(grid, "DIM_Z", string);
	substitui(grid, "ESP_X", "1.025");
	substitui(grid, "ESP_Z", "1.025");
	char  *pt = strstr(grid, "COORDS");
	fwrite(grid, sizeof(char), (long) pt-(int)grid, arq);
	int x,z;
	for(z=0; z<img->height; z++){
		for(x=0; x<img->width; x++){
			sprintf(string," %.2f",(float)img->data[x+z*img->width]/(float)REGULADOR);
			fwrite(string, sizeof(char), strlen(string), arq);
		}
		sprintf(string, "\n\t\t\t");
		fwrite(string, sizeof(char), strlen(string), arq);
	}
	fprintf(arq,"\n\t\t\t]\n\t\t}\n\t}\n  ]\n}\n\n");
}

//carrega uma imagem pgm
imagem * loadPGM(char *filename){
	FILE *arq;
	if((arq = fopen(filename,"rb"))==NULL){
		printf("Erro ao abrir a foto %s\n", filename);
		return NULL;
	}
	imagem *img = (imagem*) malloc (sizeof(imagem));
	char string[200];
	char buf;
	int  i;
	while(1){
		i=0;
		buf = 'x';
		while(buf != '\n'){
			buf = fgetc(arq);
			string[i++] = buf;
		}
		if((string[0]!='P')&&(string[0]!='#')){
			sscanf(string,"%d %d\n",&img->width,&img->height);
			break;
		}
	}
	img->data = (unsigned char*) malloc (img->width * img->height);
	fread(img->data, sizeof(char), img->width * img->height, arq);
	fclose(arq);
	return img;
}

//função para salvar uma imagem .pgm
int savePGM(imagem *img, char *filename){
	FILE *arq;
	if((arq = fopen(filename,"wb+"))==NULL){
		printf("Erro ao abrir a foto %s\n", filename);
		return 0;
	}
	fprintf(arq,"P5\n%d %d 255\n",img->width, img->height);
	fwrite(img->data, sizeof(char), img->width*img->height, arq);
	fclose(arq);
	return 1;
}

//Descobre se a imagem é de fundo branco ou de fundo preto
int PB_BP(imagem *img){
	long branco=0;
	int x,y;
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			if(img->data[x+y*img->width] < BLIMIT){
				branco++;
			}
		}
	}
	if(branco > (img->width*img->height)-branco){
		return 1;
	}
	return 0;
}

//main
int main(int argc, char **argv){
	printf("========Bem vindo ao GOOYS: Get out of your self============\n\n\n");
	int BLOCK_X,BLOCK_Y;
#ifndef USE_FPRINT
	printf("Rodando no modo Arquibo\n\n");
	if(argc<2){
		printf("O programa deve ser usadao assim: <%s> <arquivo.pgm>\n",argv[0]);
		return 1;
	}
#endif
	//aquisição do template do Elevation grid
	FILE *temp;
	if((temp = fopen("../templates/Tem_Ele.txt","r"))==NULL){
		printf("O template do elevation grid não foi achado\n");
		return 1;
	}
	char grid[1000];
	char gridaux[1000];
	int i=0;
	while(fread(&grid[0],1,1,temp)) i++;
	fseek(temp, 0, SEEK_SET);
	fread(grid, sizeof(char), i, temp);
	fclose(temp);
	imagem *img;
#ifndef USE_FPRINT
	//se não usar o frint apenas pega a imagem
	img = loadPGM(argv[1]);
#endif
#ifdef USE_FPRINT
	//se usar o fprint tem que pegar a imagem e fazer o pré-processamento dela
	printf("Rodando no modo Fprint\n\n");
	img = pegar_imagem();
	enhance(img);
#endif
	//definição do tamanho de cada Elevation grid
	BLOCK_X = 50;
	BLOCK_Y = 50;
	if(!PB_BP(img)){
		inverter_imagem(img);
	}
	imagem_binarize(img, BLIMIT);	//binariza a imagem para que as paredes tenham a mesma altura(e não de para escalá-las
	imagem_thin(img);			//afina as linhas para a espeçura de 1 pixel
	imagem_bipixel(img);			//almenta a espessura das linhas para 3 pixels
	savePGM(img, "../img_out/saida.pgm");	//salva como a imagem ficou, pode ser usado de mapa para se guiar no labirinto
	if((temp = fopen("../vrml/saida.wrl","wb+"))==NULL){
		printf("Não pode salvar o arquivo wrl\n");
		return 1;
	}
	fprintf(temp, "#VRML V2.0 utf8\n\n");
	int x,y,x1,y1;
	imagem *aux;
	aux = (imagem*) malloc (sizeof(imagem));
	aux->width = BLOCK_X;
	aux->height = BLOCK_Y;
	aux->data = (unsigned char*) malloc(BLOCK_X * BLOCK_Y);
	//montagem do arquivo vrml
	for(y=0; y<img->height; y+= BLOCK_Y){
		for(x=0; x<img->width; x+=BLOCK_X){
			for(y1=0; y1<BLOCK_Y; y1++){
				for(x1=0; x1<BLOCK_X; x1++){
					aux->data[x1+ y1*BLOCK_Y] = img->data[(x+x1)+(y+y1)*img->width];
				}
			}
			strcpy(gridaux, grid);
			makeMaze(aux, x-img->width/2, y-img->height/2, gridaux, temp);
		}
	}
	fclose(temp);
	printf("Arquivo gerado na pasta ../vrml/saida.wrl\n\n");
	return 0;
}
