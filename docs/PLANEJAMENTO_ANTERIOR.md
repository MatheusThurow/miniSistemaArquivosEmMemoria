# Planejamento — Mini-Sistema de Arquivos em Memória

## 1. Objetivo

Criar um programa em **C**, com um **menu no terminal**, que simule arquivos e pastas na memória.

O projeto deve ser **simples de usar, funcional e fácil de explicar**. Vamos usar funções pequenas, nomes claros e comentários que expliquem as partes importantes.

Este planejamento se baseia no enunciado `T3_SistemasOperacionais_2025_1.pdf`.

## 2. Ambiente: WSL

O WSL já está instalado no computador, com a distribuição **Ubuntu-24.04**. Vamos usar esse ambiente Linux para compilar, executar e testar o trabalho.

### Abrir o projeto no WSL

No PowerShell, executar:

```powershell
wsl -d Ubuntu-24.04
```

Dentro do Ubuntu, acessar a pasta do projeto:

```bash
cd /mnt/c/faculdade/SO/M3/miniSistemaArquivosEmMemoria
```

Essa é a mesma pasta `C:\faculdade\SO\M3\miniSistemaArquivosEmMemoria`, vista pelo Linux.

### Conferir o compilador

```bash
gcc --version
```

Se o GCC não estiver instalado, instalar no Ubuntu:

```bash
sudo apt update
sudo apt install build-essential
```

### Compilar e executar

Depois que os arquivos de código estiverem prontos:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g main.c sistema.c disco.c -o simulador
./simulador
```

Os avisos do compilador devem ser corrigidos antes da entrega.

## 3. Como o simulador vai funcionar

Ao iniciar, o programa cria uma pasta raiz `/` e define o usuário atual.

O usuário escolhe uma opção no menu e responde a perguntas simples, como `Nome do arquivo:` e `Conteúdo:`.

Os arquivos e as pastas simulados ficam apenas na memória. Ao sair, os dados são perdidos. As operações do simulador não alteram arquivos reais do computador.

### Menu planejado

```text
Usuário: ana | Pasta atual: /

