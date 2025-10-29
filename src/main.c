#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 1024
#define THREAD_COUNT 3
#define OPERATION_COUNT 50   /* Mantido para comparação */

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
#define PREEMPTOR_PRIO  K_PRIO_PREEMPT(1)
#define SENSOR_PRIO     K_PRIO_PREEMPT(6)

/* Thread preemptora */
K_THREAD_STACK_DEFINE(preemptor_stack, STACK_SIZE);
struct k_thread preemptor_thread_data;

/* Recurso compartilhado */
struct sensor_data {
    uint32_t timestamp;
    int16_t temperature;
    uint8_t sequence;
    bool initialized;
};

volatile struct sensor_data shared_sensor_data = {0};
volatile uint32_t operation_counter = 0;
volatile bool race_condition_detected = false;

/* ✅ Mutex para sincronização */
K_MUTEX_DEFINE(sensor_mutex);

/* Trabalho CPU-bound para simular carga */
static void cpu_work_cycles(uint32_t cycles)
{
    volatile uint32_t counter = 0;
    for (uint32_t i = 0; i < cycles; i++) {
        counter += i;
    }
}

/* Thread preemptora */
void preemptor_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    k_sleep(K_MSEC(2));

    while (1) {
        k_sleep(K_MSEC(8));
        cpu_work_cycles(25000);
        k_yield();
    }
}

/* ✅ Região crítica agora protegida por mutex */
void sensor_operation_critical(uint8_t thread_id, uint32_t base_timestamp)
{
    k_mutex_lock(&sensor_mutex, K_FOREVER);

    if (!shared_sensor_data.initialized) {
        shared_sensor_data.initialized = true;
        shared_sensor_data.sequence = 0;
        printk("Thread %d: Inicializando sensor (base=%lu)\n", thread_id, base_timestamp);
    }

    shared_sensor_data.timestamp = base_timestamp + thread_id;
    shared_sensor_data.temperature = 20 + thread_id;
    shared_sensor_data.sequence++;

    uint32_t expected_timestamp = base_timestamp + thread_id;
    int16_t expected_temp = 20 + thread_id;

    uint32_t observed_ts = shared_sensor_data.timestamp;
    int16_t observed_temp = shared_sensor_data.temperature;
    uint8_t observed_seq = shared_sensor_data.sequence;

    bool ok = (observed_ts == expected_timestamp &&
               observed_temp == expected_temp &&
               observed_seq >= 1);

    k_mutex_unlock(&sensor_mutex);

    if (!ok) {
        printk("Thread %d: *** ERRO! Inconsistência detectada (não esperado com mutex) ***\n",
               thread_id);
        race_condition_detected = true;
    } else {
        printk("Thread %d: OK - timestamp:%lu, temp:%d°C, seq:%u\n",
               thread_id, observed_ts, observed_temp, observed_seq);
    }

    operation_counter++;
}

/* Função da thread de sensor */
void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    uint8_t thread_id = (uint8_t)(uintptr_t)arg1;
    uint32_t base_timestamp = 1000 * thread_id;

    for (int i = 0; i < OPERATION_COUNT; i++) {
        uint32_t iter_base = base_timestamp + (i * 100);
        sensor_operation_critical(thread_id, iter_base);
        k_sleep(K_MSEC(5));
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
    printk("\n*** DEMO CORRIGIDA: Race Condition Eliminada com Mutex - FRDM-KL25Z ***\n");

    if (setup_leds() != 0) {
        printk("Erro ao configurar LEDs!\n");
        return;
    }

    k_thread_create(&preemptor_thread_data,
                    preemptor_stack,
                    K_THREAD_STACK_SIZEOF(preemptor_stack),
                    preemptor_thread,
                    NULL, NULL, NULL,
                    PREEMPTOR_PRIO, 0, K_NO_WAIT);

    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_create(&threads[i],
                        thread_stacks[i],
                        K_THREAD_STACK_SIZEOF(thread_stacks[i]),
                        sensor_thread,
                        (void *)(uintptr_t)(i + 1), NULL, NULL,
                        SENSOR_PRIO, 0, K_NO_WAIT);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        k_thread_join(&threads[i], K_FOREVER);
    }

    printk("\n*** Resultado Final ***\n");
    printk("Total de operacoes: %lu\n", operation_counter);
    printk("Race conditions detectadas: %s\n",
           race_condition_detected ? "SIM (não esperado!)" : "NÃO - SISTEMA CONSISTENTE ✅");
}
