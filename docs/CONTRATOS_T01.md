# T01 — Interfaces e ambiente

As interfaces estão em `sistema.h`. Este documento define decisões para implementar T02–T09; não declara operações implementadas.

## Ambiente e compilação

Ambiente local conferido em 15/11/2026: Ubuntu-24.04 no WSL, GCC 13.3.0. Valgrind ainda precisa ser instalado para T10. O ambiente da avaliação permanece pendente de confirmação.

Quando os três fontes existirem, compilar no Ubuntu, dentro da pasta do projeto:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador
./simulador
```

O header isolado pode ser validado com:

```bash
printf '#include "sistema.h"\n' | gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude -x c -fsyntax-only -
```

## Responsabilidade dos módulos

- `main.c`: fgets/strtol, descarte de linhas longas, EOF, menu, mensagens, listagem, detalhes e visualização recursiva dos blocos. Consulta structs sem modificá-las. Usa buffers de conteúdo com espaço para o limite, quebra de linha e terminador; nunca entrega a quebra de linha à escrita.
- `sistema.c`: inicialização, liberação recursiva, mensagens de resultado e auxiliares comuns.
- `pastas.c`: nomes, busca de pastas, criação, navegação e caminhos.
- `arquivos.c`: busca, consulta e operações de arquivos, verificando acesso em cada operação.
- `usuarios.c`: cadastro, troca e identificação dos usuários/grupos.
- `permissoes.c`: conversão, formatação, verificações RWX/propriedade e alteração de permissões.
- `sistema_interno.h`: declarações de auxiliares compartilhados; o menu continua usando somente `sistema.h`.
- `disco.c`: blocos, mapa, reserva e reescrita atômica. Não conhece menu, usuários ou relógio e não aloca FCBs.

Listagem e detalhes consultam `sistema.atual` e os FCBs; não precisam de funções que imprimem. A visualização de blocos percorre a árvore a partir da raiz. Buffers de leitura contêm bytes exatos; a interface imprime pela quantidade retornada.

## Propriedade e estado

O chamador mantém um `Sistema s = {0}` e chama inicialização uma vez. A raiz tem nome `/` e pai NULL. O sistema contém disco por valor e vetor de usuários alocado dinamicamente. Diretórios e arquivos também são alocados dinamicamente; cada diretório possui vetores dinâmicos de entradas. IDs de usuário serão 1=ana, 2=bruno, 3=carla; usuário atual é o índice 0 inicialmente. Grupo é enum, sem strings duplicadas nos FCBs.

Cada diretório possui seus filhos e FCBs. `pai`, `atual` e resultados de consulta são referências emprestadas. Mover transfere propriedade do mesmo ponteiro; copiar cria outro FCB e blocos independentes. Exclusão invalida referências ao FCB. Liberação recursiva zera o sistema, inclusive disco e referências.

IDs de arquivo começam em 1. Só criação/cópia bem-sucedidas consomem ID. Após atribuir UINT64_MAX, o contador recebe zero sem incrementar; futuras criações/cópias retornam SIS_ID_ESGOTADO. Falhas não consomem IDs.

## Retornos e atomicidade

SIS_OK indica sucesso. SIS_SEM_ALTERACAO indica voltar na raiz ou mover/renomear para a própria localização, após verificar propriedade no movimento. Os demais valores indicam erro e preservam estado e saídas. Não há exigência de prioridade entre erros simultâneos, mas a mensagem deve corresponder ao motivo retornado.

Argumentos nulos obrigatórios retornam SIS_ARGUMENTO_INVALIDO nas funções com Resultado. Funções void, bool e contadores exigem os argumentos válidos descritos no header. Buffers de saída não podem se sobrepor entre si ou às estruturas do sistema. Funções que retornam nomes/mensagens devolvem strings estáticas, nunca liberadas pelo chamador; enums desconhecidos produzem texto de valor inválido.

Validar e reservar antes de publicar mudanças. Obter `time(NULL)` antes de qualquer mutação que dependa de data; se falhar, retornar SIS_ERRO_RELOGIO sem alteração. Usar uma única data para os três campos de uma criação/cópia. Escrita atualiza somente modificação; leitura/execução e origem de cópia atualizam somente acesso. A resolução do relógio pode produzir datas iguais em operações próximas.

O disco usa o menor índice livre primeiro. Reserva usa vetor temporário e só publica índices/mapa se houver espaço para tudo. Reescrita valida capacidade antes de escrever, reutiliza blocos existentes e libera excedentes. Liberação zera blocos devolvidos. Bytes de sobra do último bloco são zerados; leitura considera somente tamanho. FCB novo começa com tamanho e quantidade zero. Funções internas pressupõem FCB/mapa consistentes.

## Nomes, caminhos e limites

Valem os limites do planejamento revisado. Não remover espaços de nomes; rejeitar controles ASCII (0–31 e 127), barra, vazio, somente espaços e nomes especiais. Bytes UTF-8 acima de 127 são preservados, sem normalização. Tipo aceita apenas os quatro valores do enum.

Raiz tem profundidade zero. Entrar aceita somente o nome de uma subpasta; caminhos absolutos e relativos compostos são recusados. Resolver aceita uma barra final, mas rejeita `//`, componentes vazios intermediários e componentes `.`/`..`. Caminho exibido não tem barra final, exceto raiz. TAM_CAMINHO comporta os 16 componentes máximos, barra final opcional e NUL.

