/*************************************************/
/*Universidade de Brasília - Unb                 */
/*Instituto de Ciências Exatas - IE              */
/*Departamento de Ciência da Computação - CIC    */
/*************************************************/

#ifndef __IMAGEM_H__
#define __IMAGEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libfprint/fprint.h>
#include "enhance.h"

imagem* pegar_imagem();
void free_imagem(struct imagem* imagem);
void salvar_imagem(struct imagem* imagem, char* filename);
void free_imagem(struct imagem* imagem);

#endif
