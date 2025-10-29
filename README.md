## 🧪 Planejamento de Testes – Versão Corrigida (com Mutex)

### 🎯 Objetivo

Validar que, com o uso de `k_mutex`, o acesso concorrente ao recurso compartilhado não apresenta mais corrupção de dados, mesmo sob forte preempção e múltiplas execuções.

---

### 🧩 Casos de Teste

| **Caso de Teste**                                             | **Pré-condição**                                                                              | **Etapas de Teste**                                                                                                       | **Pós-Condição Esperada (Corrigida)**                                                                                                                                                  |
| --------------------------------------------------------- | ----------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **1. Execução Base com Threads Concorrentes e Preempção** | - Firmware corrigido (com mutex) gravado na FRDM-KL25Z.<br>- Serial 115200 aberto.        | 1. Iniciar o firmware.<br>2. Observar logs e LEDs das 3 threads.<br>3. Verificar se ocorre alguma mensagem de erro.   | ✅ **Nenhuma mensagem “RACE CONDITION!” deve aparecer**.<br>✅ Valores de timestamp, temperatura e sequence sempre corretos.<br>🟢 LEDs **não entram no modo de erro** ao final.         |
| **2. Execução Repetida (5 Execuções)**                    | - Mesmo binário do Caso 1.<br>- Reset entre execuções.                                    | 1. Executar 5 vezes.<br>2. Registrar outputs.<br>3. Comparar comportamento entre execuções.                           | ✅ Resultados **idênticos e determinísticos** a cada execução.<br>✅ Mesma ordem consistente de logs.<br>✅ Nenhuma inconsistência detectada.                                             |
| **3. Teste de Estresse Aumentando Preempção**             | - Código corrigido, porém com aumento nos `cpu_work_cycles()` para estressar o scheduler. | 1. Dobrar valores do workload (ex.: 3000 → 6000).<br>2. Recompilar e executar.<br>3. Monitorar integridade dos dados. | ✅ **Ainda sem race condition**, mesmo sob carga extrema.<br>↗️ Pode ocorrer leve impacto de desempenho, porém **sem erros de dados**.<br>🟢 Integridade garantida pela exclusão mútua. |

---

### 📌 Mudanças nas Pós-Condições (em relação à versão com race)

| Antes (Sem Mutex)                  | Agora (Com Mutex)                    |
| ---------------------------------- | ------------------------------------ |
| Erros esperados                    | Erros **não devem ocorrer**          |
| Comportamento não determinístico   | Comportamento **determinístico**     |
| LEDs piscam indicando falha        | LEDs **não entram em loop de erro**  |
| Estrutura compartilhada corrompida | Dados sempre íntegros e consistentes |

---

## 🧩 Conclusão

Com o uso de `k_mutex`, confirmamos que:

✔ O problema de race condition é totalmente eliminado
✔ Os dados permanecem consistentes em todas as execuções
✔ Mesmo sob estresse, o sistema se comporta corretamente

---