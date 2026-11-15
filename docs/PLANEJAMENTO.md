# Planejamento revisado — Mini-Sistema de Arquivos em Memória

## 1. Objetivo e critérios de sucesso

Implementar em C um simulador de arquivos e diretórios em memória, operado por menu no terminal e executado em Linux/WSL. O programa deve permitir que o professor escolha livremente operações e entradas durante os testes em sala, sem depender de uma demonstração previamente montada.

O sistema inicia vazio, com a raiz `/` e a usuária `ana`. Os dados simulados desaparecem ao encerrar. O simulador não executa comandos do sistema nem altera arquivos reais para realizar suas operações.

Prioridades conforme a avaliação: funcionalidade e robustez (40%), fidelidade aos conceitos (30%), organização do código (20%) e documentação (10%). O PDF também solicita apresentação on-line via YouTube, sem detalhar duração ou formato. O teste em sala informado pelo aluno será preparado além desse entregável.

## 2. Correspondência com o enunciado

| Referência do PDF | Como será atendida |
| --- | --- |
| 3.1 — Diretórios em árvore, criação e navegação | Raiz, subpastas, ponteiro para pai, criação, listagem e navegação |
| 3.2 — FCB e metadados | Nome, ID único, tipo, tamanho em bytes, três datas e permissões |
| 3.2 — Operações | Criar, escrever, ler, copiar, mover, renomear e excluir arquivos |
| 3.3 — Proteção | RWX de proprietário, grupo e outros, com bitmasks e verificação por operação |
| 3.4 — Blocos | Disco em memória, mapa de ocupação e índices dos blocos no FCB |
| 4 — Linguagem e ambiente | C, structs, ponteiros, malloc/free, strings e testes em Linux/WSL |
| 5.2 e 5.3 — Interface e erros | Menu interativo, validação de entradas e mensagens claras |
| 5.4 e 5.5 — Entrega | Fontes organizados e README no repositório |

O texto sobre blocos é inconsistente: aparece como objetivo específico, é citado na seção de requisitos mínimos e também aparece entre os bônus na avaliação. Por isso, a implementação incluirá blocos e sua demonstração.

Reposicionar e truncar são citados na introdução, mas não aparecem na lista explícita de operações mínimas da seção 3.2. Não serão opções separadas nesta versão. A escrita por substituição aceitará conteúdo vazio, permitindo zerar o arquivo. Links, persistência e exclusão de diretórios ficam fora do escopo.

## 3. Ambiente e execução em sala

O ambiente planejado é Ubuntu-24.04 no WSL. A instalação e o funcionamento precisam ser confirmados na máquina que será usada; não presumir que o computador do professor tenha WSL ou GCC.

No PowerShell:

```powershell
wsl -d Ubuntu-24.04
```

No Ubuntu:

```bash
cd /mnt/c/faculdade/SO/M3/miniSistemaArquivosEmMemoria
gcc --version
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador
./simulador
```

Se necessário, preparar antes da aula com `sudo apt update` e `sudo apt install build-essential`. Em uma máquina Linux diferente, acessar a pasta onde os fontes foram copiados; o caminho `/mnt/c/...` é específico do WSL.

Levar os fontes e o README, testar a compilação a partir deles e evitar dependência de internet, caminhos fixos no código ou bibliotecas externas. O executável deve ser recompilável no ambiente de avaliação.

## 4. Interface

```text
Usuário: ana | Grupo: alunos | Pasta atual: /
1. Listar conteúdo
2. Criar pasta
3. Entrar em subpasta pelo nome
4. Voltar à pasta pai
5. Criar arquivo
6. Escrever em arquivo (substituir conteúdo)
7. Ler arquivo
8. Copiar arquivo
9. Mover ou renomear arquivo
10. Excluir arquivo
11. Mostrar detalhes de arquivo
12. Alterar permissões
13. Trocar usuário
14. Simular execução de arquivo
15. Mostrar blocos de armazenamento
0. Sair
```

Cada operação deve dizer claramente quais dados espera. A origem de operações com arquivos será um nome na pasta atual. Cópia e movimentação pedirão caminho absoluto do diretório de destino e nome final; renomear usará a própria pasta como destino. Isso deve aparecer no menu e no README.

