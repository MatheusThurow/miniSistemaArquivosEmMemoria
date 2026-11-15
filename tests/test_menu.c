/* Testes de integracao do menu em C11/POSIX, sem Python.
 * Executar a partir da raiz do projeto, apos compilar ./simulador.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CAP_ENTRADA 32768
#define CAP_SAIDA 262144

static char entrada[CAP_ENTRADA];
static char saida[CAP_SAIDA];
static size_t usado;

static void falha(const char *mensagem) {
    fprintf(stderr, "FALHA: %s\n", mensagem);
    exit(EXIT_FAILURE);
}
static void limpar(void) { usado = 0; entrada[0] = '\0'; }
static void adicionar(const char *trecho) {
    size_t n = strlen(trecho);
    if (n >= CAP_ENTRADA - usado) falha("entrada excede o buffer");
    memcpy(entrada + usado, trecho, n + 1);
    usado += n;
}
static void repetir(char caractere, size_t quantidade) {
    if (quantidade >= CAP_ENTRADA - usado) falha("repeticao excede o buffer");
    memset(entrada + usado, caractere, quantidade);
    usado += quantidade;
    entrada[usado] = '\0';
}
static void exigir(const char *texto) {
    if (!strstr(saida, texto)) {
        fprintf(stderr, "Nao encontrado: [%s]\n", texto);
        falha("saida inesperada do simulador");
    }
}
static void proibir(const char *texto) {
    if (strstr(saida, texto)) {
        fprintf(stderr, "Encontrado indevidamente: [%s]\n", texto);
        falha("saida inesperada do simulador");
    }
}
static size_t contar(const char *texto) {
    size_t quantidade = 0;
    size_t n = strlen(texto);
    const char *p = saida;
    while ((p = strstr(p, texto)) != NULL) {
        quantidade++;
        p += n;
    }
    return quantidade;
}
static void executar(void) {
    FILE *in = tmpfile(), *out = tmpfile();
    if (!in || !out) falha("nao foi possivel criar arquivos temporarios");
    if (fwrite(entrada, 1, usado, in) != usado || fflush(in) != 0)
        falha("nao foi possivel gravar entrada");
    rewind(in);
    pid_t pid = fork();
    if (pid < 0) falha("fork falhou");
    if (pid == 0) {
        if (dup2(fileno(in), STDIN_FILENO) < 0 ||
            dup2(fileno(out), STDOUT_FILENO) < 0 ||
            dup2(fileno(out), STDERR_FILENO) < 0) _exit(127);
        fclose(in);
        fclose(out);
        execl("./simulador", "./simulador", (char *)NULL);
        _exit(127);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) falha("waitpid falhou");
    rewind(out);
    size_t lidos = fread(saida, 1, CAP_SAIDA - 1, out);
    saida[lidos] = '\0';
    if (ferror(out)) falha("erro ao ler saida");
    fclose(in);
    fclose(out);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Saida do simulador:\n%.2000s\n", saida);
        falha("simulador encerrou com erro");
    }
    exigir("Sistema liberado.");
}
int main(void) {
    limpar();
    adicionar("abc\n99\n"); repetir('9', 2000);
    adicionar("\n2\n"); repetir('n', 63);
    adicionar("\n3\n"); repetir('n', 63);
    adicionar("\n5\nf\n4\n6\nf\n"); repetir('a', 1024);
    adicionar("\n7\nf\n6\nf\n"); repetir('b', 1025);
    adicionar("\n7\nf\n15\n0\n");
    executar();
    char a1024[1025]; memset(a1024, 'a', 1024); a1024[1024] = '\0';
    if (contar(a1024) != 2) falha("conteudo de 1024 bytes nao apareceu duas vezes");
    exigir("Entrada longa demais"); exigir("Opcao invalida");

    limpar();
    adicionar("2\na\n3\na\n2\nb\n3\nb\n5\np\n4\n6\np\n");
    repetir('x', 150);
    adicionar("\n8\np\n/\nc\n9\np\n/a\nmovido\n4\n4\n12\nc\n100\n14\nc\n13\nbruno\n7\nc\n13\nana\n10\nc\n15\n3\na\n15\n0\n");
    executar();
    exigir("22 bytes uteis"); exigir("64 bytes uteis");
    exigir("Permissao negada"); exigir("Execucao simulada");

    const char *eof[] = {
        "", "5\n", "5\nf\n", "2\na\n3\na\n5\nf\n2\n6\nf\nabc",
        "2\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n2\nok\n1\n0\n"
    };
    for (size_t i = 0; i < sizeof(eof) / sizeof(eof[0]); i++) {
        limpar(); adicionar(eof[i]); executar();
    }
    puts("Testes de menu, sessao integrada, limites e EOF aprovados.");

    limpar();
    adicionar("16\ndiego\n1\n13\ndiego\n5\nmeu\n2\n11\nmeu\n16\neva\n2\n13\neva\n6\nmeu\nnegado\n16\ndiego\n1\n16\ninvalido\n3\n0\n");
    executar();
    exigir("diego/alunos"); exigir("diego (alunos)");
    exigir("eva (visitantes)"); exigir("Permissao negada");
    exigir("Nome duplicado"); exigir("Argumento invalido");
    limpar(); adicionar("16\nnovo\n"); executar();
    puts("Menu de cadastro e troca de novos usuarios aprovado.");

    limpar();
    for (int i = 0; i < 40; i++) {
        char trecho[80];
        snprintf(trecho, sizeof trecho, "2\npasta%d\n5\narquivo%d\n2\n", i, i);
        adicionar(trecho);
    }
    adicionar("1\n"); executar();
    exigir("[pasta] pasta39"); exigir("arquivo39 | ID=40");
    puts("Menu com mais de 32 arquivos/subpastas e EOF aprovado.");

    limpar();
    adicionar("2\na\n3\n/a\n3\na\n2\nb\n3\na/b\n3\nb\n4\n4\n4\n0\n");
    executar();
    exigir("Nome invalido"); exigir("Pasta atual: /a/b");
    exigir("Sem alteracao"); exigir("Nome da subpasta:");
    proibir("nome ou absoluto");
    puts("Navegacao apenas por nome, recusa de caminhos e retorno ao pai aprovados.");
    return EXIT_SUCCESS;
}
