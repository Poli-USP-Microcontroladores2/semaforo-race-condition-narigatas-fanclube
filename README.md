# 🧪 Planejamento de Testes – Race Condition no Zephyr RTOS

Este documento descreve três casos de teste para analisar e visualizar **condições de corrida (race conditions)** no acesso a um recurso compartilhado (`shared_sensor_data`) por múltiplas threads em um sistema Zephyr RTOS rodando na placa **FRDM-KL25Z**.

---

## 🔧 Contexto

O código em teste cria **3 threads concorrentes** que acessam uma estrutura global de dados do sensor **sem mecanismos de sincronização**, resultando em comportamento inconsistente e possível corrupção de dados.

O objetivo dos testes é:
- Identificar visualmente e via logs a ocorrência de race conditions.
- Analisar o impacto da variação de carga no sistema.
- Verificar a correção com uso de mutex.

---

## 🧩 Planejamento de Testes

| **Caso de Teste** | **Pré-condição** | **Etapas de Teste** | **Pós-condição Esperada** |
|--------------------|------------------|----------------------|----------------------------|
| **1. Execução Normal sem Sincronização (Race Condition visível)** | - Código `main.c` compilado e gravado com sucesso na placa FRDM-KL25Z.<br>- Placa energizada via USB e conectada ao terminal serial (115200 baud).<br>- Nenhum periférico adicional conectado. | 1. Executar o firmware na placa.<br>2. Observar os três LEDs (vermelho, verde e azul) durante 30 segundos.<br>3. Acompanhar as mensagens no terminal serial.<br>4. Identificar mensagens de inicialização duplicadas e erros “RACE CONDITION”.<br>5. Verificar se, ao final, todos os LEDs piscam simultaneamente. | - LEDs acendem de forma aleatória, sem padrão fixo.<br>- Mensagens “RACE CONDITION!” aparecem no terminal.<br>- Valor de timestamp inconsistente entre threads.<br>- Ao final, os três LEDs piscam juntos (500 ms ligados / 500 ms desligados).<br>- Sistema reporta “SIM - SISTEMA INCONSISTENTE!”. |
| **2. Variação de Carga (Teste de Estresse de Race Condition)** | - Mesmo ambiente e firmware do Caso 1.<br>- A placa permanece conectada ao terminal serial.<br>- Adicionar tarefas externas de carga (ex: envio de logs contínuos via `printk()`). | 1. Modificar temporariamente a função `sensor_operation_critical()` adicionando `k_sleep(K_MSEC(2))` extras em pontos diferentes.<br>2. Recompilar e gravar novamente.<br>3. Observar se a frequência e a quantidade de mensagens “RACE CONDITION” aumentam.<br>4. Analisar a estabilidade do LED durante maior carga. | - A interferência entre threads se torna mais evidente.<br>- Maior frequência de mensagens “RACE CONDITION!”.<br>- LEDs mudam de estado de forma ainda mais irregular.<br>- Sistema entra em estado de erro (todos os LEDs piscando). |
| **3. Comparação com Controle de Concorrência (Teste de Correção)** | - Adicionar mutex (`K_MUTEX_DEFINE(sensor_mutex)`) ao código e proteger o acesso ao `shared_sensor_data`.<br>- Firmware recompilado e gravado na placa.<br>- Mesmo ambiente dos testes anteriores. | 1. Executar o firmware com o mutex ativo.<br>2. Observar o comportamento dos LEDs por 30 segundos.<br>3. Verificar a saída no terminal serial.<br>4. Comparar com o comportamento do Caso 1. | - Mensagens “RACE CONDITION” **não aparecem** no terminal.<br>- Dados do sensor são consistentes (timestamps corretos).<br>- LEDs acendem e apagam conforme o esperado para cada thread, sem interferência.<br>- Mensagem final: “Sistema operou corretamente (sem inconsistências)”. |

---

## 🧠 Resumo

| **Objetivo do Caso** | **Resultado Esperado** |
|-----------------------|------------------------|
| Identificar a race condition | Erros e inconsistência de dados |
| Aumentar a carga e observar o agravamento | Mais erros e instabilidade |
| Corrigir com sincronização (mutex) | Sistema consistente e estável |

---

## 📋 Observações

- Os testes devem ser realizados com **logs seriais habilitados** via `printk()`.
- Recomenda-se utilizar um **terminal serial** (como PuTTY ou minicom) para análise dos logs.
- O **comportamento dos LEDs** serve como indicador visual de race conditions e erros de sincronização.

---

## 💡 Conclusão

Esses testes permitem demonstrar, de forma prática e visual, o impacto da ausência de sincronização em sistemas embarcados multitarefa e como o uso de **mutexes** elimina a condição de corrida, garantindo integridade dos dados e estabilidade do sistema.

---