Não interpretar comandos Linux completos. O enunciado aceita menu interativo.

## 5. Estruturas e limites

| Estrutura | Campos principais |
| --- | --- |
| FCB/Arquivo | ID, nome, enum de tipo, tamanho, criação/modificação/acesso, dono, grupo, bitmask, quantidade e índices dos blocos |
| Diretório | Nome, ponteiro para pai, arrays de ponteiros para arquivos e subpastas, contadores |
| Usuário | ID, nome e grupo |
| Disco | Array de bytes e mapa de blocos livres/ocupados |

O ID é um identificador inspirado em inode, não uma implementação completa de inode Unix. Será crescente e não reutilizado durante a execução; detectar esgotamento do contador antes de incrementar.

Limites iniciais centralizados em constantes:

- 128 blocos de 64 bytes: 8.192 bytes de capacidade para conteúdo.
- Até 16 blocos por arquivo: 1.024 bytes de conteúdo.
- Nomes de até 63 bytes, além do terminador `\0` no buffer.
- Vetores dinâmicos de arquivos e subpastas por diretório, conforme memória disponível.
- Profundidade máxima de 16 níveis abaixo da raiz, para limitar navegação e liberação recursiva.

Os limites são decisões do projeto, não exigências do PDF. Tamanho significa bytes; caracteres acentuados podem ocupar mais de um byte. Buffers para strings precisam reservar espaço adicional para `\0`.

Arquivos e diretórios serão alocados com `malloc`, verificando falha antes de uso. A saída normal e o fim da entrada devem liberar recursivamente todos os recursos.

## 6. Regras funcionais

### Nomes e caminhos

- Rejeitar nome vazio, somente espaços, `.` ou `..`, `/`, caracteres de controle e tamanho acima do limite. Espaços internos são permitidos.
- Comparar nomes distinguindo maiúsculas e minúsculas.
- Arquivos e pastas compartilham o espaço de nomes: não podem ter o mesmo nome dentro da mesma pasta. Nomes iguais em pastas diferentes são permitidos.
- Resolver caminhos absolutos a partir da raiz com uma função compartilhada. Aceitar `/`, componentes válidos e barra final; rejeitar componentes vazios intermediários e `.`/`..` em caminhos absolutos com mensagem clara.
- A opção de voltar na raiz mantém a raiz. Falha ao navegar preserva a pasta atual.

### Operações e metadados

| Operação | Comportamento |
| --- | --- |
| Criar | Arquivo vazio, novo ID, dono/grupo atuais, permissão 644 e três datas inicializadas |
| Escrever | Substituir integralmente o conteúdo, incluindo por vazio; atualizar tamanho e modificação após sucesso |
| Ler | Recuperar exatamente os bytes armazenados e atualizar acesso após sucesso |
| Copiar | Exigir R da origem; criar novo FCB, novo ID e blocos independentes; dono/grupo atuais, permissão 644 e novas datas |
| Mover | Transferir o mesmo FCB para outro diretório, mantendo ID, conteúdo, blocos, dono, grupo, permissões e datas |
| Renomear | Alterar somente o nome, mantendo os demais campos |
| Excluir | Remover a entrada, liberar blocos e FCB e ajustar os contadores |
| Detalhar | Mostrar nome, ID, tipo, tamanho, dono/grupo, datas, permissões e índices dos blocos |
| Executar | Exigir tipo programa e X; mostrar mensagem de execução simulada, sem executar o conteúdo |

A cópia usará as regras acima por simplicidade; elas não pretendem reproduzir todos os detalhes de `cp` e `umask` do Linux. A leitura da origem durante uma cópia bem-sucedida atualiza seu acesso. A execução simulada bem-sucedida também atualiza acesso. Listar, detalhar e alterar permissões não modificam as três datas. O projeto não modelará `ctime` de alteração de metadados.

Mover/renomear para o mesmo diretório com o mesmo nome informa que não há alteração. Cópia para o próprio nome é recusada. Não sobrescrever destinos existentes.

Conteúdo será uma linha textual, podendo conter espaços ou ser vazia. Os quatro tipos exigidos — numérico, caractere, binário e programa — serão representados por enum e exibidos nos detalhes. O tipo é metadado: não haverá interpretação numérica, importação de bytes binários ou execução real. Essa simplificação deve ser declarada na apresentação.

