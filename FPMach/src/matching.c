#include <matching.h>

#ifdef USEOPENCV
void showCore(image *img, minutia *delta){
	int x,y;
	IplImage *output = cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_8U,1);
	for(y=0;y<img->height;y++)
		for(x=0;x<img->width;x++){
			CV_IMAGE_ELEM(output, unsigned char, y,x) = img->data[x+y*img->width];
		}
	for(x=-10;x<10;x++){
		CV_IMAGE_ELEM(output,unsigned char, delta->y-10,delta->x+x) = 255;
		CV_IMAGE_ELEM(output,unsigned char, delta->y+10,delta->x+x) = 255;
	}
	for(y=-10; y<10;y++){
		CV_IMAGE_ELEM(output,unsigned char, delta->y+y,delta->x-10) = 255;
		CV_IMAGE_ELEM(output,unsigned char, delta->y+y,delta->x+10) = 255;
	}
	cvNamedWindow("Delta",0);
	cvShowImage("Delta",output);
	cvWaitKey(0);
	cvDestroyAllWindows();
	cvReleaseImage(&output);
}
#endif

double mod(double num){
	return (num<0.0)?num*-1.0:num;
}

void direction_round(double *direcs, double *realdirecs, int size){
	int i=0;
	while(i!=size){
		if(realdirecs[i]>PI){
			direcs[i] = realdirecs[i]-PI;
		}
		if(realdirecs[i]<PI/8){
			direcs[i] = 0.0;
		}else if(realdirecs[i]<(PI/4 + PI/8)){
			direcs[i] = 1.0;
		}else if(realdirecs[i]<(PI/2 + PI/8)){
			direcs[i] = 2.0;
		}else{
			direcs[i] = 3.0;
		}
		i++;
	}
}

minutia *crownCoreTeste(image *img, minutia *root){
	double A[img->width * img->height/(5*5)];
	memset(A,0,sizeof(double)*img->width * img->height/(5*5));
	int x,y,x1,y1;
	double sum, greater = 0.0;
	int matrix[81] = {	-1,-1,-1,-1, 0, 1, 1, 1, 1,
				-1,-1,-1,-1, 0, 1, 1, 1, 1,
				-1,-1,-1,-1, 0, 1, 1, 1, 1,
				-1,-1,-1,-1, 0, 1, 1, 1, 1,
				 0, 0, 0, 0, 0, 0, 0, 0, 0,
				 1, 1, 1, 1, 0,-1,-1,-1,-1,
				 1, 1, 1, 1, 0,-1,-1,-1,-1,
				 1, 1, 1, 1, 0,-1,-1,-1,-1,
				 1, 1, 1, 1, 0,-1,-1,-1,-1 };
	double direcs[img->width*img->height/(5*5)];
	direction(img,direcs,5,5);
	//direction_round(direcs, img->width*img->height/(5*5));
	for(y=4; y<img->height/5-4; y++)
		for(x=4; x<img->width/5-4;x++){
			sum = 0.0;
			for(y1=0;y1<9;y1++)
				for(x1=0;x1<9;x1++){
					sum += matrix[x1+y1*9]*direcs[(x+x1-4)+(y+y1-4)*img->width/5];
				}
			A[x+y*img->width/5] = sum;
			if(sum>greater){
				greater = sum;
			}
		}
	for(y=0; y<img->height/5; y++)
		for(x=0; x<img->width/5;x++){
			A[x+y*img->width/5] /= greater;
		}
	char c='s';
	double th=0.34;
	double dist,distmax,distmin;
	int dx,dy;
	image *aux = (image*)malloc(sizeof(image));
	aux->width = img->width;
	aux->height = img->height;
	aux->data = (unsigned char*) malloc (aux->width*aux->height);
	memset(aux->data, 0, img->width*img->height);
	while(c != 'q'){
		for(y=0; y<img->height/5; y++)
			for(x=0; x<img->width/5;x++){
				if(A[x+y*img->width/5]>th){
					aux->data[x*5+y*5*aux->width] = 255;
				}
			}
		distmax = 0.0;
		for(y=CORECENTER_Y; y<img->height-CORECENTER_Y; y++)
			for(x=CORECENTER_X; x<img->width-CORECENTER_X;x++){
				if(aux->data[x+y*aux->width] == 255){
					distmin = 999.0;
					for(y1=CORECENTER_Y;y1<img->height-CORECENTER_Y;y1++)
						for(x1=CORECENTER_X;x1<img->width-CORECENTER_X;x1++){
							if(((x!=x1)||(y!=y1))&&(aux->data[x1+y1*img->width]==255)){
								dist=sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
								if(dist<distmin){
									distmin = dist;
								}
							}
						}
					if(distmin>distmax){
						distmax = distmin;
						dx = x;
						dy = y;
					}
				}
			}		
		memset(aux->data, 0, img->width*img->height);
		aux->data[dx+dy*aux->width] = 255;
		show(aux,0,0,aux->width,aux->height,"Teste");
		c = cvWaitKey(0);
		if(c=='w') th += 0.01;
		if(c=='s') th -= 0.01;
		if(c=='p') printf("th= %lf\n",th);
	}
	return root;
}

