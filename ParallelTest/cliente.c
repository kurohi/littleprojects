
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
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/msg.h>

#include"mensagem.h"

mensagem msgEncaminha, msgManda;
int mfilas[4][2];
int QuemSou;
int meuPid, nmensagem, enviar;

//so pra pegar o numero em uma dada string
int getNumber(char *numero){
	int i, num = 0;
	for(i=0;i<strlen(numero);i++){
		num *=10;
		num += numero[i] - '0';
	}
	return num;
}

//manda mensagem(pode ser encaminhar ou a propria mesnagem)
void mandaMensagem(){
	int resp=-1;
	if(mfilas[1][1]!=-1){
		resp = msgsnd(mfilas[1][1], &msgManda, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
		if(resp<0){
			printf("Cliente %d: Não consegui mandar a mensagem\n",QuemSou);
		}
	}if((mfilas[0][1]!=-1)&&(resp<0)){
		resp = msgsnd(mfilas[0][1], &msgManda, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
		if(resp<0){
			printf("Cliente %d: Não consegui mandar a mensagem\n",QuemSou);
		}
	}
}

//set flag para enviar sua propria mensagem
void tratarMandaMensagem(){
	enviar = 1;
}
//manda requisicao
void mandaRequisicao(){
	char permicoes = 0;
	permicoes = (QuemSou<<4)| 1;//ajeita o cabecalho e manda a mensagem
	msgManda.mensagem[0] = permicoes;
	msgManda.pid = meuPid;
	mandaMensagem();
}
//depois de receber a resposta manda os dados
void mandaDados(){
	char string[100];
	sprintf(string," Sou o Cliente %d de pid %d e essa é a mensagem %d.",QuemSou, meuPid, nmensagem);
	strcpy(msgManda.mensagem, string);
	char permicoes = 0;
	permicoes = (QuemSou<<4)| 4;
	msgManda.mensagem[0] = permicoes;
	msgManda.pid = meuPid;
	mandaMensagem();
}
//quando acabar manda a mensagem de termino
void mandaTermino(){
	char permicoes = 0;
	permicoes = (QuemSou<<4)| 7;
	msgManda.mensagem[0] = permicoes;
	mandaMensagem();
}
//auto explicativa...
void faznada(){}
//trata a mensagem recebida
int tratarmsg(mensagem msgRec){
	int tipo = (msgRec.mensagem[0] & 0x6)>>1;
	switch(tipo){
		case 0:	printf("Cliente %d: Erro, requisição errada.\n",QuemSou);
				return 1;//se for 0 erro pq cliente nao recebe requisicao
		case 1:	if(msgRec.mensagem[1]){//se for 1 eh a resposta, se for positiva manda os dados senao tenta mandar a requisicao denovo
					for(nmensagem=0;nmensagem<NMENSAGEM;nmensagem++)
						mandaDados();
					mandaTermino();
				}else{
					return 2;
				}break;
				//nos outros casos os erros explicam
		case 2:	printf("Cliente %d: Erro, recebi dados de mensagem...\n",QuemSou);
				return 1;
		case 3:	printf("Cliente %d: Erro, recebi termino de dados.\n",QuemSou);
				return 1;
		default:	printf("Cliente %d: Erro, tipo desconhecido.\n",QuemSou);
				return 1;
	}
	return 0;
}
//funcao de roteamento
int passarAdiante(){
	int resp,i,paraquem;
	if(mfilas[0][0]!=-1){//se pegar dessa conexao, testa se eh sua, depois trata ou passa adiante
		resp = msgrcv(mfilas[0][0], &msgEncaminha, sizeof(mensagem), 0, IPC_NOWAIT);
		if(resp>0){
			paraquem = (msgEncaminha.mensagem[0]&0xF0)>>4;
			if(paraquem == QuemSou){
				if(tratarmsg(msgEncaminha)==2) 
					return 2;
			}else{//se for maior eh porque esta embaixo na Mesh senao passa para esquerda
				if(paraquem > QuemSou){
					resp = msgsnd(mfilas[2][0], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
				}else{
					resp = msgsnd(mfilas[3][0], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
				}
			}
		}
	}//se tiver essa conexao, segue o mesmo esquema da de cima
	if(mfilas[1][0]!=-1){
		resp = msgrcv(mfilas[1][0], &msgEncaminha, sizeof(mensagem), 0, IPC_NOWAIT);
		if(resp>0){
			paraquem = (msgEncaminha.mensagem[0]&0xF0)>>4;
			if(paraquem == QuemSou){
				if(tratarmsg(msgEncaminha)==2) 
					return 2;
			}else{
				if(paraquem > QuemSou){
					resp = msgsnd(mfilas[2][0], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
				}else{
					resp = msgsnd(mfilas[3][0], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
				}
			}
		}
	}//se receber por essa apenas passa adiante pela direita ou por cima caso a 1º de errado
	if(mfilas[2][1]!=-1){
		resp = msgrcv(mfilas[2][1], &msgEncaminha, sizeof(mensagem), 0, IPC_NOWAIT);
		if(resp>0){
			resp=-1;
			if(mfilas[1][1]!=-1){
				resp = msgsnd(mfilas[1][1], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
			}if((mfilas[0][1]!=-1)&&(resp<0)){
				resp = msgsnd(mfilas[0][1], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
			}
		}
	}//idem com a de cima
	if(mfilas[3][1]!=-1){
		resp = msgrcv(mfilas[3][1], &msgEncaminha, sizeof(mensagem), 0, IPC_NOWAIT);
		if(resp>0){
			resp=-1;
			if(mfilas[1][1]!=-1){
				resp = msgsnd(mfilas[1][1], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
			}if((mfilas[0][1]!=-1)&&(resp<0)){
				resp = msgsnd(mfilas[0][1], &msgEncaminha, sizeof(mensagem)-sizeof(long), IPC_NOWAIT);
			}
		}
	}
	return 0;
}



int main(int argc, char **argv){
	QuemSou = getNumber(argv[1]);
	meuPid = getpid();
	enviar = 0;
	int i,resp;
	signal(SIGUSR1,faznada);
	signal(SIGUSR2,tratarMandaMensagem);
	sleep(10);//espera  o paisao acabar de fazer as filas
	switch(QuemSou){//pega as suas respectivas conexoes de acordo com qual cliente é
		case 0:	mfilas[0][0] = -1;
				mfilas[0][1] = -1;
				mfilas[1][0] =msgget(0x1000+0,0666);
				mfilas[1][1] =msgget(0x2000+0,0666);
				mfilas[2][0] =msgget(0x1000+1,0666);
				mfilas[2][1] =msgget(0x2000+1,0666);
				mfilas[3][0] = -1;
				mfilas[3][1] = -1;
				break;
		case 1:	mfilas[0][0] =msgget(0x1000+1,0666);
				mfilas[0][1] =msgget(0x2000+1,0666);
				mfilas[1][0] =msgget(0x1000+2,0666);
				mfilas[1][1] =msgget(0x2000+2,0666);
				mfilas[2][0] =msgget(0x1000+3,0666);
				mfilas[2][1] =msgget(0x2000+3,0666);
				mfilas[3][0] = -1;
				mfilas[3][1] = -1;
				break;
		case 2:	mfilas[0][0] =msgget(0x1000+3,0666);
				mfilas[0][1] =msgget(0x2000+3,0666);
				mfilas[1][0] =msgget(0x1000+4,0666);
				mfilas[1][1] =msgget(0x2000+4,0666);
				mfilas[2][0] =msgget(0x1000+5,0666);
				mfilas[2][1] =msgget(0x2000+5,0666);
				mfilas[3][0] = -1;
				mfilas[3][1] = -1;
				break;
		case 3:	mfilas[0][0] =msgget(0x1000+5,0666);
				mfilas[0][1] =msgget(0x2000+5,0666);
				mfilas[1][0] =msgget(0x1000+6,0666);
				mfilas[1][1] =msgget(0x2000+6,0666);
				mfilas[2][0] = -1;
				mfilas[2][1] = -1;
				mfilas[3][0] = -1;
				mfilas[3][1] = -1;
				break;
		case 4:	mfilas[0][0] = -1;
				mfilas[0][1] = -1;
				mfilas[1][0] =msgget(0x1000+7,0666);
				mfilas[1][1] =msgget(0x2000+7,0666);
				mfilas[2][0] =msgget(0x1000+8,0666);
				mfilas[2][1] =msgget(0x2000+8,0666);
				mfilas[3][0] =msgget(0x1000+0,0666);
				mfilas[3][1] =msgget(0x2000+0,0666);
				break;
		case 5:	mfilas[0][0] =msgget(0x1000+8,0666);
				mfilas[0][1] =msgget(0x2000+8,0666);
				mfilas[1][0] =msgget(0x1000+9,0666);
				mfilas[1][1] =msgget(0x2000+9,0666);
				mfilas[2][0] =msgget(0x1000+10,0666);
				mfilas[2][1] =msgget(0x2000+10,0666);
				mfilas[3][0] =msgget(0x1000+2,0666);
				mfilas[3][1] =msgget(0x2000+2,0666);
				break;
		case 6:	mfilas[0][0] =msgget(0x1000+10,0666);
				mfilas[0][1] =msgget(0x2000+10,0666);
				mfilas[1][0] =msgget(0x1000+11,0666);
				mfilas[1][1] =msgget(0x2000+11,0666);
				mfilas[2][0] =msgget(0x1000+12,0666);
				mfilas[2][1] =msgget(0x2000+12,0666);
				mfilas[3][0] =msgget(0x1000+4,0666);
				mfilas[3][1] =msgget(0x2000+4,0666);
				break;
		case 7:	mfilas[0][0] =msgget(0x1000+12,0666);
				mfilas[0][1] =msgget(0x2000+12,0666);
				mfilas[1][0] =msgget(0x1000+13,0666);
				mfilas[1][1] =msgget(0x2000+13,0666);
				mfilas[2][0] = -1;
				mfilas[2][1] = -1;
				mfilas[3][0] =msgget(0x1000+6,0666);
				mfilas[3][1] =msgget(0x2000+6,0666);
				break;
		case 8:	mfilas[0][0] = -1;
				mfilas[0][1] = -1;
				mfilas[1][0] =msgget(0x1000+14,0666);
				mfilas[1][1] =msgget(0x2000+14,0666);
				mfilas[2][0] =msgget(0x1000+15,0666);
				mfilas[2][1] =msgget(0x2000+15,0666);
				mfilas[3][0] =msgget(0x1000+7,0666);
				mfilas[3][1] =msgget(0x2000+7,0666);
				break;
		case 9:	mfilas[0][0] =msgget(0x1000+15,0666);
				mfilas[0][1] =msgget(0x2000+15,0666);
				mfilas[1][0] =msgget(0x1000+16,0666);
				mfilas[1][1] =msgget(0x2000+16,0666);
				mfilas[2][0] =msgget(0x1000+17,0666);
				mfilas[2][1] =msgget(0x2000+17,0666);
				mfilas[3][0] =msgget(0x1000+9,0666);
				mfilas[3][1] =msgget(0x2000+9,0666);
				break;
		case 10:	mfilas[0][0] =msgget(0x1000+17,0666);
				mfilas[0][1] =msgget(0x2000+17,0666);
				mfilas[1][0] =msgget(0x1000+18,0666);
				mfilas[1][1] =msgget(0x2000+18,0666);
				mfilas[2][0] =msgget(0x1000+19,0666);
				mfilas[2][1] =msgget(0x2000+19,0666);
				mfilas[3][0] =msgget(0x1000+11,0666);
				mfilas[3][1] =msgget(0x2000+11,0666);
				break;
		case 11:	mfilas[0][0] =msgget(0x1000+19,0666);
				mfilas[0][1] =msgget(0x2000+19,0666);
				mfilas[1][0] =msgget(0x1000+20,0666);
				mfilas[1][1] =msgget(0x2000+20,0666);
				mfilas[2][0] = -1;
				mfilas[2][1] = -1;
				mfilas[3][0] =msgget(0x1000+13,0666);
				mfilas[3][1] =msgget(0x2000+13,0666);
				break;
		case 12:	mfilas[0][0] = -1;
				mfilas[0][1] = -1;
				mfilas[1][0] =msgget(0x1000+24,0666);
				mfilas[1][1] =msgget(0x2000+24,0666);
				mfilas[2][0] =msgget(0x1000+21,0666);
				mfilas[2][1] =msgget(0x2000+21,0666);
				mfilas[3][0] =msgget(0x1000+14,0666);
				mfilas[3][1] =msgget(0x2000+14,0666);
				break;
		case 13:	mfilas[0][0] =msgget(0x1000+21,0666);
				mfilas[0][1] =msgget(0x2000+21,0666);
				mfilas[1][0] = -1;
				mfilas[1][1] = -1;
				mfilas[2][0] =msgget(0x1000+22,0666);
				mfilas[2][1] =msgget(0x2000+22,0666);
				mfilas[3][0] =msgget(0x1000+16,0666);
				mfilas[3][1] =msgget(0x2000+16,0666);
				break;
		case 14:	mfilas[0][0] =msgget(0x1000+22,0666);
				mfilas[0][1] =msgget(0x2000+22,0666);
				mfilas[1][0] = -1;
				mfilas[1][1] = -1;
				mfilas[2][0] =msgget(0x1000+23,0666);
				mfilas[2][1] =msgget(0x2000+23,0666);
				mfilas[3][0] =msgget(0x1000+18,0666);
				mfilas[3][1] =msgget(0x2000+18,0666);
				break;
		case 15:	mfilas[0][0] =msgget(0x1000+23,0666);
				mfilas[0][1] =msgget(0x2000+23,0666);
				mfilas[1][0] = -1;
				mfilas[1][1] = -1;
				mfilas[2][0] = -1;
				mfilas[2][1] = -1;
				mfilas[3][0] =msgget(0x1000+20,0666);
				mfilas[3][1] =msgget(0x2000+20,0666);
				break;
		default:	printf("Cliente %d:Não sei quem eu sou...:%d\n",QuemSou,QuemSou);
				exit(1);
	}
	while(1){//loop principal
		//sleep(1); //coloquei isso só para consomir menos cpu e fazer testes, mas nao eh necessario
		resp = passarAdiante();//pega o que tiver na fila e trata ou passa adiante
		while(resp == 2){//se tratou, era uma resposta e era negativa, manda a requisicao denovo
			mandaRequisicao();
			resp = passarAdiante();
		}//se o paisao disse para enviar...
		if(enviar){
			mandaRequisicao();
			enviar = 0;
		}
	}
	return 0;
}


