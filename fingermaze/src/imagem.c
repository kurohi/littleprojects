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
* Contém as funções de aquisição da imagem usando o fprint
*/
#include "imagem.h"

struct fp_dscv_dev * descobrir_dispositivos(struct fp_dscv_dev ** dispositivos_descobertos){
	struct fp_dscv_dev *ddev=dispositivos_descobertos[0];
	struct fp_driver *driver;
	if(!ddev){
		return NULL;
	}
	driver=fp_dscv_dev_get_driver(ddev);
	printf("Utilizando o driver: %s\n",fp_driver_get_full_name(driver));
	return ddev;
}

imagem* pegar_imagem(){
	int r=1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **dispoditivos_descobertos;
	struct fp_dev *dev;
	struct fp_img *fprint_imagem=NULL;

	r=fp_init();
	if(r<0){
		printf("Não possui o fprint\n");
		exit(1);
	}
	dispoditivos_descobertos=fp_discover_devs();
	if(!dispoditivos_descobertos){
		printf("nenhum dispositivo encontrado\n");
		fp_exit();
		exit(1);
	}
	ddev=descobrir_dispositivos(dispoditivos_descobertos);
	if(!ddev){
		printf("Não pode abrir o dispositivo\n");
		fp_exit();
		exit(1);
	}
	dev=fp_dev_open(ddev);
	fp_dscv_devs_free(dispoditivos_descobertos);
	if(!dev){
		printf("Não pode abrir o dispositivo\n");
		fp_exit();
		exit(1);
	}

	if(!fp_dev_supports_imaging(dev)){
		printf("Não é um leitor de impressão digital\n");
		fp_dev_close(dev);
		fp_exit();
		exit(1);
	}
	printf("Dispositivo aberto, começando a aquisição\n");
	r=fp_dev_img_capture(dev,0,&fprint_imagem);
	if(r){
		printf("Não pode pegar a imagem\n");
		fp_dev_close(dev);
		fp_exit();
		exit(1);
	}
	imagem* img = (imagem*) malloc (sizeof(imagem));
	img->width = fp_img_get_width(fprint_imagem);
	img->height = fp_img_get_height(fprint_imagem);
	img->data = (unsigned char*) malloc(img->width * img->height);
	unsigned char *pointer = fp_img_get_data(fprint_imagem);
	memcpy(img->data, pointer, img->width * img->height);
	
	fp_dev_close(dev);
	fp_img_free(fprint_imagem);
	fp_exit();
	return img;
}

void salvar_imagem(struct imagem* img, char* filename){
	FILE *fl,*fll;
	if((fl=fopen(filename,"wb"))==NULL){
		printf("Falha ao abrir o arquivo\n");
	}char header[20]="P5\n384 289 255 \n";
	int x;
	fwrite(header,strlen(header),1,fl);
	unsigned char uc;
	for(x=0;x<img->width*img->height;x++){
		uc=img->data[x];
		fwrite(&uc,1,1,fl);
	}fclose(fl);
}

void free_imagem(struct imagem* imagem){
	free(imagem->data);
	free(imagem);
}
