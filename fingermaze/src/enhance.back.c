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
*/
#include "enhance.h"
#include "imagem.h"

#define SIGMA 4.5
#define PI 3.14159265

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
void makeGaussianMatrix(double *matrix, int width, int height){
	int  x,y;
	for(y=-height/2; y<height/2+1 ; y++)
		for(x=-width/2; x<width/2+1; x++){
			matrix[(x+width/2) + (y+height/2)*width] = (1/(2*PI*pow(SIGMA,2)))*exp(-(pow(x,2)+pow(y,2))/(2*pow(SIGMA,2)));
		}
}

void smoothPos(double *direcMatrix, double *direcMatrix_out, int sizex, int sizey){
	double phix[sizex*sizey], phiy[sizex*sizey];
	double phixl, phiyl;
	double gauss[3*3];
	makeGaussianMatrix(gauss, 3, 3);
	int x,y,xl,yl;
	for(y=0; y<sizey; y++)
		for(x=0; x<sizex; x++){
			phix[x+y*sizex] = cos(2*direcMatrix[x+y*sizex]);
			phiy[x+y*sizex] = sin(2*direcMatrix[x+y*sizex]);
		}
	//convulação dos gausianos(vai os 2 de uma vez para ecoomizar)
	for(y=0; y<sizey; y++)
		for(x=0;x<sizex; x++){
			phixl = 0.0;
			phiyl = 0.0;
			for(yl=0;yl<3;yl++)
				for(xl=0; xl<3; xl++){
					phixl += gauss[xl + yl*3]*phix[(x+xl) + (y+yl)*sizex] / 9.0;
					phiyl += gauss[xl + yl*3]*phiy[(x+xl) + (y+yl)*sizex] / 9.0;
				}
			direcMatrix_out[x + y*sizex] = atan(phiyl/phixl)/2;
		}
}

//Método de sobel
void sobel3x3(struct imagem *img, double *direcMatrix){
	int matrixX[3*3] = {
		1, 0, -1,
		2, 0, -2,
		1, 0, -1
	};
	int matrixY[9*9] = {
		1, 2, 1,
		0, 0, 0,
		-1, -2, -1
	};
	float acumX, acumY;
	int x,y, x2,y2;
	for(y=0; y<img->height-3; y+=1){
		for(x=0; x<img->width-3; x+=1){
			acumX = acumY = 0.0;
			for(y2=0; y2<3; y2++){
				for(x2=0; x2<3; x2++){
					acumX += img->data[x+x2 + (y+y2)*img->width] * matrixX[x2+y2*3]; 
					acumY += img->data[x+x2 + (y+y2)*img->width] * matrixY[x2+y2*3]; 
				}
			}
			acumX /= 9.0;
			acumY /= 9.0;
			direcMatrix[x + y*(img->width)] = atan(acumX/acumY);
		}
	}
}

//pegar a matriz com as direções
int pegar_direcao(struct imagem* img, struct campo* ff, int block_size, int filter_size)
{
	sobel3x3(img,ff->pimg);
	smoothPos(ff->pimg, ff->pimg, img->width, img->height);
	return 1;
}



/* width */
#define BLOCK_W     16
#define BLOCK_W2     8

