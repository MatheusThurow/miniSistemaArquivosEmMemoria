# Mini-sistema de arquivos em memória

Simulador em C11 para Linux/WSL. Inicia vazio, na raiz `/`, como `ana`. Todas as operações atuam em memória; sair ou receber EOF libera a árvore e perde os dados. Nenhum conteúdo é executado como comando do computador.

## Compilar e usar

Para abrir pelo Prompt de Comando do Windows, siga [COMANDOS_CMD.txt](COMANDOS_CMD.txt).

No PowerShell, entre no Ubuntu com `wsl -d Ubuntu-24.04`. No terminal Ubuntu:

```bash
cd /mnt/c/faculdade/SO/M3/miniSistemaArquivosEmMemoria
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador
./simulador
```

Em outro Linux, entre na pasta onde copiou os fontes. Não há caminhos fixos no código, bibliotecas externas ou necessidade de internet para compilar e executar (GCC precisa estar instalado).

## Menu e entradas

| Opção | Ação | Analogia Linux |
| --- | --- | --- |
| 1 | Listar pastas e metadados dos arquivos | ls |
| 2 | Criar pasta na pasta atual | mkdir |
| 3 | Entrar pelo nome de uma subpasta | cd |
| 4 | Voltar ao pai; na raiz não altera nada | cd .. |
| 5 | Criar arquivo vazio, escolhendo tipo de 1 a 4 | touch |
| 6 | Substituir todo o conteúdo por uma linha, inclusive vazia | redirecionamento de escrita |
| 7 | Ler conteúdo | cat |
| 8 | Copiar para pasta absoluta e nome final | cp |
| 9 | Mover ou renomear para pasta absoluta e nome final | mv |
| 10 | Excluir arquivo | rm |
| 11 | Consultar metadados | stat |
| 12 | Alterar três dígitos octais de permissões | chmod |
| 13 | Trocar entre ana, bruno e carla | troca de identidade simulada |
| 14 | Simular execução de programa | execução apenas ilustrativa |
| 15 | Mostrar capacidade e blocos em toda a árvore | inspeção didática |
| 16 | Criar usuário, escolhendo alunos ou visitantes | cadastro de identidade simulada |
| 0 | Liberar memória e sair | saída |

A origem de arquivo é sempre um nome na pasta atual. Para renomear, informe como destino o caminho absoluto da própria pasta. Destinos existentes nunca são sobrescritos. Mover para o mesmo nome/local informa ausência de alteração. Copiar para o próprio nome é recusado.

Digite apenas os valores pedidos, não comandos Linux. Linhas longas são descartadas integralmente e a operação é cancelada. Letras ou números fora da faixa no menu são recusados. EOF, inclusive durante uma operação, encerra com limpeza.

## Limites e regras

- Disco: 128 blocos de 64 bytes, total de 8.192 bytes de conteúdo.
- Arquivo: até 16 blocos / 1.024 bytes; vazio ocupa zero blocos.
- Diretório: vetores dinâmicos de arquivos e subpastas, sem o antigo limite de 32 entradas; raiz mais 16 níveis. A quantidade de entradas depende da memória disponível.
- Nome: 1 a 63 bytes; diferencia maiúsculas e minúsculas, permite espaços e UTF-8. Rejeita somente espaços, `.`, `..`, barra e controles ASCII.
- Navegação: opção 3 aceita apenas o nome de uma subpasta; use a opção 4 para voltar ao pai.
- Caminho absoluto (somente destino de cópia/movimentação): `/`, componentes válidos e uma barra final opcional. Não aceita `//`, `.` ou `..` como componentes. Caminhos relativos compostos não são aceitos.
- Conteúdo: uma linha textual, sem NUL, CR ou LF. Tamanhos são bytes, não caracteres.

Os tipos são 1=numérico, 2=caractere, 3=binário e 4=programa. São metadados: não interpretam números, não importam binários e não executam código. Não há persistência, links, exclusão/movimentação de diretórios ou permissões em diretórios.

