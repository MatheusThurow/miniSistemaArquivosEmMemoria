# Divisão de tarefas — Mini-Sistema de Arquivos em Memória

Base: [PLANEJAMENTO.md](PLANEJAMENTO.md). Este documento organiza o trabalho previsto; não indica que código ou testes já estejam concluídos.

As tarefas começaram pendentes; o status abaixo registra o andamento verificado. Preencher o responsável ao distribuir o trabalho entre os integrantes. As dependências indicam o que precisa estar concluído antes de finalizar a tarefa; documentação e preparação de testes podem começar antes.

## Visão geral

| ID | Tarefa | Dependências | Responsável | Status |
| --- | --- | --- | --- | --- |
| T01 | Validar ambiente e definir interfaces | — | A definir | Concluída: avaliação na própria máquina validada |
| T02 | Implementar inicialização e liberação | T01 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T03 | Implementar entrada e menu | T02 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T04 | Implementar diretórios e caminhos | T02 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T05 | Implementar armazenamento em blocos | T01 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T06 | Implementar usuários e permissões | T02 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T07 | Implementar operações básicas de arquivos | T04, T05, T06 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T08 | Implementar cópia, movimentação e exclusão | T07 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T09 | Integrar o menu e a visualização de blocos | T03, T08 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T10 | Validar funcionamento, limites e memória | T09 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T11 | Finalizar README e conferir requisitos | T10 | A definir | Concluída localmente; evidências em [TESTES.md](TESTES.md) |
| T12 | Preparar apresentação e entrega | T11 | A definir | Parcial: roteiro local pronto; ações humanas pendentes |

## T01 — Validar ambiente e definir interfaces

- [x] Confirmar Ubuntu-24.04/WSL ou Linux e disponibilidade do GCC na máquina da avaliação: o usuário confirmou que usará sua própria máquina, já validada localmente.
- [x] Definir em `sistema.h` as structs de arquivo/FCB, diretório, usuário e disco; enum dos quatro tipos; constantes e assinaturas das funções.
- [x] Centralizar os limites: 128 blocos de 64 bytes, 16 blocos por arquivo, nomes de 63 bytes, 32 arquivos e 32 subpastas por diretório e profundidade de 16 níveis abaixo da raiz.
- [x] Definir contratos de retorno/erro, propriedade dos ponteiros e responsabilidades de alocação e liberação.

**Entrega:** interfaces compartilhadas e comando de compilação definido para `main.c`, `sistema.c` e `disco.c`.

**Conclusão:** ambiente confirmado e interfaces revisadas para permitir que os módulos sejam implementados sem contratos conflitantes.

Registro de T01 (15/11/2026): `sistema.h` e `CONTRATOS_T01.md` criados em `C:\faculdade\SO\M3\miniSistemaArquivosEmMemoria`. Header validado com GCC C11 e avisos tratados como erros. WSL Ubuntu-24.04 e GCC 13.3.0 confirmados localmente; máquina da avaliação ainda pendente. Operações ainda não implementadas.

## T02 — Implementar inicialização e liberação

- [x] Inicializar raiz `/`, disco vazio, contador de IDs e usuária atual `ana`.
- [x] Cadastrar `ana` e `bruno` em `alunos`, e `carla` em `visitantes`.
- [x] Implementar alocação verificada e liberação recursiva de arquivos e diretórios.
- [x] Tratar falhas durante a inicialização sem deixar recursos alocados.

**Arquivos principais:** `sistema.c`, `sistema.h`.

**Conclusão:** uma execução mínima inicia vazia e encerra liberando os recursos; falhas de inicialização são tratadas.

## T03 — Implementar entrada e menu

- [x] Criar as opções de 0 a 15 previstas no planejamento, exibindo usuário, grupo e caminho atual.
- [x] Implementar leitura com `fgets` e conversão validada com `strtol`.
- [x] Detectar linhas longas, consumir o restante e recusar sem truncamento silencioso, aceitando exatamente os limites permitidos.
- [x] Tratar letras, opções fora da faixa e fim da entrada, encerrando com limpeza.
- [x] Explicar nos prompts que a origem é um nome na pasta atual e que o destino de cópia/movimentação é um caminho absoluto e um nome final.

**Arquivo principal:** `main.c`.

**Conclusão:** entrada inválida não desorganiza a próxima leitura; saída normal e EOF encerram corretamente. A ligação de todas as opções será concluída em T09.

