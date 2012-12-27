/*
Igor de Carvalho Coelho - 06/20068

Não tive tempo de fazer a tarefa completa então só fiz o calculo inical da matriz fundamental...
Uso o retorno da função do opencv como função de reta para desenhar

*/

#include <opencv/cv.h>
#include <opencv/highgui.h>

int main(){
	int i;
	CvMat *points1 = cvCreateMat(1,8,CV_32FC2);
	CvMat *points2 = cvCreateMat(1,8,CV_32FC2);
	CvMat *fundaMatrix = cvCreateMat(3,3,CV_32FC1);

	points1->data.fl[0] = 330;
	points1->data.fl[1] = 620;
	points1->data.fl[3] = 304;
	points1->data.fl[4] = 414;
	points1->data.fl[5] = 760;
	points1->data.fl[6] = 526;
	points1->data.fl[7] = 506;
	points1->data.fl[8] = 568;
	points1->data.fl[9] = 706;
	points1->data.fl[10] = 512;
	points1->data.fl[11] = 36;
	points1->data.fl[12] = 298;
	points1->data.fl[13] = 868;
	points1->data.fl[14] = 188;
	points1->data.fl[15] = 916;
	points1->data.fl[16] = 410;

	points2->data.fl[0] = 204;
	points2->data.fl[1] = 558;
	points2->data.fl[3] = 168;
	points2->data.fl[4] = 384;
	points2->data.fl[5] = 642;
	points2->data.fl[6] = 574;
	points2->data.fl[7] = 378;
	points2->data.fl[8] = 552;
	points2->data.fl[9] = 612;
	points2->data.fl[10] = 548;
	points2->data.fl[11] = 160;
	points2->data.fl[12] = 248;
	points2->data.fl[13] = 900;
	points2->data.fl[14] = 232;
	points2->data.fl[15] = 870;
	points2->data.fl[16] = 492;

	IplImage *img1 = cvLoadImage("mantle1.jpg",1);
	IplImage *img2 = cvLoadImage("mantle2.jpg",1);

	cvFindFundamentalMat(points1,points2,fundaMatrix,CV_FM_8POINT,1.0,0.99,NULL);

	CvMat *lines = cvCreateMat(15,3,CV_32FC1);
	CvMat *epipoints = cvCreateMat(1,15,CV_32FC2);
	for(i=0;i<8;i++){
		epipoints->data.fl[i*2] = points1->data.fl[i*2];
		epipoints->data.fl[i*2+1] = points1->data.fl[i*2+1];
	}
	for(i=8;i<15;i++){
		epipoints->data.fl[i*2] = points1->data.fl[(i-8)*2] + 20;	
		epipoints->data.fl[i*2+1] = points1->data.fl[(i-8)*2+1] + 20;
	}

	cvComputeCorrespondEpilines(epipoints, 2, fundaMatrix, lines);
	int x1,x2,y1,y2;
	for(i=0; i<15; i++){
		x1 = 0;
		x2 = img2->width;
		y1 = (-x1*lines->data.fl[i*3] + lines->data.fl[i*3+2])/lines->data.fl[i*3+1];
		y2 = (-x2*lines->data.fl[i*3] + lines->data.fl[i*3+2])/lines->data.fl[i*3+1];
		cvDrawLine(img2,cvPoint(x1,y1),cvPoint(x2,y2), CV_RGB(0,255,0), 3, CV_AA, 0);
	}
	cvNamedWindow("Epipolar",0);
	cvShowImage("Epipolar",img2);
	cvWaitKey(0);
}

