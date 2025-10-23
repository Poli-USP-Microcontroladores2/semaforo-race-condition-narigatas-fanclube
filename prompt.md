**Crie um código funcional em C usando Zephyr RTOS 4.2 que demonstre claramente o problema de Race Condition (condição de corrida) em um cenário embarcado real para a placa FRDM-KL25Z.**

**Contexto:**
Desenvolva um sistema de controle de acesso a um recurso compartilhado (como um sensor I2C ou periférico crítico) onde múltiplas threads tentam acessar o mesmo recurso simultaneamente. Este é um problema comum em sistemas embarcados onde recursos de hardware são compartilhados.

**Requisitos Técnicos:**

1. **Target:** FRDM-KL25Z com Zephyr RTOS 4.2
2. **Crie múltiplas threads** usando `k_thread_create()` (pelo menos 3 threads)
3. **Simule um recurso compartilhado crítico** - como um buffer de dados do sensor ou registrador de hardware
4. **Implemente sem mutex/semaforos** inicialmente para demonstrar o problema
5. **Use delays não determinísticos** com `k_sleep(K_MSEC(1))` para forçar a race condition

**Cenário Real:**
Múltiplas threads de aplicação tentando acessar um sensor I2C compartilhado para leitura de temperatura, onde a sequência de operações (inicialização → leitura → processamento) pode ser corrompida por acesso concorrente.

**Funcionalidades Esperadas:**

- Variável global compartilhada representando o estado do sensor ou dados lidos
- Cada thread deve executar uma sequência de operações no recurso compartilhado
- Sem sincronização, as threads podem corromper o estado umas das outras
- Use `printk()` para logar cada operação e mostrar a condição de corrida

**Estrutura do Código Esperada:**

```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 512
#define THREAD_COUNT 3

// Recurso compartilhado - simulando dados de sensor
struct sensor_data {
    uint32_t timestamp;
    int16_t temperature;
    uint8_t sequence;
};

K_THREAD_STACK_ARRAY_DEFINE(thread_stacks, THREAD_COUNT, STACK_SIZE);
struct k_thread threads[THREAD_COUNT];

// Variável compartilhada sem proteção
struct sensor_data shared_sensor_data = {0};
```

**Saída Esperada via Serial:**
```
*** Demonstração Race Condition - Zephyr RTOS ***

Thread 1: Lendo sensor - timestamp: 1000, temp: 25°C
Thread 2: Lendo sensor - timestamp: 1000, temp: 25°C  
Thread 3: Corrompendo dados - escrevendo timestamp: 2000
Thread 1: Dados corrompidos! Esperado: 1000, Recebido: 2000
Thread 2: Dados corrompidos! Esperado: 1000, Recebido: 2000

*** RACE CONDITION DETECTADA: Dados do sensor corrompidos! ***
```

**Bônus:**
- Inclua uma versão corrigida usando `K_MUTEX_DEFINE()` ou `K_SEM_DEFINE()`
- Mostre a diferença de comportamento entre as duas versões
- Use LEDs da placa (LED1, LED2) para indicar estados das threads

**Configuração do Projeto:**
Incluir seções necessárias no `prj.conf`:
```
CONFIG_PRINTK=y
CONFIG_MAIN_STACK_SIZE=1024
CONFIG_HEAP_MEM_POOL_SIZE=4096
```

**Instruções de Compilação:**
O código deve incluir comentários sobre como compilar para FRDM-KL25Z:
```bash
west build -b frdm_kl25z
west flash
```

**Monitoramento:**
Instruções para monitorar via serial (115200 baud):
```bash
picocom -b 115200 /dev/ttyACM0
