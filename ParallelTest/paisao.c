/*
 * Universidade de Brasilia - UnB
 * Instituto de Ciencias Exatas - IE
 * Departamento de Ciencia da Computacao - CIC
 * 
 * Sistemas Operacionais
 * Professora: Alba
 * 
 * Aluno: Igor de Carvalho Coelho
 * Matricula: 06/20068
 * SO: Linux, 2.6.28.16-generic
 * Compilador: gcc-4.3.3
 * Nº de executaveis: 5
 * Ordem de lancamento dos processos:
 * 		1-paisao
 * 		2-clientes(16 deles) gerados pelo paisao
 * 		3-gen_in,gen_out e gen_print. todos gerados pelo paisao tammem
 * Mecanismos IPC utilizados:
 *  Filas de mensagens e sinais.
 *  Usei elas para poder simular os fios existentes entre os clientes e essa estrutura não é blocante.
 *  No README ha uma explicação mais detalhada sobre a organizaçao. 
 *  Os sinais são usados como uma maneira assincrona de mandar os clientes começarem a mandar mensagens.
 *  Ele é usado em 2 situação:
 * 		1º-Para retirar os clientes e os servidores do sleep depois do paisao ter criado as filas
 * 		2º-Dizer para uns dados clientes para mandar mensagens
 */
//bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

//pid dos filhos e do servidor
int nos[16];
int impress[3];
//filas para os contatos
int filas[25][2];
int fila_servidor;

typedef struct filamsg{
	int pid;
	struct filamsg *prox;
}filamsg;

//funcao para fechar as estruturas e matar os filhos
void matatudo(){
	printf("Paisão: Vou ter que matar meus filhos...\n");
	int i,resp;
	for(i=0;i<16;i++){//mata todos os clientes
		kill(nos[i],SIGTERM);
	}
	for(i=0;i<3;i++){//mata todos os servidores
		kill(impress[i],SIGTERM);
	}
	for(i=0;i<25;i++){//se desfaz das filas
		resp = msgctl(filas[i][0],IPC_RMID, NULL);
		if(resp<0){
			printf("Erro na eliminação da fila\n");
		}
		resp = msgctl(filas[i][1],IPC_RMID, NULL);
		if(resp<0){
			printf("Erro na eliminação da fila\n");
		}
	}
	resp = msgctl(fila_servidor, IPC_RMID, NULL);
	if(resp<0){
		printf("Erro na eliminação da fila\n");
	}
	printf("\nPaisão: Todos os filhos morreram... a vida é injusta por isso vou me matar\n");
	kill(getpid(),9); //suicício
}
//funcao que dis as clientes para mandar mensages(a mensagem é estatica e só pode ser mudada no codigo)
void mandarMensagens(int quantos){
	filamsg *mfila,*aux,*aux2;
	mfila = NULL;
	int i=0;
	int ncliente;
	while(quantos-->0){
		printf("%dº cliente a mandar: ",i++);
		scanf("%d",&ncliente); //assumindo que sempre entra com um valor entre 0-15, posso por uma verificação depois
		if(mfila == NULL){
			mfila = (filamsg*)malloc(sizeof(filamsg));
			mfila->prox = NULL;
			mfila->pid = nos[ncliente];
		}else{
			aux = (filamsg*)malloc(sizeof(filamsg));
			aux->prox = NULL;
			aux->pid = nos[ncliente];
			aux2 = mfila;
			while(aux2->prox != NULL) aux2 = aux2->prox;
			aux2->prox = aux;
		}
	}
	while(mfila != NULL){
		kill(mfila->pid,SIGUSR2);
		aux = mfila;
		mfila = mfila->prox;
		free(aux);
	}
}

int main(){
	//declaração de variaveis
	int i,resp,idmem;
	char numero[5] = "0000\0";
	char idmems[10];
	signal(SIGTERM,matatudo);
	//laco para criar os "caminhos" de ida e volta de mensagens
	for(i=0;i<25;i++){
		if((filas[i][0] = msgget(0x1000+i, IPC_CREAT|0666))<0){
			printf("Paisão: Erro na criação das filas, abortando...\n");
			matatudo();
		}
		if((filas[i][1] = msgget(0x2000+i, IPC_CREAT|0666))<0){
			printf("Paisão: Erro na criação das filas, abortando...\n");
			matatudo();
		}
	}
	if((fila_servidor = msgget(0x3000, IPC_CREAT|0666))<0){
		printf("Paisão: Erro na criação das filas, abortando...\n");
		matatudo();
	}
	//criando os processos
	for(i=0;i<16;i++){
		nos[i] = fork();
		if(nos[i]==0){ //se for filho executa seu codigo correspondente mandando qual seu numero como parametro
			numero[3] = i+'0';
			resp=execl("./cliente","cliente\0",numero,(char*) 0);
			if(resp<0){
				printf("\nPaisão: Cliente %s não pode ser criado, tentando denovo.\n",numero);
				i--;
			}
		}else if(nos[i]<0){
			printf("\nPaisão: Não conseguiu fazer o fork do cliente %s, tentando denovo.\n",numero);
			i--;
		}
	}
	impress[0] = fork();
	if(impress[0] == 0){//cria os servidores de impressão
		resp=execl("./gen_in","gen_in\0",(char*) 0);
		if(resp<0){
			printf("\nPaisão: O Servidor de entrada não pode ser criado.\n");
			matatudo();
		}
	}
	impress[1] = fork();
	if(impress[1] == 0){//cria os servidores de impressão
		resp=execl("./gen_print","gen_print\0",(char*) 0);
		if(resp<0){
			printf("\nPaisão: O Servidor de impressao não pode ser criado.\n");
			matatudo();
		}
	}
	impress[2] = fork();
	if(impress[2] == 0){//cria os servidores de impressão
		resp=execl("./gen_out","gen_out\0",(char*) 0);
		if(resp<0){
			printf("\nPaisão: O Servidor saida não pode ser criado.\n");
			matatudo();
		}
	}
	//acorda os processos filhos para que eles possam se ligar
	for(i=0;i<16;i++){
		kill(nos[i],SIGUSR1);
	}
	for(i=0;i<3;i++){
		kill(impress[i],SIGUSR1);
	}
	//interface com o usuário
	int quantos = 0;
	while(1){
		printf("Quantas mensagens serão enviadas\n");
		scanf("%d",&quantos);
		if(quantos>0)
			mandarMensagens(quantos);
		else{
			matatudo();
			break;
		}
	}
	return 0;
}


