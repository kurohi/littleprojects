#include<stdio.h>
#include<stdlib.h>
/* uso:cripto chave arquivo_entrada arquivo_saida */
void main (int argc, char *argv[]) { 
   FILE *entrada, *saida;
   char *key;
   int c;
   /*programa*/
   if ((key = argv[1]) && *key!='\0') {
      if ((entrada = fopen(argv[2],"rb"))!=NULL) {
         if ((saida = fopen(argv[3],"wb"))!=NULL) {
            while((c = getc(entrada))!=EOF) {
               if (!*key) key = argv[1];
               c ^= *(key++);               /*XOR*/
               putc(c,saida);
            } fclose(saida);
         } fclose(entrada);
      }
   }
}

