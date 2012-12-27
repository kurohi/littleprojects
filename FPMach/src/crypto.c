#include <crypto.h>
#include <enhance.h>

ulong expmod(ulong a, ulong y, ulong m){
	ulong r = 1;
	ulong s = a%m;
	while(y){
		if(y&1){
			r = (r*s) % m;
		}
		s = (s*s) % m;
		y = y>>1;
	}
	return r;
}

unsigned int intrand(unsigned int range){
	unsigned char byte;
	ulong r;
	int i;
	for(i=0;i<sizeof(int);i++){
		byte = rand()%0xFF;
		r = (r<<7)|byte;
	}
	r = r%range;
	return r;
}

unsigned int mdc(unsigned int a, unsigned int b){
	ulong did,div,c;
	did = a;
	div = b;
	while((did%div)!=0){
		c = did%div;
		did = div;
		div = c;
	}
	return div;
}

int testPrime(unsigned int prime, unsigned int uncertain){
	unsigned int a;
	ulong t;
	int r=1;
	while(uncertain && r){
		uncertain--;
		a = intrand(prime - 1);
		t = expmod(a, (prime-1)/2, prime);
		if((t!=1)&&(t!=(prime-1))) r=0;
	}
	return r;
}

ulong generatePrime(unsigned int uncertain){
	ulong prime;
	int count=0;
	prime = (intrand((unsigned int)pow(2,sizeof(int)*8-1))<<1)+1; //primo impar com o tamanho do int
	while(testPrime(prime,uncertain)==0){
		prime+=2;
		printf("%d\n",++count);
	}
	return prime;
}

ulong euclext(ulong a, ulong b, ulong c){
	ulong r = b%a;
	if(r==0){
		return (c/a)%(b/a);	
	}else{
		return ((euclext(r,a,-c)*b+c)/a)%b;
	}
}

void generateKeys(RSAKey *publicKey, RSAKey *privateKey, minutia *root){
	srand(time(NULL));
	unsigned int seed=0xFFFFFFFF;
	int i = MAXMINUTIA;
	while((root!=NULL)&&(i>0)){
		seed ^= root->distance;
		seed ^= root->distance>>(sizeof(long)*4);
		root = root->next;
		i--;
	}
	printf("semente gerada: %d\n",seed);
	ulong p = generatePrime(UNCERTAIN);
	ulong q = generatePrime(UNCERTAIN);
	ulong phi = (p-1)*(q-1);
	ulong n = p*q;
	privateKey->key = (rand()<<(sizeof(long)*4-1))|rand();
	privateKey->key /= mdc(privateKey->key,phi);
	privateKey->mod = n;
	publicKey->key = euclext(privateKey->key, phi, 1);
	publicKey->mod = n;
}

int encript(RSAKey key, char *inname, char *exitname){
	ulong buffer;
	int size;
	FILE *filein, *fileout;
	if((filein = fopen(inname,"rb"))==NULL){
		return FAILURE;
	}
	if((fileout = fopen(exitname,"wb+"))==NULL){
		return FAILURE;
	}
	while((size = fread(&buffer,1,sizeof(long),filein))>0){
		if(size != sizeof(long)){
			//buffer = buffer<<((sizeof(long)-size)*8-1);
		}
		buffer = expmod(buffer, key.key, key.mod);
		fwrite(&buffer,size,1,fileout);
	}
	printf("%ld\n%ld\n",key.key,key.mod);
	fclose(filein);
	fclose(fileout);
	return SUCCESS;
}

int saveKeys(RSAKey publicKey, RSAKey privateKey, char *prefix){
	char filename[strlen(prefix)+20];
	strcpy(filename, prefix);
	strcat(filename, "public.key");
	FILE *arq;
	if((arq=fopen(filename,"w+"))==NULL){
		return FAILURE;
	}
	fprintf(arq,"%lu\n%lu",publicKey.key,publicKey.mod);
	fclose(arq);
	strcpy(filename, prefix);
	strcat(filename, "private.key");
	if((arq=fopen(filename,"w+"))==NULL){
		return FAILURE;
	}
	fprintf(arq,"%lu\n%lu",privateKey.key,privateKey.mod);
	fclose(arq);
	return SUCCESS;
}

int loadKey(RSAKey *key, char *filename){
	FILE *arq;
	if((arq=fopen(filename,"r+"))==NULL){
		return FAILURE;
	}
	fscanf(arq,"%lu\n%lu",&(key->key),&(key->mod));
	fclose(arq);
	return SUCCESS;
}

minutia *crowCentroidMinutia(minutia *root, int minfreq){
	minutia *aux,*aux2, *closer;
	double distance;
	double mindistance = 99999999.0;
	aux = root;
	while(aux != NULL){
		if(aux2->mincount>=minfreq){
			aux2 = root;
			distance = 0.0;
			while(aux2 != NULL){
				if((aux2 != aux)&&(aux2->mincount>=minfreq))
					distance += sqrt(pow(aux->x-aux2->x,2)+pow(aux->y-aux2->y,2));
				aux2 = aux2->next;
			}
			if(distance<mindistance){
				mindistance = distance;
				closer = aux2;
			}
		}
		aux = aux->next;
	}
	if(root == closer) return root;
	aux = root;
	while(aux->next != closer)aux = aux->next;
	aux->next = closer ->next;
	closer->next = root;
	return closer;	
}