## T04 — Implementar diretórios e caminhos

- [x] Implementar criação e listagem de pastas, entrada por nome ou caminho absoluto, retorno ao pai e construção do caminho atual.
- [x] Compartilhar a validação de nomes entre arquivos e pastas: rejeitar vazio, somente espaços, `.`, `..`, barra, controle e excesso de bytes.
- [x] Impedir colisão entre nomes de arquivo e pasta no mesmo diretório, distinguindo maiúsculas de minúsculas.
- [x] Resolver caminhos absolutos a partir da raiz, aceitando `/` e barra final, e rejeitando componentes vazios intermediários e `.`/`..`.
- [x] Aplicar limites de quantidade e profundidade; preservar a pasta atual quando a navegação falhar.

**Arquivos principais:** `sistema.c`, `sistema.h`.

**Conclusão:** testar árvore aninhada, raiz, nomes com espaços, colisões, caminho por arquivo, 32 subpastas e profundidade máxima.

## T05 — Implementar armazenamento em blocos

- [x] Implementar disco em memória, mapa de ocupação, reserva, leitura, escrita e liberação.
- [x] Usar alocação indexada simplificada, com os índices no FCB e sem bloco de índice adicional.
- [x] Calcular blocos por arredondamento para cima; arquivo vazio usa zero blocos e leitura respeita o tamanho em bytes.
- [x] Na reescrita, reaproveitar blocos próprios, reservar apenas os adicionais e liberar a sobra ao reduzir.
- [x] Garantir que falta de espaço preserve conteúdo e mapa anteriores e que reservas incompletas sejam desfeitas.

**Arquivo principal:** `disco.c`.

**Conclusão:** conferir limites de blocos, disco cheio, reutilização e ausência de blocos compartilhados ou reservas perdidas.

## T06 — Implementar usuários e permissões

- [x] Implementar troca entre os três usuários fixos.
- [x] Converter exatamente três dígitos de `0` a `7` em bitmask e exibir formato numérico e simbólico.
- [x] Selecionar uma única classe: proprietário, senão grupo, senão outros; nunca recorrer a outra classe após negar acesso.
- [x] Disponibilizar verificações de R, W, X e propriedade para uso dentro das operações.
- [x] Implementar alteração de permissões apenas pelo proprietário, sem modificar datas.

**Arquivos principais:** `sistema.c`, `sistema.h`.

**Conclusão:** validar máscaras `640`, `000`, `004`, `020` e `100`, além de rejeitar `888`, `64`, `0644` e letras. W e X devem funcionar independentemente de R.

## T07 — Implementar operações básicas de arquivos

- [x] Criar arquivos vazios com tipo, ID crescente não reutilizado, dono/grupo atuais, máscara 644 e três datas; detectar esgotamento do ID.
- [x] Escrever uma linha textual por substituição, inclusive vazia, exigindo W e atualizando tamanho/modificação somente após sucesso.
- [x] Ler os bytes exatos, exigindo R e atualizando acesso somente após sucesso.
- [x] Listar e detalhar nome, ID, tipo, tamanho, dono/grupo, datas, permissões e índices dos blocos sem alterar datas.
- [x] Simular execução apenas para tipo programa com X, atualizando acesso sem executar conteúdo real.
- [x] Validar capacidade, alocações, nomes e permissões antes de publicar mudanças.

**Arquivo principal:** `sistema.c`, integrado a `disco.c`.

**Conclusão:** testar os quatro tipos; conteúdo de 0, 1, 63, 64, 65, 1.023 e 1.024 bytes; rejeição de 1.025 bytes; detalhes e execução protegida. Negativas devem preservar estado.

## T08 — Implementar cópia, movimentação, renomeação e exclusão

- [x] Copiar com R na origem, novo FCB/ID, blocos independentes, dono/grupo atuais, máscara 644 e novas datas; atualizar acesso da origem apenas no sucesso.
- [x] Mover o mesmo FCB, preservando ID, conteúdo, blocos, dono/grupo, permissões e datas.
- [x] Renomear alterando somente o nome; mesmo diretório e mesmo nome informa ausência de alteração.
- [x] Exigir propriedade para mover, renomear e excluir.
- [x] Recusar sobrescrita e cópia para o próprio nome; validar destino antes de remover a origem.
- [x] Excluir liberando blocos e FCB e ajustando entradas e contadores.

