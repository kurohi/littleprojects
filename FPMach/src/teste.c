#include <image.h>
#include <enhance.h>
#include <minutia.h>
#include <matching.h>
#include <crypto.h>
#include <sys/time.h>

void showMinutiaEX(image *img, minutia *root,int win_x, int win_y, int freq){
	IplImage *fp = cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_8U,3);
	int x,y,i;
	unsigned char *ptr;
	for(y=0;y<img->height;y++){
		ptr = (unsigned char*) (fp->imageData + y*fp->widthStep);
		for(x=0;x<img->width;x++){
			for(i=0;i<3;i++)
				ptr[3*x+i] = img->data[x+y*img->width];
		}
	}
	int k,x1,y1;
	double dirX,dirY;
	i=0;
	while(root!=NULL){
			if(root->mincount >= freq){
				i++;
				ptr = (unsigned char*) (fp->imageData + root->y*fp->widthStep);
				if(root->type==0){
					ptr[3*root->x+0]=0;
					ptr[3*root->x+1]=255;
					ptr[3*root->x+2]=0;
				}else if(root->type == 1){
					ptr[3*root->x+0]=255;
					ptr[3*root->x+1]=0;
					ptr[3*root->x+2]=0;
			
				}else if((root->type|2) !=0 ){
					for(y=0;y<6;y++){
						ptr = (unsigned char*) (fp->imageData + (root->y+y)*fp->widthStep);
						ptr[3*root->x+0]=255;
						ptr[3*root->x+1]=0;
						ptr[3*root->x+2]=0;
						ptr[3*(root->x+1)+0]=255;
						ptr[3*(root->x+1)+1]=0;
						ptr[3*(root->x+1)+2]=0;
						ptr[3*(root->x-1)+0]=255;
						ptr[3*(root->x-1)+1]=0;
						ptr[3*(root->x-1)+2]=0;
					}
				}
				dirY = sin(root->angle+PI/2);	//pega o tanto que deve andar para o eixo Y
				dirX = cos(root->angle+PI/2);	//e para o eixo x
				k = (win_x+win_y)/2;
				x = x1 = root->x;
				y = y1 = root->y;
				while((y<root->y+win_y)&&(x<root->x+win_x)&&(y>=root->y-win_y)&&(x>=root->x-win_x)){
					k++;
					ptr = (unsigned char*) (fp->imageData + y*fp->widthStep);
					ptr[3*x+2]=255;
					x = x1 + (k-(win_x+win_y)/2)*dirX;
					y = y1 + (k-(win_x+win_y)/2)*dirY;
				}
		}
		root=root->next;
	}
	cvNamedWindow("Minutia",0);
	cvShowImage("Minutia",fp);
	//cvWaitKey(0);
	//cvDestroyAllWindows();
	//cvReleaseImage(&fp);
}