## 7. Permissões

Usuários fixos: `ana` e `bruno` no grupo `alunos`; `carla` no grupo `visitantes`. Não haverá usuário com acesso administrativo especial.

Selecionar exatamente uma classe: proprietário se o ID for o do dono; senão grupo se os grupos coincidirem; senão outros. Nunca tentar outra classe depois de negar acesso na classe escolhida.

Usar R=4, W=2 e X=1 em cada conjunto de bits. Receber exatamente três caracteres entre `0` e `7`, converter para bitmask e exibir tanto o formato numérico quanto o simbólico, por exemplo `754` e `rwxr-xr--`. Não armazenar 754 decimal como se fosse a máscara octal.

- Ler e copiar exigem R.
- Escrever exige W; W não depende de R.
- Executar exige X e tipo programa; X não depende de R.
- Alterar permissões, mover, renomear e excluir exigem ser proprietário.
- Ser proprietário não concede leitura, escrita ou execução quando os respectivos bits estiverem desligados.
- Criar e listar pastas e consultar metadados são operações permitidas a todos nesta versão.

Não implementar permissões de diretórios. Explicar no README que, no Linux real, permissões do diretório influenciam criação, remoção e movimentação. As regras do simulador para essas operações são uma simplificação própria.

Negação deve produzir mensagem clara e preservar conteúdo, metadados, árvore e ocupação dos blocos.

## 8. Armazenamento e integridade

Implementar alocação indexada simplificada: o FCB contém os índices dos blocos de dados, sem um bloco de índice adicional no disco simulado.

Quantidade de blocos = `(tamanho + TAM_BLOCO - 1) / TAM_BLOCO`. Arquivo vazio usa zero blocos. A leitura usa o tamanho do FCB, sem depender de terminador dentro dos blocos.

Antes de alterar qualquer estado, validar nomes, origem/destino, permissões, capacidade do diretório, tamanho, disponibilidade de blocos e alocações necessárias.

Na reescrita, reaproveitar os blocos do próprio arquivo e reservar apenas os adicionais necessários. Se faltar espaço, conservar integralmente o conteúdo anterior. Ao reduzir o arquivo, liberar a sobra. Na cópia, reservar todos os blocos independentes antes de publicar o novo arquivo. Na movimentação, validar o destino antes de retirar a entrada da origem.

Invariantes a conferir nos testes:

- Nenhum bloco pertence simultaneamente a arquivos diferentes.
- Todo bloco referenciado está marcado como ocupado e tem índice válido.
- A quantidade de blocos corresponde ao tamanho do conteúdo.
- Blocos ocupados mais livres somam 128.
- Cada arquivo pertence a um único diretório.
- Operações que falham não deixam reservas, entradas ou ponteiros parciais.

A opção de mostrar blocos exibirá capacidade, livres, ocupados e blocos por arquivo, incluindo arquivos em subpastas. Um arquivo de 150 bytes deve usar três blocos, com 64, 64 e 22 bytes úteis.

## 9. Entrada e tratamento de erros

Usar leitura de linhas com `fgets` e conversão validada com `strtol` para opções e números. Evitar misturar `scanf` com leitura de linhas.

Detectar linha longa demais, descartar seu restante e recusar a operação sem truncar silenciosamente nomes ou conteúdo. Aceitar exatamente o tamanho máximo previsto. Tratar fim da entrada encerrando com limpeza, sem repetir o menu indefinidamente.

Prever item inexistente, caminho inválido, tipo de item incorreto, nome duplicado, permissão negada, entrada inválida, diretório cheio, profundidade máxima, arquivo grande demais, disco cheio e falha de alocação. Todas as mensagens devem permitir continuar usando o programa quando possível.

## 10. Organização e etapas

```text
main.c          Menu, leitura e mensagens
sistema.h       Estruturas, constantes e interfaces
sistema.c       Diretórios, arquivos, usuários e permissões
disco.c         Alocação, leitura, escrita e liberação dos blocos
README.md       Compilação, uso, teoria, limites e testes
docs/PLANEJAMENTO.md Plano do projeto
```

Etapas, concluídas somente após compilação e teste:

