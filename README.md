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

Os diretórios formam uma árvore com alocação dinâmica. Cada arquivo possui um FCB com seus metadados e índices de blocos. As permissões usam máscaras de bits para proprietário, grupo e outros.

Veja os comandos de teste em [docs/TESTES.md](docs/TESTES.md) e um roteiro de uso em [docs/DEMONSTRACAO.md](docs/DEMONSTRACAO.md).
