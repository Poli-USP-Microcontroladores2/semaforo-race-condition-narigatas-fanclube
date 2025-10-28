# 🧪 CASO DE TESTE 1: DEMONSTRAÇÃO E CORREÇÃO DE RACE CONDITION NO ZEPHYR RTOS 4.2

## 🔎 OBJETIVO
Demonstrar o comportamento de uma Race Condition em um sistema embarcado com múltiplas threads acessando o mesmo recurso compartilhado sem controle de acesso, e em seguida apresentar a correção utilizando um **semáforo (k_sem)** para garantir sincronização adequada entre as threads.

---

## ⚙️ PRÉ-CONDIÇÃO
- A placa **FRDM-KL25Z** deve estar corretamente conectada à fonte de energia ou à porta USB do computador.  
- O ambiente de desenvolvimento deve estar configurado com **Zephyr RTOS 4.2** e suporte à placa FRDM-KL25Z.  
- O terminal serial deve estar aberto e configurado corretamente (exemplo: `COM5`, **baud rate 115200**).  
- O código-fonte `main.c` deve conter duas versões de execução:  
  - **Versão 1:** acesso concorrente à variável `shared_counter` **sem sincronização**.  
  - **Versão 2:** acesso controlado com o uso de **semáforo (k_sem)**.

---

## 🧩 ETAPAS DE TESTE

### 🧪 Etapa 1 — Execução sem sincronização
1. Compilar e gravar a **versão sem semáforo** do firmware na placa FRDM-KL25Z.  
2. Executar o sistema e observar a saída no terminal serial por aproximadamente **30 segundos**.  
3. Identificar mensagens no formato:
[A] read value: X
[B] read value: X
[A] wrote value: Y
[B] wrote value: Y
4. Verificar se as duas threads (`A` e `B`) acessam simultaneamente a variável `shared_counter`.  
5. Observar que o valor final não corresponde ao número esperado de incrementos — comportamento incorreto devido à Race Condition.  
6. Exemplo de saída observada:
[A] read value: 2
[B] read value: 2
[A] wrote value: 3
[B] wrote value: 3 -> shared_counter = 3


---

### 🧪 Etapa 2 — Execução com semáforo
1. Compilar e gravar a **versão corrigida** do firmware, agora utilizando um **semáforo (k_sem)** para proteger o acesso ao recurso compartilhado.  
2. Executar o sistema novamente e observar a saída no terminal serial.  
3. Verificar que as threads agora acessam a variável de forma alternada e ordenada.  
4. Exemplo de saída corrigida:
race_demo: [A] read value: 2
race_demo: [A] wrote value: 3
race_demo: [B] read value: 3
race_demo: [B] wrote value: 4
race_demo: [A] read value: 4
race_demo: [A] wrote value: 5
race_demo: [B] read value: 5
race_demo: [B] wrote value: 6
5. Observar que o contador (`shared_counter`) aumenta de forma previsível e consistente.  
6. Confirmar que não há mais sobreposição de acesso nem perda de incrementos.

---

## ✅ PÓS-CONDIÇÃO ESPERADA
- **Antes da correção (sem sincronização):**
- O terminal exibe mensagens de leitura e escrita intercaladas entre as threads, com valores inconsistentes.  
- O contador apresenta **valores incorretos**, não refletindo todos os incrementos.  
- Fica evidente a presença de **Race Condition** — o sistema falha em garantir acesso exclusivo ao recurso.  

- **Após a correção (com semáforo):**
- O terminal exibe mensagens de acesso alternado e previsível.  
- O valor de `shared_counter` é atualizado de forma **sequencial e consistente**.  
- Não há mais conflitos de leitura ou escrita simultânea.  
- O uso do **semáforo (k_sem)** garante exclusão mútua e sincronização adequada entre as threads.  

---

## 🧰 AMBIENTE DE TESTE
- **Hardware:** FRDM-KL25Z  
- **RTOS:** Zephyr 4.2  
- **Linguagem:** C/C++  
- **Recursos Utilizados:** Threads, Semáforos (`k_sem`), UART/Serial Output  

---

## 📜 CONCLUSÃO
Este caso de teste demonstra de forma clara o impacto da ausência de sincronização em sistemas multitarefa e como o uso de semáforos corrige o problema.  
Sem sincronização, o sistema apresenta **Race Condition**, resultando em comportamento imprevisível e dados incorretos.  
Com o uso do **semáforo (k_sem)**, o acesso ao recurso compartilhado passa a ser controlado, garantindo que apenas uma thread entre na seção crítica por vez.  
Como resultado, o sistema opera de forma **determinística, estável e previsível**, eliminando completamente a Race Condition e garantindo integridade no valor do contador global.