1. Confirmar ambiente e definir contratos das estruturas e funções.
2. Inicializar raiz, usuários, disco, menu e limpeza de memória.
3. Implementar diretórios, nomes, caminhos e limites.
4. Implementar FCB, criação, blocos, escrita, leitura e detalhes.
5. Implementar permissões junto das operações protegidas.
6. Implementar cópia, movimentação, renomeação e exclusão com preservação em falhas.
7. Executar os testes de limites, permissões e memória.
8. Escrever README e conferir a cobertura do PDF.
9. Ensaiar testes livres em sala e preparar o vídeo solicitado.

## 11. Testes de aceitação para a avaliação em sala

Os cenários abaixo são preparação sugerida, não previsão das entradas que o professor escolherá. Registrar resultados reais quando o programa existir.

| Cenário | Resultado esperado |
| --- | --- |
| Criar `/a/b`, navegar, voltar e tentar voltar acima da raiz | Caminho correto; raiz preservada |
| Criar nomes iguais em pastas diferentes | Permitido |
| Duplicar nome entre arquivo e pasta na mesma pasta | Recusado |
| Nome vazio, somente espaços, barra e nome longo demais | Recusado sem alteração |
| Nome com espaço e conteúdo com espaços | Preservados corretamente |
| Item inexistente ou caminho passando por arquivo | Mensagem clara; estado preservado |
| Conteúdo de 0, 1, 63, 64, 65, 1.023 e 1.024 bytes | Uso de 0, 1, 1, 1, 2, 16 e 16 blocos; leitura exata |
| Conteúdo de 1.025 bytes | Recusado, sem truncamento nem perda do conteúdo anterior |
| Reescrever arquivo grande com pequeno e depois vazio | Liberar blocos excedentes e atualizar tamanho |
| Copiar e modificar ou excluir a cópia | Original intacto; IDs e blocos distintos |
| Mover e renomear | Mesmo ID, conteúdo e blocos |
| Destino cheio, inexistente ou com nome repetido | Recusado; origem preservada |
| Oito arquivos com 1.024 bytes cada | Disco cheio, com 128 blocos usados |
| Criar arquivo vazio com disco cheio | Permitido se houver vaga no diretório |
| Escrever ou copiar exigindo mais espaço com disco cheio | Recusado; sem perda ou vazamento de blocos |
| Reescrever com mesmo tamanho ou menor com disco cheio | Permitido, reaproveitando blocos próprios |
| Excluir arquivo e gravar outro | Blocos liberados reutilizados |
| Ultrapassar 32 entradas de cada categoria | Vetores crescem, preservando entradas e contadores |
| Atingir profundidade máxima e exceder | Recusar nova subpasta sem alterar a árvore |
| Menu com letras, número fora da faixa e linha longa | Recusar e processar corretamente a entrada seguinte |
| Fim da entrada e saída com árvore preenchida | Encerrar e liberar toda a memória |

### Matriz mínima de permissões

Usar arquivo pertencente a `ana`, grupo `alunos`, e trocar entre os três usuários.

| Permissão | Verificação |
| --- | --- |
| 640 | Ana lê/escreve; Bruno lê e não escreve; Carla não lê nem escreve |
| 000 | Nem Ana lê/escreve/executa; Ana ainda pode alterar permissões |
| 004 | Ana e Bruno não leem; Carla lê; comprova ausência de fallback |
| 020 | Bruno escreve sem precisar ler; Ana não escreve |
| 100 em tipo programa | Ana executa sem precisar ler; Bruno e Carla não executam |
| 777 em tipo não programa | Execução recusada pelo tipo |
| Entrada 888, 64, 0644 ou letras | Alteração recusada; máscara anterior preservada |

Também testar Bruno e Carla tentando alterar permissões, mover, renomear e excluir arquivo de Ana. As tentativas devem ser negadas. Repetir leitura, escrita e cópia sem autorização e confirmar preservação das datas e dos blocos.

### Memória

