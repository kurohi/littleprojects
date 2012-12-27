#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//função que conta as ocorrencias iguais no texto encriptado
//Se fosse usar o vetor invertido iria ter que percorrer o loop do mesmo jeito, então preferi fazer assim
int count_pair(char* string, int keysize, int criptsize){
	int i,pair=0;
	for(i=0;i<criptsize;i++){
		if(string[i] == string[(i+keysize)%criptsize]) pair++;
	}
	return pair;
}

//função que pega o texto encriptado através do arquivo de texto do professo, só que editado para conter só os numeros
//precisa salvar o temanho pq caso encontre o caractere 0 não dá mais pra usar strlen()
char *makestring(char *filename, int* size){
	FILE *arq;
	if((arq = fopen(filename,"r"))==NULL) return NULL;
	int i=0;
	char byte;
	while(fread(&byte,1,1,arq)){
		if(byte == '\n') i++;
	}
	unsigned char *string = (unsigned char*) malloc(sizeof(char)*i+1);
	fseek(arq,0,SEEK_SET);
	i=0;
	while(fscanf(arq,"%d\n",&(string[i++]))!=EOF);
	*size = i-1;
	fclose(arq);
	return string;
}
//funçaõ que caucula a frequencia deos caracteres em cada posição da chave para cada caractere
unsigned char *character_frequency(unsigned char *string, int keysize, int criptsize){
	unsigned char *charcount = (unsigned char*) malloc(sizeof(char)*keysize*256);	
	int totalrep=0,i,j;
	for(i=0;i<keysize;i++)
		for(j=0;j<256;j++)
			charcount[i + j*keysize] = 0;
	for(i=0;i<keysize;i++){
		for(j=i;j<criptsize;j+=keysize){
			//usa o próprio caractere como indice
			charcount[i + string[j]*keysize]++;
		}
	}

	return charcount;
}
//pega a frequencia da lingua utilizada através de um arquivo que só tem float, o programa getfrquency.c
//faz esse arquivo a partir de um textão no idioma
//esse arquivo cria uma lista dos caracteres mais usados em um vetor de char, maxletters é o numero máximo de caracteres nesse vetor
char *language_frequency(char *filename, int maxletters){
	FILE *arq;
	if((arq = fopen(filename,"r"))==NULL) return NULL;
	int i,j;
	char *c = (char*) malloc(sizeof(char)*maxletters);
	float freq;
	float *lanfreq = (float*) malloc(sizeof(float)*256);
	i=0;
	while(fscanf(arq,"%f\n",&(lanfreq[i++]))!=EOF);
	fclose(arq);
	for(i=0;i<maxletters;i++){
		freq=0.0;
		for(j=0;j<256;j++){
			if(lanfreq[j]>freq){
				freq = lanfreq[j];
				c[i] = j;
			}
		}
		lanfreq[c[i]] = 0.0;
	}
	return c;
}

//aqui se faz a primerira adivinhação que qual é a chave, fazendo um xor do caractere mais frequente na posição
//com o mais n-frequente na lingua, que estará na variavel langfreq
void getKey(char *charfreq,char langfreq, int keysize, char *key){
	int keypos, ch, ch2, ingreat;
	char great;
	for(keypos=0; keypos<keysize; keypos++){
		great=0.0;
		for(ch=0; ch<256; ch++){
			if(great < charfreq[keypos + ch*keysize]){
				great = charfreq[keypos + ch*keysize];
				ingreat = ch;
			}
		}
		key[keypos] = ingreat^langfreq;
	}
}
//aki voce pode mudar apenas um caractere na chave
get1Key(unsigned char *charfreq,char langfreq, int keysize, char *key, int letter){
	int keypos, ch, ch2, ingreat;
	float great;
	great=0.0;
	for(ch=0; ch<256; ch++){
		if(great < charfreq[letter + ch*keysize]){
			great = charfreq[letter + ch*keysize];
			ingreat = ch;
		}
	}
	key[letter] = ingreat^langfreq;
}

