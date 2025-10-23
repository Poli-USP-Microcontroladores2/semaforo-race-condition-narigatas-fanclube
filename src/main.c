#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

/* Registro do módulo de log (permite usar LOG_INF com timestamp) */
LOG_MODULE_REGISTER(race_demo, LOG_LEVEL_INF);

/* Prioridades e tamanho das pilhas das threads */
#define PRIO_THREAD_A 5
#define PRIO_THREAD_B 6
#define STACK_SIZE 512

/* Variável global compartilhada (sem proteção) */
static volatile int shared_counter = 0;

/*
 * Função de incremento insegura:
 * 1. Lê o valor atual da variável compartilhada.
 * 2. Dorme por ~50 ms para simular uma troca de contexto.
 * 3. Incrementa o valor local e grava de volta.
 * 
 * Esse comportamento causa uma condição de corrida, pois duas threads podem
 * ler o mesmo valor antes de gravar, resultando em "incrementos perdidos".
 */
static void unsafe_increment(const char *owner)
{
    int local_value;

    /* Etapa 1: leitura */
    local_value = shared_counter;
    LOG_INF("[%s] read value: %d", owner, local_value);

    /* Etapa 2: simulação de atraso (context switch) */
    k_msleep(50);

    /* Etapa 3: escrita */
    local_value = local_value + 1;
    shared_counter = local_value;
    LOG_INF("[%s] wrote value: %d", owner, local_value);
}

/* Thread A: incrementa continuamente */
void thread_a(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        unsafe_increment("A");
        k_msleep(100);  /* pequeno atraso entre incrementos */
    }
}

/* Thread B: faz o mesmo que A */
void thread_b(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        unsafe_increment("B");
        k_msleep(100);
    }
}

/* Criação das threads */
K_THREAD_DEFINE(thread_a_id, STACK_SIZE, thread_a, NULL, NULL, NULL,
                PRIO_THREAD_A, 0, 0);

K_THREAD_DEFINE(thread_b_id, STACK_SIZE, thread_b, NULL, NULL, NULL,
                PRIO_THREAD_B, 0, 0);

/* Função principal */
void main(void)
{
    printk("=== Demonstração de Condição de Corrida no Zephyr RTOS ===\n");
    printk("Duas threads incrementam a mesma variável sem sincronização.\n");
    printk("shared_counter inicia em %d\n\n", shared_counter);

    /* Loop principal: apenas exibe o valor observado periodicamente */
    while (1) {
        k_msleep(1000);
        printk("-> observed shared_counter = %d\n", shared_counter);
    }
}



