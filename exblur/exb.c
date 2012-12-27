#include <stdio.h>
#include <stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#define LIMITE 50.0f
#define BLURTIME 1
#define BLURTAM 1

void gray(const IplImage *dst, IplImage *src){
	int x,y,i,sum;
	for(y=1;y<src->height-1;y++){
		for(x=1;x<src->width-1;x++){
			sum=0;
			for(i=0;i<3;i++){
				sum += (uchar)(src->imageData + y*src->widthStep)[x*src->nChannels + i];
			}sum/=3;
			CV_IMAGE_ELEM(dst,uchar,y,x) = sum;
		}
	}
}


uchar max(uchar n1,uchar n2,uchar n3){
	if((n1>n2)&&(n1>n3))	return n1;
	if((n2>n1)&&(n2>n3))	return n2;
	if((n3>n1)&&(n3>n2))	return n3;
	return n1;
}
uchar min(uchar n1,uchar n2,uchar n3){
	if((n1<n2)&&(n1<n3))	return n1;
	if((n2<n1)&&(n2<n3))	return n2;
	if((n3<n1)&&(n3<n2))	return n3;
	return n1;
}
int absss(int yup){
	return (yup>0)?yup:-yup;
}

int testePele(int r, int g, int b){
	if((r>95)&&(g>40)&&(b>20)){
		if((max(r,g,b)-min(r,b,g))>15){
			if(absss(r-g)>15){
				if((r>g)&&(r>b)){
					return 1;
				}
			}
		}
	}
	return 0;
}

void segpele(uchar *matrix, IplImage *src){
	int x,y,r,g,b;
	for(y=1;y<src->height-1;y++){
		for(x=1;x<src->width-1;x++){
			b = (int)(src->imageData + y*src->widthStep)[x*src->nChannels + 0];
			g = (int)(src->imageData + y*src->widthStep)[x*src->nChannels + 1];
			r = (int)(src->imageData + y*src->widthStep)[x*src->nChannels + 2];
			if(testePele(r,g,b)){
				matrix[x+y*src->width] = 1;
			}else{
				matrix[x+y*src->width] = 0;
			}
		}
	}
}

void remakeMedia(double *media,uchar *tempo, IplImage *dst, IplImage *old){
	int x,y,i;
	double val;
	for(y=0; y<old->height; y++){
		for(x=0; x<old->width; x++){
			val = media[x+(y*old->width)] - (double) CV_IMAGE_ELEM(old,uchar,y,x);
			val = val * val;
			media[x+(y*old->width)] += (double) CV_IMAGE_ELEM(old,uchar,y,x);
			media[x+(y*old->width)] /= 2;
			if(val<=LIMITE){
				if(tempo[x+y*old->width]==0){
					CV_IMAGE_ELEM(dst,uchar,y,x) = 255;
				}else{
					tempo[x+y*old->width]--;
				}
			}else{
				CV_IMAGE_ELEM(dst,uchar,y,x) = 0;
				tempo[x+y*old->width]=BLURTAM;
			}
		}
	}
}


void makeblur(uchar *matrix,IplImage *dst, IplImage *src){
	int x,y;
	for(y=0; y<src->height; y++){
		for(x=0; x<src->width; x++){
			if((CV_IMAGE_ELEM(src,uchar,y,x)==0)&&(matrix[x+y*src->width]==0)){
				(dst->imageData + y*dst->widthStep)[x*dst->nChannels + 0]=255;
				(dst->imageData + y*dst->widthStep)[x*dst->nChannels + 1]+=10;
				(dst->imageData + y*dst->widthStep)[x*dst->nChannels + 2]+=10;
			}
		}
	}
}

int main(int argc, char **argv){
	CvCapture *captura;
	IplImage *orig,*dst,*old,*origg;
	
	int x;
	captura = cvCaptureFromCAM(0);
	if( !captura ){
		printf("Could not initialize capturing. Exiting...\n");
		return -1;
	}
	//cvSetCaptureProperty(captura, CV_CAP_PROP_FRAME_WIDTH,620);
	//cvSetCaptureProperty(captura, CV_CAP_PROP_FRAME_HEIGHT,320); 
	orig = cvQueryFrame( captura );
	CvSize size = cvSize(orig->width,orig->height);

	double media[orig->width*orig->height];
	uchar matrix[orig->width*orig->height];
	uchar tempo[orig->width*orig->height];
	char flag=0;
	for(x=0; x<orig->width*orig->height; x++){
		media[x] = 128.0f;
		matrix[x] = 1;
	}

	dst = cvCreateImage(size,IPL_DEPTH_8U,1);	
	origg = cvCreateImage(size,IPL_DEPTH_8U,1);

	cvNamedWindow( "Webcam", 1 );
	//cvNamedWindow( "Resultado", 1 );
	while(1){
		orig = cvQueryFrame( captura );
		segpele(matrix, orig);
		gray(origg,orig);
		if(flag == 0)
			remakeMedia(media, tempo, dst, origg);
		makeblur(matrix,orig,dst);
		int c = cvWaitKey(10);
       	if( (char)c == 27 ) break;
		//flag = (flag+1)%BLURTIME;		//comenta isso para ter um blur continuo
		cvShowImage( "Webcam", orig);
		//cvShowImage( "Resultado", dst);
		
	}
	cvReleaseCapture( &captura ); 
	cvReleaseImage(&orig);   
	cvReleaseImage(&origg);
	cvReleaseImage(&dst);  
	cvDestroyAllWindows();
	return 0;
}