## Proteção e datas

`ana` e `bruno` pertencem a `alunos`; `carla`, a `visitantes`. São os três usuários iniciais. A opção 16 cadastra novos usuários (até 67 no total), com nome único de até 63 bytes seguindo as mesmas regras dos nomes de arquivos, e grupo 1=alunos ou 2=visitantes. O cadastro não troca o usuário atual; use a opção 13, que lista todos os cadastrados. Cada cadastro recebe ID próprio, usado como dono dos arquivos que criar. Qualquer usuário pode cadastrar: não há autenticação nem administração de contas reais. Cadastros desaparecem ao sair; não há remoção de usuários ou criação de novos grupos. Não há administrador. Escolhe-se exatamente uma classe: dono, senão grupo, senão outros. Uma negativa não tenta outra classe. R=4, W=2 e X=1; a conversão de `754` gera uma máscara octal e a exibição `rwxr-xr--`.

Ler/copiar exige R; escrever exige W; executar exige X e tipo programa. W e X independem de R. Só o dono pode alterar permissões, mover, renomear e excluir; ser dono não concede R/W/X implicitamente. Todos podem criar pastas/arquivos, listar e consultar metadados. No Linux real, as permissões do diretório também controlam criação, remoção e movimento; aqui essas regras são simplificadas.

Criação e cópia recebem novo ID, dono/grupo atuais, máscara 644 e três datas novas. A cópia tem FCB e blocos independentes, e atualiza o acesso da origem somente no sucesso. Escrita atualiza modificação; leitura e execução atualizam acesso. Movimento preserva identidade, blocos e datas; renomeação só muda o nome. Listagem, detalhes e chmod não mudam datas. Não há `ctime`. Operações próximas podem ter horários iguais pela resolução de `time()`.

IDs começam em 1, crescem sem reutilização e detectam esgotamento após UINT64_MAX. Erros preservam estado, datas e saídas. Reserva de blocos é integral; reescrita reaproveita os próprios blocos e só exige espaço adicional para expansão. Redução e exclusão devolvem e zeram blocos.

## Estruturas e conceitos

`sistema.h` é a interface usada pelo menu; `sistema_interno.h` declara apenas os auxiliares compartilhados entre os módulos.

`sistema.h` centraliza constantes, enums, structs e contratos. `main.c` cuida da entrada, mensagens e consultas; `sistema.c`, da inicialização, liberação e auxiliares comuns; `pastas.c`, da árvore e caminhos; `arquivos.c`, das operações de arquivos; `usuarios.c`, dos cadastros; `permissoes.c`, do controle de acesso; `disco.c`, dos dados e mapa de blocos.

`Sistema` contém o disco por valor, um vetor dinâmico de usuários e referências à raiz e pasta atual. Cada `Diretorio` possui vetores dinâmicos de ponteiros para filhos e arquivos; o ponteiro `pai` permite voltar. Essa árvore organiza nomes hierarquicamente. Raiz, filhos, FCBs e vetores são alocados com `calloc` (alocação dinâmica como `malloc`, mas inicializada com zeros). Cada vetor registra quantidade em uso e capacidade alocada.

Os vetores de arquivos/subpastas começam sem espaço reservado. Na primeira inclusão, recebem quatro posições; quando lotam, a capacidade dobra. O vetor de usuários começa com três posições e também cresce conforme o cadastro, mantendo o limite funcional de 67 solicitado pelo usuário. Crescer envolve alocar um vetor maior, copiar as entradas e liberar o antigo. É uma alternativa explícita a `realloc`: o vetor antigo continua válido caso alguma etapa falhe. O FCB e as pastas permanecem nos mesmos endereços, preservando referências durante crescimento e movimento.