long extractKey(minutia *root){
	int freq = 0;
	double distance;
	long key = 0;
	minutia *aux = root;
	minutia *aux2, *min;
	while(aux!=NULL){
		if(aux->mincount > freq) freq = aux->mincount;
		aux = aux->next;
	}
	freq -= FREQ_TOLERANCE;
	root = crowCentroidMinutia(root, freq);
	aux = root->next;
	while(aux!=NULL){
		if(aux->mincount >= freq){
			distance = 999999.0;
			aux2 = aux;
			while(aux2!=NULL){
				if(aux2->mincount >= freq){
					if(sqrt(pow(aux2->x-root->x,2)+pow(aux2->y-root->y,2))<distance){
						distance = sqrt(pow(aux2->x-root->x,2)+pow(aux2->y-root->y,2));
						min = aux2;
					}
				}
				aux2 = aux2->next;
			}
			key ^= min->distance;	//aki que cria a chave, pode mudar para uma concatenação de bytes.
		}
		aux = aux->next;
	}
	return key;
}

void minutiaFilter(image *img, minutia *root, int ray, int freq){
	int x,y,cross,ok;
	minutia *aux1, *aux2;
	aux1 = root;
	while(root!=NULL){
		if(root->mincount == freq){
			if(root->distance>DIST_LIM){
				root->mincount = -9999;
				root = root->next;
				continue;
			}
			ok=1;
			y = (root->y-ray>0)?root->y-ray:0;
			x = (root->x-ray>0)?root->x-ray:0;
			for(;(y<img->height)&&(y<root->y+ray)&&(ok);y++){
				for(;(x<img->width)&&(x<root->x+ray)&&(ok);x++){
					if(img->data[x+y*img->width] == 255){
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

						if(((cross==1)&&(root->type==ENDLINE))||((cross==3)&&((root->type&BIFURCATION)==BIFURCATION))){
							ok=0;
							root->mincount += 10;
							root->x = x;
							root->y = y;
							aux2=aux1;
							while(aux2!=NULL){
								if((aux2->x == root->x)&&(aux2->y == root->y)&&(aux2!=root)&&(aux2->mincount > 0)){
									root->mincount = -9999;
									break;
								}
								aux2 = aux2->next;
							}
						}
					}
				}
				x = (root->x-ray>0)?root->x-ray:0;
			}
			if(ok==1){
				root->mincount = -9999;
			}
		}
		root = root->next;
	}
}

long decayLong(long num){
	int i,j;
	i = num&0xFFFF;
	j = num>>8;
	if(i>224){
		j++;
	}
	return j;
}

struct statis{
	double anglemean;
	double minanglemean;
	double anglesd;
	double minanglesd;
	int mincount;
};

unsigned long askquestions(struct statis *area){
	unsigned long saida = 0;
	
}

unsigned long radialextraction(image *img, minutia *minlist){
	int x,y,ray,raymax;
	float angle;
	raymax = 0;
	for(angle = 0.0;angle<2*PI; angle += PI/4){
		x = minlist->x;
		y = minlist->y;
		for(ray=0;(x<img->width)&&(x>=0)&&(y<img->width)&&(y>=0); ray++){
			if(img->data[x+y*img->width]==0){
				break;
			}
			x = minlist->x+cos(angle)*ray;
			y = minlist->y+sin(angle)*ray;
		}
		if(ray>raymax){
			raymax = ray;
		}
	}
	if((img->height>img->width)&&(raymax>img->width)) raymax = img->width;
	else if((img->height<img->width)&&(raymax>img->height)) raymax = img->height;
	int i;
	minutia *aux=minlist;
	struct statis area[5];
	for(i=0;i<5;i++){
		area[i].anglemean = 0.0;
		area[i].minanglemean = 0.0;
		area[i].anglesd = 0.0;
		area[i].minanglesd = 0.0;
		area[i].mincount = 0;
	}
	while(aux!=NULL){
		if((aux->distance>raymax*raymax/10)&&(aux->distance<raymax*raymax)){
			angle = atan2(aux->y-minlist->y,aux->x-minlist->x);
			if((angle>=0)&&(angle<2*PI/5))	i=0;
			else if(angle<4*PI/5)		i=1;
			else if(angle<6*PI/5)		i=2;
			else if(angle<8*PI/5)		i=3;
			else if(angle<2*PI/5)		i=4;
			area[i].minanglemean += aux->angle;
			area[i].mincount++;
		}
		aux = aux->next;
	}
	for(i=0;i<5;i++){
		area[i].minanglemean /= area[i].mincount;
	}
	aux = minlist;
	while(aux!=NULL){
		if((aux->distance>raymax*raymax/100)&&(aux->distance<raymax*raymax)){
			angle = atan2(aux->y-minlist->y,aux->x-minlist->x);
			if((angle>=0)&&(angle<2*PI/5))	i=0;
			else if(angle<4*PI/5)		i=1;
			else if(angle<6*PI/5)		i=2;
			else if(angle<8*PI/5)		i=3;
			else if(angle<2*PI/5)		i=4;
			area[i].minanglesd += pow(aux->angle-area[i].minanglemean,2);
		}
		aux = aux->next;
	}
	for(i=0;i<5;i++){
		area[i].minanglesd /= area[i].mincount;
	}

	for(i=0;i<5;i++){
		printf("media\t\tdp\t\tcontagem\n%f\t%f\t%d\n",area[i].minanglemean,area[i].minanglesd,area[i].mincount);
	}

	return 0;//askquestions(area);
}
