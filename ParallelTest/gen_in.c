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

int fila_in;
int fila_out;
int fila_serv;
mensagem msg;
mensagem msginter;

void faznada(){}

int main(int agrc,char **argv){
	int resp,meupid;
	char tipo;
	char qualCliente;
	signal(SIGUSR1,faznada);
	meupid = getpid();
	sleep(10);//espera  o paisao acabar de fazer as filas
	fila_in = msgget(0x2018, 0666);
	fila_out = msgget(0x1018, 0666);
	fila_serv = msgget(0x3000, 0666);
	while(1){//se tiver mensagem na fila dos clientes pega, senao se bloqueia
		resp = msgrcv(fila_in, &msg, sizeof(mensagem), 0, 0);
		qualCliente = (msg.mensagem[0]&0xF0)>>4;
		tipo = (msg.mensagem[0]&0x6)>>1;//pega o tipo da mensagem
		if(tipo == 0){//se for requisicao
			msginter.pid = meupid;
			msginter.mensagem[0] = 0;
			msginter.mensagem[0] = (qualCliente<<4)|0x8;
			msginter.mensagem[1] = 2;//codigo do gen_out
			resp = msgsnd(fila_serv,&msginter,sizeof(mensagem)-sizeof(long), 0);//manda para o gen_out
			if(resp<0){
				printf("Gen_in: Erro no envio de requisição para a fila\n");
			}
			while(1){//fica esperando a resposta do gen_out
				resp = msgrcv(fila_serv, &msginter, sizeof(mensagem), 0, 0);//se nao for para ele devolve para a fila
				if((msginter.mensagem[0]&0x8==0)||(msginter.mensagem[1] != 0)){
					resp = msgsnd(fila_serv,&msginter,sizeof(mensagem)-sizeof(long), 0);
				}else{//se for testa constroe a resposta e manda para o cliente
					msg.pid = meupid;
					msg.mensagem[0] = (qualCliente<<4)|0x2;
					if((msginter.mensagem[0]&0x1)==1){
						msg.mensagem[1] = 1;
					}else{
						msg.mensagem[1] = 0;
					}
					resp = msgsnd(fila_out, &msg, sizeof(mensagem)-sizeof(long),0);
					break;
				}
			}
		}else if(tipo == 2){//se for dado testa se ele esta no buffer e manda os dados para o gen_print
			msginter.pid = meupid;
			msginter.mensagem[0] = 0;
			msginter.mensagem[0] = (qualCliente<<4)|0xC;
			msginter.mensagem[1] = 2;//codigo do gen_out
			resp = msgsnd(fila_serv,&msginter,sizeof(mensagem)-sizeof(long), 0);
			if(resp<0){
				printf("Gen_in: Erro no envio de requisiçao de dados para a fila\n");
			}
			while(1){//espera a resposta
				resp = msgrcv(fila_serv,&msginter, sizeof(mensagem),0,0);
				if((msginter.mensagem[0]&0x8==0)||(msginter.mensagem[1] != 0)){
					resp = msgsnd(fila_serv,&msginter,sizeof(mensagem)-sizeof(long), 0);
					if(resp<0){
						printf("Gen_in: Erro no envio da mensagem para a fila\n");
					}
				}else{
					if(((msginter.mensagem[0]&0xF)==0xD)&&(((msg.mensagem[0]&0xF0)>>4)==qualCliente)){//se estiver manda os dados para o gen_print
						resp = msgsnd(fila_serv, &msg, sizeof(mensagem)-sizeof(long), 0);
						if(resp<0){
							printf("Gen_in: Não consegui mandar os dados para o servidor de inpressao\n");
						}
					}else{
						printf("Gen_in: Recebi dados de um cliente que não está no spool\n");
					}break;
				}
			}
		}else if(tipo == 3){//se for termino só passa para o gen_out
			msginter.pid = meupid;
			msginter.mensagem[0] = 0;
			msginter.mensagem[0] = (qualCliente<<4)|0xE;
			msginter.mensagem[1] = 2;
			resp = msgsnd(fila_serv,&msginter,sizeof(mensagem)-sizeof(long), 0);
			if(resp<0){
				printf("Gen_in: Erro no envio de requisição para a fila\n");
			}
		}
	}
}
