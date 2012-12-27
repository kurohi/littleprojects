#include <minutia.h>

#ifdef USEOPENCV
void showMinutia(image *img, minutia *root,int win_x, int win_y){
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
		root=root->next;
	}
	cvNamedWindow("Minutia",0);
	cvShowImage("Minutia",fp);
	//cvWaitKey(0);
	//cvDestroyAllWindows();
	//cvReleaseImage(&fp);
}
#endif

minutia *findMinutia(image *img, double *direcs, int win_x, int win_y){
	int x,y;
	int cross;
	minutia *root = NULL;
	for(y=LIM_Y; y<img->height-LIM_Y; y++)
		for(x=LIM_X; x<img->width-LIM_X; x++){
			if(img->data[x+y*img->width]==255){
				cross = 0;
				//P1->P2
				if((img->data[(x-1)+(y-1)*img->width]==255)&&(img->data[(x-0)+(y-1)*img->width]==0)) cross++;
				//P2->P3
				if((img->data[(x-0)+(y-1)*img->width]==255)&&(img->data[(x+1)+(y-1)*img->width]==0)) cross++;
				//P3->P4
				if((img->data[(x+1)+(y-1)*img->width]==255)&&(img->data[(x+1)+(y-0)*img->width]==0)) cross++;
				//P4->P5
				if((img->data[(x+1)+(y-0)*img->width]==255)&&(img->data[(x+1)+(y+1)*img->width]==0)) cross++;
				//P5->P6
				if((img->data[(x+1)+(y+1)*img->width]==255)&&(img->data[(x-0)+(y+1)*img->width]==0)) cross++;
				//P6->P7
				if((img->data[(x-0)+(y+1)*img->width]==255)&&(img->data[(x-1)+(y+1)*img->width]==0)) cross++;
				//P7->P8
				if((img->data[(x-1)+(y+1)*img->width]==255)&&(img->data[(x-1)+(y-0)*img->width]==0)) cross++;
				//P8->P1
				if((img->data[(x-1)+(y-0)*img->width]==255)&&(img->data[(x-1)+(y-1)*img->width]==0)) cross++;

				switch(cross){
					case 1: root = allocMinutia(root, x, y, direcs[(x+1)/win_x + ((y+1)/win_y)*(img->width/win_x)], ENDLINE);break;
					case 3: root = allocMinutia(root, x, y, direcs[(x+1)/win_x + ((y+1)/win_y)*(img->width/win_x)], BIFURCATION);break;
				}
			}
		}
	return root;
}

minutia *allocMinutia(minutia *root, int x, int y, double angle, char type){
	minutia *new = (minutia*) malloc (sizeof(minutia));
	new->x = x;
	new->y = y;
	new->angle = angle;
	new->type = type;
	new->mincount = 0;
	new->next = NULL;
	if(root==NULL){
		return new;
	}
	minutia *aux = root;
	minutia *aux2;
	
	while(aux != NULL){
		if(sqrt(pow(aux->x-new->x,2)+pow(aux->y-new->y,2))<RANGE){
			free(new);
			return root;
		}
		aux2 = aux;
		aux = aux->next;
	}
	aux2->next = new;
	return root;
}

void freeMinutiaList(minutia *root){
	if(root == NULL) return;
	minutia *aux = root;
	while(aux->next != NULL){
		aux = aux->next;
	}
	aux->next = root;
	minutia *aux2 = root->next;
	while(root!=aux2){
		aux = aux2;
		aux2 = aux2->next;
		free(aux);
	}
	free(root);
}

minutia *deleteMinutia(minutia *root, minutia *minu){
	minutia *aux = root;
	if(root == minu){
		aux = root->next;
		free(root);
		return aux;
	}
	while(aux->next != minu){
		aux = aux->next;
	}
	aux->next = minu->next;
	free(minu);
	return root;
}

void markLine(image *img, unsigned char value, int x, int y, int win_x, int win_y){
	int newX,newY;
	img->data[x+y*win_x] = value;
	while((x>=0)&&(y>=0)&&(x<img->width)&&(y<img->height)){
		newX = newY = -1;
		if(img->data[(x+1)+(y+0)*win_x]==255){
			img->data[(x+1)+(y+0)*win_x] = value;
			newX = x+1;
			newY = y;
		}if(img->data[(x+1)+(y+1)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x+1, y+1, win_x, win_y);
			}else{
				img->data[(x+1)+(y+1)*win_x] = value;
				newX = x+1;
				newY = y+1;
			}
		}if(img->data[(x+0)+(y+1)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x+0, y+1, win_x, win_y);
			}else{
				img->data[(x+0)+(y+1)*win_x] = value;
				newX = x;
				newY = y+1;
			}
		}if(img->data[(x-1)+(y+1)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x-1, y+1, win_x, win_y);
			}else{
				img->data[(x-1)+(y+1)*win_x] = value;
				newX = x-1;
				newY = y+1;
			}
		}if(img->data[(x-1)+(y+0)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x-1, y+0, win_x, win_y);
			}else{
				img->data[(x-1)+(y+0)*win_x] = value;
				newX = x-1;
				newY = y;
			}
		}if(img->data[(x-1)+(y-1)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x-1, y-1, win_x, win_y);
			}else{
				img->data[(x-1)+(y-1)*win_x] = value;
				newX = x-1;
				newY = y-1;
			}
		}if(img->data[(x+0)+(y-1)*win_x]==255){
			if((newX>-1)||(newY>-1)){
				markLine(img, value, x+0, y-1, win_x, win_y);
			}else{
				img->data[(x+0)+(y-1)*win_x] = value;
				newX = x;
				newY = y-1;
			}
		}if(img->data[(x+1)+(y-1)*win_x]==255){
			if((newX!=-1)||(newY!=-1)){
				markLine(img, value, x+1, y-1, win_x, win_y);
			}else{
				img->data[(x+1)+(y-1)*win_x] = value;
				newX = x+1;
				newY = y-1;
			}
		}
		x = newX;
		y = newY;
	}
}

