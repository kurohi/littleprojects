/*
* Universidade de brasília - UnB
* Instituto de Ciências Exatas - IE
* Departamento de Ciência da Computação - CiC
* 
* Disciplina: Programação avançada
* Professor: Jan Correia
* 
* Aluno: Igor de Carvalho Coelho
* Matricula: 06/20068
*
* Funções de pré-processamento, algumas aqui foram apenas copiadas de artigos cientificos
*/
#include "enhance.h"


//cria uma matriz para guardar a frequencia ou a direção
struct campo* campo_alloc(struct imagem* img)
{
	struct campo* campo=(struct campo*) malloc(sizeof(struct campo));
	int campotam = img->width * img->height * sizeof(double);
	campo->pimg = (double*) malloc(campotam);
	memset(campo->pimg, 0, campotam);
	return campo;
}

//liberadores de espaço
void free_campo(struct campo* campo)
{
	free(campo->pimg);
	free(campo);
}


//filtro passa baixa
static int filtro_direc_low_pass(struct imagem* img, double *theta, double *ffbuf, int filter_size)
{

	
	double *fbuf;
	double *phix, *phiy;
	double *phi2x, *phi2y;
	int fsize = filter_size * 2 + 1;
	int fbuf_size = fsize * fsize * sizeof(double);
	int result;
	size_t nbytes = img->width * img->height * sizeof(double);
	double nx, ny;
	int val;
	int i, j;
	int x, y;


	fbuf = (double*) malloc(fbuf_size);
	phix = (double*) malloc(nbytes);
	phiy = (double*) malloc(nbytes);
	phi2x = (double*) malloc(nbytes);
	phi2y = (double*) malloc(nbytes);

//inicializa todos com 0
	memset(fbuf, 0, fbuf_size);
	memset(phix, 0, nbytes);
	memset(phiy, 0, nbytes);
	memset(phi2x, 0, nbytes);
	memset(phi2y, 0, nbytes);

	//Computando os angulos
	for (y = 0; y < img->height; y++)
		for (x = 0; x < img->width; x++)
		{
			val = x + y * img->width;
			phix[val] = cos(theta[val]);
			phiy[val] = sin(theta[val]);
		}

	nx = 0.0;
	for (j = 0; j < fsize; j++)
		for (i = 0; i < fsize; i++)
		{
			fbuf[j * fsize + i] = 1.0;
			nx += fbuf[j*fsize+i]; 
		}
	if (nx > 1.0)
	{
		for (j = 0; j < fsize; j++)
			for (i = 0; i < fsize; i++)
				fbuf[j * fsize + i] /= nx;
	}
	for (y = 0; y < img->height - fsize; y++)
		for (x = 0; x < img->width - fsize; x++)
		{
			nx = 0.0;
			ny = 0.0;
			for (j = 0; j < fsize; j++)
				for (i = 0; i < fsize; i++)
				{
					val = (x + i) + (j + y) * img->width;
					nx += fbuf[j * fsize + i] * phix[val];
					ny += fbuf[j * fsize + i] * phiy[val];
				}
			val = x + y * img->width;
			phi2x[val] = nx;
			phi2y[val] = ny;
		}


	for (y = 0; y < img->height - fsize; y++)
		for (x = 0; x < img->width - fsize; x++)
		{
			val = x + y * img->width;
			ffbuf[val] = atan2(phi2y[val], phi2x[val]) * 0.5;
		}

free:
	if (phix)
		free(phix);

	if (phiy)
		free(phiy);

	if (phi2x)
		free(phi2x);

	if (phi2y)
		free(phi2y);

	if (fbuf)
		free(fbuf);


	return result;

}