minutia *crownCore(image *img, minutia *root){

	int x,y,x1,y1,x2,y2,k;
	int size = 10;
	double distance, distanceaux;
	double sigk,poincare,poincareaux,theta;
	double direcs[(img->width/size)*(img->height/size)];
	double realdirecs[(img->width/size)*(img->height/size)];
	double d[8*CORERAY];
	double d2[8*(CORERAY+1)];
	int A[(img->width/size)*(img->height/size)];
	memset(A,0,(img->width/size)*(img->height/size)*sizeof(int));
	direction(img, realdirecs, size,size);
	gaussian(realdirecs, img->width/size, img->height/size);
	direction_round(direcs,realdirecs ,(img->width/size)*(img->height/size));	


	for(y=CORECENTER_Y/size;y<(img->height-CORECENTER_Y)/size; y++)
		for(x=CORECENTER_X/size;x<(img->width-CORECENTER_X)/size; x++){
			poincare = 0.0;
			if((img->data[x+y*img->width]==0)&&
				(img->data[(x+1)+y*img->width]==0)&&
				(img->data[(x+1)+(y-1)*img->width]==0)&&
				(img->data[x+(y-1)*img->width]==0)){
				continue;
			}
			k=0;
			for(theta=0;theta<2*PI;theta+=(2*PI)/(8*CORERAY)){
				d[k++] = direcs[(x+(int)((double)CORERAY*(double)cos(theta)))+(y+(int)((double)CORERAY*(double)sin(theta)))*img->width/size];
			}
			for(k=0;k<8*CORERAY;k++){
				sigk = d[(k+1)%(8*CORERAY)] - d[k];
				if(abs(sigk)<=PI/2){
					poincare += sigk;
				}else if(sigk<=-PI/2){
					poincare += PI + sigk;
				}else{
					poincare += PI - sigk;
				}
			}
			poincare /= 2.0*PI;
			if(poincare==0.5){
				A[(x+3)+y*img->width/size] = 1;
			}else if(poincare==-0.5){
				A[x+y*img->width/size] = 2;
			}
		}
	for(y=CORECENTER_Y/size;y<(img->height-CORECENTER_Y)/size; y++)
		for(x=CORECENTER_X/size;x<(img->height-CORECENTER_X)/size; x++){
			if(A[x+y*img->width/size]==1){
				k=0;
				for(theta=0;theta<2*PI;theta+=(2*PI)/(8*CORERAY)){
					d[k++] = direcs[(x+(int)((double)CORERAY*(double)cos(theta)))+(y+(int)((double)CORERAY*(double)sin(theta)))*img->width/size];
				}
				k=0;
				for(theta=0;theta<2*PI;theta+=(2*PI)/(8*(CORERAY+1))){
					d2[k++] = direcs[(x+(int)((double)(CORERAY+1)*(double)cos(theta)))+(y+(int)((double)(CORERAY+1)*(double)sin(theta)))*img->width/size];
				}
				poincare = poincareaux = 0.0;
				for(k=0;k<8*CORERAY;k++){
					sigk = d[(k+1)%(8*CORERAY)] - d[k];
					if(abs(sigk)<=PI/2){
						poincare += sigk;
					}else if(sigk<=-PI/2){
						poincare += PI + sigk;
					}else{
						poincare += PI - sigk;
					}
				}
				for(k=0;k<8*(CORERAY+1);k++){
					sigk = d2[(k+1)%(8*(CORERAY+1))] - d2[k];
					if(abs(sigk)<=PI/2){
						poincareaux += sigk;
					}else if(sigk<=-PI/2){
						poincareaux += PI + sigk;
					}else{
						poincareaux += PI - sigk;
					}
				}
				if(poincare != poincareaux){
					A[x+y*img->width/size]=0;
				}
			}
		}

	for(y=CORECENTER_Y/size;y<(img->height-CORECENTER_Y)/size; y++)
		for(x=CORECENTER_X/size;x<(img->width-CORECENTER_X)/size; x++){
			if(A[x+y*img->width/size]==1){
				k=0;
				for(y1=-1;y1<=1;y1++)
					for(x1=-1;x1<=1;x1++){
						if(A[(x+x1)+(y+y1)*img->width/size]==1){
							k++;
						}
					}
				if(k>1) A[x+y*img->width/size] = 3;
			}else if(A[x+y*img->width/size]==2){
				k=0;
				for(y1=-1;y1<=1;y1++)
					for(x1=-1;x1<=1;x1++){
						if(A[(x+x1)+(y+y1)*img->width/size]==2){
							k++;
						}
					}
				if(k>1) A[x+y*img->width/size] = 4;
			}
		}
	distance = 999999.0;
	x2=y2=0;
	for(y=CORECENTER_Y/size;y<(img->height-CORECENTER_Y)/size; y++)
		for(x=CORECENTER_X/size;x<(img->width-CORECENTER_X)/size; x++){
			if(A[x+y*img->width/size]==3){
				distanceaux = 0.0;
				for(y1=CORECENTER_Y/size;y1<(img->height-CORECENTER_Y)/size; y1++)
					for(x1=CORECENTER_X/size;x1<(img->width-CORECENTER_X)/size; x1++){
						if(A[x1+y1*img->width/size]==3){
							distanceaux += sqrt(pow(x-x1,2)+pow(y-y1,2));
						}
					}
				if(distanceaux < distance){
					distance = distanceaux;
					x2 = x;
					y2 = y;
				}
			}
		}
	A[x2+y2*img->width/size] = 5;

	minutia auxm;
	distance=999999.0;
	for(y=CORECENTER_Y/size;y<(img->height-CORECENTER_Y)/size; y++)
		for(x=CORECENTER_X/size;x<(img->width-CORECENTER_X)/size; x++){
			if(A[x+y*img->width/size]==5){
				distanceaux = pow((x*size)-(img->width/2),2);
				distanceaux += pow((y*size)-(img->height/2),2);
				distanceaux = sqrt(distanceaux);
				if(distanceaux<distance){
					distance = distanceaux;
					auxm.x = x*size;
					auxm.y = y*size;
				}
			}
		}
	minutia *aux2 = root;
	minutia *aux3;
	/*aux3 = (minutia*) malloc (sizeof(minutia));
	aux3->x = auxm.x;
	aux3->y = auxm.y;
	aux3->type = 3;
	aux3->next = root;*/

	distance = 99999999.0;
	while(aux2 != NULL){
		distanceaux = (aux2->x-auxm.x)*(aux2->x-auxm.x);
		distanceaux += (aux2->y-auxm.y)*(aux2->y-auxm.y);
		if(distanceaux<distance){
			distance = distanceaux;
			aux3 = aux2;
		}
		aux2 = aux2->next;
	}
	if(aux3==root)return aux3;
	aux2 = root;
	while(aux2->next!=aux3) aux2 = aux2->next;
	aux2->next = aux3->next;
	aux3->next = root;
	aux3->type |= CORE;
	aux3->distance = 0;
	return aux3;
}