minutia *posprocessing(image *img, minutia *root, int win_x, int win_y){
	minutia *aux = root;
	minutia *aux2;
	image *window = (image *) calloc (1,sizeof(image));
	window->data = (unsigned char*) calloc (win_x*win_y,1);
	window->width = win_x;
	window->height = win_y;
	int x,y,i,j;
	char ok,mark;
	while(aux!=NULL){
		//para esse loop estou contando que a captura das minucias teve uma restrição para o centro da imagem maior que a metade de win_x e win_y
		for(i=0,y=aux->y-win_y/2;(i<window->height)&&(y<aux->y+win_y/2);i++, y++)
			for(j=0, x=aux->x-win_x/2;(j<window->width)&&(x<aux->x+win_x/2);j++, x++){
				window->data[j+i*window->width] = img->data[x+y*img->width];
			}
		x = win_x/2;
		y = win_y/2;
		aux2 = aux;
		aux = aux->next;
		switch(aux2->type){
			case ENDLINE:
				window->data[x+y*win_x] = 40;
				if(window->data[(x+1)+(y+0)*win_x]==255){
					window->data[(x+1)+(y+0)*win_x] = 1;
					markLine(window, 1, x+1, y, win_x, win_y);
				}else if(window->data[(x+1)+(y+1)*win_x]==255){
					window->data[(x+1)+(y+1)*win_x] = 1;
					markLine(window, 1, x+1, y+1, win_x, win_y);
				}else if(window->data[(x+0)+(y+1)*win_x]==255){
					window->data[(x+0)+(y+1)*win_x] = 1;
					markLine(window, 1, x, y+1, win_x, win_y);
				}else if(window->data[(x-1)+(y+1)*win_x]==255){
					window->data[(x-1)+(y+1)*win_x] = 1;
					markLine(window, 1, x-1, y+1, win_x, win_y);
				}else if(window->data[(x-1)+(y+0)*win_x]==255){
					window->data[(x-1)+(y+0)*win_x] = 1;
					markLine(window, 1, x-1, y, win_x, win_y);
				}else if(window->data[(x-1)+(y-1)*win_x]==255){
					window->data[(x-1)+(y-1)*win_x] = 1;
					markLine(window, 1, x-1, y-1, win_x, win_y);
				}else if(window->data[(x+0)+(y-1)*win_x]==255){
					window->data[(x+0)+(y-1)*win_x] = 1;
					markLine(window, 1, x, y-1, win_x, win_y);
				}else if(window->data[(x+1)+(y-1)*win_x]==255){
					window->data[(x+1)+(y-1)*win_x] = 1;
					markLine(window, 1, x+1, y-1, win_x, win_y);
				}
				ok = 0;
				for(x=1;(ok==0)&&(x<window->width);x++){
					if((window->data[(x-1)] == 0)&&(window->data[x]==1)){
						ok=1;
					}
					if((window->data[x+(window->height-1)*window->width]==0)&&(window->data[(x-1)+(window->height-1)*window->width]==1)){
						ok=1;
					}
				}
				for(y=1;(ok==0)&&(y<window->height);y++){
					if((window->data[y*window->width]==0)&&(window->data[(y-1)*window->width]==1)){
						ok=1;
					}
					if((window->data[(window->width-1)+y*window->width] == 1)&&(window->data[(window->width-1)+(y-1)*window->width]==0)){
						ok=1;
					}
				}
				if(ok==0){
					root = deleteMinutia(root,aux2);
				}
				break;
				
			case BIFURCATION:
				mark=1;
				window->data[x+y*win_x] = 40;
				if(window->data[(x+1)+(y+0)*win_x]==255){
					window->data[(x+1)+(y+0)*win_x] = mark;
					mark+=1;
				}if(window->data[(x+1)+(y+1)*win_x]==255){
					window->data[(x+1)+(y+1)*win_x] = mark;
					mark+=1;
				}if(window->data[(x+0)+(y+1)*win_x]==255){
					window->data[(x+0)+(y+1)*win_x] = mark;
					mark+=1;
				}if(window->data[(x-1)+(y+1)*win_x]==255){
					window->data[(x-1)+(y+1)*win_x] = mark;
					mark+=1;
				}if(window->data[(x-1)+(y+0)*win_x]==255){
					window->data[(x-1)+(y+0)*win_x] = mark;
					mark+=1;
				}if(window->data[(x-1)+(y-1)*win_x]==255){
					window->data[(x-1)+(y-1)*win_x] = mark;
					mark+=1;
				}if(window->data[(x+0)+(y-1)*win_x]==255){
					window->data[(x+0)+(y-1)*win_x] = mark;
					mark+=1;
				}if(window->data[(x+1)+(y-1)*win_x]==255){
					window->data[(x+1)+(y-1)*win_x] = mark;
					mark+=1;
				}
				mark=1;
				if(window->data[(x+1)+(y+0)*win_x]==mark){
					markLine(window, mark, x+1, y, win_x, win_y);
					mark+=1;
				}if(window->data[(x+1)+(y+1)*win_x]==mark){
					markLine(window, mark, x+1, y+1, win_x, win_y);
					mark+=1;
				}if(window->data[(x+0)+(y+1)*win_x]==mark){
					markLine(window, mark, x, y+1, win_x, win_y);
					mark+=1;
				}if(window->data[(x-1)+(y+1)*win_x]==mark){
					markLine(window, mark, x-1, y+1, win_x, win_y);
					mark+=1;
				}if(window->data[(x-1)+(y+0)*win_x]==mark){
					markLine(window, mark, x-1, y, win_x, win_y);
					mark+=1;
				}if(window->data[(x-1)+(y-1)*win_x]==mark){
					markLine(window, mark, x-1, y-1, win_x, win_y);
					mark+=1;
				}if(window->data[(x+0)+(y-1)*win_x]==mark){
					markLine(window, mark, x, y-1, win_x, win_y);
					mark+=1;
				}if(window->data[(x+1)+(y-1)*win_x]==mark){
					markLine(window, mark, x+1, y-1, win_x, win_y);
					mark+=1;
				}
				mark=0;
				for(x=1;(mark!=7)&&(x<window->width);x++){
					for(y=1;y<4;y++){
						if((window->data[(x-1)] == 0)&&(window->data[x]==y)){
							mark|=1<<(y-1);
						}
						if((window->data[x+(window->height-1)*window->width]==0)&&(window->data[(x-1)+(window->height-1)*window->width]==y)){
							mark|=1<<(y-1);
						}
					}
				}
				for(y=1;(mark!=7)&&(y<window->height);y++){
					for(x=1;x<4;x++){
						if((window->data[y*window->width]==0)&&(window->data[(y-1)*window->width]==x)){
							mark|=1<<(x-1);
						}
						if((window->data[(window->width-1)+y*window->width] == x)&&(window->data[(window->width-1)+(y-1)*window->width]==0)){
							mark|=1<<(x-1);
						}
					}
				}
				if(mark!=7){
					root=deleteMinutia(root,aux2);
				}
				break;
		}
		
	}
	root = removeBorderMinutia(img,root);
	return root;
}

