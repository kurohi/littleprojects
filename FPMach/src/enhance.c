#include <enhance.h>

#ifdef USEOPENCV
void show(image *img, int x, int y, int jx, int jy, char *winname){
	IplImage *imga = cvCreateImage(cvSize(jx,jy),IPL_DEPTH_8U,1);
	int xx,yy;
	for(yy=y; yy<y+jy; yy++)
		for(xx=x; xx<x+jx; xx++){
			CV_IMAGE_ELEM(imga,unsigned char,yy-y,xx-x) = img->data[xx+yy*img->width];
		}
	cvShowImage(winname,imga);
}

void showgraph(unsigned char *graph, int size, char *winame){
	IplImage *img = cvCreateImage(cvSize(size,256),IPL_DEPTH_8U,1);
	int x,i;
	for(i=0;i<img->height;i++)
		for(x=0;x<img->width;x++)
			CV_IMAGE_ELEM(img, unsigned char, i, x) = 0;
	for(x=0;x<size;x++){
		//printf("%d ",graph[x]);
		for(i=0;i<255-graph[x];i++)
			CV_IMAGE_ELEM(img, unsigned char, i, x) = 255;
	}
	cvShowImage(winame,img);
}

void showdirec(image *img, int win_x, int win_y, double *direc, char *winname){
	int x, y, x1, y1, x2, y2, k;
	double dirX, dirY;
	int count=0;
	for(y=1; (y+win_y)<=img->height; y+=win_y)
		for(x=1; (x+win_x)<=img->width; x+=win_x){
			dirY = sin(direc[count]+PI/2);	//pega o tanto que deve andar para o eixo Y
			dirX = cos(direc[count]+PI/2);	//e para o eixo x

			k = (win_x+win_y)/2;
			x1 = x2 = x+(win_x/2);
			y1 = y2 = y+(win_y/2);
			while((y1<y+win_y)&&(x1<x+win_x)&&(y1>=y)&&(x1>=x)){
				k++;
				img->data[x1+y1*img->width] = 255;	//cria metade da linha
				x1 = x2 + (k-(win_x+win_y)/2)*dirX;
				y1 = y2 + (k-(win_x+win_y)/2)*dirY;
			}

			k = (win_x+win_y)/2;
			x1 = x+(win_x/2);
			y1 = y+(win_y/2);
			while((y1<y+win_y)&&(x1<x+win_x)&&(y1>=y)&&(x1>=x)){
				k--;
				img->data[x1+y1*img->width] = 255;	//cria a outra metade
				x1 = x2 + (k-(win_x+win_y)/2)*dirX;
				y1 = y2 + (k-(win_x+win_y)/2)*dirY;
			}
			count++;
		}
	show(img,0,0,img->width,img->height,winname);
}
#endif
void segmentation(image *img, image *dst, int win_x, int win_y){
	int x,y,x1,y1;
	double mean,sd;
	for(y=0; (y+win_y)<img->height; y+=win_y)
		for(x=0; (x+win_x)<img->width; x+=win_x){
			mean = 0.0;
			for(y1=0; y1<win_y; y1++)
				for(x1=0; x1<win_x; x1++){
					mean += img->data[(x+x1)+(y+y1)*img->width];
				}
			mean /= win_y*win_x;
			sd = 0.0;
			for(y1=0; y1<win_y; y1++)
				for(x1=0; x1<win_x; x1++){
					sd += pow(img->data[(x+x1)+(y+y1)*img->width] - mean,2);
				}
			sd /= pow(win_x*win_y, 2);
			if(sd<SEGLIM){
				for(y1=0; y1<win_y; y1++)
					for(x1=0; x1<win_x; x1++){
						dst->data[(x+x1)+(y+y1)*dst->width] = 0;
					}
			}else{
				for(y1=0; y1<win_y; y1++)
					for(x1=0; x1<win_x; x1++){
						dst->data[(x+x1)+(y+y1)*dst->width] = dst->data[(x+x1)+(y+y1)*dst->width];
					}
			}
		}
}

