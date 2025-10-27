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




