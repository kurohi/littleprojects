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

int main(int argc, char **argv){
	int resp,meupid;
	char *msgpt;
	signal(SIGUSR1,faznada);
	meupid = getpid();
	sleep(10);
	fila_serv = msgget(0x3000, 0666);
	while(1){//pega a mensagem da fila dos servidores se nao for para ele devolve para a fila.
		resp = msgrcv(fila_serv, &msg, sizeof(mensagem), 0, 0);
		if((msg.mensagem[0]&0x8)==0){//ele sabe que eh para ele pelo bit especial definido no README
			//se for para ele apenas imprimi, so havera mensagem para o gen_print
			//depois de uma checagem do gen_in, entao não ha necessidade de outra checagem aqui
			msgpt = &msg.mensagem[1];
			printf("%s\n",msgpt);
		}else{
			resp = msgsnd(fila_serv, &msg, sizeof(mensagem)-sizeof(long),0);
		}
	}
}
