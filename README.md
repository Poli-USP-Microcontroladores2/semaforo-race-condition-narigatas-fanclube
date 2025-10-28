# 🧠 Demonstração de Race Condition no Zephyr RTOS 4.2

Este projeto demonstra o comportamento de **condição de corrida (Race Condition)** em sistemas embarcados utilizando **duas threads** acessando um mesmo recurso compartilhado **sem sincronização**.  
Em seguida, é apresentada a **correção** com o uso de **semáforo**, garantindo o funcionamento esperado.

---

## ⚙️ Contexto

Em sistemas embarcados reais, múltiplas *threads* podem tentar acessar o mesmo recurso — por exemplo, o **display** ou uma **variável compartilhada**.  
Quando isso acontece **sem controle de acesso**, pode ocorrer uma *Race Condition*, onde duas threads executam operações de leitura e escrita simultaneamente, gerando resultados inconsistentes.

---

## 🚫 Comportamento sem Sincronização

**Descrição:**  
As duas funções (threads) leem e escrevem na mesma variável ao mesmo tempo, resultando em comportamento incorreto.

### 🔍 Saída Serial Observada
[A] read value: 2
[B] read value: 2
[A] wrote value: 3
[B] wrote value: 3
-> observed shared_counter = 3


### 💬 Análise
O sistema não realiza seu objetivo corretamente.  
Em vez de cada thread adicionar 1 de forma alternada, ambas tentam adicionar ao mesmo tempo — o resultado final não reflete os dois incrementos esperados.

---

## ✅ Solução: Uso de Semáforo

A correção foi feita utilizando um **semáforo** para garantir que apenas **uma thread** acesse o recurso compartilhado por vez.

### 🔧 Explicação
- O semáforo atua como um **controle de acesso** ao recurso.
- Apenas uma thread pode “entrar na seção crítica”.
- Quando termina, ela libera o semáforo, permitindo que a próxima thread prossiga.

---

## 🧩 Comportamento Corrigido

Agora, o acesso é sincronizado corretamente.  
Cada thread incrementa o contador uma de cada vez.

### 📜 Saída Serial com Semáforo
race_demo: [A] read value: 2
-[00:00:00.200,000] <inf> race_demo: [A] wrote value: 3
-[00:00:00.201,000] <inf> race_demo: [B] read value: 3
-[00:00:00.251,000] <inf> race_demo: [B] wrote value: 4
-[00:00:00.301,000] <inf> race_demo: [A] read value: 4
-[00:00:00.351,000] <inf> race_demo: [A] wrote value: 5
-[00:00:00.351,000] <inf> race_demo: [B] read value: 5
-[00:00:00.401,000] <inf> race_demo: [B] wrote value: 6
.............


### 🏁 Resultado Esperado
Agora o sistema realiza o objetivo corretamente:  
Cada thread adiciona **+1** por vez, sem conflitos de leitura ou escrita.

---

## 🧰 Tecnologias Utilizadas

- **Zephyr RTOS 4.2**
- **C / C++**
- **Threads**
- **Semáforos**
- **UART / Serial Output**

---

## 🧩 Conclusão

Este exemplo demonstra claramente:
- O impacto da **falta de sincronização** entre threads.
- Como o uso de **semáforos** pode evitar *Race Conditions*.
- A importância de garantir acesso exclusivo a recursos compartilhados em sistemas multitarefa.

---

> ✍️ 



