# Demonstração a partir do sistema vazio

Compile seguindo o [README](../README.md) e execute `./simulador`. Os nomes abaixo são valores para responder aos prompts, não comandos digitados no menu.

1. Mostre usuário ana, grupo alunos e raiz. Use 2 para criar `a`, 3 para entrar, 2 para criar `b` e 3 para entrar. Caminho esperado: `/a/b`. Use 4 e volte a entrar em `b`.
2. Use 5: nome `texto`, tipo 2. Use 6 para escrever `conteudo com espacos`; use 7 para conferir e 11 para registrar ID, datas, dono, máscara 644 e blocos.
3. Use 8: origem `texto`, destino `/a`, nome `copia`. Use 4 para voltar a `/a` e detalhe `copia`: ID novo, dono ana, máscara 644. Substitua seu conteúdo e exclua. Entre pelo nome `b` para retornar a `/a/b` e leia `texto`: original intacto.
4. Use 9: `texto`, destino `/a`, nome `movido`. Use 4 para voltar a `/a` e detalhe: mesmo ID e blocos da origem. Renomeie para `final` usando destino `/a`; confira identidade. Repita mesmo destino/nome: sem alteração.
5. Use 12 em `final`: 640. Troque (13) para bruno: ler funciona, escrever falha. Troque para carla: leitura falha; detalhes continuam disponíveis. Retorne a ana.
6. Configure 004: ana não lê; bruno não lê; carla lê. Isso demonstra classe exclusiva sem fallback. Configure 020: bruno escreve sem ler; ana não escreve. Ana ainda consegue alterar permissões.
7. Como ana, crie `programa`, tipo 4; configure 100. A opção 14 simula execução sem leitura. Crie um tipo 2 com 777 e tente executá-lo: tipo incorreto.
8. Para blocos, escreva em `programa` os 150 caracteres desta linha (três trechos de 50, sem espaços adicionais):

```text
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbcccccccccccccccccccccccccccccccccccccccccccccccccc
```

Antes, configure 700 para permitir escrita. Use 15: três blocos com 64, 64 e 22 bytes úteis, inclusive em subpastas. Escreva uma linha vazia: volta a zero blocos. Exclua e confira os livres.

9. Digite `abc` no menu e tente criar nome duplicado. Depois liste para demonstrar continuação. Volte à raiz e use 4: sem alteração.
10. Falha de capacidade: em uma sessão vazia, crie oito arquivos e escreva 1.024 bytes em cada. O disco mostra 128 ocupados; crie um nono vazio (permitido) e tente escrever um byte (disco cheio). Reescreva um dos oito com 64 bytes e tente novamente: sucesso. Para reproduzir automaticamente os limites sem contar caracteres manualmente, execute `tests/test_sistema` conforme [TESTES.md](TESTES.md).
11. Encerre com 0. Em nova execução, tudo está vazio. Demonstre também Ctrl+D/EOF no Linux, que libera memória.

## Perguntas para ensaio

- Quem é dono dos ponteiros? Por que pai e atual não são liberados separadamente?
- O que o FCB contém? Por que o ID não muda ao mover?
- Qual a diferença entre a cópia e a transferência do mesmo ponteiro?
- Como 150 bytes se distribuem em blocos? Por que vazio não usa bloco?
- Por que W e X funcionam sem R? Por que ana não lê com 004?
- Como reserva atômica e reaproveitamento evitam perder conteúdo com disco cheio?
- Quais diferenças existem em relação ao Linux e aos três métodos de alocação?

## Checklist local e ações humanas

- [x] Fontes, header, contratos, README, planejamento vigente e testes disponíveis na pasta do projeto.
- [x] Compilação local reproduzida, testes automatizados e Valgrind executados.
- [x] Roteiro preparado a partir do sistema vazio, com resultados esperados.
- [ ] Integrantes ensaiarem operações livres e responderem às perguntas.
- [x] Máquina da aula confirmada pelo usuário: sua própria máquina, com Linux/WSL, GCC e recompilação já validados.
- [ ] Copiar fontes e documentação para mídia/acesso offline; conferir a cópia no destino.
- [ ] Consultar PDF original e orientações finais de entrega.
- [ ] Gravar e conferir vídeo para YouTube conforme as orientações; publicação/envio são ações externas pendentes.

A pasta atual não tem repositório Git inicializado. Os arquivos estão disponíveis localmente; versionamento/entrega da disciplina ainda precisam ser combinados pelos integrantes.



## Demonstrar cadastro de usuário

Use 16, informe `diego` e grupo 1 (alunos). Use 13: a lista agora inclui diego; digite seu nome para trocar. Crie um arquivo e consulte os detalhes: dono diego, grupo alunos. Troque para ana para continuar o roteiro. Também pode cadastrar `eva` no grupo 2 (visitantes) para demonstrar a classe outros. Repetir um nome existente é recusado.


## Explicar memória dinâmica

Cada pasta e arquivo é criado em memória conforme necessário. Os vetores de entradas começam vazios e aumentam quando lotam: alocam outro espaço, copiam os ponteiros e liberam o vetor antigo. Usuários usam o mesmo princípio, mantendo a regra de até 67 cadastros. Pastas podem ultrapassar as antigas 32 entradas. O disco continua com capacidade finita, então a demonstração de disco cheio acima permanece válida. Ao sair, todos os vetores e objetos alocados são liberados. Execute tests/test_dinamico conforme TESTES.md para demonstrar crescimento e tratamento de falhas; consulte os logs Valgrind para a liberação.


Navegação atual: opção 3 aceita somente o nome de uma subpasta. Para chegar à raiz, repita a opção 4; para descer, use 3 com um nome por vez. Caminhos absolutos permanecem apenas nos destinos de cópia e movimento.
