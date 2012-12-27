#ifndef _MATCHING_H_
#define _MATCHING_H_

#include <minutia.h>
#include <enhance.h>

//tipos de minucias para o matchng
#define	CORE	2	//tipo complementar, apenas uma minucia pode ser a core
#define OK	4	//para marcar que ja foi checada no matching

//Área de procura do core
#define CORECENTER_Y	60
#define CORECENTER_X	60
#define CORERAY		5

//Raio maximo aceitavel para nao criar uma minucia core nova
#define CORERANGE	5

//Penalidades para o matching
#define	PEN_TRANSLATE	1	//penalidade por apenas ter que mover uma minucia
#define	PEN_INCLUDE	50	//penalidade por ter que incluir uma minucia inexistente
#define PEN_DELETE	40	//penalidade por ter que excluir uma minucia
#define	MINLIM		20	//faz com que só se use as MINLIM minucias mais perto da minucia core
#define PEN_THRESHOLD	350	//limite de penalidade para dizer se é ou não a pessoa
#define	FINGER_MATCH	1
#define	FINGER_UNMATCH	0

//Matching em espiral
#define	MATCH_LIM	50	//porcentagem de acertos para considerar a mesma pessoa
#define	SPIN_MATCH	1	//flag que a tiva o giro das minucias
#define	SPIN_STEP	PI/20	//passo para o giro das minucias
#define	MATCH_RAY	169	//pixels de arredondamento para o match(esta ao quadrado, para economizar um sqrt)
#define SPIRAL_RAY	100	//metade do numero de voltas da espiral

#ifdef USEOPENCV
/*
---showCore---
Mostra uma imagem com o core marcado para debug
Entrada:
   -image *img: imagem da impressao digital para mostrar o core
   -minutia *core: estrutura contendo os dados do core
Saida: Void
*/
void showCore(image *img, minutia *core);
#endif

/*
---crownCore---
Procura na imagem o ponto core, o que possui a curva mais acentuada no dedo, e coloca a minucia
mais proxima desso ponto na primera posição da lista.
Entrada:
   -double *direcs: vetor com as direções das linhas para achar o core
   -minutia *root: vetor com as minucias para marcar ou acrescentar a minucia core
 Saida:
   -minutia *: vetor das minucias com a minucia core na primeira posição
 */
minutia *crownCore(image *img, minutia *root);

/*
---getDistances---
Considerando que a primeria minucia no vetor é a minucia mais proxima do ponto core,
a função coloca a distancia de cada uma a este ponto.
Entrada:
   -minutia *root: Vetor com a lista das minucias, tendo a mais proxima do ponto core na primeira posicao
Saida: Void
*/
void getDistances(minutia *root);


/*
---saveTemplate---
Salva um template para ser usado depois para a verificação. No template há os dados de todas as minucias
Entrada:
   -minutia *root: Vetor com a lista de minucias
   -char *filename: Endereço do arquivo que guardará as informações
Saida: int
   -retorna se conseguiu ou nao salvar(SUCCESS ou FAILURE)
*/
int saveTemplate(minutia *root, char *filename);

void orderMinutia(minutia *root);

/*
---validate---
Dadas as penalidades para cada tipo, diz se a impressão digital pertence ao templete dado se a penalidae ficar
abaixo de um limiar. 
Entrada:
   -minutia *temp: Lista com o template das minucias
   -minutia *test: Lista para comparar com o template
Saida: int
   -SUCCESS : 1/ FAILURE : 0
*/

int validadeFP(minutia *temp, minutia *test);

/*
---testMatch---
Calcula a porcentagem de minucias do template que se encontratam com as do set de teste.
Entrada:
   -minutia *temp: template das minucias
   -minutia *test: set de teste a ser comparado
Saida: int
   Porcentagem de minucias acertadas
*/
float testMatch(minutia *temp, minutia *test);

/*
---spinMatching---
Realiza o teste de matching em espiral
Entrada:
   -minutia *temp: template das minucias
   -minutia *test: set de teste
   -int flags: serve para determinar se vai ter giro das minucias ou não
   -int with: largura da imagem para a rotação
   -int height: altura da imagem, pelo mesmo motivo
Saida: int
   -SUCCESS : 1/ FAILURE : 0
*/
int spiralMatching(minutia *temp, minutia *test, int flags, int with, int height);

#endif
