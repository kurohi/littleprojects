#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<minutia.h>

#define	ulong	unsigned long

typedef struct RSAKey{
	ulong key;
	ulong mod;
}RSAKey;


#define	MAXMINUTIA	10
#define	UNCERTAIN	1000
#define	FAILURE		0
#define SUCCESS		1
#define	FREQ_TOLERANCE	1
#define DIST_LIM	20000

/*
---expmod---
Essa função realiza a operação a^x mod m
Entrada:
   ulong a: valor a na expressão
   ulong x: valor x na expressão(expoente)
   ulong m: valor m na expressão(modulo)
Saida:
   ulong: Resultado da operação
*/
ulong expmod(ulong a, ulong x, ulong m);

/*
---testPrime---
Teste probabilistico de primicidade de Lehmann
Entrada:
   ulong prime: Numero primo sendo testado
   unsigned int uncertain: Nivel de incertesa desejado(quanto maior, mais certesa)
Saida:
   int: 1 se for primo 0 se nao for
*/
int testPrime(unsigned int prime, unsigned int uncertain);

/*
---generatePrime---
Gera um numero primo do tamanho de um long com apenas 2 bits 1, assim agiliza os outro processos
Entrada:
   unsigned int uncertain: Nivel de incertesa desejado(quanto maior, mais certesa)
Saida:
   ulong: Primo calculado
*/
ulong generatePrime(unsigned int uncertain);

/*
---euclext---
Algoritimo exendido de Euclides. Dados a, b, c onde mdc(a,b) divide c, retorna o menor x>0 tal que a*x = c mod b
Entrada:
   ulong a: valor a da expressao
   ulong b: valor b da expressao
   ulong c: valor c da expressao
Saida:
   ulong: Resultado da expressao
*/
ulong euclext(ulong a, ulong b, ulong c);

/*
---generateKeys---
Gera as chaves publica e privada. Tanto faz qual é a publica e qual é a privada, então pode-se
trocar as chaves de ordem.
Entrada:
   RSAKey *publicKey: Ponteiro para salvar a chave publica
   RSAKey *privateKey
*/
void generateKeys(RSAKey *publicKey, RSAKey *privateKey, minutia *root);

/*
---encript---
Encripta e decripta dada a chave e nomes dos arquivos de entrada e de saida
Entrada:
   RSAKey key: Chave usada para a criptografia/decriptografia
   char *inname: Nome do arquivo de entrada;
   char *exitname: Nome do arquivo de saida
Saida:
   int: Sucesso(1) ou falha(0)
*/
int encript(RSAKey key, char *inname, char *exitname);

/*
---saveKeys---
Salva as chaves publica e privada em dois arquivos binarios
Entrada:
   RSAKey publicKey: chave publica para salvar
   RSAKey privateKey: Chave privada para salvar
   char *prefix: prefixo do arquivo salvo (ficará prexipubic.key e prefixprivate.key);
Saida:
   int: Sucesso(1) ou falha(0)
*/
int saveKeys(RSAKey publicKey, RSAKey privateKey, char *prefix);

/*
---loadKey---
Carrega uma chave dado o nome do arquivo
Entrada:
   RSAKey *key: ponteiro para salvar a chave
   char *filename: nome do arquivo em que a chave está salva
Saida:
   int: Sucesso ou falha
*/
int loadKey(RSAKey *key, char *filename);
void minutiaFilter(image *img, minutia *root, int ray, int freq);
unsigned long radialextraction(image *img, minutia *minlist);
#endif
