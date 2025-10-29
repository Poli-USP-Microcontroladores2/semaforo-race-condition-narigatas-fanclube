#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

/* Registro do módulo de log */
LOG_MODULE_REGISTER(race_demo, LOG_LEVEL_INF);

/* Prioridades e tamanho das pilhas das threads */
#define PRIO_THREAD_A 5
#define PRIO_THREAD_B 6
#define STACK_SIZE 512

/* Variável global compartilhada */
static volatile int shared_counter = 0;

/* DEFINE E INICIALIZA O MUTEX */
K_MUTEX_DEFINE(counter_mutex);

/*
 * Função de incremento segura (Thread-Safe):
 * 1. Tenta bloquear o mutex. A thread para aqui se o mutex estiver trancado.
 * 2. Lê, atrasa e escreve o valor da variável compartilhada (Seção Crítica).
 * 3. Libera o mutex, permitindo que outra thread entre.
 */
static void safe_increment(const char *owner)
{
    int local_value;
    int ret;

    /* PASSO 1: Tenta bloquear o mutex (espera indefinidamente se ocupado) */
    ret = k_mutex_lock(&counter_mutex, K_FOREVER);
    if (ret != 0) {
        LOG_ERR("Falha ao trancar o mutex! Erro: %d", ret);
        return;
    }

    // ================= SEÇÃO CRÍTICA =================
    /* Etapa 1: leitura */
    local_value = shared_counter;
    LOG_INF("[%s] read value: %d", owner, local_value);

    /* Etapa 2: simulação de atraso (o mutex impede o race condition) */
    k_msleep(50);

    /* Etapa 3: escrita */
    local_value = local_value + 1;
    shared_counter = local_value;
    LOG_INF("[%s] wrote value: %d", owner, local_value);
    // ================= FIM SEÇÃO CRÍTICA =================

    /* PASSO 2: Libera o mutex */
    k_mutex_unlock(&counter_mutex);
}

/* Thread A: incrementa continuamente */
void thread_a(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        safe_increment("A");
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
        safe_increment("B");
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
    printk("=== Demonstração de Solução de Condição de Corrida (Mutex) ===\n");
    printk("Duas threads incrementam a mesma variável COM sincronização.\n");
    printk("shared_counter inicia em %d\n\n", shared_counter);

    /* Loop principal: apenas exibe o valor observado periodicamente */
    while (1) {
        k_msleep(1000);
        printk("-> observed shared_counter = %d\n", shared_counter);
    }
}


