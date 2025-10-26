/*
 * Demonstração forte de Race Condition no Zephyr RTOS v4.2
 * Placa: FRDM-KL25Z
 *
 * *** VERSÃO CORRIGIDA COM MUTEX ***
 *
 * Estratégia:
 * - Contador 64 bits dividido em duas partes de 32 bits.
 * - Várias threads incrementam esse contador.
 * - Um Mutex (counter_mutex) protege TODAS as leituras e escritas.
 * - Uma thread monitora inconsistências (agora não deve encontrar nenhuma).
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>
#include <inttypes.h>

#define STACKSIZE        1024
#define THREAD_PRIORITY  7
#define NUM_WORKERS      8
// #define MAX_BUSY_US      30000U // Não é mais necessário dentro da seção crítica
#define ITER_DELAY_MS    5U

typedef struct {
    volatile uint32_t low;
    volatile uint32_t high;
} split_counter_t;

static split_counter_t shared_counter = {0, 0};

// <<< MUDANÇA: 1. Definir o Mutex >>>
// Define um mutex para proteger o acesso ao shared_counter
K_MUTEX_DEFINE(counter_mutex);


/* Stacks e structs das threads */
K_THREAD_STACK_ARRAY_DEFINE(worker_stacks, NUM_WORKERS, STACKSIZE);
static struct k_thread worker_threads[NUM_WORKERS];

K_THREAD_STACK_DEFINE(monitor_stack, STACKSIZE);
static struct k_thread monitor_thread_data;

// <<< MUDANÇA: 2. Proteger a função de incremento (escrita) >>>
// Renomeada de "non_atomic_increment" para "protected_increment"
void protected_increment(split_counter_t *c)
{
    // Trava o mutex antes de acessar o recurso compartilhado
    k_mutex_lock(&counter_mutex, K_FOREVER);

    // --- Início da Seção Crítica ---
    uint32_t low = c->low;

    // Os k_busy_wait() foram REMOVIDOS daqui.
    // Não queremos segurar o mutex durante uma espera.

    low++;
    c->low = low;

    if (low == 0) {
        // Ocorreu overflow na parte baixa, incrementa a parte alta
        uint32_t high = c->high;
        c->high = high + 1;
    }
    // --- Fim da Seção Crítica ---

    // Libera o mutex
    k_mutex_unlock(&counter_mutex);
}

// <<< MUDANÇA: 3. Proteger a função de leitura >>>
uint64_t read_combined(split_counter_t *c)
{
    uint64_t combined_value;

    // Trava o mutex antes de ler
    k_mutex_lock(&counter_mutex, K_FOREVER);

    // --- Início da Seção Crítica ---
    
    // Como o acesso está protegido, não precisamos mais da lógica
    // complexa de ler 'high' duas vezes. A leitura será consistente.
    combined_value = ((uint64_t)c->high << 32) | c->low;
    
    // --- Fim da Seção Crítica ---

    // Libera o mutex
    k_mutex_unlock(&counter_mutex);

    return combined_value;
}

/* Thread de trabalho */
void worker_entry(void *p1, void *p2, void *p3)
{
    int id = (int)(uintptr_t)p1;

    while (1) {
        // <<< MUDANÇA: 4. Chamar a função protegida >>>
        protected_increment(&shared_counter);

        k_yield();
        k_msleep(ITER_DELAY_MS);

        if ((k_cycle_get_32() & 0xFF) == 0) {
            // A função read_combined() já está protegida internamente
            uint64_t v = read_combined(&shared_counter);
            printk("[W%d] snapshot = %" PRIu64 "\n", id, v);
        }
    }
}

/* Thread monitor */
void monitor_entry(void *p1, void *p2, void *p3)
{
    uint64_t last = 0;

    printk(">>> MONITOR INICIADO. Não devem ocorrer erros.\n");

    while (1) {
        // A função read_combined() já está protegida internamente
        uint64_t now = read_combined(&shared_counter);

        if (now < last) {
            printk(">>> MONITOR: REGRESSÃO! last=%" PRIu64 " now=%" PRIu64 "\n", last, now);
        } else if (now == last) {
            // Esta repetição ainda pode ocorrer se o monitor
            // executar duas vezes antes de qualquer worker incrementar.
            // Isso NÃO é uma race condition, é apenas amostragem.
        } else if (now - last > (NUM_WORKERS * 2)) { 
            // Aumentei a tolerância de salto, já que os workers
            // podem incrementar várias vezes entre as leituras do monitor.
            // O importante é que 'now' nunca será inconsistente (ex: 0x1 FFFFFFFF)
            printk(">>> MONITOR: SALTO GRANDE! last=%" PRIu64 " now=%" PRIu64 "\n", last, now);
        }

        last = now;
        k_msleep(50);
    }
}

/* Função principal */
void main(void)
{
    printk("\n=== Demonstração de Correção de Race Condition (Mutex) ===\n"); // <<< MUDANÇA
    printk("FRDM-KL25Z | Zephyr 4.2\n");
    printk("%d threads incrementam o mesmo contador com sincronização (mutex).\n", NUM_WORKERS); // <<< MUDANÇA
    printk("O monitor não deve detectar regressões.\n\n"); // <<< MUDANÇA

    for (int i = 0; i < NUM_WORKERS; i++) {
        k_thread_create(&worker_threads[i], worker_stacks[i],
                        K_THREAD_STACK_SIZEOF(worker_stacks[i]),
                        worker_entry,
                        (void *)(uintptr_t)(i + 1), NULL, NULL,
                        THREAD_PRIORITY, 0, K_NO_WAIT);

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