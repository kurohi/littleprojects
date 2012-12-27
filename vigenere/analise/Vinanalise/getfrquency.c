#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	if(argc!=3){
		printf("Usar: %s <arquivo na linguagem> <arquivo de saida>\n",argv[0]);
		return 1;
	}
	long charcount[256];
	long total;
	char byte;
	int i;
	FILE *arq;
	if((arq=fopen(argv[1],"r"))==NULL){
		printf("Não pode abrir o arquivo de entrada\n");
		return 1;
	}
	for(i=0; i<256; i++){
		charcount[i] = 0;
	}
	while(fread(&byte,1,1,arq)){
		if(byte != '\n'){
			charcount[byte]++;
			total++;
			if(!(total%100000)){
				printf("Total até agora: %ld\n",total);
			}
		}
	}
	fclose(arq);
	if((arq=fopen(argv[2],"w+"))==NULL){
		printf("Não pode abrir o arquivo de saida\n");
		return 1;
	}	
	for(i=0; i<256; i++){
		fprintf(arq,"%f\n",(float) charcount[i]/(float)total);
	}
	fclose(arq);
	return 0;
}