//tendo a chave usa-se essa função para decriptar amount caracteres do texto,
//bom para ficar testando mudanças na chave
void uncript(char *cripted, char *uncripted, char *key, int keysize, int amount, int criptsize){
	int i,j;
	if(amount>0) uncripted[amount] = 0;
	for(i=0;(amount>0)&&(i<criptsize);){
		for(j=0;j<keysize;j++){
			uncripted[i] = cripted[i]^key[j];
			amount--;
			i++;
		}
	}
}
//função main \o/
int main(int argc, char **argv){
	if(argc != 4){
		printf("Modo de usar: %s <endereço do arquivo> <tamanho de chave máximo> <arqivo com as frequencias de cada caractere>\n",argv[0]);
		return 1;
	}
	//declaração de variaveis
	int i, maxkeysize, greater=0, indexgreater, criptsize, charnum=0, letter;
	char saida='n';
	FILE *arqsaida;
	unsigned char *string = makestring(argv[1], &criptsize);
	sscanf(argv[2], "%d", &maxkeysize);//pega o tamanho maximo para procurar a chave
	int freqs[maxkeysize];//vetor local para as frequencias
	for(i=1; i<maxkeysize; i++){
		//vai tentando até o máximo a procurar e depois pega o nº com o maior numero de ocorrencias iguais
		freqs[i] = count_pair(string, i, criptsize);
		if(greater<freqs[i]){
			greater = freqs[i];
			indexgreater = i;
		}
	}
	printf("O tamanho da chave é %d\n",indexgreater);
	//pega a frequencia dos caracteres
	unsigned char *charfreq = character_frequency(string, indexgreater,criptsize);
	//pega a frequencia dos caracteres do idioma
	char *langfreq = language_frequency(argv[3],10);
	char key[indexgreater+1];	//chave
	char uncripted[criptsize+1];	//texto decriptado
	//tenta pegar a chave a 1ª vez
	getKey(charfreq, langfreq[charnum], indexgreater, key);
	//decripta só 2x a chave para ver se já acertou tudo
	uncript(string, uncripted, key, indexgreater, indexgreater*2, criptsize);
	printf("A chave encontrada é: %s\n",key);
	printf("%s\n",uncripted);
	while(saida!='s'){
		printf("O texto é legivel? ");
		scanf("\n%c",&saida);
		if(saida == 'n'){//menuzinho para a interação e modificação da chave(auto-explicativo)
			printf("Digite\n\t1-trocar um caractere pelo proximo mais frequente\n");
			printf("\t2-digitar a senha toda\n");
			printf("\t3-sair\n:");
			scanf("\n%c",&saida);
			switch(saida){
				case '1':	//-------------------------------
						//IMPORTANTE: a contagem aki começa do 0
						//-------------------------------
						printf("Qual letra?  ");
						scanf("\n%d",&letter);
						printf("Qual posição de frequencia(0 é o mais frequente)?  ");
						scanf("\n%d",&charnum);
						//muda só um caractere da chave
						get1Key(charfreq, langfreq[charnum], indexgreater, key, letter);
						printf("A chave nova encontrada é: %s\n",key);
						break;
				case '2':	printf("Digite a nova senha (%d caracteres): ",indexgreater);
						scanf("\n%s",key);
						break;
			}//texta a nova chave
			uncript(string, uncripted, key, indexgreater, indexgreater*2, criptsize);
			printf("%s\n",uncripted);
		}
	}
	//decripta todo o texto
	uncript(string, uncripted, key, indexgreater, criptsize, criptsize);
	uncripted[criptsize]=0;
	printf("--------------------\ntexto decriptado:\n\n%s\n",uncripted); //imprimi o dito-cujo
	if((arqsaida=fopen("decripted.txt","w+"))==NULL){
		printf("Não foi possivel salvar no arquivo de saida\n");
		free(langfreq);
		free(charfreq);
		free(string);
		return 1;
	}
	free(langfreq);
	free(charfreq);
	free(string);
	fprintf(arqsaida,"A chave eh %s\nE o texto decriptado eh:\n\n%s\n",key,uncripted);
	//salva e pronto, agora seja feliz com o seu texto decriptado
	return 0;
}
//and good Hacking