void equalization(image *img){
	unsigned char aux[img->width*img->height];
	int histogram[256];
	unsigned int cdf[256];
	unsigned int cdfmin=0,cdftot=0;
	long total=0;
	int x,y;
	for(x=0;x<256;x++){
		histogram[x] = 0;
		cdf[x] = 0;
	}for(y=0;y<img->height;y++)
		for(x=0;x<img->width;x++){
			if(img->data[x+y*img->width]!=0){
				histogram[img->data[x+y*img->width]]++;
				total++;
			}
		}
	for(x=1;x<256;x++){
		cdftot += histogram[x];
		cdf[x] = cdftot;
	}
	x=0;
	while(cdfmin==0) cdfmin = cdf[x++];
	for(y=0;y<img->height;y++)
		for(x=0;x<img->width;x++){
			if(img->data[x+y*img->width]!=0){
				aux[x+y*img->width] = ((cdf[img->data[x+y*img->width]] - cdfmin) / (float)(total - cdfmin))*255;
			}
		}
	for(y=0;y<img->height;y++)
		for(x=0;x<img->width;x++){
			if(img->data[x+y*img->width]!=0){
				img->data[x+y*img->width] = aux[x+y*img->width];
			}
		}
}

void direction(image *img, double *direcs,int win_x, int win_y){
	int x,y,i,j,k,pos=0;
	int x2,y2;
	int acumX,acumY;
	char matrixY[9] =	{1,2,1,
				0,0,0,
				-1,-2,-1};
	char matrixX[9] =	{1,0,-1,
				2,0,-2,
				1,0,-1};

	double temp_direcX[win_x*win_y];
	double temp_direcY[win_x*win_y];
	double Vx,Vy;
	//divide em blocos
	for(y=1;(y+win_y)<=img->height;y+=win_y){
		for(x=1;(x+win_x)<=img->width;x+=win_x){
			if(img->data[x+y*img->width]+img->data[(x+1)+y*img->width]+img->data[x+(y+1)*img->width]+img->data[(x+1)+(y+1)*img->width] != 0){
				//aplica sobel em apenas um bloco
				for(y2=0; y2<win_y; y2++){
					for(x2=0; x2<win_x; x2++){
						k=0;
						acumX = 0;
						acumY = 0;
						for(i=(y2+y)-1; i<=(y2+y)+1; i++){
							for(j=(x2+x)-1; j<=(x2+x)+1; j++){
								acumX += img->data[j+i*img->width]*matrixX[k];
								acumY += img->data[j+i*img->width]*matrixY[k];
								k++;
							}
						}
						//guarda as derivadas
						temp_direcX[x2+y2*win_x] = acumX;
						temp_direcY[x2+y2*win_x] = acumY;
					}
				}
				// Aproximação usando o artigo
				Vx=0;
				Vy=0;
				for(i=0; i<win_x*win_y; i++){
					Vx += 2*temp_direcX[i]*temp_direcY[i];
					Vy += pow(temp_direcX[i],2) - pow(temp_direcY[i],2);
				}
				//direção estimada do bloco
				if(Vy>=0){
					direcs[pos] = 90;
				}else if(Vx>=0){
					direcs[pos] = 180;
				}else{
					direcs[pos] = 0;
				}
				if(Vy == 0) Vy+=1;
				direcs[pos] += (90/PI) * atan(Vx/Vy);
				direcs[pos++] *= PI/180;
				direcs[pos-1] += PI/2;
			}else{
				direcs[pos++] = 0.0;
			}
		}
	}
}

void makeGaussianMatrix(double *matrix, int width, int height){
	int  x,y;
	for(y=-height/2; y<height/2+1 ; y++){
		for(x=-width/2; x<width/2+1; x++){
			matrix[(x+width/2) + (y+height/2)*width] = (1/(2*PI*pow(GAUSSIGMA,2)))*exp(-1*((pow(x,2)+pow(y,2))/(2*pow(GAUSSIGMA,2))));
		}
	}
}

