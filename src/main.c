#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 512
#define THREAD_COUNT 3
#define OPERATION_COUNT 5

// Definições dos LEDs da FRDM-KL25Z
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

K_THREAD_STACK_ARRAY_DEFINE(thread_stacks, THREAD_COUNT, STACK_SIZE);
struct k_thread threads[THREAD_COUNT];

// Recurso compartilhado - simulando dados de sensor I2C
struct sensor_data {
    uint32_t timestamp;
    int16_t temperature;
    uint8_t sequence;
    bool initialized;
};

// Variável compartilhada SEM proteção (causa race condition)
volatile struct sensor_data shared_sensor_data = {0};
volatile uint32_t operation_counter = 0;
volatile bool race_condition_detected = false;

// Função para simular operação crítica em sensor I2C
void sensor_operation_critical(uint8_t thread_id, uint32_t base_timestamp) {
    // Operação 1: Inicializar sensor (se necessário)
    if (!shared_sensor_data.initialized) {
        k_sleep(K_MSEC(1));  // Delay não determinístico
        shared_sensor_data.initialized = true;
        shared_sensor_data.sequence = 0;
        printk("Thread %d: Inicializando sensor\n", thread_id);
    }
    
    // Operação 2: Configurar timestamp
    k_sleep(K_MSEC(1));  // Delay não determinístico
    shared_sensor_data.timestamp = base_timestamp + thread_id;
    
    // Operação 3: Simular leitura de temperatura
    k_sleep(K_MSEC(1));  // Delay não determinístico
    shared_sensor_data.temperature = 20 + thread_id;
    
    // Operação 4: Incrementar sequência
    k_sleep(K_MSEC(1));  // Delay não determinístico
    shared_sensor_data.sequence++;
    
    // Operação 5: Verificar consistência dos dados
    k_sleep(K_MSEC(1));  // Delay não determinístico
    uint32_t expected_timestamp = base_timestamp + thread_id;
    
    // Verificar se os dados foram corrompidos por outra thread
    if (shared_sensor_data.timestamp != expected_timestamp) {
        printk("Thread %d: *** RACE CONDITION! Esperado: %lu, Recebido: %lu\n", 
               thread_id, expected_timestamp, shared_sensor_data.timestamp);
        race_condition_detected = true;
        
        // Acender LED indicando erro
        switch(thread_id) {
            case 1: gpio_pin_set_dt(&led0, 1); break;
            case 2: gpio_pin_set_dt(&led1, 1); break;
            case 3: gpio_pin_set_dt(&led2, 1); break;
        }
    } else {
        printk("Thread %d: Leitura OK - timestamp: %lu, temp: %d°C, seq: %d\n",
               thread_id, shared_sensor_data.timestamp, 
               shared_sensor_data.temperature, shared_sensor_data.sequence);
    }
    
    operation_counter++;
}

// Função da thread
void sensor_thread(void *arg1, void *arg2, void *arg3) {
    uint8_t thread_id = (uint8_t)(uintptr_t)arg1;
    uint32_t base_timestamp = 1000 * thread_id;
    
    // Acender LED correspondente à thread
    switch(thread_id) {
        case 1: gpio_pin_set_dt(&led0, 1); break;
        case 2: gpio_pin_set_dt(&led1, 1); break;
        case 3: gpio_pin_set_dt(&led2, 1); break;
    }
    
    for (int i = 0; i < OPERATION_COUNT; i++) {
        sensor_operation_critical(thread_id, base_timestamp + (i * 100));
        k_sleep(K_MSEC(10));  // Pequeno delay entre operações
    }
    
    // Apagar LED quando thread terminar
    switch(thread_id) {
        case 1: gpio_pin_set_dt(&led0, 0); break;
        case 2: gpio_pin_set_dt(&led1, 0); break;
        case 3: gpio_pin_set_dt(&led2, 0); break;
    }
}

// Configurar LEDs
int setup_leds(void) {
    int ret;
    
    if (!gpio_is_ready_dt(&led0)) {
        return -1;
    }
    if (!gpio_is_ready_dt(&led1)) {
        return -1;
    }
    if (!gpio_is_ready_dt(&led2)) {
        return -1;
    }
    
    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return ret;
    }
    
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return ret;
    }
    
    ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return ret;
    }
    
    // Inicializar LEDs apagados
    gpio_pin_set_dt(&led0, 0);
    gpio_pin_set_dt(&led1, 0);
    gpio_pin_set_dt(&led2, 0);
    
    return 0;
}

void main(void) {
    printk("\n*** Demonstracao Race Condition - Zephyr RTOS 4.2 - FRDM-KL25Z ***\n");
    printk("3 threads acessando recurso compartilhado SEM sincronizacao\n\n");
    
    // Configurar LEDs
    if (setup_leds() != 0) {
        printk("Erro ao configurar LEDs!\n");
        return;
    }
    
    // Criar múltiplas threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_create(&threads[i],
                       thread_stacks[i],
                       K_THREAD_STACK_SIZEOF(thread_stacks[i]),
                       sensor_thread,
                       (void *)(uintptr_t)(i + 1), NULL, NULL,
                       K_PRIO_PREEMPT(5), 0, K_NO_WAIT);
    }
    
    // Aguardar todas as threads terminarem
    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_join(&threads[i], K_FOREVER);
    }
    
    // Resultado final
    printk("\n*** Resultado Final ***\n");
    printk("Total de operacoes: %lu\n", operation_counter);
    printk("Race conditions detectadas: %s\n", 
           race_condition_detected ? "SIM - SISTEMA INCONSISTENTE!" : "Nenhuma");
    
    if (race_condition_detected) {
        printk("*** ALERTA: Dados do sensor corrompidos por race condition! ***\n");
        // Piscar todos os LEDs para indicar erro
        while (1) {
            gpio_pin_set_dt(&led0, 1);
            gpio_pin_set_dt(&led1, 1);
            gpio_pin_set_dt(&led2, 1);
            k_sleep(K_MSEC(500));
            gpio_pin_set_dt(&led0, 0);
            gpio_pin_set_dt(&led1, 0);
            gpio_pin_set_dt(&led2, 0);
            k_sleep(K_MSEC(500));
        }
    } else {
        printk("Sistema operou corretamente (improvável sem sincronizacao)\n");
    }
}