# Registro de testes — 15/11/2026

Ambiente: Ubuntu-24.04 no WSL, GCC 13.3.0, Valgrind 3.22.0. Executar os comandos abaixo na pasta do projeto, no Ubuntu.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -x c -fsyntax-only include/sistema.h
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude tests/test_sistema.c src/sistema.c src/pastas.c src/arquivos.c src/usuarios.c src/permissoes.c src/disco.c -o tests/test_sistema
./tests/test_sistema
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g tests/test_menu.c -o /tmp/m3_test_menu
/tmp/m3_test_menu
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude tests/test_falhas.c src/sistema.c src/pastas.c src/arquivos.c src/usuarios.c src/permissoes.c src/disco.c -Wl,--wrap=calloc -Wl,--wrap=time -o tests/test_falhas
./tests/test_falhas
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./tests/test_sistema
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./simulador < tests/sessao.txt
```

Todos executados com retorno zero. Header e fontes também compilados com `-Werror`: sem avisos. Testes usam asserts ativos (não compilar com NDEBUG).

| Grupo | Evidência executada | Resultado |
| --- | --- | --- |
| Inicialização/limpeza | Raiz, ana, IDs, liberação repetida e estrutura zerada | Aprovado |
| Árvore | /a/b, barra final, raiz, navegação inválida preservada, nome com espaço, colisão arquivo/pasta, caminho por arquivo | Aprovado |
| Nomes | Vazio, espaços, ponto, ponto duplo, barra, controle, 63 e 64 bytes | Aprovado |
| Conteúdo | 0, 1, 63, 64, 65, 1023, 1024; leitura exata e 1025 recusado | Aprovado |
| Redução | Grande para 150, 1 e vazio; contagem/mapa consistentes | Aprovado |
| Cópia | IDs/FCBs/blocos independentes, modificação/exclusão sem afetar origem, próprio nome recusado | Aprovado |
| Movimento | Mesmo ponteiro e todos os metadados preservados, renomeação em pasta cheia, mesmo destino/nome | Aprovado |
| Capacidade | Oito arquivos de 1024, vazio com disco cheio, expansão/cópia recusadas sem consumir ID, reescrita igual/menor, reutilização | Aprovado |
| Diretórios | 32 arquivos e 32 pastas, excedentes, movimento para destino cheio, 16 níveis e excesso | Aprovado |
| Permissões | Todas as combinações usuário/classe das máscaras 640, 000, 004, 020, 100 e 777; R/W/X e cópia | Aprovado |
| Propriedade | Bruno/Carla não alteram máscara, não movem/renomeiam/excluem arquivo de Ana | Aprovado |
| Máscaras/tipos | 888, 64, 0644 e letras recusados; 754 formatado; quatro tipos; X em não programa recusado | Aprovado |
| IDs | Atribuição de UINT64_MAX e recusa seguinte sem overflow | Aprovado |
| Invariantes | Índices válidos, blocos não compartilhados, mapa/quantidade/tamanho, contadores compactos, pai correto | Aprovado |
| Menu | Letras, fora da faixa, linha longa, nome 63, conteúdo 1024/1025, continuação após erro | Aprovado |
| Integração | Árvore, cópia, movimento, permissões, execução e 150 bytes com 64/64/22 úteis | Aprovado |
| EOF | Sistema vazio, durante criação/tipo, árvore preenchida, conteúdo sem newline final | Aprovado |
| Falhas injetadas | calloc na raiz/pasta/FCB/cópia; time em criação/escrita/leitura/cópia/execução; estado e saídas preservados | Aprovado |

Comparações de FCB/disco por snapshots verificam preservação em negativas de tamanho, disco cheio, permissões e falhas injetadas. As invariantes percorrem recursivamente toda a árvore construída. Casos ainda não cobertos individualmente não devem ser interpretados como uma prova exaustiva.

Valgrind API: 113 alocações e 113 liberações, 0 bytes em uso ao sair, ERROR SUMMARY: 0. Valgrind menu: 7 alocações e 7 liberações, 0 bytes em uso ao sair, ERROR SUMMARY: 0. Logs em `tests/valgrind_api.log` e `tests/valgrind_menu.log`; entrada integrada em `tests/sessao.txt`. O Valgrind foi instalado via apt no WSL; a tentativa inicial com sudo sem senha não funcionou, e a instalação pelo usuário root do WSL funcionou.

Não foi necessário alterar as assinaturas do header. Os módulos foram validados em conjunto; os registros não representam uma validação separada de cada versão intermediária do menu.

Pendências externas: máquina da avaliação, ensaio dos integrantes, vídeo, orientações de envio e conferência direta do PDF original. Nenhuma falha conhecida permaneceu nos cenários executados.

Validação adicional executada: `gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude tests/test_limites.c src/sistema.c src/pastas.c src/arquivos.c src/usuarios.c src/permissoes.c src/disco.c -o tests/test_limites` e `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./tests/test_limites`. Caminho máximo de 1.024 bytes, barra final (1.025), buffer insuficiente preservando saídas, distinção f/F, nomes iguais em pastas diferentes, destino inexistente e conteúdo inválido aprovados. Valgrind: zero erros, zero bytes pendentes; log `tests/valgrind_limites.log`.

Atualização de 15/11/2026: a avaliação será na própria máquina do usuário, já validada com WSL, GCC e recompilação. A confirmação do ambiente deixou de ser pendência; os demais itens humanos continuam pendentes.


## Extensão: cadastro de usuários

Executados após a alteração: compilação de main.c/sistema.c/disco.c com C11, Wall, Wextra, Wpedantic e Werror; recompilação e execução de test_sistema, test_limites, test_falhas e test_menu.c, todos aprovados. Nova suíte:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude tests/test_usuarios.c src/sistema.c src/pastas.c src/arquivos.c src/usuarios.c src/permissoes.c src/disco.c -o tests/test_usuarios
./tests/test_usuarios
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./tests/test_usuarios
```