void gaussian(double *direcMatrix, int width, int height){
	double phix[width*height], phiy[width*height];
	double phixl, phiyl;
	double gauss[GAUSSSIZE*GAUSSSIZE];
	makeGaussianMatrix(gauss, GAUSSSIZE, GAUSSSIZE);
	int x,y,xl,yl;
	for(y=0; y<height; y++)
		for(x=0; x<width; x++){
			phix[x+y*width] = cos(2*direcMatrix[x+y*width]);
			phiy[x+y*width] = sin(2*direcMatrix[x+y*width]);
		}
	//convulação dos gausianos(vai os 2 de uma vez para ecoomizar)
	for(y=GAUSSSIZE/2; y<height-GAUSSSIZE/2; y++)
		for(x=GAUSSSIZE/2;x<width-GAUSSSIZE/2; x++){
			phixl = 0.0;
			phiyl = 0.0;
			for(yl=-GAUSSSIZE/2;yl<GAUSSSIZE/2;yl++)
				for(xl=-GAUSSSIZE/2; xl<GAUSSSIZE/2; xl++){
					phixl += gauss[(xl+GAUSSSIZE/2) + (yl+GAUSSSIZE/2)*GAUSSSIZE]*phix[(x+xl) + (y+yl)*width];
					phiyl += gauss[(xl+GAUSSSIZE/2) + (yl+GAUSSSIZE/2)*GAUSSSIZE]*phiy[(x+xl) + (y+yl)*width];
				}
			direcMatrix[x + y*width] = atan2(phiyl,phixl)/2;
		}
}

void frequency(image *img, double *direc, double *freq, int win_x, int win_y){
	int x, y, x1, y1, x2, y2, k;
	double dirX, dirY;
	unsigned char onda[win_x+win_y];
	unsigned char maior,menor;
	float mean[win_x+win_y];
	int distancia[5],picos;
	int count=0;
	for(y=1; (y+win_y)<=img->height; y+=win_y)
		for(x=1; (x+win_x)<=img->width; x+=win_x){
			dirY = sin(direc[count]+PI/2);
			dirX = cos(direc[count]+PI/2);
			memset(onda, 0, win_x+win_y);
			memset(mean, 0, win_x+win_y);

			k = (win_x+win_y)/2;
			x1 = x2 = x+(win_x/2);
			y1 = y2 = y+(win_y/2);
			while((y1<y+win_y)&&(x1<x+win_x)&&(y1>=y)&&(x1>=x)){
				onda[k++] = (unsigned char) img->data[x1+y1*img->width];
				x1 = x2 + (k-(win_x+win_y)/2)*dirX;
				y1 = y2 + (k-(win_x+win_y)/2)*dirY;
			}

			k = (win_x+win_y)/2;
			x1 = x+(win_x/2);
			y1 = y+(win_y/2);
			while((y1<y+win_y)&&(x1<x+win_x)&&(y1>=y)&&(x1>=x)){
				onda[k--] = (unsigned char) img->data[x1+y1*img->width];
				x1 = x2 + (k-(win_x+win_y)/2)*dirX;
				y1 = y2 + (k-(win_x+win_y)/2)*dirY;
			}
			maior = menor = 128;
			for(k=1;k<win_x+win_y-1;k++){
				mean[k] = (onda[k-1]+onda[k]+onda[k+1])/3;
				if(mean[k]>maior) maior = mean[k];
				if(mean[k]<menor) menor = mean[k];
			}
			if(maior-menor > FREQLIM){
				picos = 0;
				distancia[0] = distancia[1] = distancia[2] = distancia[3] = distancia[4] = 0;
				for(y1=0;y1<win_x+win_y;y1++){
					if(mean[y1]!=0.0) break;
				}
				for(; y1<win_x+win_y -2; y1++){
					distancia[picos]++;
					if((mean[y1-2] < mean[y1-1]) && 
					   (mean[y1-1] < mean[y1]) &&
					   (mean[y1] >= mean[y1+1]) &&
					   (mean[y1+1] > mean[y1+2])) picos++;
					if((mean[y1]==0) && (mean[y1+1]==0)) break;
				}
				if(picos == 0){
					freq[count] = (distancia[0]!=0)?1/(double)distancia[0]:0.0;
				}else{
					for(y1=0;y1<=picos;y1++){
						freq[count] += (distancia[y1]/(double)picos);
					}
					freq[count] = 1/freq[count];
				}
				if(freq[count] < FREQMIN){
					if(freq[count-1] > FREQMIN)
						freq[count] = freq[count-1];
					else if((count>win_x)&&(freq[count-win_x] > FREQMIN))
						freq[count] = freq[count-win_x];
				}
				if(freq[count] < FREQMAX){
					if(freq[count-1] < FREQMAX)
						freq[count] = freq[count-1];
					else if((count>win_x)&&(freq[count-win_x] < FREQMAX))
						freq[count] = freq[count-win_x];
				}
			}else{
				freq[count] = freq[count-1];
			}
			count++;
		}
}