1. Listar conteúdo
2. Criar pasta
3. Entrar em uma pasta
4. Voltar para a pasta anterior
5. Criar arquivo
6. Escrever em arquivo
7. Ler arquivo
8. Copiar arquivo
9. Mover ou renomear arquivo
10. Excluir arquivo
11. Mostrar detalhes de um arquivo
12. Alterar permissões
13. Trocar usuário
14. Simular execução de arquivo
15. Mostrar blocos de armazenamento
0. Sair
```

## 4. Estruturas de dados

| Estrutura | Para que serve | O que guarda |
| --- | --- | --- |
| Arquivo / FCB | Representar o arquivo e seus metadados | ID, nome, tipo, tamanho, datas, dono, grupo, permissões e índices dos blocos |
| Diretório | Representar uma pasta da árvore | Nome, ponteiro para a pasta pai, arquivos e subpastas |
| Usuário | Identificar quem usa o sistema | Nome, ID e grupo |
| Disco simulado | Armazenar o conteúdo dos arquivos | Blocos de bytes e indicação dos blocos livres e ocupados |

O **FCB** é a ficha de informações do arquivo. Seu ID único representa um **inode simplificado**.

Cada diretório terá arrays de ponteiros para seus arquivos e subpastas. Os limites ficarão em constantes, para facilitar a leitura e os ajustes.

Arquivos e diretórios serão criados com `malloc` e liberados com `free`. Ao encerrar, o programa percorrerá a árvore para liberar toda a memória.

## 5. Funcionalidades e regras

| Funcionalidade | Regra simples |
| --- | --- |
| Listar conteúdo | Mostrar arquivos e subpastas da pasta atual |
| Criar pasta | Criar uma subpasta na pasta atual |
| Navegar | Entrar em uma subpasta ou voltar para a pasta pai |
| Criar arquivo | Pedir nome e tipo; gerar um ID único e registrar as datas |
| Escrever | Substituir o conteúdo e atualizar tamanho e data de modificação |
| Ler | Mostrar o conteúdo e atualizar a data de acesso |
| Copiar | Criar um arquivo com novo ID e blocos próprios; exigir leitura da origem |
| Mover | Transferir o arquivo para outra pasta, mantendo ID e conteúdo |
| Renomear | Alterar o nome, mantendo o ID |
| Excluir | Liberar os blocos e a memória do arquivo |
| Mostrar detalhes | Exibir metadados, permissões e índices dos blocos |
| Alterar permissões | Aceitar três dígitos de 0 a 7; permitir alteração apenas pelo proprietário |
| Trocar usuário | Escolher um dos usuários predefinidos |
| Simular execução | Verificar se o tipo é programa e se o usuário tem permissão de execução |
| Mostrar blocos | Exibir blocos livres, ocupados e os blocos usados por cada arquivo |

### Decisões para manter o projeto simples

- Usar menu numérico, sem interpretar comandos de um terminal Linux.
- Usar uma função compartilhada para localizar pastas por caminho absoluto, como `/documentos`, nas operações de cópia e movimentação.
- Não permitir nomes vazios, nomes `.` e `..`, barras no nome ou nomes repetidos na mesma pasta.
- Não sobrescrever automaticamente arquivos ao copiar ou mover.
- Registrar os tipos numérico, caractere, binário e programa usando `enum`.
- Receber conteúdo textual pelo menu. O tipo será um metadado; não haverá editor binário nem execução real de programas.
- Usar `time_t` para as datas de criação, modificação e acesso.
- Manter as permissões nos arquivos. Não reproduzir todas as regras de permissões de diretórios do Linux.
- Para mover, renomear e excluir, exigir que o usuário seja o proprietário. Essa é uma regra simplificada do simulador e será explicada no README.
- Deixar persistência em disco, links, interface gráfica e exclusão de pastas fora desta versão.

## 6. Permissões: quem pode fazer o quê

O programa verificará as permissões do usuário atual antes de ler, escrever ou simular a execução de um arquivo.

| Permissão | Letra | Valor |
| --- | --- | --- |
| Ler | R | 4 |
| Escrever | W | 2 |
| Executar | X | 1 |

As permissões serão separadas em **proprietário, grupo e outros**, usando operações de bits.

Exemplo: `754` significa:

- Proprietário: ler, escrever e executar (`7 = 4 + 2 + 1`).
- Grupo: ler e executar (`5 = 4 + 1`).
- Outros: apenas ler (`4`).

Vamos usar três usuários fixos:

| Usuário | Grupo | Finalidade |
| --- | --- | --- |
| ana | alunos | Criar arquivos e testar acesso como proprietária |
| bruno | alunos | Testar acesso pelo mesmo grupo |
| carla | visitantes | Testar acesso como outros |

O programa escolherá uma única classe: proprietário, caso o usuário seja o dono; senão, grupo, caso pertença ao grupo do arquivo; senão, outros.

Arquivos novos começarão com permissão `644`: o proprietário pode ler e escrever; grupo e outros podem ler. A execução poderá ser habilitada pelo menu.

Uma operação sem autorização mostrará `Permissão negada` e preservará os dados.

## 7. Armazenamento em blocos

Vamos usar **alocação indexada simplificada**. Isso significa que cada arquivo terá uma lista com os números dos blocos que guardam seu conteúdo.

Valores iniciais propostos:

- 128 blocos no disco simulado.
- 64 bytes por bloco.
- Até 16 blocos por arquivo: máximo de 1.024 bytes de conteúdo.

Exemplo:

```text
Arquivo com 150 bytes
  bloco 2: 64 bytes
  bloco 5: 64 bytes
  bloco 8: 22 bytes
