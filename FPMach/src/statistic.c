#include <image.h>
#include <enhance.h>
#include <minutia.h>
#include <matching.h>
#include <crypto.h>
#include <sys/time.h>


int main(int argc, char **argv){
	struct timeval init, end;
	double timePenal, timeSpiral;
	image *template,*teste, *aux, *aux2;
	minutia *temp,*test;
	double *tempDirecs=NULL, *testDirecs=NULL;
	int penality,nphotos,i,k	;
	int matched[2],notmatched[2];
	if(argc<2){
		printf("%s <flag> <lista das imagens>\n", argv[0]);
		printf("-p : para testar falsos-positivos\n-f : para testar falsos negativos\n");
		printf("para o teste de falso positivo a primeira imagem será a de template\n");
		printf("para o teste de falso negativo todas as imagens da lista serão testadas 2 a 2\n");
		exit(0);
	}
	nphotos = argc - 2;
	matched[0] = matched[1] = notmatched[0] = notmatched[1] = 0;
	timePenal = timeSpiral = 0.0;
	if(argv[1][1] == 'p'){
		i = nphotos-1;
		while(i>=2){
			template = loadPGM(argv[i]);
			if(template == NULL){
				printf("erro carregando foto template: %s\n",argv[i]);
				i--;
				continue;
			}
			show(template,0,0,template->width,template->height,"Template");
			if(tempDirecs == NULL){
				tempDirecs = (double*) malloc(sizeof(double)*template->width/10*template->height/10);
			}
			aux = clone_image(template);
			direction(template,tempDirecs,10,10);
			if(enhance(template,10,10)==FAILURE){
				printf("Enhance do template %s deu errado\n",argv[i]);
				i--;
				continue;
			}
			k=i-1;
			i--;
			while(k >= 2){
				temp = findMinutia(template,tempDirecs,10,10);
				temp = posprocessing(template,temp,10,10);
				temp = crownCore(aux,temp);
				getDistances(temp);
				orderMinutia(temp);
				teste = loadPGM(argv[k]);
				k--;
				if(teste == NULL){
					printf("erro carregando foto %s\n",argv[k]);
					continue;
				}
				show(teste,0,0,teste->width,teste->height,"Teste");
				if(testDirecs == NULL){
					testDirecs = (double*) malloc(sizeof(double)*teste->width/10*teste->height/10);
				}
				aux2 = clone_image(teste);
				direction(teste,testDirecs,10,10);
				if(enhance(teste,10,10)==FAILURE){
					printf("Erro no melhoramento da %s\n",argv[k]);
					continue;
				}
				test = findMinutia(teste,testDirecs,10,10);
				test = posprocessing(teste,test,10,10);
				test = crownCore(aux2,test);
				getDistances(test);

				gettimeofday(&init,NULL);
				penality = validadeFP(temp,test);
				if(penality == FINGER_MATCH){
					matched[0]++;
				}else{
					notmatched[0]++;
				}
				gettimeofday(&end,NULL);
				if(timePenal == 0.0){
					timePenal = (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
				}else{
					timePenal += (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
					timePenal /= 2.0;
				}

				gettimeofday(&init,NULL);
				if(spiralMatching(temp, test, 0, teste->width, teste->height)==FINGER_MATCH){
					matched[1]++;
				}else{
					notmatched[1]++;
				}
				gettimeofday(&end,NULL);
				if(timeSpiral == 0.0){
					timeSpiral = (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
				}else{
					timeSpiral += (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
					timeSpiral /= 2.0;
				}

				free_image(teste);
				free_image(aux2);
				freeMinutiaList(temp);
				freeMinutiaList(test);
				temp =  test = NULL;
				aux2 = teste = NULL;
			}
			free_image(template);
			free_image(aux);
			template = aux = NULL;
		}
		printf("Tecnica das penalidades:\nHouveram \t%d matches\ne \t%d notmaches\nEntão a porcentagem de falso-positivo foi: %f\n\n",matched[0],notmatched[0],(float)matched[0]/(float)(matched[0]+notmatched[0]));
		printf("Matching em espiral:\nHouveram \t%d matches\ne \t%d notmaches\nEntão a porcentagem de falso-positivo foi: %f\n\n",matched[1],notmatched[1],(float)matched[1]/(float)(matched[1]+notmatched[1]));

		printf("Tempo em media:\nPenalidades: %lf\nEspiral: %lf\n\n",timePenal, timeSpiral);
	}else if(argv[1][1] == 'f'){
		while(nphotos > 2){
			template = loadPGM(argv[nphotos-1]);
			if(template == NULL) printf("erro carregando foto %s\n",argv[3]);
			show(template,0,0,template->width,template->height,"Template");
			if(tempDirecs == NULL){
				tempDirecs = (double*) malloc(sizeof(double)*template->width/10*template->height/10);
			}
			aux = clone_image(template);
			direction(template,tempDirecs,10,10);
			if(enhance(template,10,10)==FAILURE){
				nphotos--;
				printf("Erro no melhoramento da imagem de template: %s\n",argv[nphotos]);
				continue;
			}
			i = nphotos - 2;
			while(i>=2){
				temp = findMinutia(template,tempDirecs,10,10);
				temp = posprocessing(template,temp,10,10);
				temp = crownCore(aux,temp);
				getDistances(temp);
				orderMinutia(temp);

				teste = loadPGM(argv[i]);
				i--;
				if(teste == NULL) printf("erro carregando foto %s\n",argv[nphotos]);;
				show(teste,0,0,teste->width,teste->height,"Teste");
				if(testDirecs == NULL){
					testDirecs = (double*) malloc(sizeof(double)*teste->width/10*teste->height/10);
				}
				aux2 = clone_image(teste);
				direction(teste,testDirecs,10,10);
				if(enhance(teste,10,10)==FAILURE){
					printf("Erro no melhoramento da %s\n",argv[i+1]);
					continue;
				}
				test = findMinutia(teste,testDirecs,10,10);
				test = posprocessing(teste,test,10,10);
				test = crownCore(aux2,test);
				getDistances(test);

				gettimeofday(&init,NULL);
				penality = validadeFP(temp,test);
				if(penality == FINGER_MATCH){
					matched[0]++;
				}else{
					notmatched[0]++;
				}
				gettimeofday(&end,NULL);
				if(timePenal == 0.0){
					timePenal = (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
				}else{
					timePenal += (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
					timePenal /= 2.0;
				}

				gettimeofday(&init,NULL);
				if(spiralMatching(temp, test, 0, teste->width, teste->height)==FINGER_MATCH){
					matched[1]++;
				}else{
					notmatched[1]++;
				}
				gettimeofday(&end,NULL);
				if(timeSpiral == 0.0){
					timeSpiral = (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
				}else{
					timeSpiral += (double)(end.tv_sec-init.tv_sec)*1000000.0+(double)(end.tv_usec-init.tv_usec);
					timeSpiral /= 2.0;
				}

				free_image(teste);
				free_image(aux2);
				freeMinutiaList(temp);
				freeMinutiaList(test);
				temp = test = NULL;
			}
			free_image(template);
			free_image(aux);
			nphotos--;
		}
		printf("Tecnica das penalidades:\nHouveram \t%d matches\ne \t%d notmaches\nEntão a porcentagem de falso-negativos foi: %f\n\n",matched[0],notmatched[0],(float)notmatched[0]/(float)(matched[0]+notmatched[0]));
		printf("Matching em espiral:\nHouveram \t%d matches\ne \t%d notmaches\nEntão a porcentagem de falso-negativos foi: %f\n\n",matched[1],notmatched[1],(float)notmatched[1]/(float)(matched[1]+notmatched[1]));
		printf("Tempo em media:\nPenalidades: %lf\nEspiral: %lf\n\n",timePenal, timeSpiral);
	}
	return 0;
}