**Arquivo principal:** `sistema.c`.

**Conclusão:** cópia independente; movimento preserva identidade; destino cheio/inexistente/duplicado, disco cheio e negação de acesso preservam origem e recursos.

## T09 — Integrar menu e visualização de blocos

- [x] Conectar todas as opções às operações, com mensagens claras para cada erro previsto.
- [x] Mostrar capacidade, blocos livres e ocupados e blocos por arquivo, percorrendo também as subpastas.
- [x] Verificar um arquivo de 150 bytes em três blocos, com 64, 64 e 22 bytes úteis.
- [x] Garantir que as operações só alterem estruturas e disco simulados em memória.

**Arquivos principais:** `main.c`, `sistema.c`, `disco.c`.

**Conclusão:** todas as opções funcionam em uma sessão integrada e permitem continuar após falhas.

## T10 — Validar funcionamento, limites e memória

- [x] Executar e registrar todos os cenários da seção 11 do planejamento e sua matriz de permissões.
- [x] Cobrir 32 arquivos e 32 subpastas, profundidade máxima, entradas longas, EOF e nomes nos limites.
- [x] Encher o disco com oito arquivos de 1.024 bytes; testar arquivo vazio, falha de expansão/cópia e reescrita igual ou menor sem espaço livre.
- [x] Conferir índices válidos, ausência de compartilhamento, quantidade de blocos por tamanho, soma de livres/ocupados igual a 128 e um diretório por arquivo.
- [x] Comparar conteúdo, datas, árvore e mapa antes/depois de operações recusadas.
- [x] Compilar com `gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador` e corrigir avisos.
- [x] Executar Valgrind com árvore aninhada, cópias, falhas, exclusões e saída; corrigir acessos inválidos, valores não inicializados e vazamentos.

**Entrega:** registro de cenários, resultados reais, falhas encontradas e correções verificadas.

**Conclusão:** cenários aprovados após execução, compilação sem avisos pendentes e memória sem erros do projeto. Corrigir os módulos responsáveis e repetir os testes afetados.

## T11 — Finalizar README e conferir requisitos

- [x] Documentar compilação/execução, menu, entradas, nomes, caminhos, limites, política de cópia, datas e permissões.
- [x] Explicar structs, ponteiros, `malloc/free`, FCB, ID inspirado em inode, árvore e bitmasks.
- [x] Comparar alocação contínua, encadeada e indexada e justificar a escolha.
- [x] Relacionar operações a `mkdir`, `cd`, `ls`, `touch`, `cat`, `cp`, `mv`, `rm`, `stat` e `chmod`.
- [x] Declarar simplificações: conteúdo textual, tipo como metadado, execução simulada, ausência de permissões de diretórios, persistência, links e exclusão de pastas.
- [x] Incluir testes executados e resultados; conferir a correspondência com o enunciado apresentada no planejamento e, se disponível, com o PDF original.

**Entrega:** `README.md` coerente com o comportamento implementado.

**Conclusão:** outro integrante consegue compilar e usar o simulador seguindo o README; requisitos e limitações estão explícitos.

## T12 — Preparar apresentação e entrega

- [ ] Ensaiar a partir do sistema vazio: árvore aninhada, criação/escrita/leitura, metadados, cópia independente, movimento com mesmo ID e três classes de permissão.
- [ ] Demonstrar execução simulada, múltiplos blocos, liberação, uma falha de entrada e uma de capacidade.
- [ ] Ensaiar entradas livres, sem depender de uma sequência decorada.
- [ ] Preparar o vídeo solicitado para YouTube, verificando eventuais orientações adicionais de formato e duração.
- [ ] Conferir fontes e documentação no repositório e disponíveis para avaliação; recompilar no ambiente da aula e preparar acesso sem internet.
- [ ] Garantir que todos os integrantes expliquem estruturas, blocos, bitmasks e decisões do código.

**Conclusão:** checklist da seção 13 do planejamento conferido, demonstração ensaiada e vídeo preparado conforme o enunciado. Publicação ou envio devem seguir a orientação de entrega da disciplina.

## Sugestão de frentes de trabalho

Distribuir estas frentes conforme o tamanho da equipe; uma pessoa pode assumir mais de uma. Não representam tarefas já atribuídas.