//pegar a matriz com as direções
int pegar_direcao(struct imagem* img, struct campo* ff, int block_size, int filter_size)
{

	
	int i, j;
	int u, v;
	int x, y;
	int result = 0;
	double nx, ny;
	double *ffbuf = ff->pimg;
	double *theta = NULL;
	unsigned char *imgbuf = img->data;
	int diff_size = block_size * 2 + 1;
	double dx[diff_size * diff_size], dy[diff_size * diff_size];



	if (filter_size > 0) {
		int theta_size = img->width * img->height * sizeof(double);
		theta = (double*) malloc(theta_size);
		if (theta == NULL)
		{
			errno = ENOMEM;
			return -1;
		}

		memset(theta, 0, theta_size);
	}


	for (y = block_size + 1; y < img->height - block_size - 1; y++)
		for (x = block_size + 1; x < img->width - block_size - 1; x++)
		{

			for (j = 0; j < diff_size; j++)
				for (i = 0; i < diff_size; i++)
				{
					dx[i * diff_size + j] = (double)
						(((int32_t) imgbuf[(x + i - block_size) + (y + j - block_size) * img->width]) -
						((int32_t) imgbuf[(x + i - block_size-1) + (y + j - block_size) * img->width]));
					dy[i * diff_size + j] = (double)
						(((int32_t) imgbuf[(x + i - block_size) + (y + j - block_size) * img->width]) -
						((int32_t) imgbuf[(x + i - block_size) + (y + j - block_size - 1) * img->width]));
				}

			nx = 0.0;
			ny = 0.0;
			for (v = 0; v < diff_size; v++)
				for (u = 0; u < diff_size; u++)
				{
					nx += 2 * dx[u * diff_size + v] * dy[u * diff_size + v];
					ny += dx[u * diff_size + v] * dx[u * diff_size + v]
						- dy[u * diff_size + v] * dy[u * diff_size + v];
				}

			if (filter_size > 0)
				theta[x + y * img->width] = atan2(nx, ny);
			else
				ffbuf[x + y * img->width] = atan2(nx, ny) * 0.5;
		}


	if (filter_size > 0)
		result = filtro_direc_low_pass(img ,theta, ffbuf, filter_size);

	if (theta)
		free(theta);

	return result;

}



#define BLOCK_W     16
#define BLOCK_W2     8


#define BLOCK_L     32
#define BLOCK_L2    16

#define EPSILON     0.0001
#define LPSIZE      3
#define LPFACTOR    (1.0 / ((LPSIZE * 2 + 1) * (LPSIZE * 2 + 1)))

int imagem_get_frequency(struct imagem *img,
	struct campo *direction, struct campo *frequency)
{
	
	int x, y;
	int u, v;
	int d, k;
	double *out;
	double *freq = frequency->pimg;
	double *orientation = direction->pimg;
	unsigned char *imgbuf = img->data;
	size_t size = img->width * img->height * sizeof(double);

	double dir = 0.0, cosdir = 0.0, sindir = 0.0;
	int peak_pos[BLOCK_L];
	int peak_cnt;     
	double peak_freq;    
	double Xsig[BLOCK_L];
	double pmin, pmax;



	out = malloc(size);
	if (out == NULL)
	{
		return -1;
	}

	memset(out, 0, size);
	memset(freq, 0, size);


	for (y = BLOCK_L2; y < img->height - BLOCK_L2; y++)
		for (x = BLOCK_L2; x < img->width - BLOCK_L2; x++)
		{

			dir = orientation[(x + BLOCK_W2) + (y + BLOCK_W2) * img->width];
			cosdir = -sin(dir); 
			sindir = cos(dir);   


			for (k = 0; k < BLOCK_L; k++) {
				Xsig[k] = 0.0;
				for (d = 0; d < BLOCK_W; d++)
				{
					u = (int) (x + (d - BLOCK_W2) * cosdir
							+ (k - BLOCK_L2) * sindir);
					v = (int) (y + (d - BLOCK_W2) * sindir
							- (k - BLOCK_L2) * cosdir);

					if (u < 0)
						u = 0;
					else if (u > img->width - 1)
						u = img->width - 1;

					if (v < 0)
						v = 0;
					else if (v > img->height - 1)
						v = img->height - 1;

					Xsig[k] += imgbuf[u + (v * img->width)];
				}
				Xsig[k] /= BLOCK_W;
			}

			peak_cnt = 0;

			pmax = pmin = Xsig[0];
			for (k = 1; k < BLOCK_L; k++)
			{
				if (pmin>Xsig[k]) pmin = Xsig[k];
				if (pmax<Xsig[k]) pmax = Xsig[k];
			}

			if ((pmax - pmin) > 64.0)
				for (k = 1; k < BLOCK_L-1; k++)
					if ((Xsig[k-1] < Xsig[k]) &&
							(Xsig[k] >= Xsig[k+1])) 
						peak_pos[peak_cnt++] = k;


			peak_freq = 0.0;
			if (peak_cnt >= 2)
			{
				for (k = 0; k < peak_cnt - 1; k++)
					peak_freq += peak_pos[k + 1] - peak_pos[k];
				peak_freq /= peak_cnt - 1;
			}


			if (peak_freq > 30.0)
				out[x + y * img->width] = 0.0;
			else if (peak_freq < 2.0)
				out[x + y * img->width] = 0.0;
			else
				out[x + y * img->width] = 1.0 / peak_freq;
		}