Antes de confirmar uma operação, todas as alocações necessárias são verificadas. Falha preserva entradas, conteúdo, IDs e blocos. Vetores temporários são liberados no erro. Ao excluir um arquivo, seu FCB e seus blocos são liberados; a capacidade do vetor é mantida para reutilização. Na saída, inclusive EOF, `free` libera os FCBs, cada vetor de arquivos/subpastas, os diretórios e o vetor de usuários. Pai, pasta atual e consultas são referências emprestadas e não recebem uma segunda liberação. A liberação do sistema pode ser repetida.

Continuam fixos por decisão do simulador: disco de 8.192 bytes, até 1.024 bytes por arquivo, nomes de 63 bytes e profundidade de 16 níveis. A alocação dinâmica dos cadastros e da árvore não implica disco infinito.

O `Arquivo` é o FCB: guarda nome, tipo, tamanho, dono/grupo, bitmask, datas e índices de blocos. Seu ID é inspirado em inode, sem implementar um inode Unix completo. Mover transfere o mesmo ponteiro; copiar aloca outro FCB. O mapa de ocupação evita compartilhar blocos. Bitmasks permitem testar os bits da classe selecionada com deslocamento e AND.

| Alocação | Organização | Consequência |
| --- | --- | --- |
| Contínua | Blocos consecutivos | Acesso simples, mas fragmentação externa e expansão difícil |
| Encadeada | Cada bloco liga ao próximo | Cresce usando espaços dispersos, mas acesso aleatório exige percorrer a cadeia |
| Indexada | Lista de índices por arquivo | Acesso direto e expansão dispersa, com custo para armazenar índices |

Foi escolhida alocação indexada simplificada: o FCB mantém até 16 índices; não há bloco de índice extra no disco. `ceil(tamanho/64)` determina a quantidade. A leitura respeita o tamanho, sem depender de NUL nos blocos. Para 150 bytes, três blocos têm 64, 64 e 22 bytes úteis.

## Verificação e entrega

Consulte [TESTES.md](docs/TESTES.md) para comandos e resultados reproduzíveis e [DEMONSTRACAO.md](docs/DEMONSTRACAO.md) para ensaio. Compilação com GCC 13.3.0/Ubuntu-24.04, testes automatizados e Valgrind 3.22.0 foram executados em 15/11/2026: sem avisos, sem erros de memória e sem vazamentos nos cenários executados. Isso não prova ausência de todos os defeitos possíveis.

[PLANEJAMENTO.md](docs/PLANEJAMENTO.md) é a versão revisada vigente; [PLANEJAMENTO_ANTERIOR.md](docs/PLANEJAMENTO_ANTERIOR.md) preserva a anterior. [CONTRATOS_T01.md](docs/CONTRATOS_T01.md) descreve as interfaces e [TASKS.md](docs/TASKS.md) registra o estado atual. O PDF original foi disponibilizado e suas seções 3.1, 3.4 e 4.1 foram conferidas quanto a arrays e alocação dinâmica; ele exige uso de alocação dinâmica, mas não proíbe vetores com capacidade limitada. Continuam pendentes confirmação e recompilação na máquina da aula, ensaio dos integrantes, vídeo e orientações finais de entrega. Não houve publicação ou entrega externa.

Atualização de 15/11/2026: a avaliação será na própria máquina do usuário, já validada com WSL, GCC e recompilação. A confirmação do ambiente deixou de ser pendência; os demais itens humanos continuam pendentes.



## Organização da pasta

```text
miniSistemaArquivosEmMemoria/
├── src/
│   ├── main.c
│   ├── sistema.c
│   ├── pastas.c
│   ├── arquivos.c
│   ├── usuarios.c
│   ├── permissoes.c
│   └── disco.c
├── include/
│   ├── sistema.h
│   └── sistema_interno.h
├── docs/
├── tests/
├── README.md
├── COMANDOS_CMD.txt
├── simulador
├── .clang-format
└── .gitignore
```

Execute os comandos de compilação e testes na raiz do projeto; os arquivos `.c` ficam em `src/` e os headers `.h` em `include/`.