| Frente | Tarefas principais | Coordenação necessária |
| --- | --- | --- |
| Interface e integração | T03, T09 | Usar os contratos de T01 e as operações de T04–T08 |
| Árvore, arquivos e proteção | T02, T04, T06, T07, T08 | Concentrar ou coordenar edições em `sistema.c` |
| Armazenamento | T05 | Alinhar contratos de reserva e reescrita com T07/T08 |
| Qualidade e entrega | T10, T11, T12 | Preparar cenários e documentação durante a implementação |

T01 deve ser alinhada por todos antes da divisão dos módulos. Após T02, entrada, diretórios e permissões podem avançar em paralelo; T05 pode começar após T01. A integração final depende das operações prontas. Testes e revisão são responsabilidade compartilhada, mesmo que uma pessoa mantenha o registro.

## Regra para concluir tarefas

Marcar uma tarefa como concluída apenas após verificar seus critérios. Para tarefas de código, compilar e executar os testes correspondentes; para documentação e apresentação, revisar os entregáveis. Nenhuma caixa deste documento representa teste já aprovado.


## Registro de execução — 15/11/2026

T02–T11 implementadas e verificadas localmente. Fontes, README, TESTES.md, DEMONSTRACAO.md e testes automatizados estão em C:\faculdade\SO\M3\miniSistemaArquivosEmMemoria. Compilação sem avisos; API, menu e falhas injetadas aprovados; Valgrind com zero erros e zero bytes pendentes. Consulte [TESTES.md](TESTES.md) para limites das evidências. T01 permanece parcial pela máquina da avaliação; T12 permanece parcial porque ensaio humano, vídeo, recompilação em aula, acesso offline no destino e entrega não foram realizados. PDF original indisponível. Nenhum envio externo efetuado. A pasta de fontes não possui repositório Git inicializado; arquivos disponíveis localmente.


Atualização de 15/11/2026: o usuário confirmou que a avaliação será nesta própria máquina. WSL Ubuntu-24.04, GCC e recompilação já foram verificados nela; T01 concluída. As referências anteriores à máquina indefinida são históricas. Ensaio dos integrantes, vídeo e orientações de entrega permanecem pendentes.



## Extensão autorizada — cadastro de usuários

A opção 16 permite cadastrar usuários em memória, até MAX_USUARIOS=67 incluindo os três iniciais. Nome único validado por nome_validar; grupo alunos ou visitantes. IDs são sequenciais, independentes dos IDs de arquivos. quantidade_usuarios delimita as entradas ativas; TOTAL_USUARIOS=3 indica apenas a quantidade inicial. O cadastro preserva a identidade atual. A opção 13 lista e permite selecionar qualquer cadastrado. Não há remoção, autenticação, cadastro de grupos ou persistência. Esta extensão substitui a restrição anterior de apenas três usuários fixos. Testes de cadastro, permissões, limites e regressão executados com sucesso; detalhes em [TESTES.md](TESTES.md).



## Revisão dinâmica — execução concluída localmente

- [x] Transformar vetores de arquivos, subpastas e usuários em alocações dinâmicas com quantidade/capacidade.
- [x] Remover limites de 32 arquivos/subpastas e preservar o limite solicitado de 67 usuários.
- [x] Preservar estado em falhas de crescimento, criação, cópia e movimento.
- [x] Liberar vetores temporários, vetores da árvore e usuários; aceitar liberação repetida.
- [x] Validar 300 arquivos e 300 subpastas no mesmo diretório, cadastros até 67 e falhas injetadas em cada etapa de alocação.
- [x] Executar regressão e Valgrind; resultados atuais em TESTES.md.

Os critérios anteriores que exigiam recusa após 32 entradas são históricos e foram substituídos por crescimento dinâmico. Disco, nomes, profundidade e conteúdo continuam limitados conforme README. PDF original disponibilizado e lido para conferir a exigência de memória dinâmica; apresentação humana e entrega externa continuam separadas.


## Organização em módulos

Divisão de sistema.c concluída e validada: pastas.c, arquivos.c, usuarios.c e permissoes.c. Auxiliares comuns declarados em sistema_interno.h. Comandos e documentação atualizados; regressão e Valgrind aprovados conforme TESTES.md.

Fontes e headers movidos para src/. Comandos e testes ajustados e validados conforme TESTES.md.