minutia *cloneMinutiaList(minutia *temp,minutia *last){
	minutia *dst=NULL;
	if(last == NULL){
		while(temp!=NULL){
			dst = allocMinutia(dst, temp->x, temp->y, temp->angle, temp->type);
			temp = temp->next;
		}
		return dst;
	}else{
		dst = last;
		while(temp != NULL){
			last->x = temp->x;
			last->y = temp->y;
			last->angle = temp->angle;
			last->type = temp->type;
			last->mincount = temp->mincount;
			temp = temp->next;
			if(last->next == NULL){
				while(temp != NULL){
					dst = allocMinutia(dst, temp->x, temp->y, temp->angle, temp->type);
					temp = temp->next;
				}
				return dst;
			}
			last = last->next;
		}
		return dst;
	}
}

minutia *removeBorderMinutia(image *img, minutia *root){
	minutia *aux = root;
	minutia *aux2;
	int x,y,k;
	while(aux!=NULL){
		if((aux->type&1) == 0){
			x = aux->x+cos(aux->angle)*2;
			y = aux->y+cos(aux->angle)*2;
			k=3;
			while((x<img->width)&&(x>=0)&&(y<img->height)&&(y>=0)&&(k<BORDER_MAX)){
				if(img->data[x+y*img->width] == 255){
					k=0;
					break;
				}
				x+=cos(aux->angle)*k;
				y+=sin(aux->angle)*k;
				k++;
			}
			if(k!=0){
				aux2 = aux->next;
				root = deleteMinutia(root, aux);
				aux = aux2;
			}else{
				aux = aux->next;
			}
		}else{
			aux = aux->next;
		}
	}
	return root;
}