	for (y = BLOCK_L2; y < img->height - BLOCK_L2; y++)
		for (x = BLOCK_L2; x < img->width - BLOCK_L2; x++)
			if (out[x + y * img->width] < EPSILON)
			{
				if (out[x + (y - 1) * img->width] > EPSILON)
					out[x + (y * img->width)] =
						out[x + (y - 1) * img->width];
				else if (out[x - 1 + (y * img->width)] > EPSILON)
					out[x + (y * img->width)] =
						out[x - 1 + (y * img->width)];
			}


	for (y = BLOCK_L2; y < img->height - BLOCK_L2; y++)
		for (x = BLOCK_L2; x < img->width - BLOCK_L2; x++)
		{
			k = x + y * img->width;
			peak_freq = 0.0;
			for (v = -LPSIZE; v <= LPSIZE; v++)
				for (u = -LPSIZE; u <= LPSIZE; u++)
					peak_freq += out[(x + u) + (y + v) * img->width];
			freq[k] = peak_freq * LPFACTOR;
		}

	free(out);


	return 0;
}


static void image_dilate(struct imagem *img)
{
	unsigned char *imgbuf = img->data;
	int x , y;

	for (y = 1; y < img->height - 1; y++)
		for (x = 1; x < img->width - 1; x++)
			if (imgbuf[(x) + (y) * img->width] == 0xff)
			{
				imgbuf[(x - 1) + (y) * img->width] |= 0x80;
				imgbuf[(x + 1) + (y) * img->width] |= 0x80;
				imgbuf[(x) + (y - 1) * img->width] |= 0x80;
				imgbuf[(x) + (y + 1) * img->width] |= 0x80;
			}

	for (y = 0; y < img->width * img->height; y++)
		if (imgbuf[y])
			imgbuf[y] = 0xff;
}

static void image_erode(struct imagem *img)
{
	unsigned char *imgbuf = img->data;
	int x, y;

	for (y = 1; y < img->height - 1; y++)
		for (x = 1; x < img->width - 1; x++)
			if (imgbuf[(x) + (y) * img->width] == 0)
			{
				imgbuf[(x - 1) + (y) * img->width] &= 0x80;
				imgbuf[(x + 1) + (y) * img->width] &= 0x80;
				imgbuf[(x) + (y - 1) * img->width] &= 0x80;
				imgbuf[(x) + (y + 1) * img->width] &= 0x80;
			}

	for (y = 0; y < img->width * img->height; y++)
		if (imgbuf[y] != 0xff)
			imgbuf[y] = 0;
}

int imagem_get_mask(struct imagem *img, struct campo *direction,
	struct campo *frequency, struct imagem *mask)
{
	
	int pos, posout;
	int x, y;
	unsigned char *out = mask->data;
	double *freq = frequency->pimg;
	double freqmin = 1.0 / 25, freqmax = 1.0 / 3;


	memset(out, 0, img->width * img->height);

	for (y = 0; y < img->height; y++)
		for (x = 0; x < img->width; x++)
		{
			pos = x + y * img->width;
			posout = x + y * img->width;
			out[posout] = 0;
			if (freq[pos] >= freqmin && freq[pos] <= freqmax)

				out[posout] = 255;
		}


	for (y = 0; y < 4; y++)
		image_dilate(mask);


	for (y = 0; y < 12; y++)
		image_erode(mask);


	return 0;
}


static double enhance_gabor(double x, double y, double phi, double f, double r2)
{
	double dy2 = 1.0 / r2, dx2 = 1.0 / r2;
	double x2, y2;

	phi += M_PI / 2;
	x2 = -x * sin(phi) + y * cos(phi);
	y2 =  x * cos(phi) + y * sin(phi);

	return exp(-0.5 * (x2 * x2 * dx2 + y2 * y2 * dy2)) * cos(2 * M_PI * x2 * f);
}


int imagem_enhance_gabor(struct imagem *img,
	struct campo *direction, struct campo *frequency,
	struct imagem *mask, double radius)
{
	
