/*
 * Demonstração forte de Race Condition no Zephyr RTOS v4.2
 * Placa: FRDM-KL25Z
 *
 * Estratégia:
 *  - Contador 64 bits dividido em duas partes de 32 bits.
 *  - Várias threads incrementam esse contador de forma não atômica.
 *  - Inserção de delays e yields para forçar preempções.
 *  - Uma thread monitora inconsistências (regressões, saltos, repetições).
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>
#include <inttypes.h>

#define STACKSIZE        1024
#define THREAD_PRIORITY  7
#define NUM_WORKERS      8
#define MAX_BUSY_US      30000U   /* microsegundos de espera entre etapas */
#define ITER_DELAY_MS    5U

typedef struct {
    volatile uint32_t low;
    volatile uint32_t high;
} split_counter_t;

static split_counter_t shared_counter = {0, 0};

/* Stacks e structs das threads */
K_THREAD_STACK_ARRAY_DEFINE(worker_stacks, NUM_WORKERS, STACKSIZE);
static struct k_thread worker_threads[NUM_WORKERS];

K_THREAD_STACK_DEFINE(monitor_stack, STACKSIZE);
static struct k_thread monitor_thread_data;

/* Incremento não atômico */
void non_atomic_increment(split_counter_t *c)
{
    uint32_t low = c->low;

    uint32_t wait = (k_cycle_get_32() % MAX_BUSY_US) + 1;
    k_busy_wait(wait);

    low++;
    c->low = low;

    if (low == 0) {
        uint32_t high = c->high;
        wait = (k_cycle_get_32() % (MAX_BUSY_US / 4)) + 1;
        k_busy_wait(wait);
        c->high = high + 1;
    }
}

/* Leitura "não atômica" */
uint64_t read_combined(split_counter_t *c)
{
    uint32_t high1 = c->high;
    uint32_t low   = c->low;
    uint32_t high2 = c->high;

    if (high1 != high2) {
        return ((uint64_t)high2 << 32) | low;
    } else {
        return ((uint64_t)high1 << 32) | low;
    }
}

/* Thread de trabalho */
void worker_entry(void *p1, void *p2, void *p3)
{
    int id = (int)(uintptr_t)p1;

    while (1) {
        non_atomic_increment(&shared_counter);

        k_yield();
        k_msleep(ITER_DELAY_MS);

        if ((k_cycle_get_32() & 0xFF) == 0) {
            uint64_t v = read_combined(&shared_counter);
            printk("[W%d] snapshot = %" PRIu64 " (h=0x%08x l=0x%08x)\n",
                   id, v, (uint32_t)(v >> 32), (uint32_t)(v & 0xFFFFFFFF));
        }
    }
}

/* Thread monitor */
void monitor_entry(void *p1, void *p2, void *p3)
{
    uint64_t last = 0;

    while (1) {
        uint64_t now = read_combined(&shared_counter);

        if (now < last) {
            printk(">>> MONITOR: REGRESSÃO! last=%" PRIu64 " now=%" PRIu64 "\n", last, now);
        } else if (now == last) {
            printk(">>> MONITOR: REPETIÇÃO! value=%" PRIu64 "\n", now);
        } else if (now - last > 1) {
            printk(">>> MONITOR: SALTO! last=%" PRIu64 " now=%" PRIu64 "\n", last, now);
        }

        last = now;
        k_msleep(50);
    }
}

/* Função principal */
void main(void)
{
    printk("\n=== Demonstração de Race Condition (contador 64-bit dividido) ===\n");
    printk("FRDM-KL25Z | Zephyr 4.2\n");
    printk("%d threads incrementam o mesmo contador sem sincronização.\n", NUM_WORKERS);
    printk("O monitor detectará regressões, repetições e saltos.\n\n");

    for (int i = 0; i < NUM_WORKERS; i++) {
        k_thread_create(&worker_threads[i], worker_stacks[i],
                        K_THREAD_STACK_SIZEOF(worker_stacks[i]),
                        worker_entry,
                        (void *)(uintptr_t)(i + 1), NULL, NULL,
                        THREAD_PRIORITY, 0, K_NO_WAIT);

        /* Define o nome corretamente (2 argumentos apenas) */
        char name[16];
        snprintf(name, sizeof(name), "worker%d", i + 1);
        k_thread_name_set(&worker_threads[i], name);
    }

    k_thread_create(&monitor_thread_data, monitor_stack,
                    K_THREAD_STACK_SIZEOF(monitor_stack),
                    monitor_entry,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&monitor_thread_data, "monitor");

    while (1) {
        k_msleep(1000);
    }
}