void make_gabor_Matrix(double direc, double freq, int width, int height, double *gMatrix){
	double dot_x, dot_y;
	double x,y;
	int count=0;
	for(y=-height/2; y<height/2; y++)
		for(x=-width/2; x<width/2; x++){
			dot_x =  x*cos(direc) + y*sin(direc);
			dot_y = -x*sin(direc) + y*cos(direc);

			gMatrix[count++] = exp(-(dot_x*dot_x + GABGAMMA*GABGAMMA*dot_y*dot_y)/(2*GABSIGMA*GABSIGMA)) * cos(2*PI*dot_x*freq + GABPHASE);
		}
}

void gabor_filter(image *img, image *dst, double *direcs, double *freqs, int win_x, int win_y){
	int x,x1,x2,y,y1,y2,aux_conv;
	int i,end;
	double gMatrix[win_x*win_y];
	for(y=0;(y+win_y)<img->height;y+=win_y)
		for(x=0;(x+win_x)<img->width;x+=win_x){
			end = (x+1)/win_x + ((y+1)/win_y)*(img->width/win_x);
			if((direcs[end]!=0.0)&&(freqs[end]!=0.0)){
				make_gabor_Matrix(direcs[end],freqs[end],GABWIDTH,GABHEIGHT,gMatrix);
				for(y1=y;y1<y+win_y;y1++)
					for(x1=x;x1<x+win_x;x1++){
						aux_conv=0;
						i=0;
						for(y2=-GABHEIGHT/2;y2<GABHEIGHT/2;y2++)
							for(x2=-GABWIDTH/2;x2<GABWIDTH/2;x2++){
								if(((y1+y2)<0)||((y1+y2)>img->height)||((y1+y2)<0)||((y1+y2)>img->width)){
									i++;
								}else{
									aux_conv += gMatrix[i++] * img->data[(x1+x2)+(y1+y2)*img->width];
								}
							}
						aux_conv = (aux_conv>BINLIMIAR)?LINE:BACK;
						dst->data[x1+y1*dst->width] = aux_conv;
					}
			}else{
				for(y1=y;y1<y+win_y;y1++)
					for(x1=x;x1<x+win_x;x1++){
						dst->data[x1+y1*dst->width] = 0;
					}
			}
		}
}

void dilate(image *img){
	int x,y;
	for(y=1;y<img->height-1;y++)
		for(x=1;x<img->width-1;x++){
			if(img->data[x+y*img->width] == LINE){
				if(img->data[(x+1)+y*img->width] == BACK)img->data[(x+1)+y*img->width] = 2;
				if(img->data[(x-1)+y*img->width] == BACK)img->data[(x-1)+y*img->width] = 2;
				if(img->data[x+(y+1)*img->width] == BACK)img->data[x+(y+1)*img->width] = 2;
				if(img->data[x+(y-1)*img->width] == BACK)img->data[x+(y-1)*img->width] = 2;
			}
		}
	for(y=1;y<img->height-1;y++)
		for(x=1;x<img->width-1;x++){
			if(img->data[x+y*img->width] == 2) img->data[x+y*img->width] = LINE;
		}
}

void erode(image *img){
	int x,y;
	for(y=1;y<img->height-1;y++)
		for(x=1;x<img->width;x++){
			if(((img->data[x+y*img->width] == LINE)||(img->data[x+y*img->width] == 2))&&
			((img->data[(x+1)+y*img->width] == LINE)||(img->data[(x+1)+y*img->width] == 2))&&
			((img->data[(x-1)+y*img->width] == LINE)||(img->data[(x-1)+y*img->width] == 2))&&
			((img->data[x+(y+1)*img->width] == LINE)||(img->data[x+(y+1)*img->width] == 2))&&
			((img->data[x+(y-1)*img->width] == LINE)||(img->data[x+(y-1)*img->width] == 2))){
				img->data[x+y*img->width] = 2;
			}
		}
	for(y=1;y<img->height-1;y++)
		for(x=1;x<img->width;x++){
			if(img->data[x+y*img->width] == LINE) img->data[x+y*img->width] = BACK;
			else if(img->data[x+y*img->width] == 2) img->data[x+y*img->width] = LINE;
		}
}

