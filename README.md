# Mini-sistema de arquivos em memória

Projeto de Sistemas Operacionais desenvolvido em C. Simula arquivos, pastas, usuários, permissões RWX e alocação de blocos. Os dados ficam na memória e são apagados ao encerrar o programa.

## Clonar o projeto

```bash
git clone https://github.com/MatheusThurow/miniSistemaArquivosEmMemoria.git
cd miniSistemaArquivosEmMemoria
```

## Compilar e executar

No Linux ou Ubuntu pelo WSL, com GCC instalado, execute na pasta do projeto:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o simulador
./simulador
```

No Windows, entre no Ubuntu com `wsl -d Ubuntu-24.04`. Para usar a pasta local pelo CMD, consulte [COMANDOS_CMD.txt](COMANDOS_CMD.txt).

## Como usar

Escolha as opções do menu para criar e navegar entre pastas, criar, escrever, ler, copiar, mover e excluir arquivos, alterar permissões e gerenciar usuários. Digite `0` para sair.

O disco possui 128 blocos de 64 bytes (8 KB), e cada arquivo pode ter até 1.024 bytes. A execução de programas é apenas simulada.

## Organização do código

- `src/main.c`: menu e entrada de dados
- `src/sistema.c`: inicialização e liberação da memória
- `src/pastas.c`: diretórios e caminhos
- `src/arquivos.c`: operações com arquivos
- `src/usuarios.c`: cadastro e troca de usuários
- `src/permissoes.c`: permissões de acesso
- `src/disco.c`: armazenamento e alocação de blocos
- `include/`: estruturas e declarações das funções
- `tests/`: testes automatizados
- `docs/`: documentação técnica

## Como funciona

- **Diretórios.** A struct `Diretorio` tem um ponteiro para a pasta pai e vetores de ponteiros para as subpastas e os arquivos. A árvore agrupa os arquivos por pasta, permite repetir nomes em pastas diferentes e limita a busca de um nome às entradas da pasta consultada.
- **Arquivos.** A struct `Arquivo` é o FCB (bloco de controle do arquivo). Ela guarda nome, tipo, tamanho em bytes, datas de criação, modificação e acesso, dono, grupo, permissões e índices dos blocos. O ID único simula o número de um inode. Mover ou renomear preserva o ID e copiar cria outro.
- **Permissões.** A opção **12** funciona como um `chmod` numérico. Cada dígito representa dono, grupo e outros, nessa ordem. Os bits valem 4 para leitura, 2 para escrita e 1 para execução. Com `640`, o dono lê e escreve, o grupo só lê e os demais ficam sem acesso ao conteúdo. O simulador confere esses bits ao ler, escrever ou simular execução.
- **Blocos.** A alocação contínua guarda os blocos lado a lado. Na encadeada, cada bloco aponta para o próximo. O simulador usa **alocação indexada** porque o vetor de índices no FCB permite localizar cada bloco e aproveitar espaços livres espalhados no disco. Um arquivo de 65 bytes ocupa dois blocos. A opção **15** mostra os blocos usados.

## Exemplos no menu e no Linux

| Operação | No simulador | No Linux |
| --- | --- | --- |
| Criar e entrar em uma pasta | **2** e **3**, informe `aula` em cada | `mkdir aula` e `cd aula` |
| Criar um arquivo | **5**, informe `texto` e tipo `2` | `touch texto` |
| Escrever e ler | **6**, grave `ola` em `texto`. Depois, leia com **7** | `echo ola > texto` e `cat texto` |
| Copiar para a raiz | **8**, informe `texto`, `/` e `copia` | `cp texto ../copia` (a partir de `aula`) |
| Renomear na pasta atual | **9**, informe `texto`, `/aula` e `final` | `mv texto final` |
| Ver detalhes e alterar permissões | **11**, informe `final`. Use **12** com `final` e `640` | `stat final` e `chmod 640 final` |
| Excluir | **10**, informe `final` | `rm final` |

## Teste rápido

1. Abra o simulador como `ana` na raiz `/`. Crie a pasta `aula` com **2** e entre nela com **3**.
2. Use **5** para criar `texto` com tipo `2`. Use **6** para gravar `ola` e **7** para conferir o conteúdo.
3. Use **12** para definir a permissão de `texto` como `640`. Troque para `bruno` com **13** e confirme que ele consegue ler com **7**, mas recebe uma mensagem de permissão negada ao tentar escrever com **6**.
4. Use **15** para conferir que `texto` ocupa um bloco e **0** para sair.

Os comandos dos testes automatizados estão em [docs/TESTES.md](docs/TESTES.md).
