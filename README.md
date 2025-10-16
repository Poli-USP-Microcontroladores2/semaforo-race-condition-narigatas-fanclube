**Comportamento Esperado**

Saída Serial: Mensagens de erro mostrando dados corrompidos

LEDs: Piscam todos simultaneamente indicando estado de erro

Comportamento: Timestamps inconsistentes, sequências quebradas


**Análise do Problema**

O código demonstra um cenário real de sistema embarcado onde:

Múltiplas threads acessam um periférico compartilhado (sensor I2C)

Operações não atômicas são interrompidas por mudanças de contexto

Delays não determinísticos exacerbam o problema

Dados críticos são corrompidos sem sincronização adequada



**Comportamento Observado:**

*** Demonstração Race Condition - Zephyr RTOS 4.2 - FRDM-KL25Z ***
3 threads acessando recurso compartilhado SEM sincronização

Thread 1: Inicializando sensor
Thread 2: *** RACE CONDITION! Esperado: 2001, Recebido: 1001
Thread 3: *** RACE CONDITION! Esperado: 3001, Recebido: 1001
Thread 1: Leitura OK - timestamp: 1001, temp: 21°C, seq: 3
Thread 2: *** RACE CONDITION! Esperado: 2101, Recebido: 1101


**Análise Técnica dos Problemas:**
1. Corrupção de Dados de Timestamp:
Thread 1 define timestamp = 1001

Thread 2 é agendada e sobrescreve com timestamp = 2001

Thread 1 retorna e encontra 2001 em vez do esperado 1001

Resultado: Dados de temporalidade completamente inválidos

2. Sequência de Operações Quebrada:
// Operação corrompida:
Thread 1: shared_sensor_data.timestamp = 1001;  // Executa
// ↓ Context switch para Thread 2
Thread 2: shared_sensor_data.timestamp = 2001;  // Executa  
Thread 2: shared_sensor_data.temperature = 22;  // Executa
// ↓ Context switch de volta para Thread 1
Thread 1: shared_sensor_data.temperature = 21;  // CORROMPE!
Thread 1: shared_sensor_data.sequence++;        // Seq = 1
Resultado: Temperatura da Thread 1 (21°C) com timestamp da Thread 2 (2001)

3. Contador de Sequência Inconsistente:
Cada thread incrementa sequence++, mas sem atomicidade

Valor final esperado: 15 (3 threads × 5 operações)

Valor real obtido: Pode ser qualquer valor entre 5-15 devido a sobrescritas

4. Inicialização Múltipla:
if (!shared_sensor_data.initialized) {
    k_sleep(K_MSEC(1));  // ← Race condition aqui!
    shared_sensor_data.initialized = true;
}

Múltiplas threads podem entrar no if simultaneamente
Re-inicializam o sensor repetidamente

**Indicadores Visuais (LEDs):**
LEDs 0, 1, 2: Acendem aleatoriamente indicando acesso concorrente

Todos LEDs piscando: Estado de erro crítico permanente

Feedback visual: Mostra a natureza não determinística do problema