Aprovados: cadastro nos dois grupos, troca para novos nomes, novo proprietário no FCB, acesso por grupo/outros, nome duplicado/inválido, grupo inválido, NULL, nomes de 63/64 bytes, lotação de 32 usuários e preservação do sistema nas falhas. Menu: cadastro, listagem na troca, detalhes do novo dono, duplicação, grupo inválido e EOF no cadastro. Valgrind: 4 alocações, 4 liberações, zero bytes pendentes e zero erros; log tests/valgrind_usuarios.log.
`nLimite atualizado por solicitação do usuário para 67 usuários no total. Simulador recompilado com -Werror, sem avisos; test_usuarios recompilado e aprovado, incluindo preenchimento até MAX_USUARIOS=67 e recusa do 68º cadastro sem alteração do estado. O registro anterior de 32 corresponde à execução antes dessa mudança.

Revisão de legibilidade: nomes abreviados substituídos por sistema, arquivo, diretorio, resultado e tamanho; auxiliares internos receberam nomes descritivos; seleção de classe de permissão e contagem de blocos foram escritas em passos explícitos. Compilação com -Werror sem avisos; test_sistema, test_usuarios (limite 67), test_limites, test_falhas e test_menu.c recompilados/executados e aprovados. Interfaces públicas mantidas.


## Validação atual — vetores dinâmicos

Esta seção substitui os resultados históricos de limite fixo de 32 entradas. Os logs API/menu/usuários foram atualizados com a execução atual; as contagens antigas acima pertencem às versões anteriores.

Comandos executados na raiz do projeto (Ubuntu/WSL):

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude src/*.c -o simulador
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Iinclude tests/test_dinamico.c src/sistema.c src/pastas.c src/arquivos.c src/usuarios.c src/permissoes.c src/disco.c -Wl,--wrap=calloc -o tests/test_dinamico
./tests/test_dinamico
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./tests/test_dinamico
```

Também recompilados e executados test_sistema, test_usuarios, test_limites e test_falhas com os comandos anteriores; test_sistema atualizado para esperar crescimento após 32 entradas. test_menu.c ampliado para criar 40 pastas e 40 arquivos, listar e terminar por EOF. Todos aprovados, sem avisos na compilação.

Nova suíte verifica 300 arquivos e 300 subpastas por diretório, preservação de ID/pai/referências, cópia independente, movimento, usuários até 67 e arquivo criado pelo usuário 67. Injeta falha em cada uma das duas alocações de inicialização, criação de pasta/arquivo/cópia, e no crescimento do destino de movimento e vetor de usuários. Compara Sistema, diretórios, vetor de entradas, FCB e usuários após recusas. Todos aprovados.

Valgrind executado com leak-check=full, show-leak-kinds=all, track-origins=yes e error-exitcode=1:

| Execução | Alocações/liberações | Em uso ao sair | Erros |
| --- | --- | --- | --- |
| test_dinamico | 729 / 729 | 0 bytes | 0 |
| test_sistema | 155 / 155 | 0 bytes | 0 |
| test_usuarios | 11 / 11 | 0 bytes | 0 |
| simulador com tests/sessao.txt | 13 / 13 | 0 bytes | 0 |

Logs atuais em tests/valgrind_dinamico.log, tests/valgrind_api.log, tests/valgrind_usuarios.log e tests/valgrind_menu.log. Os testes não simulam o esgotamento físico de toda a RAM: exercitam falhas controladas de calloc e validam o tratamento. Limites de disco, conteúdo, nome, profundidade e regra de 67 usuários continuam intencionais.


## Divisão em módulos

As 39 funções antes concentradas em sistema.c foram distribuídas entre sistema.c, pastas.c, arquivos.c, usuarios.c e permissoes.c, sem alterar seus corpos; os auxiliares usados por vários módulos passaram a ter declarações em sistema_interno.h. Funções exclusivas de cada módulo continuam static. Interfaces públicas preservadas.

Comandos de compilação deste documento, README e COMANDOS_CMD.txt atualizados para todos os fontes. Compilação com C11, Wall, Wextra, Wpedantic e Werror sem avisos. Recompilados/executados test_sistema, test_limites, test_usuarios, test_falhas e test_dinamico, além de test_menu.c: todos aprovados. Valgrind repetido em test_sistema, test_dinamico e sessão do menu: zero erros, zero bytes em uso ao sair. Comando do COMANDOS_CMD.txt executado pelo CMD: compilação, abertura e saída com opção 0 aprovadas.

## Fontes movidos para src/

Todos os .c e .h do programa foram movidos para src/, sem mudar seus conteudos. README, COMANDOS_CMD.txt e comandos de teste atualizados: src/*.c para o programa e -Iinclude nos testes. Compilacao com -Werror sem avisos; cinco suites C recompiladas e aprovadas (sistema, limites, usuarios, falhas e dinamico), alem de test_menu.c. O comando atualizado foi executado pelo CMD, compilou, abriu o menu e saiu com opcao 0.

## Headers separados em include/

Os arquivos .c ficam em src/; sistema.h e sistema_interno.h agora ficam em include/. Comandos atualizados com -Iinclude. Programa e cinco suites C recompilados com -Werror sem avisos; todas as suites e test_menu.c aprovados. Comando pelo CMD tambem compilou, abriu o menu e encerrou com opcao 0.


## Navegação somente por nome

Menu e operação alterados para aceitar apenas nomes simples. Testes adaptados para subir pela operação de voltar ao pai; a resolução de caminhos absolutos permanece testada separadamente para cópia/movimento. Adicionado cenário de menu que recusa /a e a/b sem mudar a pasta, entra por a e b e retorna ao pai até a raiz. Programa compilado com -Werror sem avisos; todas as cinco suítes C e test_menu.c aprovados. Sessão gravada e roteiro ajustados à navegação por nome.

## Migração e validação do menu em C

`tests/test_menu.c` substitui o script anterior, usando como referência o PR #1 (https://github.com/MatheusThurow/miniSistemaArquivosEmMemoria/pull/1). Todos os cenários foram preservados: entradas inválidas, nomes de 63/64 bytes, conteúdo de 1024/1025 bytes, sessão integrada, permissões, usuários, EOF, 40 pastas/arquivos e navegação por nome. A captura também recusa saída maior que o buffer disponível.

O teste usa C11 e recursos POSIX (`fork`, `execl`, `dup2` e arquivos temporários); execute no Ubuntu/WSL, a partir da raiz do projeto, após compilar `./simulador`. O executável do teste é gerado em `/tmp/m3_test_menu` para não adicionar binários ao repositório. Os demais testes em C foram mantidos sem alterações.

Validação em 15/11/2026: header, programa e seis suítes compilados com `-std=c11 -Wall -Wextra -Wpedantic -Werror`; todas as suítes aprovadas (sistema, limites, usuários, falhas, dinâmico e menu).

Valgrind executado nas cinco suítes de API e no teste de menu (com --trace-children=yes): todas as execuções retornaram zero, sem erros de memória ou vazamentos reportados. No processo do teste de menu: 45 alocações, 45 liberações e zero bytes em uso ao sair.
