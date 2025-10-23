/*
 * Copyright (c) 2024, Exemplo Acadêmico para Poli-USP
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/*
 * ARQUIVO: main.c
 * OBJETIVO: Demonstrar uma Race Condition no Zephyr RTOS v4.2
 * utilizando um LED (visual) e o console serial (logs).
 * PLACA: FRDM-KL25Z
 *
 * DESCRIÇÃO:
 * Este código cria duas threads de mesma prioridade que disputam o controle
 * de dois recursos compartilhados sem qualquer mecanismo de sincronização:
 * 1. O LED azul da placa (recurso de hardware).
 * 2. O console serial (acessado via `printk`).
 *
 * - thread_a_fast_blinker: Tenta piscar o LED rapidamente e imprime um log.
 * - thread_b_slow_blinker: Tenta piscar o LED lentamente e imprime um log.
 *
 * A "condição de corrida" acontece porque o escalonador do Zephyr pode
 * interromper (preemptar) uma thread a qualquer momento para executar a outra.
 * Uma thread pode ser interrompida, por exemplo, depois de imprimir seu log,
 * mas antes de conseguir alterar o estado do LED. A outra thread então assume,
 * imprime seu próprio log e altera o LED.
 *
 * RESULTADO ESPERADO:
 * - Visual: O LED piscará de forma caótica, não seguindo nem o padrão rápido
 * nem o lento.
 * - Console Serial: Os logs de [THREAD A] e [THREAD B] aparecerão
 * intercalados e de forma imprevisível, provando que nenhuma das threads
 * consegue executar sua sequência lógica (log -> set led -> sleep) de
 * forma atômica.
 */


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>


/* ========================================================================= */
/* CONFIGURAÇÕES DAS THREADS E DO HARDWARE                            */
/* ========================================================================= */


#define STACKSIZE 1024
#define THREAD_PRIORITY 7 // Mesma prioridade para ambas as threads


// Períodos de pisca para cada thread (em milissegundos)
#define THREAD_A_SLEEP_MS 100 // Pisca Rápido
#define THREAD_B_SLEEP_MS 500 // Pisca Lento


/*
 * OS RECURSOS COMPARTILHADOS E DISPUTADOS:
 * 1. 'led': Estrutura que representa o pino do LED azul. Ambas as threads
 * tentarão escrever neste recurso de hardware.
 * 2. Console (via printk): A UART subjacente é um recurso compartilhado.
 * Ambas as threads tentarão escrever seus logs, disputando o acesso.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);


// Declaração das pilhas e estruturas de controle das threads
K_THREAD_STACK_DEFINE(thread_a_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(thread_b_stack_area, STACKSIZE);
struct k_thread thread_a_data;
struct k_thread thread_b_data;




/* ========================================================================= */
/* DEFINIÇÃO DAS THREADS CONCORRENTES                                 */
/* ========================================================================= */


/**
 * @brief Ponto de entrada da Thread A (Pisca Rápido & Log).
 */
void thread_a_fast_blinker_entry(void *p1, void *p2, void *p3)
{
    bool led_state = true;
    while (1) {
        /*
         * PONTO DA RACE CONDITION (Console e GPIO):
         * A thread A tenta executar duas ações: imprimir e mudar o LED.
         * O escalonador pode pausar esta thread DEPOIS do printk e ANTES
         * do gpio_pin_set_dt, permitindo que a thread B execute e
         * altere o estado do LED, invalidando a intenção da thread A.
         */
        printk("[THREAD A] Mudando estado do LED.\n");
        gpio_pin_set_dt(&led, (int)led_state);
       
        k_msleep(THREAD_A_SLEEP_MS);
        led_state = !led_state;
    }
}


/**
 * @brief Ponto de entrada da Thread B (Pisca Lento & Log).
 */
void thread_b_slow_blinker_entry(void *p1, void *p2, void *p3)
{
    bool led_state = true;
    while (1) {
        /*
         * PONTO DA RACE CONDITION (Console e GPIO):
         * O mesmo problema ocorre aqui. A thread B pode ser interrompida
         * a qualquer momento, fazendo com que a sequência lógica de suas
         * operações seja corrompida pela execução da thread A.
         */
        printk("[THREAD B] ----> Mudando estado do LED.\n");
        gpio_pin_set_dt(&led, (int)led_state);
       
        k_msleep(THREAD_B_SLEEP_MS);
        led_state = !led_state;
    }
}




/* ========================================================================= */
/* FUNÇÃO PRINCIPAL (MAIN)                                            */
/* ========================================================================= */


int main(void)
{
    // 1. Validação e configuração do pino de GPIO do LED
    if (!gpio_is_ready_dt(&led)) {
        printk("Erro: Dispositivo GPIO do LED não está pronto.\n");
        return 0;
    }


    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) {
        printk("Erro: Falha ao configurar o pino do LED.\n");
        return 0;
    }
   
    printk("Iniciando demonstração de Race Condition no Zephyr v4.2\n");
    printk("Observe o LED azul e os logs no console.\n\n");




    // 2. Criação e inicialização das threads
    k_thread_create(&thread_a_data, thread_a_stack_area,
            K_THREAD_STACK_SIZEOF(thread_a_stack_area),
            thread_a_fast_blinker_entry,
            NULL, NULL, NULL,
            THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&thread_a_data, "thread_a_fast_blinker");


    k_thread_create(&thread_b_data, thread_b_stack_area,
            K_THREAD_STACK_SIZEOF(thread_b_stack_area),
            thread_b_slow_blinker_entry,
            NULL, NULL, NULL,
            THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&thread_b_data, "thread_b_slow_blinker");


    // A função main cede o controle para o escalonador do Zephyr.
    return 0;
}