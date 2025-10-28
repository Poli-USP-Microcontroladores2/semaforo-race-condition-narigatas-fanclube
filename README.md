# 🧪 CASO DE TESTE 1: DEMONSTRAÇÃO DE RACE CONDITION SEM SINCRONIZAÇÃO

## 🔎 OBJETIVO
Demonstrar o comportamento de uma Race Condition em sistemas embarcados quando duas threads acessam simultaneamente um mesmo recurso compartilhado sem qualquer mecanismo de sincronização. Este teste tem como finalidade evidenciar o comportamento incorreto e imprevisível causado pela ausência de controle de acesso entre tarefas concorrentes no Zephyr RTOS 4.2.

## ⚙️ PRÉ-CONDIÇÃO
- A placa **FRDM-KL25Z** deve estar corretamente conectada à fonte de energia ou à porta USB do computador.  
- O código **main.c** deve estar configurado com **duas threads** acessando a variável global `shared_counter` **sem utilização de semáforo ou mutex**.  
- O firmware deve ser compilado e gravado com sucesso na placa, utilizando o **Zephyr RTOS 4.2**.  
- O terminal serial deve estar aberto e configurado na porta correta (ex: `COM5`), com **baud rate 115200**.  
- O sistema deve estar em execução e imprimindo mensagens de leitura e escrita das threads, no formato:  
[A] read value: X
[B] read value: X
[A] wrote value: Y
[B] wrote value: Y


## 🧩 ETAPAS DE TESTE
1. Ligar a placa FRDM-KL25Z e iniciar a execução do firmware.  
2. Observar a saída serial no terminal por aproximadamente **30 segundos**.  
3. Identificar mensagens de leitura e escrita intercaladas entre as threads **A** e **B**.  
4. Verificar se ambas as threads realizam leituras e escritas simultâneas sobre a variável `shared_counter`.  
5. Analisar se o valor final do contador não corresponde à soma esperada (deveria aumentar em +2 por ciclo, mas nem sempre ocorre).  
6. Registrar um exemplo de comportamento incorreto, como:  
[A] read value: 2
[B] read value: 2
[A] wrote value: 3
[B] wrote value: 3 → shared_counter = 3

7. Confirmar que, ao final do teste, o valor do contador global apresenta resultados inconsistentes e não previsíveis.

## ✅ PÓS-CONDIÇÃO ESPERADA
- O terminal exibe mensagens de leitura e escrita **sobrepostas** entre as threads A e B.  
- O valor da variável `shared_counter` **não evolui corretamente**, apresentando perdas de incremento.  
- O resultado final **não reflete os dois incrementos esperados** por ciclo de execução.  
- O comportamento observado confirma a ocorrência de **Race Condition**, pois duas threads acessam o mesmo recurso sem sincronização.  
- Conclusão: o sistema falha em atingir o comportamento esperado devido à ausência de controle de acesso ao recurso compartilhado.

## 🧰 AMBIENTE DE TESTE
- **Hardware:** FRDM-KL25Z  
- **RTOS:** Zephyr 4.2  
- **Linguagem:** C/C++  
- **Recursos Utilizados:** Threads, Variáveis Globais, UART/Serial Output  

## 📜 CONCLUSÃO
O caso de teste demonstra claramente o impacto da ausência de sincronização entre threads em sistemas multitarefa. Ao permitir que duas threads leiam e escrevam simultaneamente na mesma variável, ocorre uma Race Condition que causa inconsistências no valor final. Este comportamento evidencia a necessidade do uso de mecanismos de exclusão mútua, como semáforos ou mutex, para garantir acesso seguro e previsível a recursos compartilhados. A partir deste resultado, o próximo caso de teste aplicará um semáforo para corrigir o problema e validar a sincronização adequada.