Movimento no mesmo diretório com nome novo não precisa de vaga adicional. Não há mais limite de 32 arquivos/subpastas; vetores crescem conforme necessário. Cópia sempre exige nova vaga, novo ID e blocos independentes. Todos os nomes de origem de arquivos são relativos à pasta atual. Diretórios não são movidos ou excluídos.

## Situação da T01

Interfaces e comando definidos; ambiente local disponível. Validar o header não equivale a testar as operações futuras. A confirmação da máquina da avaliação continua pendente, portanto T01 não deve ser registrada como integralmente concluída ainda.


## Extensão autorizada — cadastro de usuários

A opção 16 permite cadastrar usuários em memória, até MAX_USUARIOS=67 incluindo os três iniciais. Nome único validado por nome_validar; grupo alunos ou visitantes. IDs são sequenciais, independentes dos IDs de arquivos. quantidade_usuarios delimita as entradas ativas; TOTAL_USUARIOS=3 indica apenas a quantidade inicial. O cadastro preserva a identidade atual. A opção 13 lista e permite selecionar qualquer cadastrado. Não há remoção, autenticação, cadastro de grupos ou persistência. Esta extensão substitui a restrição anterior de apenas três usuários fixos. Testes de cadastro, permissões, limites e regressão executados com sucesso; detalhes em [TESTES.md](TESTES.md).



## Atualização: vetores dinâmicos

Diretorio possui arquivos/subpastas como ponteiros para vetores alocados e registra capacidade_arquivos/capacidade_subpastas. Sistema possui Usuario *usuarios e capacidade_usuarios. Quantidades delimitam posições ativas. MAX_ARQUIVOS e MAX_SUBPASTAS foram removidos; MAX_USUARIOS=67 permanece como regra solicitada, sem reserva antecipada de todas as posições.

O crescimento aloca outro vetor, copia as entradas e confirma a troca somente quando não há mais etapas que possam falhar. Overflow de capacidade e falha de calloc retornam SIS_SEM_MEMORIA sem mutação. Criação/cópia descartam FCBs e vetores temporários quando necessário. Movimento só remove a origem após preparar o destino. Referências a FCBs/diretórios permanecem estáveis; ponteiros emprestados para entradas do vetor de usuários podem ser invalidados por um novo cadastro. O código usa IDs/índices para preservar a identidade.

Liberação recursiva inclui os vetores de cada diretório e o vetor de usuários, depois zera Sistema. Excluir arquivo mantém a capacidade do vetor para reutilização. Nomes, profundidade e armazenamento conservam os limites documentados no README. As notas anteriores sobre disco e permissões continuam válidas.

Organizacao atual: todos os fontes e headers ficam em src/. Os comandos devem ser executados na raiz do projeto; testes usam -Iinclude para localizar sistema.h.

Atualizacao da organizacao: apenas .c em src/; headers .h em include/. Compilacao e testes usam -Iinclude.