void getDistances(minutia *root){
	minutia *core = root;
	root = root->next;
	while(root!=NULL){
		root->distance = (root->x - core->x)*(root->x - core->x);
		root->distance += (root->y - core->y)*(root->y - core->y);
		root = root->next;
	}
}

static int compMinutia(const void *pt1, const void *pt2){
	minutia **min1 = (minutia**) pt1;
	minutia **min2 = (minutia**) pt2;
	return (*min1)->distance - (*min2)->distance;
}

void orderMinutia(minutia *root){
	int i,mincount=0;
	minutia *aux = root;
	while(aux!=NULL){
		mincount++;
		aux=aux->next;
	}
	minutia *minlist[mincount];
	aux = root;
	for(i=0;i<mincount;i++){
		minlist[i] = aux;
		aux = aux->next;
	}
	qsort(minlist,mincount,sizeof(minutia*),compMinutia);
	for(i=0;i<mincount-1;i++){
		minlist[i]->next = minlist[i+1];
	}
	minlist[i]->next=NULL;
}

int saveTemplate(minutia *root,char *filename){
	FILE *arq;
	if((arq=fopen(filename,"w+"))==NULL){
		//Nao conseguiu abrir o arquivo
		return 0;
	}
	while(root!=NULL){
		fprintf(arq,"(%d,%d)(%lf)(%ld)(%c)\n",root->x,root->y,root->angle,root->distance,root->type);
		root = root->next;
	}
	fclose(arq);
	return 1;
}

int loadTemplate(minutia *root, char *filename){
	FILE *arq;
	if((arq=fopen(filename,"r"))==NULL){
		//Nao conseguiu abrir o arquivo
		return 0;
	}
	char string[100];
	int nminutia = 0;
	while(fread(&string[0],1,1,arq)){
		if(string[0] == '\n') nminutia++;
	}
	fseek(arq,0,SEEK_SET);
	root = (minutia*) malloc(sizeof(minutia));
	fscanf(arq,"(%d,%d)(%lf)(%ld)(%c)\n",&root->x,&root->y,&root->angle,&root->distance,&root->type);
	minutia *aux = root;
	minutia *aux2;
	while(nminutia != 0){
		aux2 = (minutia*) malloc(sizeof(minutia));
		aux->next = aux2;
		aux = aux2;
		fscanf(arq,"(%d,%d)(%lf)(%ld)(%c)\n",&aux->x,&aux->y,&aux->angle,&aux->distance,&aux->type);
		aux->next = NULL;
		nminutia--;
	}
	return 1;
}

