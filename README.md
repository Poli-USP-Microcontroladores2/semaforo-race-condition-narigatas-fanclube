# 🧪 Planejamento de Testes – Visualização de Race Condition no Zephyr RTOS

## 🎯 Objetivo
Demonstrar e visualizar a ocorrência de **race condition** quando múltiplas threads acessam simultaneamente um recurso compartilhado (`shared_sensor_data`) **sem mecanismos de sincronização** no Zephyr RTOS, utilizando a placa **FRDM-KL25Z**.

---

## 🧩 Casos de Teste

| **Caso de Teste** | **Pré-condição** | **Etapas de Teste** | **Pós-condição Esperada** |
|--------------------|------------------|----------------------|----------------------------|
| **1. Execução Padrão com Três Threads Concorrentes** | - Firmware compilado e gravado na FRDM-KL25Z.<br>- Conexão serial ativa (115200 baud).<br>- Nenhum outro processo em execução na placa.<br>- LEDs conectados corretamente. | 1. Energizar a placa e iniciar a execução do firmware.<br>2. Observar os LEDs correspondentes a cada thread (led0, led1, led2).<br>3. Monitorar o terminal serial para saída de logs.<br>4. Anotar mensagens “RACE CONDITION!”. | - LEDs piscam de maneira não determinística.<br>- Logs mostram mensagens “RACE CONDITION!” em diferentes threads.<br>- Valor de timestamp e sequência inconsistentes.<br>- Ao final, LEDs piscam continuamente (erro detectado). |
| **2. Execução Múltipla (Testar Reprodutibilidade do Problema)** | - Mesmo firmware e ambiente do Caso 1.<br>- Terminal serial aberto para leitura.<br>- Nenhuma modificação no código. | 1. Executar o firmware repetidas vezes (mínimo 5 execuções).<br>2. Registrar a ocorrência (ou não) de mensagens “RACE CONDITION!”.<br>3. Observar se a ordem das mensagens muda entre execuções.<br>4. Comparar o comportamento dos LEDs entre tentativas. | - Ocorrem mensagens “RACE CONDITION!” em execuções diferentes.<br>- Ordem e thread afetada variam (comportamento não determinístico).<br>- LEDs podem acender de forma diferente a cada execução.<br>- Sistema entra no estado de erro (LEDs piscando em loop). |
| **3. Teste de Estresse com Atrasos Aleatórios** | - Mesmo ambiente e código base.<br>- Inserido atraso extra (`k_sleep(K_MSEC(2))`) em pontos diferentes da função `sensor_operation_critical()` para simular variação de tempo de CPU. | 1. Modificar o código para inserir `k_sleep()` adicional.<br>2. Recompilar e gravar o firmware na placa.<br>3. Executar e observar o comportamento dos LEDs e logs.<br>4. Contar quantas mensagens “RACE CONDITION!” são exibidas. | - O número de erros “RACE CONDITION!” aumenta com atrasos.<br>- Threads interferem mais frequentemente nos dados compartilhados.<br>- LEDs piscam de forma irregular e rápida.<br>- Sistema entra em estado de erro (pisca contínuo). |

---

## 🧠 Observações Gerais

- O **problema é proposital**: o código **não utiliza mecanismos de exclusão mútua**, como `k_mutex_lock()` ou `atomic` operations.  
- O comportamento **não determinístico** é esperado e serve para **ilustrar a race condition**.  
- O LED piscando constantemente ao final indica que a variável `race_condition_detected = true`.  
- Os logs via `printk()` mostram o conflito entre threads durante a escrita de `shared_sensor_data.timestamp`.

---

## 📊 Resultados Esperados

| **Indicador** | **Descrição** |
|----------------|----------------|
| `RACE CONDITION!` no terminal | Confirmação da condição de corrida detectada |
| LEDs piscando juntos | Indicação visual de erro global |
| Saídas diferentes a cada execução | Evidência de comportamento não determinístico |
| Valor de sequência e timestamp corrompidos | Demonstra falha no acesso simultâneo ao recurso |

---

## 🧩 Conclusão

Os testes demonstram que, em um ambiente multitarefa sem sincronização adequada, o acesso simultâneo a variáveis compartilhadas resulta em **inconsistência de dados** e **comportamento imprevisível** — características clássicas de uma **race condition**.
