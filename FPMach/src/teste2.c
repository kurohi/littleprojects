#include <image.h>
#include <enhance.h>
#include <minutia.h>
#include <matching.h>
#include <crypto.h>
#include <sys/time.h>

int main(int argc, char**argv){
	if(argc<2){
		printf("Tem que por o nome do arquivo\n");
		exit(0);
	}
	image *img = loadPGM(argv[1]);
	image *enhanced;
	double *direcs = (double*) malloc(sizeof(double)*img->width/10*img->height/10);;
	minutia *minlist;
	direction(img,direcs,10,10);
	enhanced = clone_image(img);
	enhance(enhanced,10,10);
	minlist = findMinutia(enhanced,direcs,10,10);
	minlist = posprocessing(enhanced,minlist,20,20);
	minlist = crownCore(img,minlist);
	getDistances(minlist);
	orderMinutia(minlist);
	showMinutia(enhanced,minlist,10,10);cvWaitKey(0);
	printf("%s\n",argv[1]);
	radialextraction(img, minlist);
	printf("\n\n");
	return 0;
}
