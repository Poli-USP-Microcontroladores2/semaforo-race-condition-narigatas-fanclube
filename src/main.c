#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 1024
#define THREAD_COUNT 3
#define OPERATION_COUNT 50   /* Aumentado para estresse/agressivo */

/* Definições dos LEDs da FRDM-KL25Z */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

/* Pilhas e control blocks das threads de sensor */
K_THREAD_STACK_ARRAY_DEFINE(thread_stacks, THREAD_COUNT, STACK_SIZE);
struct k_thread threads[THREAD_COUNT];

/* Prioridades: menor número = maior prioridade */
#define PREEMPTOR_PRIO  K_PRIO_PREEMPT(1)   /* prioridade bem alta (agressiva) */
#define SENSOR_PRIO     K_PRIO_PREEMPT(6)   /* prioridade média para sensores */

/* Thread preemptor */
K_THREAD_STACK_DEFINE(preemptor_stack, STACK_SIZE);
struct k_thread preemptor_thread_data;

/* Recurso compartilhado (sem proteção) */
struct sensor_data {
    uint32_t timestamp;
    int16_t temperature;
    uint8_t sequence;
    bool initialized;
};

volatile struct sensor_data shared_sensor_data = {0};
volatile uint32_t operation_counter = 0;
volatile bool race_condition_detected = false;

/* Trabalho CPU-bound para abrir janelas de preempção.
 * Valor relativamente grande para ser agressivo em demonstração.
 */
static void cpu_work_cycles(uint32_t cycles)
{
    volatile uint32_t counter = 0;
    for (uint32_t i = 0; i < cycles; i++) {
        counter += i;
    }
}

/* Thread preemptor: acorda frequentemente e faz trabalho CPU-bound curto.
 * Isso força o scheduler a preemptar as threads de sensor em momentos variados.
 */
void preemptor_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    /* Pequeno atraso inicial para deixar threads de sensor iniciarem quase ao mesmo tempo */
    k_sleep(K_MSEC(2));

    while (1) {
        /* Frequência agressiva: acorda a cada 8 ms */
        k_sleep(K_MSEC(8));

        /* Trabalho intenso curto para preempção (aumente se quiser mais efeito) */
        cpu_work_cycles(25000);

        /* Forçar cede explícita (ajuda em sistemas coop./preemptivos) */
        k_yield();
    }
}

/* Operação crítica (SEM proteção) — estrutura com vários pontos que podem ser preemptados */
void sensor_operation_critical(uint8_t thread_id, uint32_t base_timestamp)
{
    /* 1) Inicialização condicional */
    if (!shared_sensor_data.initialized) {
        /* trabalho breve para abrir janela */
        cpu_work_cycles(4000);
        k_yield();

        shared_sensor_data.initialized = true;
        shared_sensor_data.sequence = 0;
        printk("Thread %d: Inicializando sensor (base=%lu)\n", thread_id, base_timestamp);
    }

    /* 2) Escrever timestamp */
    cpu_work_cycles(3000);
    k_yield();
    shared_sensor_data.timestamp = base_timestamp + thread_id;

    /* 3) Mais trabalho e possível preempção */
    cpu_work_cycles(3000);
    k_yield();

    /* 4) Escrever temperatura */
    cpu_work_cycles(3000);
    k_yield();
    shared_sensor_data.temperature = 20 + thread_id;

    /* 5) Trabalho antes de incrementar sequência */
    cpu_work_cycles(3000);
    k_yield();
    shared_sensor_data.sequence++;

    /* 6) Verificação final */
    cpu_work_cycles(2000);
    k_yield();

    uint32_t expected_timestamp = base_timestamp + thread_id;
    int16_t expected_temp = 20 + thread_id;
    uint8_t observed_seq = shared_sensor_data.sequence;
    uint32_t observed_ts = shared_sensor_data.timestamp;
    int16_t observed_temp = shared_sensor_data.temperature;

    if (observed_ts != expected_timestamp ||
        observed_temp != expected_temp ||
        observed_seq == 0) {

        printk("Thread %d: *** RACE CONDITION! Esperado(ts=%lu,temp=%d,seq>=1) -> Recebido(ts=%lu,temp=%d,seq=%u)\n",
               thread_id, expected_timestamp, expected_temp, observed_ts, observed_temp, observed_seq);

        race_condition_detected = true;

        /* Acender LED indicando qual thread detectou */
        switch (thread_id) {
            case 1: gpio_pin_set_dt(&led0, 1); break;
            case 2: gpio_pin_set_dt(&led1, 1); break;
            case 3: gpio_pin_set_dt(&led2, 1); break;
            default: break;
        }
    } else {
        printk("Thread %d: Leitura OK - timestamp: %lu, temp: %d°C, seq: %u\n",
               thread_id, observed_ts, observed_temp, observed_seq);
    }

    operation_counter++;
}

