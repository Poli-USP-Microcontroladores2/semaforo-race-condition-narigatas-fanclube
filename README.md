# TESTES PARA VISUALIZAR O CASO DE RACE CONDITION:

## CASO DE TESTE 2:

### PRÉ-CONDIÇÃO:
- A placa FRDM-KL25Z deve estar corretamente conectada à fonte de energia ou à porta USB do computador;
- O código main.c foi compilado e gravado com sucesso na placa, utilizando Zephyr RTOS 4.2;
- O terminal serial está aberto e configurado na porta correta (ex: COM5) com baud rate 115200;
- O sistema está em execução e imprimindo saídas no terminal serial.

### ETAPAS DE TESTE:
- Observar a saída serial por aproximadamente 30 segundos;
- Identificar linhas no formato:
>>> MONITOR: SALTO! last=XX now=YY
>>> MONITOR: REGRESSÃO! last=XX now=YY
>>> MONITOR: REPETIÇÃO! value=XX;
- Verificar se os valores de contagem apresentados não seguem uma sequência estritamente crescente (1, 2, 3, 4, …);
- Observar se os valores “saltam”, “voltam” ou “repetem” de forma irregular, sem qualquer padrão previsível;
- Confirmar que essas anomalias ocorrem mesmo sem interação externa com a placa.

### PÓS-CONDIÇÃO ESPERADA:
- O terminal exibe repetidamente mensagens indicando anomalias de concorrência, como “SALTO!”, “REGRESSÃO!” ou “REPETIÇÃO!”;
- Os valores de contagem (now) apresentam inconsistências, como retrocessos (ex: last=7 now=4) ou pulos (ex: last=33 now=38), demonstrando operações intercaladas entre threads;
- Nenhum padrão constante de incremento é observado — a sequência é visivelmente caótica e não determinística;
- Conclusão: o comportamento confirma a ocorrência de race condition na manipulação do contador global, causada pela falta de sincronização entre threads concorrentes.