```

Um arquivo vazio não ocupará blocos. O tamanho indicará quantos bytes devem ser lidos, sem depender de um terminador de string dentro do disco.

Antes de escrever, o programa verificará se há espaço suficiente, considerando os blocos que já pertencem ao arquivo. Se não houver, manterá o conteúdo anterior. Ao excluir, liberará os blocos para reutilização.

O enunciado apresenta a alocação de blocos com redação ambígua entre requisito e bônus. Vamos incluí-la para cobrir esse objetivo e demonstrar o armazenamento dos dados.

## 8. Organização dos arquivos

```text
miniSistemaArquivosEmMemoria/
├── PLANEJAMENTO.md  # Este plano
├── main.c           # Menu e entrada de dados
├── sistema.h        # Estruturas, constantes e declarações
├── sistema.c        # Pastas, arquivos, usuários e permissões
├── disco.c          # Controle dos blocos de armazenamento
└── README.md        # Instruções e explicação do trabalho
```

Vamos preferir funções pequenas, como `criar_arquivo`, `ler_arquivo`, `verificar_permissao` e `liberar_blocos`.

## 9. Etapas de desenvolvimento

Cada etapa só será marcada como concluída depois de funcionar no WSL.

- [ ] **Etapa 1 — Preparar o ambiente:** abrir o Ubuntu pelo WSL, acessar a pasta e conferir o GCC.
- [ ] **Etapa 2 — Criar a base:** definir estruturas, inicializar a raiz e criar um menu que abre e encerra corretamente.
- [ ] **Etapa 3 — Implementar pastas:** criar, listar, entrar e voltar; mostrar o caminho atual.
- [ ] **Etapa 4 — Implementar arquivos e blocos:** criar, escrever, ler e mostrar os metadados e os blocos usados.
- [ ] **Etapa 5 — Completar as operações:** copiar, mover, renomear e excluir arquivos.
- [ ] **Etapa 6 — Implementar proteção:** trocar usuário, alterar permissões e verificar leitura, escrita e execução.
- [ ] **Etapa 7 — Revisar erros e memória:** validar entradas, tratar limites e liberar todos os recursos.
- [ ] **Etapa 8 — Documentar:** escrever o README com exemplos e relação com a teoria.
- [ ] **Etapa 9 — Preparar a apresentação:** ensaiar a demonstração e gravar o vídeo para o YouTube solicitado no enunciado.

## 10. Testes no WSL

| Teste | Resultado esperado |
| --- | --- |
| Criar uma pasta, entrar e voltar | Caminho atual atualizado corretamente |
| Voltar estando na raiz | Continuar na raiz, sem erro |
| Criar nome vazio, inválido ou repetido | Mostrar mensagem clara e não criar |
| Informar uma opção inválida ou uma entrada longa demais | Rejeitar com mensagem e continuar funcionando |
| Acessar um arquivo ou pasta inexistente | Mostrar que o item não foi encontrado |
| Escrever e ler um arquivo | Recuperar exatamente o conteúdo informado |
| Escrever mais de 64 bytes | Usar mais de um bloco corretamente |
| Substituir conteúdo por outro menor | Liberar blocos que deixaram de ser necessários |
| Copiar e depois editar a cópia | Preservar o conteúdo do original |
| Mover ou renomear | Preservar ID, conteúdo e blocos |
| Copiar ou mover para um nome já existente | Recusar sem sobrescrever |
| Trocar entre proprietário, mesmo grupo e outro grupo | Aplicar a classe correta de permissões |
| Ler, escrever ou executar sem permissão | Negar a operação e preservar os dados |
| Alterar permissões sem ser proprietário | Negar a alteração |
| Atingir limite de pasta, arquivo ou disco | Informar o limite sem perder dados |
| Escrever sem espaço suficiente | Manter o conteúdo anterior |
| Excluir um arquivo | Liberar os blocos para reutilização |
| Encerrar o programa | Liberar toda a memória alocada |

Além dos testes manuais, podemos conferir o gerenciamento de memória com o Valgrind no WSL:

```bash
sudo apt install valgrind
valgrind --leak-check=full ./simulador
```

O objetivo é terminar sem acessos inválidos e sem memória perdida.

## 11. O que colocar no README

- Objetivo do simulador e seus limites.
- Como abrir a pasta pelo WSL, compilar e executar.
- Como usar o menu, com exemplos.
- Como as structs, os ponteiros, `malloc` e `free` são usados.
- O que são o FCB, os metadados e o ID que simula o inode.
- Como funciona a árvore de diretórios e por que ela ajuda a organizar arquivos.
- Como funcionam as permissões RWX e as operações de bits.
- Como o conteúdo é dividido e armazenado nos blocos.
- Comparação breve entre alocação contínua, encadeada e indexada, explicando a escolha feita.
- Comparação das opções do menu com `mkdir`, `cd`, `ls`, `touch`, `cat`, `cp`, `mv`, `rm`, `stat` e `chmod` do Linux.
- Diferenças entre as regras simplificadas do simulador e um sistema Linux real.
- Testes realizados e seus resultados.

## 12. Roteiro simples para a apresentação

1. Explicar o objetivo e mostrar o programa rodando no WSL.
2. Criar uma pasta e entrar nela.
3. Criar um arquivo, escrever e ler seu conteúdo.
4. Mostrar o FCB: ID, tamanho, tipo, datas e permissões.
5. Copiar, renomear e mover o arquivo.
6. Trocar usuário e demonstrar uma permissão negada.
7. Alterar a permissão com o proprietário e repetir a operação.
8. Demonstrar a execução simulada de um arquivo do tipo programa.
9. Mostrar um arquivo com vários blocos.
10. Excluir um arquivo e mostrar a liberação dos blocos.
11. Apresentar rapidamente as principais structs e funções.

## 13. Quando o trabalho estará pronto

- [ ] Todas as funcionalidades planejadas funcionam no WSL.
- [ ] O código compila sem avisos com as opções indicadas.
- [ ] As mensagens de erro são claras e o programa continua funcionando.
- [ ] As permissões realmente permitem ou bloqueiam as operações.
- [ ] Os blocos são reservados e liberados corretamente.
- [ ] A memória é liberada ao excluir itens e ao sair.
- [ ] O README explica como usar o programa e como ele se relaciona com a teoria.
- [ ] A apresentação demonstra os requisitos do enunciado.
- [ ] Conseguimos explicar cada estrutura e cada função com palavras simples.