double getDistance(minutia *min1, minutia *min2){
	double dist = (min1->x-min2->x)*(min1->x-min2->x);
	dist += (min1->y-min2->y)*(min1->y-min2->y);
	return sqrt(dist);
}

void centerCores(minutia core,minutia *test){
	int disX,disY;
	disX = core.x - test->x;
	disY = core.y - test->y;
	test = test->next;
	while(test != NULL){
		test->x += disX;
		test->y += disY;
		test = test->next;
	}
}

int validadeFP(minutia *temp, minutia *test){
	minutia *core;
	int i=0,k;
	long penality = 0;
	int range,delete,include;
	range=delete=include=0;
	centerCores(*temp, test);
	k=0;
	while((test!=NULL)&&(i<MINLIM)){
		core = temp;
		while((core!=NULL)&&(k<MINLIM)){
			if((core->type|OK) !=0){
				if(sqrt(getDistance(core,test)) <= CORERANGE){
					range++;
					penality += PEN_TRANSLATE*sqrt(getDistance(core,test));
					test->type |= OK;
					core->type |= OK;
					k++;
					break;
				}
			}
			core = core->next;
		}
		if(core == NULL){
			delete++;
			penality += PEN_DELETE;
			test->type |= OK;
		}
		i++;
		test = test->next;
	}
	core = temp;
	while((core!=NULL)&&(k<MINLIM)){
		if((core->type &= OK)!=OK){
			include++;
			penality += PEN_INCLUDE;
			core->type |= OK;
			k++;
		}
		core = core->next;
	}
	if(penality<=PEN_THRESHOLD){
		return FINGER_MATCH;
	}else{
		return FINGER_UNMATCH;
	}
}

float testMatch(minutia *temp, minutia *test){
	int hit,total;
	hit = total = 0;
	minutia *aux=test;
	while(aux!=NULL){
		total++;
		aux = aux->next;
	}
	while(temp != NULL){
		aux = test;
		while(aux != NULL){
			if((pow(temp->x-aux->x,2)+pow(temp->y-aux->y,2))<MATCH_RAY){
				hit++;
			}
			aux = aux->next;
		}
		temp = temp->next;
	}
	return ((float)hit/(float)total)*100.0;
}

minutia *increaseMincount(minutia *temp, minutia *test){
	minutia *aux;
	while(test!=NULL){
		aux = temp;
		while(aux!=NULL){
			if((pow(aux->x-test->x,2)+pow(aux->y-test->y,2))<MATCH_RAY){
				aux->mincount++;
				break;
			}
			aux = aux->next;
		}
		if(aux == NULL){
			temp = allocMinutia(temp, test->x, test->y, test->angle, test->type);
		}
		test = test->next;
	}
	return temp;
}

int spiralMatching(minutia *temp, minutia *test, int flags, int width, int height){
	int i,j;
	int maioh,bla;
	double theta;
	minutia *aux,*aux2;
	if((flags & SPIN_MATCH)==1){
		theta = 2*PI;
	}else{
		theta = 0;
	}
	aux = NULL;
	while(theta>=0){
		aux = cloneMinutiaList(test, aux);
		theta -= SPIN_STEP;
		if(theta>0){
			aux2 = aux;
			while(aux2!=NULL){
				aux2->x += cos(theta)*(sqrt(pow(aux2->x-width/2,2)+pow(aux2->y-height/2,2)));
				aux2->y += sin(theta)*(sqrt(pow(aux2->x-width/2,2)+pow(aux2->y-height/2,2)));
				aux2 = aux2->next;
			}
		}
		for(i=0;i<SPIRAL_RAY;i++){
			aux2 = aux;
			while(aux2!=NULL){
				aux2->x += pow(-1,i)*i;
				aux2 = aux2->next;
			}
			if((bla=testMatch(temp,aux))>=MATCH_LIM){
				increaseMincount(temp,aux);
				//printf("maior: %d\n",bla);
				return FINGER_MATCH;
			}
			if(bla>maioh) maioh = bla;
			aux2 = aux;
			while(aux2!=NULL){
				aux2->y += pow(-1,i)*i;
				aux2 = aux2->next;
			}
			if((bla=testMatch(temp,aux))>=MATCH_LIM){
				increaseMincount(temp,aux);
				//printf("maior: %d\n",bla);
				return FINGER_MATCH;
			}
			if(bla>maioh) maioh = bla;
		}
		//printf("maior: %d\n",maioh);
		//freeMinutiaList(aux);
	}
	return FINGER_UNMATCH;
}