/* Função da thread de sensor */
void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    uint8_t thread_id = (uint8_t)(uintptr_t)arg1;
    uint32_t base_timestamp = 1000 * thread_id;

    /* Indica atividade */
    switch (thread_id) {
        case 1: gpio_pin_set_dt(&led0, 1); break;
        case 2: gpio_pin_set_dt(&led1, 1); break;
        case 3: gpio_pin_set_dt(&led2, 1); break;
    }

    for (int i = 0; i < OPERATION_COUNT; i++) {
        uint32_t iter_base = base_timestamp + (i * 100);

        /* Região crítica sem proteção */
        sensor_operation_critical(thread_id, iter_base);

        /* Sleep curto e constante entre operações (não é fonte de RNG) */
        k_sleep(K_MSEC(5));
    }

    /* Apagar LED ao terminar */
    switch (thread_id) {
        case 1: gpio_pin_set_dt(&led0, 0); break;
        case 2: gpio_pin_set_dt(&led1, 0); break;
        case 3: gpio_pin_set_dt(&led2, 0); break;
    }
}

/* Configurar LEDs */
int setup_leds(void)
{
    int ret;

    if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2)) {
        return -1;
    }

    if ((ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE)) < 0) return ret;
    if ((ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE)) < 0) return ret;
    if ((ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE)) < 0) return ret;

    gpio_pin_set_dt(&led0, 0);
    gpio_pin_set_dt(&led1, 0);
    gpio_pin_set_dt(&led2, 0);

    return 0;
}

void main(void)
{
    printk("\n*** Demo AGRESSIVA: Race Condition - Zephyr RTOS - FRDM-KL25Z ***\n");
    printk("OPERATION_COUNT=%d | Preemptor: sleep=8ms cpu_work=25000 cycles\n\n", OPERATION_COUNT);

    if (setup_leds() != 0) {
        printk("Erro ao configurar LEDs!\n");
        return;
    }

    /* Criar thread preemptora de alta prioridade */
    k_thread_create(&preemptor_thread_data,
                    preemptor_stack,
                    K_THREAD_STACK_SIZEOF(preemptor_stack),
                    preemptor_thread,
                    NULL, NULL, NULL,
                    PREEMPTOR_PRIO, 0, K_NO_WAIT);

    /* Criar threads de sensor (prioridade menor que preemptor) */
    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_create(&threads[i],
                        thread_stacks[i],
                        K_THREAD_STACK_SIZEOF(thread_stacks[i]),
                        sensor_thread,
                        (void *)(uintptr_t)(i + 1), NULL, NULL,
                        SENSOR_PRIO, 0, K_NO_WAIT);
    }

    /* Opcional: aguardar término das threads de sensor.
     * Como OPERATION_COUNT é grande, isso leva tempo; mantenha para relatório final.
     */
    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_join(&threads[i], K_FOREVER);
    }

    printk("\n*** Resultado Final ***\n");
    printk("Total de operacoes: %lu\n", operation_counter);
    printk("Race conditions detectadas: %s\n",
           race_condition_detected ? "SIM - SISTEMA INCONSISTENTE!" : "Nenhuma");

    if (race_condition_detected) {
        printk("*** ALERTA: Dados do sensor corrompidos por race condition! ***\n");
        while (1) {
            gpio_pin_set_dt(&led0, 1);
            gpio_pin_set_dt(&led1, 1);
            gpio_pin_set_dt(&led2, 1);
            k_sleep(K_MSEC(200));
            gpio_pin_set_dt(&led0, 0);
            gpio_pin_set_dt(&led1, 0);
            gpio_pin_set_dt(&led2, 0);
            k_sleep(K_MSEC(200));
        }
    } else {
        printk("Sistema operou corretamente (muito improvável sem sincronizacao)\n");
    }
}
