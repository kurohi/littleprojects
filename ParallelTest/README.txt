para compilar
sh compilatudo.sh

para executar
./paisao
ele pede o numero de mensagens e depois quais filhos irão mandar quais mensagens.
Cada filho só manda uma vez(por usar signal) por isso não adianta repetir o numero do filho na hora de dizer quais filhos mandarão mensagens.
Para terminar o programa entre com um numero de mensagens <=0


Descrição da execução:

Para cada "conexao" entre os clientes se tem 2 filas de mensagens uma para ir ao servidor e outra que volta do servidor.
Desse modo evita-se dead-lock pois as recepções são não-blocantes.
As filas no total são um vetor de duas dimensões com a primeira sendo o numero da conexão e a segunda se ela é de ida ou volta.
o numero corresponde a essa tabela da foto conexoes.pdf.
o primeiro byte da mensagem é um header orgaizado da seguinte maneira:
(4bits, nº do cliente)(1bit,especial)(2bits,tipo da mensagem)(1bit,se a mensagem é de servidor ou de cliente)
o tipo pode ser(binário):
00 - requisição
01 - resposta
10 - dados
11 - termino de conexão

o bit "especial" fica setado em 1 quando as mensagem estao circulando na fila dos gerentes de impressao

nos processos do servidor de impressão os 3 usam a mesma fila para se counicarem.
O gen_in cuida das mensagens vindas dos clientes
o gen_print imprimi o que for passado para ele
o gen_out cuida do buffer, adicionando, removendo e checando quem está nele.

Se o gen_in recebe uma requisição ele passa para o gen_out que checa se tem espaço no buffer se tiver ele coloca, se não tiver ele manda uma mensagem dizendo que não deu.
Se o gen_in recebe dados ele pergunta para o gen_out se o cliente está cadastrado, se estiver o gen_in passa a mensagem para o gen_print que a imprimi
Se o gen_in recebe uma mensagem de termino de impressao e passa para o gen_out que retira esse cliente do buffer.

os primeiros 2 bytes da mensagem são cabeçalhos e são organizados dessa maneira:

1º byte: (4bits:qual cliente)(1bit,pegar resposta)(2bits, tipo da mensagem)(1bit se é cliente ou servidor)
2º byte: A quem se destina(é um disperdio de espaço eu sei, mas fica mais simples)

no caso de passar 
