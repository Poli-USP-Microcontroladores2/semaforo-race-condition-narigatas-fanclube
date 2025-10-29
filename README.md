# 🧪 Planejamento de Testes – Demonstração Agressiva de Race Condition no Zephyr RTOS

## 🎯 Objetivo

Evidenciar a ocorrência de **race conditions** causadas por acesso concorrente ao recurso compartilhado `shared_sensor_data` entre múltiplas threads de sensor, sob forte preempção induzida por uma **thread de alta prioridade**, utilizando a placa **FRDM-KL25Z** e o Zephyr RTOS.

---

## 🧩 Casos de Teste 

| **Caso de Teste**                                                                                  | **Pré-condição**                                                                                                       | **Etapas de Teste**                                                                                                                                                                                                       | **Resultado Esperado**                                                                                                                                                                                                        |
| -------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **1. Execução Base com Threads Concorrentes e Preempção** *(Caso principal e obrigatório)*         | - Firmware original gravado na FRDM-KL25Z.<br>- Comunicação serial ativa a 115200 baud.<br>- LEDs da placa funcionais. | 1. Energizar a placa.<br>2. Observar atividade dos LEDs das 3 threads de sensor.<br>3. Monitorar o terminal serial.<br>4. Anotar ocorrências de `*** RACE CONDITION!`.                                                    | - Logs exibem mensagens de race condition.<br>- Dados inconsistentes de timestamp/temperatura/sequence aparecem.<br>- LED da thread que detectou o problema acende.<br>- Ao final, LEDs piscam juntos indicando falha global. |
| **2. Execução Repetida para Observação de Não-Determinismo** *(Mostra comportamento imprevisível)* | - Mesmo firmware do Caso 1.<br>- Terminal conectado.                                                                   | 1. Executar o firmware pelo menos 5 vezes (reset entre execuções).<br>2. Registrar qual thread detecta o erro primeiro em cada execução.<br>3. Comparar ordem e quantidade de erros entre execuções.                      | - A falha ocorre em execuções diferentes, porém **não de forma igual**.<br>- Ordem das mensagens e thread afetada varia.<br>- Evidencia comportamento não determinístico típico de race condition.                            |
| **3. Teste de Estresse Aumentando Preempção** *(Sensibiliza o erro para facilitar visualização)*   | - Firmware modificado para aumentar carga de CPU dentro de `cpu_work_cycles()` (ex.: dobrar valores).                  | 1. Alterar valores de `cpu_work_cycles()` para aumentar janelas de preempção.<br>2. Recompilar, gravar e executar.<br>3. Observar logs e intensidade dos LEDs.<br>4. Contar quantidade de `*** RACE CONDITION!` exibidas. | - Quantidade de erros aumenta significativamente.<br>- Race condition ocorre mais cedo e mais vezes.<br>- LEDs mostram interferência mais intensa.<br>- Demonstra que quanto maior a preempção, maior o risco de corrida.     |

---


## 🧠 Observações Importantes

* A **thread preemptora** é responsável por gerar preempção frequente, criando as janelas críticas onde o contexto muda no meio de operações não atômicas.
* A variável `shared_sensor_data` contém múltiplos campos, sendo escrita e lida parcialmente — situação clássica de race condition em estruturas compostas.
* `volatile` **não elimina** race conditions — apenas impede otimizações de compilador.
* `k_yield()` foi propositalmente adicionado para forçar mudanças de contexto nos piores momentos possíveis.

---

## 📊 Resultados Esperados

| **Indicador**                                | **Descrição**                          |
| -------------------------------------------- | -------------------------------------- |
| Logs contendo `*** RACE CONDITION!`          | Evidência direta da falha              |
| Campos inconsistentes (timestamp, temp, seq) | Prova de corrupção de dados            |
| LED da thread que detectou o problema acende | Identifica qual thread observou o erro |
| LEDs piscando juntos no final                | Indica falha global após detecção      |

---

## 🧩 Conclusão

Os testes demonstram que:

* A ausência de mecanismos de sincronização como `k_mutex`, `k_spinlock`, atomic APIs ou semáforos leva a **corrupção de dados compartilhados** em ambiente multitarefa.
* A introdução de uma thread preemptora de alta prioridade gera um cenário ideal para **exposição de race conditions ocultas**.
* O comportamento **não determinístico** observado nas execuções prova a imprevisibilidade e o risco do acesso concorrente sem proteção.