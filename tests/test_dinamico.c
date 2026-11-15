#include "sistema.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t falhar_na, chamadas;
void *__real_calloc(size_t, size_t);
void *__wrap_calloc(size_t quantidade, size_t tamanho) {
    chamadas++;
    if (falhar_na && chamadas == falhar_na) {
        return NULL;
    }
    return __real_calloc(quantidade, tamanho);
}
static void falhar(size_t chamada) { chamadas = 0; falhar_na = chamada; }
static void preencher(Sistema *s) {
    assert(sistema_inicializar(s) == SIS_OK);
    for (int i=0; i<4; i++) {
        char nome[16]; snprintf(nome,sizeof nome,"f%d",i);
        assert(sistema_criar_arquivo(s,nome,TIPO_PROGRAMA)==SIS_OK);
        snprintf(nome,sizeof nome,"d%d",i);
        assert(sistema_criar_pasta(s,nome)==SIS_OK);
    }
    assert(s->raiz->capacidade_arquivos==4 && s->raiz->capacidade_subpastas==4);
    assert(sistema_escrever_arquivo(s,"f0",(unsigned char *)"abc",3)==SIS_OK);
}
static void falhas(void) {
    Sistema zero={0};
    for (size_t i=1; i<=2; i++) {
        Sistema s={0}; falhar(i);
        assert(sistema_inicializar(&s)==SIS_SEM_MEMORIA);
        assert(!memcmp(&s,&zero,sizeof s)); sistema_liberar(&s);
    }
    falhar(0);
    for (int operacao=0; operacao<5; operacao++) {
        size_t alocacoes = operacao<3 ? 2 : 1;
        for(size_t i=1; i<=alocacoes; i++) {
            Sistema s={0}; preencher(&s);
            /* movimento parte de outra pasta, para uma raiz com vetor lotado */
            if(operacao==3) {
                assert(sistema_entrar_pasta(&s,"d0")==SIS_OK);
                assert(sistema_criar_arquivo(&s,"origem",TIPO_CARACTERE)==SIS_OK);
            }
            Sistema antes=s;
            Diretorio raiz=*s.raiz, atual=*s.atual;
            Arquivo arquivo=*s.raiz->arquivos[0];
            Arquivo *entradas[4]; memcpy(entradas,s.raiz->arquivos,sizeof entradas);
            Usuario usuarios[3]; memcpy(usuarios,s.usuarios,sizeof usuarios);
            falhar(i); Resultado r;
            if(operacao==0) r=sistema_criar_arquivo(&s,"novo",TIPO_CARACTERE);
            else if(operacao==1) r=sistema_criar_pasta(&s,"nova");
            else if(operacao==2) r=sistema_copiar_arquivo(&s,"f0","/","copia");
            else if(operacao==3) r=sistema_mover_arquivo(&s,"origem","/","movido");
            else r=sistema_criar_usuario(&s,"novo",GRUPO_ALUNOS);
            assert(r==SIS_SEM_MEMORIA);
            assert(!memcmp(&antes,&s,sizeof s));
            assert(!memcmp(&raiz,s.raiz,sizeof raiz));
            assert(!memcmp(&atual,s.atual,sizeof atual));
            assert(!memcmp(&arquivo,s.raiz->arquivos[0],sizeof arquivo));
            assert(!memcmp(entradas,s.raiz->arquivos,sizeof entradas));
            assert(!memcmp(usuarios,s.usuarios,sizeof usuarios));
            falhar(0); sistema_liberar(&s);
        }
    }
}
static void crescimento(void) {
    Sistema s={0}; assert(sistema_inicializar(&s)==SIS_OK);
    Diretorio *raiz=s.raiz;
    for(int i=0;i<300;i++) {
        char nome[32]; snprintf(nome,sizeof nome,"arquivo%d",i);
        assert(sistema_criar_arquivo(&s,nome,TIPO_CARACTERE)==SIS_OK);
        snprintf(nome,sizeof nome,"pasta%d",i);
        assert(sistema_criar_pasta(&s,nome)==SIS_OK);
    }
    assert(raiz==s.raiz && raiz->quantidade_arquivos==300 && raiz->quantidade_subpastas==300);
    for(size_t i=0;i<300;i++) {
        char nome[32]; const Arquivo *a; snprintf(nome,sizeof nome,"arquivo%zu",i);
        assert(sistema_consultar_arquivo(&s,nome,&a)==SIS_OK);
        assert(a->id==i+1 && a->tamanho==0);
        snprintf(nome,sizeof nome,"pasta%zu",i);
        assert(raiz->subpastas[i]->pai==raiz && !strcmp(raiz->subpastas[i]->nome,nome));
    }
    const Arquivo *original; assert(sistema_consultar_arquivo(&s,"arquivo0",&original)==SIS_OK);
    assert(sistema_escrever_arquivo(&s,"arquivo0",(unsigned char *)"conteudo",8)==SIS_OK);
    assert(sistema_copiar_arquivo(&s,"arquivo0","/pasta0","copia")==SIS_OK);
    assert(sistema_mover_arquivo(&s,"arquivo0","/pasta0","movido")==SIS_OK);
    assert(sistema_entrar_pasta(&s,"pasta0")==SIS_OK);
    const Arquivo *movido; assert(sistema_consultar_arquivo(&s,"movido",&movido)==SIS_OK);
    assert(movido==original);
    for(size_t i=s.quantidade_usuarios;i<MAX_USUARIOS;i++) {
        char nome[32]; snprintf(nome,sizeof nome,"usuario%zu",i);
        assert(sistema_criar_usuario(&s,nome,GRUPO_ALUNOS)==SIS_OK);
    }
    assert(s.usuarios[66].id==67);
    assert(sistema_trocar_usuario(&s,"usuario66")==SIS_OK);
    assert(sistema_criar_arquivo(&s,"meu",TIPO_CARACTERE)==SIS_OK);
    const Arquivo *meu; assert(sistema_consultar_arquivo(&s,"meu",&meu)==SIS_OK);
    assert(meu->dono_id==67);
    sistema_liberar(&s); sistema_liberar(&s);
}
int main(void) { falhas(); crescimento(); puts("Crescimento dinamico e falhas de alocacao aprovados."); }