No WSL, se necessário instalar Valgrind antes da aula. Executar uma sessão com árvore aninhada, cópias, falhas, exclusões e saída:

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./simulador
```

Critério: nenhum acesso inválido, nenhum uso de valor não inicializado e nenhuma alocação do projeto sem liberação. Não marcar testes como aprovados sem executá-los.

## 12. README e demonstração

O README deve explicar compilação e execução, menu, limites, regras de nomes e caminhos, política de cópia, permissões, datas e limitações dos tipos. Deve relacionar explicitamente structs, ponteiros, malloc/free, FCB, ID inspirado em inode, árvore e proteção aos conceitos da disciplina.

Comparar alocação contínua, encadeada e indexada e justificar a opção usada. Mostrar correspondências com `mkdir`, `cd`, `ls`, `touch`, `cat`, `cp`, `mv`, `rm`, `stat` e `chmod`. Explicar diferenças reais, especialmente permissões de diretórios e ausência de execução binária. Incluir testes executados e resultados.

Na demonstração, partir do sistema vazio: criar duas pastas aninhadas, criar/escrever/ler arquivo, mostrar metadados, copiar e verificar independência, mover e verificar ID, testar as três classes de permissões, simular execução, mostrar múltiplos blocos e sua liberação. Incluir ao menos uma falha de entrada e uma falha de capacidade, provando que o programa continua consistente.

## 13. Condições para entrega

- [ ] Fontes compilam no Linux/WSL com os avisos indicados, sem avisos pendentes.
- [ ] Operações exigidas funcionam em sequências variadas.
- [ ] Permissões são verificadas dentro das operações, não apenas no menu.
- [ ] Limites e falhas preservam o estado anterior.
- [ ] Blocos e memória são liberados corretamente.
- [ ] README cobre os itens teóricos e exemplos exigidos pelo PDF.
- [ ] Fontes e documentação estão no repositório e disponíveis para avaliação.
- [ ] Compilação e demonstração foram ensaiadas no ambiente da aula.
- [ ] Vídeo no YouTube foi preparado conforme solicitado no enunciado.
- [ ] Os integrantes conseguem explicar estruturas, bitmasks, blocos e decisões do código.

Esta revisão valida o planejamento. Não atesta funcionamento de código nem execução dos testes.


## Extensão autorizada — cadastro de usuários

A opção 16 permite cadastrar usuários em memória, até MAX_USUARIOS=67 incluindo os três iniciais. Nome único validado por nome_validar; grupo alunos ou visitantes. IDs são sequenciais, independentes dos IDs de arquivos. quantidade_usuarios delimita as entradas ativas; TOTAL_USUARIOS=3 indica apenas a quantidade inicial. O cadastro preserva a identidade atual. A opção 13 lista e permite selecionar qualquer cadastrado. Não há remoção, autenticação, cadastro de grupos ou persistência. Esta extensão substitui a restrição anterior de apenas três usuários fixos. Testes de cadastro, permissões, limites e regressão executados com sucesso; detalhes em [TESTES.md](TESTES.md).



## Atualização de implementação — árvore e usuários dinâmicos

A pedido do usuário, arquivos/subpastas por diretório passam a usar vetores com crescimento dinâmico, removendo o limite anterior de 32 entradas. Usuários também são armazenados em vetor dinâmico, mantendo o limite funcional de 67 solicitado anteriormente. Cada vetor tem quantidade e capacidade; o crescimento copia para novo espaço antes de liberar o anterior, preservando atomicidade. Todos esses vetores são liberados ao sair. Disco, nomes, profundidade e tamanho máximo de arquivo conservam seus limites. Casos históricos de diretório cheio devem ser substituídos por testes de crescimento e falha de alocação. PDF original recebido: alocação dinâmica é exigida na seção 4.1; arrays são admitidos nas seções 3.1 e 3.4.


## Organização atual dos fontes

main.c: menu; sistema.c: inicialização, liberação e auxiliares comuns; pastas.c: árvore/caminhos; arquivos.c: operações de arquivos; usuarios.c: cadastros; permissoes.c: proteção; disco.c: blocos. sistema.h mantém a interface pública e sistema_interno.h os auxiliares entre módulos. Os comandos de compilação acima incluem todos os fontes.


Atualização de navegação: a opção 3 e sistema_entrar_pasta aceitam apenas o nome de uma subpasta da pasta atual. A opção 4 sobe ao pai. A resolução absoluta permanece para os destinos de cópia/movimentação.
