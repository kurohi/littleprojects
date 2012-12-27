#include<image.h>

//função que descobre qual driver usar baseado nos dispositivos encontrados
struct fp_dscv_dev * search_driver(struct fp_dscv_dev ** found_devices){
	struct fp_dscv_dev *ddev=found_devices[0];
	struct fp_driver *driver;
	if(!ddev){
		return NULL;
	}
	driver=fp_dscv_dev_get_driver(ddev);
	return ddev;
}

image *get_image(){
	int r=1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **found_devices;
	struct fp_dev *dev;
	struct fp_img *fprint_image=NULL;

	r=fp_init();
	if(r<0){
		printf("Não possui o fprint\n");
		return NULL;
	}
	found_devices=fp_discover_devs();
	if(!found_devices){
		printf("Nenhum dispositivo encontrado\n");
		fp_exit();
		return NULL;
	}
	ddev=search_driver(found_devices);
	if(!ddev){
		printf("Não pode abrir o dispositivo\n");
		fp_exit();
		return NULL;
	}
	dev=fp_dev_open(ddev);
	fp_dscv_devs_free(found_devices);
	if(!dev){
		printf("Não pode abrir o dispositivo\n");
		fp_exit();
		return NULL;
	}

	if(!fp_dev_supports_imaging(dev)){
		printf("Não é um leitor de impressão digital\n");
		fp_dev_close(dev);
		fp_exit();
		return NULL;
	}
	r=fp_dev_img_capture(dev,0,&fprint_image);
	if(r){
		printf("Não pode pegar a imagem\n");
		fp_dev_close(dev);
		fp_exit();
		return NULL;
	}
	image* img = (image*) malloc (sizeof(image));
	img->width = fp_img_get_width(fprint_image);
	img->height = fp_img_get_height(fprint_image);
	img->data = (unsigned char*) malloc(img->width * img->height);
	unsigned char *pointer = fp_img_get_data(fprint_image);
	memcpy(img->data, pointer, img->width * img->height);
	
	fp_dev_close(dev);
	fp_img_free(fprint_image);
	fp_exit();
	return img;
}

void free_image(image* img){
	if(img == NULL) return;
	free(img->data);
	free(img);
}

void save_image(image* img, char *filename){
	FILE *fl;
	if((fl=fopen(filename,"wb+"))==NULL){
		printf("Falha ao abrir o arquivo\n");
		return;
	}char header[20];
	sprintf(header,"P5\n%d %d\n255\n",img->width,img->height);
	int x;
	fwrite(header,strlen(header),1,fl);
	unsigned char uc;
	for(x=0;x<img->width*img->height;x++){
		uc = img->data[x];
		fwrite(&uc,1,1,fl);
	}fclose(fl);
}

image *loadPGM(char *filename){
	FILE *arq;
	if((arq = fopen(filename,"rb"))==NULL){
		printf("Erro ao abrir a foto %s\n", filename);
		return NULL;
	}
	image *img = (image*) malloc (sizeof(image));
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
			buf = 'x';
			while(buf != '\n'){
				buf = fgetc(arq);
			}
			break;
		}
	}
	img->data = (unsigned char*) malloc (img->width * img->height);
	fread(img->data, sizeof(char), img->width * img->height, arq);
	fclose(arq);
	return img;
}

image *clone_image(image *img){
	image *dst = (image *) malloc(sizeof(image));
	dst->data = (unsigned char*) malloc(img->width*img->height);
	dst->width = img->width;
	dst->height = img->height;
	int x;
	for(x=0;x<img->width*img->height;x++){
		dst->data[x] = img->data[x];
	}
	return dst;
}
