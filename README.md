# TESTES PARA VALIDAR A CORREÇÃO (MUTEX):

## CASO DE TESTE 3: VALIDAÇÃO DA SINCRONIZAÇÃO COM MUTEX

### PRÉ-CONDIÇÃO:
- A placa FRDM-KL25Z deve estar corretamente conectada à fonte de energia ou à porta USB do computador;
- O código main.c corrigido com a implementação de um mutex (k_mutex) foi compilado e gravado com sucesso na placa, utilizando Zephyr RTOS 4.2;
- O terminal serial está aberto e configurado na porta correta (ex: COM5) com baud rate 115200;
- O sistema está em execução e imprimindo saídas no terminal serial, incluindo a mensagem O monitor não deve detectar regressões..

### ETAPAS DE TESTE:
- Observar a saída serial por aproximadamente 30 segundos;
- Identificar linhas no formato: >>> MONITOR: SALTO GRANDE! last=XX now=YY >>> MONITOR: REGRESSÃO! last=XX now=YY >>> MONITOR: REPETIÇÃO! value=XX;
- Verificar se a mensagem >>> MONITOR: REGRESSÃO! NUNCA aparece no log;
- Observar se os valores de contagem apresentados seguem uma sequência estritamente "monotonicamente crescente" (ou seja, now é sempre maior ou igual a last);
- Confirmar que, embora "SALTOS" ocorram (ex: last=80 now=160), eles são previsíveis e resultado do agendamento (monitor dorme por 50ms enquanto workers executam), e não de leituras corrompidas.

### PÓS-CONDIÇÃO ESPERADA:
- O terminal NÃO EXIBE nenhuma mensagem de >>> MONITOR: REGRESSÃO!;
- O terminal exibe mensagens de >>> MONITOR: SALTO GRANDE!. Isso é esperado e correto, pois as 8 threads de trabalho executam múltiplas vezes (aprox. 80 incrementos) enquanto a thread do monitor está suspensa (k_msleep(50));
- Mensagens de >>> MONITOR: REPETIÇÃO! podem ocorrer ocasionalmente se a thread do monitor executar duas vezes antes de qualquer thread de trabalho conseguir rodar e incrementar o contador. Isso não indica um erro.
- A sequência de valores now é sempre crescente (ex: 80, 160, 240, 320...), provando que o contador nunca retrocede;
- Conclusão: O comportamento confirma que a race condition foi eliminada. O mutex garante a atomicidade das operações de leitura e escrita, e o contador global permanece consistente, mesmo sob concorrência.