int maain(int argc, char **argv){
	struct timeval beg,end;
	image *template,*teste, *aux;
	minutia *temp,*test, *maux;
	char menu=0;
	double *tempDirecs=NULL, *testDirecs=NULL;
	int penality,nminutia,nminutia2;
	cvNamedWindow("Template",0);
	cvNamedWindow("Teste",0);
	while(menu!='q'){
		menu = cvWaitKey(0);
		if(menu=='e'){
			template = get_image();
			if(template == NULL) continue;
			show(template,0,0,template->width,template->height,"Template");
			if(tempDirecs == NULL){
				tempDirecs = (double*) malloc(sizeof(double)*template->width/10*template->height/10);
			}
			aux = clone_image(template);
			direction(template,tempDirecs,10,10);
			if(enhance(template,10,10)==FAILURE){
				printf("Erro, foto muito borrada\n");
				continue;
			}
			temp = findMinutia(template,tempDirecs,10,10);
			temp = posprocessing(template,temp,20,20);
			temp = crownCore(aux,temp);
			getDistances(temp);
			orderMinutia(temp);
			maux = temp;
			nminutia = 0;
			while(maux!=NULL){
				nminutia++;
				maux = maux->next;
			}
			showMinutia(template,temp,10,10);
			maux = temp;
			penality = 5;
			while((maux!=NULL)&&(penality>0)){
				printf("%ld\t|\t%lf\n", decayLong(maux->distance), maux->angle);
				maux = maux->next;
				penality--;
			}printf("\n");
		}else if(menu=='r'){
			teste = get_image();
			if(teste == NULL) continue;
			show(teste,0,0,teste->width,teste->height,"Teste");
			if(testDirecs == NULL){
				testDirecs = (double*) malloc(sizeof(double)*teste->width/10*teste->height/10);
			}
			aux = clone_image(teste);
			direction(teste,testDirecs,10,10);
			if(enhance(teste,10,10)==FAILURE){
				printf("Erro, foto muito borrada\n");
				continue;
			}
			test = findMinutia(teste,testDirecs,10,10);
			test = posprocessing(teste,test,20,20);
			test = crownCore(aux,test);
			getDistances(test);
			orderMinutia(test);
			showMinutia(teste,test,10,10);
		}else if(menu == 't'){
			gettimeofday(&beg,NULL);
			penality = validadeFP(temp,test);
			if(penality == FINGER_MATCH){
				printf("MATCHED!!!\n");
			}else{
				printf("NOT MATCHED...\n");
			}
			gettimeofday(&end,NULL);
			printf("Tempo de execução: %lf\n", (double)(end.tv_sec-beg.tv_sec)+(double)(end.tv_usec-beg.tv_usec)/1000000.0);
		}else if(menu == 's'){
			gettimeofday(&beg,NULL);
			if(spiralMatching(temp, test, 0, teste->width, teste->height)==FINGER_MATCH){
				printf("MATCHED!!!\n");
			}else{
				printf("NOT MATCHED....\n");
			}
			gettimeofday(&end,NULL);
			printf("Tempo de execução: %lf\n", (double)(end.tv_sec-beg.tv_sec)+(double)(end.tv_usec-beg.tv_usec)/1000000.0);
		}else if(menu == 'd'){
			freeMinutiaList(temp);
			freeMinutiaList(test);
			free_image(template);
			free_image(teste);
			temp = test = NULL;
			template = teste = NULL;
		}else if(menu == 'c'){
			RSAKey publicKey,privateKey;
			generateKeys(&publicKey,&privateKey,temp);
			saveKeys(publicKey,privateKey,"../crypto/igor");
			encript(privateKey,"../crypto/teste.txt","../crypto/teste.crypt");
			encript(publicKey,"../crypto/teste.crypt","../crypto/teste.uncrypt");
			
		}else if(menu == 'h'){
			int k;
			char p;
			for (k=0;k<5;){
				teste = get_image();
				if(teste == NULL) continue;
				show(teste,0,0,teste->width,teste->height,"Teste");
				if(testDirecs == NULL){
					testDirecs = (double*) malloc(sizeof(double)*teste->width/10*teste->height/10);
				}
				direction(teste,testDirecs,10,10);
				if(enhance(teste,10,10)==FAILURE){
					printf("Erro, foto muito borrada\n");
					continue;
				}cvWaitKey(50);
				test = findMinutia(teste,testDirecs,10,10);
				test = posprocessing(teste,test,20,20);
				showMinutia(teste,test,10,10);
				p = cvWaitKey(0);
				if(p=='n') continue;
				if(spiralMatching(temp,test,SPIN_MATCH,template->width,template->height)==SUCCESS){
					getDistances(temp);
					orderMinutia(temp);
					printf("MATCHED\n");
					k++;
				}else{
					printf("NOT MATCHED\n");
				}
			}
			maux = temp;
			nminutia = 0;
			while(maux!=NULL){
				if(maux->mincount>nminutia) nminutia = maux->mincount;
				maux = maux->next;
			}
			penality = 10;
			while(penality>0){
				maux = temp;
				minutiaFilter(template, temp, sqrt(MATCH_RAY), nminutia);
				while((maux!=NULL)&&(penality>0)){
					if(maux->mincount >= nminutia){
						printf("%ld\t|\t%lf\t%d\n", decayLong(maux->distance), maux->angle,maux->mincount);
						maux->mincount = -1000;
						penality--;
					}
					maux = maux->next;
				}
				nminutia--;
				if(nminutia < -1000) break;
			}
			showMinutiaEX(aux,temp,10,10,nminutia);			
		}
	}
	return 0;
}