/* length */
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
	int peak_pos[BLOCK_L]; /* peak positions */
	int peak_cnt;          /* peak count     */
	double peak_freq;         /* peak frequence */
	double Xsig[BLOCK_L];     /* x signature    */
	double pmin, pmax;


	/* allocate memory for the output */
	out = malloc(size);
	if (out == NULL)
	{
		return -1;
	}

	memset(out, 0, size);
	memset(freq, 0, size);

	/* 1 - Divide G into blocks of BLOCK_W x BLOCK_W - (16 x 16) */
	for (y = BLOCK_L2; y < img->height - BLOCK_L2; y++)
		for (x = BLOCK_L2; x < img->width - BLOCK_L2; x++)
		{
			/* 2 - oriented window of size l x w (32 x 16) in the ridge dir */
			dir = orientation[(x + BLOCK_W2) + (y + BLOCK_W2) * img->width];
			cosdir = -sin(dir);  /* ever > 0 */
			sindir = cos(dir);   /* -1 ... 1 */

			/* 3 - compute the x-signature X[0], X[1], ... X[l-1] */
			for (k = 0; k < BLOCK_L; k++) {
				Xsig[k] = 0.0;
				for (d = 0; d < BLOCK_W; d++)
				{
					u = (int) (x + (d - BLOCK_W2) * cosdir
							+ (k - BLOCK_L2) * sindir);
					v = (int) (y + (d - BLOCK_W2) * sindir
							- (k - BLOCK_L2) * cosdir);
					/* clipping */
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

			/* Let T(i,j) be the avg number of pixels between 2 peaks */
			/* find peaks in the x signature */
			peak_cnt = 0;
			/* test if the max - min or peak to peak value too small is,
			   then we ignore this point */
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

			/* compute mean value */
			peak_freq = 0.0;
			if (peak_cnt >= 2)
			{
				for (k = 0; k < peak_cnt - 1; k++)
					peak_freq += peak_pos[k + 1] - peak_pos[k];
				peak_freq /= peak_cnt - 1;
			}

			/* 4 - must lie in a certain range [1/25-1/3] */
			/*     changed to range [1/30-1/2] */
			if (peak_freq > 30.0)
				out[x + y * img->width] = 0.0;
			else if (peak_freq < 2.0)
				out[x + y * img->width] = 0.0;
			else
				out[x + y * img->width] = 1.0 / peak_freq;
		}

	/* 5 - interpolated ridge period for the unknown points */
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

	/* 6 - Inter-ridges distance change slowly in a local neighbourhood */
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
				/*              out[posout] = (uint8_t)(10.0/freq[pos]);*/
				out[posout] = 255;
		}

	/* fill in the holes */
	for (y = 0; y < 4; y++)
		image_dilate(mask);

	/* remove borders */
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

/* Enhance a fingerprint image */
int imagem_enhance_gabor(struct imagem *img,
	struct campo *direction, struct campo *frequency,
	struct imagem *mask, double radius)
{
	
	int Wg2 = 8; /* from -5 to 5 are 11 */
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

	/* take square */
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

				/* printf("%6.1f ", sum);*/
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

/* Transform the gray image into a black & white binary image */
void imagem_binarize(struct imagem *img, unsigned char limit)
	{
	int n;
	unsigned char *imgbuf = img->data;

	/* loop through each pixel */
	for (n = 0; n < img->width * img->height; n++)
		/* now a do some math to decided if its white or black */
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
	int	x, y; /* Pixel location */
	int	i; /* Pass index */
	int	pc = 0; /* Pass count */
	int	count = 1; /* Deleted pixel count */
	int	p, q; /* Neighborhood maps of adjacent cells */
	unsigned char qb[img->width]; /* Neighborhood maps of prev scanline */
	unsigned char *imgbuf = img->data;

	qb[img->width - 1] = 0;		/* Used for lower-right pixel	*/

	/* Scan image while deletions */
	while (count)
	{
		pc++;
		count = 0;

		for (i = 0 ; i < 4; i++)
		{
			int m = masks[i]; /* deletion direction mask */

			/* Build initial previous scan buffer. */
			p = imgbuf[0] != 0;
			for (x = 0; x < img->width - 1; x++)
				qb[x] = p = ((p << 1) & 0006) | (imgbuf[x + 1] != 0);

			/* Scan image for pixel deletion candidates. */
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

				/* Process right edge pixel. */
				p = (p << 1) & 0666;
				if	((p & m) == 0 && delet[p])
				{
					count++;
					imgbuf[(y * img->width) + img->width - 1] = 0;
				}
			}

			/* Process bottom scan line. */
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
