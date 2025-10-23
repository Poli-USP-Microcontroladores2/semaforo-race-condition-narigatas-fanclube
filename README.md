### TESTES PARA VIZUALIZAR O CASO DE RACE CONDITION:

## CASO DE TESTE 1:

# PRÉ-CONDIÇÃO:
- A placa deve estar ligada à uma fonte de energia;
- O código main.c está compilado e gravado com sucesso na placa FRDM-KL25Z;
- O sistema está em execução.

# ETAPAS DE TESTE:
- Observar o LED azul por 30 segundos;
- Identificar vizualmente um padrão consistente de pisca rápido (100ms e deslisgado);
- Tentar identificar visualmente um padrão consistente de pisca lento (500ms ligado / 500ms desligado);
- Tentar encontrar na saída serial um padrão constante e organizado de saídas.

# PÓS-CONDIÇÃO ESPERADA:
- O LED pisca em um padrão visivelmente caótico e irregular, que não corresponde nem ao padrão rápido (Thread A) nem ao padrão lento (Thread B);
- O padrão rápido falha em ser mantido. O LED claramente permanece aceso ou apagado por períodos muito mais longos que 100ms, indicando a interferência da Thread B;
- O padrão lento falha em ser mantido. O LED claramente é interrompido e pisca em intervalos muito mais curtos que 500ms, indicando a interferência da Thread A;
- O padrão da serial falha e retorna saídas extremamente desorganizadas e caóticas;
- Conlusão de que acontece um problema de race condition na main.c.