	int Wg2 = 8;
	int i, j;
	int u,v;
	double *orientation = direction->pimg;
	double *frequence = frequency->pimg;
	unsigned char *enhanced = (unsigned char*) malloc(img->width * img->height);
	unsigned char *imgbuf = img->data;
	double sum, f, o;


	if (enhanced == NULL)
	{
		errno = ENOMEM;
		return -1;
	}
	memset(enhanced, 0, img->width * img->height);


	radius = radius * radius;

	for (j = Wg2; j < img->height - Wg2; j++)
		for (i = Wg2; i < img->width - Wg2; i++)
			if (mask == NULL || mask->data[i + j * img->width] != 0)
			{
				sum = 0.0;
				o = orientation[i + j * img->width];
				f = frequence[i + j * img->width];
				for (v = -Wg2; v <= Wg2; v++)
					for (u = -Wg2; u <= Wg2; u++)
						sum += enhance_gabor((double) u, (double) v, o, f,
								radius) *
								imgbuf[(i - u) + (j - v) * img->width];


				if (sum > 255.0)
					sum = 255.0;
				if (sum < 0.0)
					sum = 0.0;

				enhanced[i + j * img->width] = (unsigned char) sum;
			}
	memcpy(imgbuf, enhanced, img->width * img->height);
	free(enhanced);

	return 0;
}


void imagem_binarize(struct imagem *img, unsigned char limit)
	{
	int n;
	unsigned char *imgbuf = img->data;


	for (n = 0; n < img->width * img->height; n++)

		imgbuf[n] = (imgbuf[n] > limit) ? 0xff : 0;
}

//-----------------------afinamento----------------------------
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


void imagem_thin(struct imagem *img)
{
	int	x, y; 
	int	i; 
	int	pc = 0; 
	int	count = 1;
	int	p, q; 
	unsigned char qb[img->width];
	unsigned char *imgbuf = img->data;

	qb[img->width - 1] = 0;		

	
	while (count)
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


void inverter_imagem(struct imagem* img)
{
	int x,y;
	for(x=0;x<img->width;x++)
	{
		for(y=0;y<img->height;y++)
		{
			img->data[x+(y*img->width)]=255-img->data[x+y*img->width];
		}
	}
}

void imagem_bipixel(struct imagem *img){
	int x,y;
	for(y=1; y<img->height-1; y++)
		for(x=1;x<img->width-1; x++){
			if(img->data[x+y*img->width] == 255){
				img->data[(x)+(y)*img->width] = 5;
				img->data[(x)+(y+1)*img->width] = 5;
				img->data[(x)+(y-1)*img->width] = 5;
				img->data[(x+1)+(y)*img->width] = 5;
				img->data[(x+1)+(y+1)*img->width] = 5;
				img->data[(x+1)+(y-1)*img->width] = 5;
				img->data[(x-1)+(y)*img->width] = 5;
				img->data[(x-1)+(y+1)*img->width] = 5;
				img->data[(x-1)+(y-1)*img->width] = 5;
			}
		}
	for(y=1; y<img->height-1; y++)
		for(x=1;x<img->width-1; x++){
			if(img->data[x+y*img->width] == 5){
				img->data[x+y*img->width] = 255;
			}
		}
}



//------------------main----------------------
void enhance(imagem* img)
{
	struct campo* direcao=campo_alloc(img);
	struct campo* frequencia=campo_alloc(img);
	struct imagem* mascara=(struct imagem*) malloc(sizeof(struct imagem));
	mascara->data=(unsigned char*) malloc(img->width*img->height);
	mascara->width=img->width;
	mascara->height=img->height;
	printf("Pegando direções da imagem\n");
	if(pegar_direcao(img,direcao,7,8)==-1)
	{
		printf("Erro na direcao");
		exit(0);
	}printf("Pegando frequencisas da imagem\n");
	if(imagem_get_frequency(img,direcao,frequencia))
	{
		printf("Erro na frequencia");
		exit(0);
	}printf("Pegando a mascara\n");
	if(imagem_get_mask(img,direcao,frequencia,mascara))
	{
		printf("Erro na mascara");
		exit(0);
	}printf("Passando o filtro de Gabor\n");
	if(imagem_enhance_gabor(img,direcao,frequencia,mascara,4.0))
	{
		printf("Erro no filtro");
		exit(0);
	}
}
