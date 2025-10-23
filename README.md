Comportamento Esperado

Saída Serial: As duas funções leem o mesmo valor da variável e adicionam ao mesmo valor, ou seja ficam assim:

[A] read value: 2
[B] read value: 2
[A] wrote value: 3
[B] wrote value: 3
-> observed shared_counter = 3

Comportamento: O sistema não realiza seu objetivo, ou seja, em vez de cada uma adicionar um por vez, as duas tentam adicionar ao mesmo tempo

Análise do Problema

O código demonstra um cenário real de sistema embarcado onde:

Múltiplas threads acessam o display do meu dispositivo, sendo assim

elas tentam printar ao mesmo tempo impactando no resultado esperado no 

resultado do processo
Comportamento Observado:

*** Demonstração Race Condition - Zephyr RTOS 4.2 - 2 threads acessando o mesmo recurso compartilhado SEM sincronização entre elas



