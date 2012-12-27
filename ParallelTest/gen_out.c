
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "mensagem.h"

int fila_serv;
mensagem msg;

void faznada(){}

//procura um espaco vazio no buffer e coloca o cliente, se o buffer estiver cheio retorna 0
int searchPut(int *clientes, int cli){
	int i;
	for(i=0;i<BUFFER;i++){
		if((clientes[i] == -1)||(clientes[i] == cli)){
			clientes[i] = cli;
			return 1;
		}
	}
	return 0;
}
//testa se um dado cliente esta no buffer
int searchCli(int *clientes, int cli){
	int i;
	for(i=0;i<BUFFER;i++){
		if(clientes[i] == cli){
			return 1;
		}
	}
	return 0;
}
//retira um cliente do buffer
int removeCli(int *clientes, int cli){
	int i;
	for(i=0;i<BUFFER;i++){
		if(clientes[i] == cli){
			clientes[i] = -1;
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv){
	int resp, meupid, nclientes, tipo, qualCliente;
	int clientes[BUFFER];
	for(resp=0; resp<BUFFER; resp++){
		clientes[resp] = -1;
	}
	signal(SIGUSR1,faznada);
	meupid = getpid();
	sleep(10);//espera as filas ficarem prontas
	fila_serv = msgget(0x3000, 0666);
	while(1){//se a mensagem nao for para ele retorna ela para a fila
		resp = msgrcv(fila_serv, &msg, sizeof(mensagem), 0, 0);
		if(((msg.mensagem[0]&0x8)==0)||(msg.mensagem[1]!=2)){
			resp = msgsnd(fila_serv, &msg, sizeof(mensagem)-sizeof(long),0);
		}else{
			tipo = (msg.mensagem[0]&0x6)>>1;	//pega o tipo da mensagem
			qualCliente = (msg.mensagem[0]&0xF0)>>4;
			if(tipo == 0){//se for requisicao
				msg.pid = meupid;
				msg.mensagem[1] = 0;
				if(searchPut(clientes, qualCliente)){
					msg.mensagem[0] = 0x9;
				}else{
					msg.mensagem[0] = 0x8;
				}
				resp = msgsnd(fila_serv, &msg, sizeof(mensagem)-sizeof(long),0);
			}else if(tipo == 1){//se for resposta(nao era pra receber isso)
				printf("Gen_out: Recebi lixo...");
			}else if(tipo == 2){//se for dado, testa se o cliente esta no buffer
				msg.pid = meupid;
				msg.mensagem[1] = 0;
				if(searchCli(clientes,qualCliente)){
					msg.mensagem[0] = 0xD;
				}else{
					msg.mensagem[1] = 0xC;
				}
				msg.mensagem[0] |= qualCliente<<4;
				resp = msgsnd(fila_serv, &msg, sizeof(mensagem)-sizeof(long),0);
			}else if(tipo == 3){//se for termino
				removeCli(clientes, qualCliente);
			}
		}
	}
}
