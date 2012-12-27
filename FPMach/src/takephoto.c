#include<image.h>
#include<enhance.h>
#include<matching.h>
#include<minutia.h>

void convert(IplImage **img, image *fp){
	*img = cvCreateImage(cvSize(fp->width,fp->height),IPL_DEPTH_8U,1);
	int x,y;
	for(y=0;y<fp->height;y++)
		for(x=0;x<fp->width;x++){
			CV_IMAGE_ELEM(*img,unsigned char,y,x) = fp->data[x+y*fp->width];
		}
}

int main(int argc, char ** argv){
	char saida = 's';
	char aux;
	char *ptaux;
	char name[10],straux[50],address[20]="../photos/";
	char handaddress[20] = "../photos/hand0.jpg";
	image *img=NULL;
	image *iaux;
	minutia *mins;
	double *direcs=NULL;
	int count;
	cvNamedWindow("Dedo",0);
	cvNamedWindow("Impressao",0);
	cvNamedWindow("Melhorada",0);
	cvMoveWindow("Dedo",0,50);
	cvMoveWindow("Impressao",400,50);
	cvMoveWindow("Melhorada",0,400);
	IplImage *hand,*finger;
	while(saida == 's'){
		printf("Entre com o nome do unuário: ");
		scanf("%s",name);
		printf("Coleque o dedo indicado assim que a luz vermelha acender no aparelho\n");
		printf("A imagem adiquirida aparecerá na tela ao lado, se estiver OK tecle s se não tecle n e uma nova aquisição será feita\n");
		printf("Para começar aperte qualquer tecla\n");
		cvWaitKey(0);
		strcat(address,name);
		strcat(address,"/");
		straux[0]=0;
		strcat(straux,"mkdir ");
		strcat(straux,address);
		system(straux);
		strcat(address,"finger");
		strcat(address,"0.pgm");
		for(count=0;count<10;count++){
			hand = cvLoadImage(handaddress,0);
			cvShowImage("Impressao",hand);
			cvWaitKey(50);
			do{
				free_image(img);
				do{
					img = get_image();
				}while(img==NULL);
				convert(&finger,img);
				iaux = clone_image(img);
				if(direcs == NULL){
					direcs = (double*) malloc(sizeof(double)*img->width/10*img->height/10);
				}
				direction(img,direcs,10,10);
				enhance(iaux,10,10);
				mins = findMinutia(iaux,direcs,10,10);
				mins = posprocessing(iaux,mins,10,10);
				mins = crownCore(img,mins);
				getDistances(mins);
				orderMinutia(mins);
				showMinutia(iaux,mins,10,10);
				cvShowImage("Dedo",finger);
				aux = cvWaitKey(0);
				cvReleaseImage(&finger);
			}while(aux!='s');
			save_image(img,address);
			ptaux = strstr(address,"finger");
			ptaux[6] = count+'1';
			ptaux = strstr(handaddress,"hand");
			ptaux[4] = count+'1';
		}
		printf("Colocar mais usuarios? s/n\n");
		saida = cvWaitKey(0);
	}
	return 0;
}