void thinning(image *img){
	int x,y,np,sp,cond1,cond2,cond3=1;
	while(cond3==1){
		cond3=0;
		for(y=1;y<img->height-1;y++)
			for(x=1;x<img->width-1;x++){
				if(img->data[x+y*img->width] == LINE){
					np = 0;
					sp = 0;
					cond1 = 0;
					cond2 = 0;
					//P1
					if(img->data[(x-1)+(y-1)*img->width] == LINE){
						np++;
						if(img->data[(x-1)+(y-0)*img->width] == BACK){
							sp++;
						}
					}//P2
					if(img->data[(x-0)+(y-1)*img->width] == LINE){
						np++;
						if(img->data[(x-1)+(y-1)*img->width] == BACK){
							sp++;
						}
					}else{
						cond1 += 1;
					}//P3
					if(img->data[(x+1)+(y-1)*img->width] == LINE){
						np++;
						if(img->data[(x-0)+(y-1)*img->width] == BACK){
							sp++;
						}
					}//P4
					if(img->data[(x+1)+(y-0)*img->width] == LINE){
						np++;
						if(img->data[(x+1)+(y-1)*img->width] == BACK){
							sp++;
						}
					}else{
						cond1 += 1;
						cond2 += 1;
					}//P5
					if(img->data[(x+1)+(y+1)*img->width] == LINE){
						np++;
						if(img->data[(x+1)+(y-0)*img->width] == BACK){
							sp++;
						}
					}//P6
					if(img->data[(x-0)+(y+1)*img->width] == LINE){
						np++;
						if(img->data[(x+1)+(y+1)*img->width] == BACK){
							sp++;
						}
					}else{
						cond1 += 1;
						cond2 += 1;
					}//P7
					if(img->data[(x-1)+(y+1)*img->width] == LINE){
						np++;
						if(img->data[(x-0)+(y+1)*img->width] == BACK){
							sp++;
						}
					}//P8
					if(img->data[(x-1)+(y-0)*img->width] == LINE){
						np++;
						if(img->data[(x-1)+(y+1)*img->width] == BACK){
							sp++;
						}
					}else{
						cond2 += 1;
					}
					if((np>=2)&&(np<=6)&&(sp==1)&&(cond1>0)&&(cond2>0)){
						cond3=1;
						img->data[x+y*img->width] = BACK;
					}
				}
			}
	}
	unsigned char masks[4][9]={	{0,	255,	1,
					255,	255,	1,
					1,	1,	0},

					{1,	255,	0,
					1,	255,	255,
					0,	1,	1},
					
					{0,	1,	1,
					1,	255,	255,
					1,	255,	0},

					{1,	1,	0,
					255,	255,	1,
					0,	255,	1}};
	int xx,yy,k,i;
	for(y=1;y<img->height-1;y++)
		for(x=1;x<img->width;x++){
			for(i=0;i<4;i++){
				k=0;
				for(yy=0;yy<3;yy++)
					for(xx=0;xx<3;xx++){
						if((masks[i][xx+yy*3]!=1)&&(masks[i][xx+yy*3]==img->data[(x+xx-1)+(y+yy-1)*img->width])){
							k++;
						}
					}
				if(k==5){
					img->data[x+y*img->width]=0;
					break;
				}
			}
		}
}


static const int masks[] = { 0200, 0002, 0040, 0010 };

static const unsigned char delet[512] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};


void imagem_thin(image *img)
{
	int	x, y; 
	int	i; 
	int	pc = 0; 
	int	count = 1;
	int	p, q;
	unsigned char qb[img->width]; 
	unsigned char *imgbuf = img->data;

	qb[img->width - 1] = 0;	
	while ((pc<50)&&(count))
	{
		pc++;
		count = 0;

		for (i = 0 ; i < 4; i++)
		{
			int m = masks[i];

			p = imgbuf[0] != 0;
			for (x = 0; x < img->width - 1; x++)
				qb[x] = p = ((p << 1) & 0006) | (imgbuf[x + 1] != 0);

			for (y = 0; y < img->height - 1; y++)
			{
				q = qb[0];
				p = ((q<<3)&0110) | (imgbuf[(y + 1) * img->width] != 0);

				for (x = 0; x < img->width - 1; x++) {
					q = qb[x];
					p = ((p << 1) & 0666) | ((q << 3) & 0110) |
						(imgbuf[((y + 1) * img->width) + x + 1] != 0);
					qb[x] = p;
					if  (((p & m) == 0) && delet[p]) {
						count++;
						imgbuf[(y * img->width) +x] = 0;
					}
				}

				p = (p << 1) & 0666;
				if	((p & m) == 0 && delet[p])
				{
					count++;
					imgbuf[(y * img->width) + img->width - 1] = 0;
				}
			}

			for (x = 0 ; x < img->width; x++)
			{
				q = qb[x];
				p = ((p << 1) & 0666) | ((q << 3) & 0110);
				if	((p & m) == 0 && delet[p])
				{
					count++;
					imgbuf[((img->height - 1) * img->width) + x] = 0;
				}
			}
		}
	}
}

