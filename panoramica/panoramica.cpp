#include<opencv/cv.h>
#include<opencv/highgui.h>
#include<stdio.h>
#include<stdlib.h>
int lclick,rclick,lmousex,lmousey,rmousex,rmousey;

void lmouse_callback(int event, int x, int y, int flags, void *param){
	if(event == CV_EVENT_LBUTTONDBLCLK){
		lclick = 1;
		lmousex=x;
		lmousey=y;	
	}
}

void rmouse_callback(int event, int x, int y, int flags, void *param){
	if(event == CV_EVENT_LBUTTONDBLCLK){
		rclick = 1;
		rmousex=x;
		rmousey=y;	
	}
}

int main(int argc, char **argv){
	int nfotos;
	printf("Digite o numero de fotos a ser utilizadas\n");
	scanf("%d",&nfotos);
	IplImage *(fotos[nfotos]);
	IplImage *fotofinal;
	int i,x,y,j;
	char filename[100];
	x=0;
	i=0;
	while(i!=nfotos){
		printf("Entre com a %d foto: ",i);
		scanf("%s",filename);
		fotos[i] = cvLoadImage(filename,1);
		x+=fotos[i]->width;
		i++;
	}
	y=fotos[0]->height*2;
	fotofinal = cvCreateImage(cvSize(x,y),IPL_DEPTH_8U,3);
	CvPoint2D32f corresp1[4];
	CvPoint2D32f corresp2[4];
	CvMat *homog = cvCreateMat(3,3,CV_32FC1);
	printf("Marque na imagem da esquerda e da direita 4 pontos em comum\n");
	cvNamedWindow("Left",0);
	cvNamedWindow("Right",0);
	cvSetMouseCallback("Left",lmouse_callback,NULL);
	cvSetMouseCallback("Right",rmouse_callback,NULL);
	int rnpoints, lnpoints;
	rnpoints=lnpoints=0;
	IplImage *right;
	fotofinal = cvCreateImage(cvGetSize(fotofinal),IPL_DEPTH_8U,3);
	right = cvCreateImage(cvGetSize(fotos[0]),IPL_DEPTH_8U,3);
	unsigned char *ptr,*ptr2;
	for(y=0;y<fotos[0]->height;y++){
		ptr = (unsigned char*) (fotos[0]->imageData + y*fotos[0]->widthStep);
		ptr2 = (unsigned char*) (fotofinal->imageData + (y+fotos[0]->height/2)*fotofinal->widthStep);
		for(x=0;x<fotos[0]->width;x++){
			for(i=0;i<3;i++){
				ptr2[3*(x+(fotofinal->width/2)-(fotos[0]->width/2))+i] = ptr[3*x+i];
			}
		}
	}
	cvResizeWindow("Left",fotofinal->width,fotofinal->height);
	right = cvCloneImage(fotos[1]);
	cvResizeWindow("Right",right->width/2,right->height/2);
	IplImage *fotoaux = cvCreateImage(cvGetSize(fotofinal),IPL_DEPTH_8U,3);
	for(i=0;i<nfotos-1;){
		if(lclick==1){
			if(lnpoints<4){
				corresp1[lnpoints].x=lmousex;
				corresp1[lnpoints].y=lmousey;
				cvCircle(fotofinal, cvPoint(lmousex,lmousey), 2, CV_RGB(0,0,255), 3);
				lnpoints++;
			}
			lclick=0;
		}
		if(rclick==1){
			if(rnpoints<4){
				corresp2[rnpoints].x=rmousex;
				corresp2[rnpoints].y=rmousey;
				cvCircle(right, cvPoint(rmousex,rmousey), 2, CV_RGB(0,0,255), 3);
				rnpoints++;
			}
			rclick=0;
		}
		if((rnpoints==4)&&(lnpoints==4)){
			cvGetPerspectiveTransform(corresp2,corresp1,homog);
			cvWarpPerspective(right,fotoaux,homog,0);
			for(y=0;y<fotofinal->height;y++){
				ptr = (unsigned char*) (fotoaux->imageData + y*fotoaux->widthStep);
				ptr2 = (unsigned char*) (fotofinal->imageData + y*fotofinal->widthStep);
				for(x=0;x<fotofinal->width;x++){
						if((ptr[3*x+0]!=0)||(ptr[3*x+1]!=0)||(ptr[3*x+2]!=0)){
							ptr2[3*x+0] = ptr[3*x+0];
							ptr2[3*x+1] = ptr[3*x+1];
							ptr2[3*x+2] = ptr[3*x+2];
						}
				}
			}
			rnpoints=0;
			lnpoints=0;
			i++;
			right = cvCloneImage(fotos[(i+1)%nfotos]);
		}
		cvShowImage("Left",fotofinal);
		cvShowImage("Right",right);
		filename[0] = cvWaitKey(50);
		if(filename[0]==27)return 0;
	}
	cvSaveImage("saida.jpg",fotofinal);
}