int testImage(image *img){
	long i,zero=0;
	for(i=0;i<img->width*img->height; i++){
		if(img->data[i] == 0) zero++;
	}
	if(((double)zero*100/(double)(img->width*img->height))>TEST_IMAGE){
		return FAILURE;
	}else{
		return SUCCESS;
	}
}


int enhance(image *img, int win_x, int win_y){
	image *dst = clone_image(img);
	//alocando os vetores localmente
	double direcs[(img->width/win_x) * (img->height/win_y)];
	double freqs[(img->width/win_x) * (img->height/win_y)];
	int i;
	//iniciando os vetores
	for(i=0;i<(img->width/win_x) * (img->height/win_y);i++){
		direcs[i] = freqs[i] = 0.0;
	}
	i=0;
//se o modo visualização estiver ligado
#ifdef USEOPENCV
	image *original = clone_image(img);
	segmentation(img,img,win_x,win_y);
	save_image(img,"segmentada.pgm");
	if(testImage(img)==FAILURE){
		return FAILURE;
	}
	image *segmentada = clone_image(img);
	equalization(img);
	image *equalizada = clone_image(img);
	direction(img,direcs,win_x,win_y);
	gaussian(direcs,img->width/win_x, img->height/win_y);
	image *direcional = clone_image(img);
	for(i=0;i<img->width*img->height;i++) direcional->data[i] = 0;
	frequency(img, direcs, freqs, win_x, win_y);
	gabor_filter(img, dst, direcs, freqs, win_x, win_y);
	//dilate(dst);
	//erode(dst);
	dilate(dst);
	erode(dst);
	dilate(dst);
	erode(dst);
	image *gabor = clone_image(dst);
	//thinning(dst);
	imagem_thin(dst);
	for(i=0;i<img->width*img->height;i++){
		img->data[i] = dst->data[i];
	}

	cvNamedWindow("Original",0);
	cvMoveWindow("Original",0,0);
	show(original,0,0,dst->width,dst->height,"Original");
	cvNamedWindow("Segmentada",0);
	cvMoveWindow("Segmentada",img->width+5,0);
	show(segmentada,0,0,dst->width,dst->height,"Segmentada");
	cvNamedWindow("Equalizada",0);
	cvMoveWindow("Equalizada",2*(img->width+5),0);
	show(equalizada,0,0,dst->width,dst->height,"Equalizada");
	cvNamedWindow("Direcional",0);
	cvMoveWindow("Direcional",0,img->height+10);
	showdirec(direcional,win_x,win_y,direcs,"Direcional");
	cvNamedWindow("Gabor",0);
	cvMoveWindow("Gabor",img->width+5,img->height+10);
	show(gabor,0,0,dst->width,dst->height,"Gabor");
	cvNamedWindow("Afinada",0);
	cvMoveWindow("Afinada",2*(img->width+5),img->height+10);
	show(dst,0,0,dst->width,dst->height,"Afinada");

	//cvWaitKey(0);
	//cvDestroyAllWindows();
#else
	segmentation(img,img,win_x,win_y);
	if(testImage(img)==FAILURE){
		return FAILURE;
	}
	equalization(img);
	direction(img,direcs,win_x,win_y);
	gaussian(direcs, img->width/win_x, img->height/win_y);
	frequency(img, direcs, freqs, win_x, win_y);
	gabor_filter(img, dst, direcs, freqs, win_x, win_y);
	dilate(dst);
	erode(dst);
	dilate(dst);
	erode(dst);
	dilate(dst);
	erode(dst);
	//thinning(dst);
	imagem_thin(dst);
	for(i=0;i<img->width*img->height; i++){
		img->data[i] = dst->data[i];
	}
	//descomente para ver o resultado
	//save_image(dst,"enhanced.pgm");
#endif
	free_image(dst);
	return SUCCESS;
}

