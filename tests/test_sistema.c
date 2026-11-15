#include "sistema.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define OK(x) assert((x) == SIS_OK)
static size_t vistos[TOTAL_BLOCOS];
static void arvore(const Diretorio *d) {
    for(size_t i=0;i<d->quantidade_arquivos;i++) {
        const Arquivo *a=d->arquivos[i];
        assert(a->quantidade_blocos==disco_blocos_necessarios(a->tamanho));
        for(size_t j=0;j<a->quantidade_blocos;j++) { assert(a->blocos[j]<TOTAL_BLOCOS); assert(!vistos[a->blocos[j]]++); }
        for(size_t j=i+1;j<d->quantidade_arquivos;j++) assert(a!=d->arquivos[j]);
    }
    for(size_t i=d->quantidade_arquivos;i<d->capacidade_arquivos;i++) assert(!d->arquivos[i]);
    for(size_t i=0;i<d->quantidade_subpastas;i++) { assert(d->subpastas[i]->pai==d); arvore(d->subpastas[i]); }
}
static void invariantes(Sistema *s) {
    memset(vistos,0,sizeof vistos); arvore(s->raiz); size_t n=0;
    for(size_t i=0;i<TOTAL_BLOCOS;i++) { assert(!!vistos[i]==s->disco.ocupado[i]); n+=!!vistos[i]; }
    assert(n+disco_blocos_livres(&s->disco)==TOTAL_BLOCOS);
}
static Arquivo *arquivo(Sistema *s,const char *nome) { const Arquivo *a; OK(sistema_consultar_arquivo(s,nome,&a)); return (Arquivo *)a; }
static void igual(Sistema *s,const char *nome, const unsigned char *c,size_t n) {
    unsigned char b[MAX_CONTEUDO]; size_t l=9999; OK(sistema_ler_arquivo(s,nome,b,sizeof b,&l)); assert(l==n && !memcmp(b,c,n)); invariantes(s);
}
static void basicos(void) {
    Sistema s={0}; OK(sistema_inicializar(&s)); assert(s.usuario_atual==0 && s.proximo_id==1);
    OK(sistema_criar_pasta(&s,"a")); OK(sistema_entrar_pasta(&s,"a")); OK(sistema_criar_pasta(&s,"b")); OK(sistema_entrar_pasta(&s,"b"));
    char path[TAM_CAMINHO]; OK(sistema_caminho_atual(&s,path,sizeof path)); assert(!strcmp(path,"/a/b"));
    const char *invalidos[]={"", "   ", ".", "..", "a/b", "x\t"};
    for(size_t i=0;i<sizeof invalidos/sizeof *invalidos;i++) assert(nome_validar(invalidos[i])==SIS_NOME_INVALIDO);
    const char *caminhos[]={"/", "/a/b", "b/", "//", "/a//b", "/a/./b", "/a/../b", "a/b", "/nao"};
    Diretorio *antes=s.atual;
    for(size_t i=0;i<sizeof caminhos/sizeof *caminhos;i++) { assert(sistema_entrar_pasta(&s,caminhos[i])!=SIS_OK); assert(s.atual==antes); }
    char nome[65]; memset(nome,'n',63); nome[63]=0; OK(sistema_criar_arquivo(&s,nome,TIPO_NUMERICO)); nome[63]='n'; nome[64]=0; assert(sistema_criar_pasta(&s,nome)==SIS_NOME_INVALIDO);
    OK(sistema_criar_arquivo(&s,"com espaco",TIPO_CARACTERE)); assert(sistema_criar_pasta(&s,"com espaco")==SIS_NOME_DUPLICADO);
    assert(sistema_entrar_pasta(&s,"com espaco")==SIS_TIPO_INCORRETO);
    unsigned char dados[1025]; memset(dados,'z',sizeof dados); size_t tamanhos[]={0,1,63,64,65,1023,1024,150,1,0};
    for(size_t i=0;i<sizeof tamanhos/sizeof *tamanhos;i++) { OK(sistema_escrever_arquivo(&s,"com espaco",dados,tamanhos[i])); igual(&s,"com espaco",dados,tamanhos[i]); }
    Arquivo old=*arquivo(&s,"com espaco"); Disco disco=s.disco;
    assert(sistema_escrever_arquivo(&s,"com espaco",dados,1025)==SIS_ARQUIVO_GRANDE); assert(!memcmp(&old,arquivo(&s,"com espaco"),sizeof old)); assert(!memcmp(&disco,&s.disco,sizeof disco));
    OK(sistema_escrever_arquivo(&s,"com espaco",dados,150));
    OK(sistema_copiar_arquivo(&s,"com espaco","/a","copia")); Arquivo *original=arquivo(&s,"com espaco");
    OK(sistema_voltar_pai(&s)); Arquivo *copia=arquivo(&s,"copia"); assert(original!=copia && original->id!=copia->id); assert(original->blocos[0]!=copia->blocos[0]);
    OK(sistema_escrever_arquivo(&s,"copia",dados,1)); OK(sistema_excluir_arquivo(&s,"copia"));
    OK(sistema_entrar_pasta(&s,"b")); igual(&s,"com espaco",dados,150); old=*original;
    OK(sistema_mover_arquivo(&s,"com espaco","/a","novo")); assert(!strcmp(original->nome,"novo")); strcpy(old.nome,"novo"); assert(!memcmp(&old,original,sizeof old));
    OK(sistema_voltar_pai(&s)); assert(arquivo(&s,"novo")==original); assert(sistema_mover_arquivo(&s,"novo","/a","novo")==SIS_SEM_ALTERACAO);
    assert(sistema_copiar_arquivo(&s,"novo","/a","novo")==SIS_NOME_DUPLICADO);
    while(s.atual->pai) { OK(sistema_voltar_pai(&s)); } assert(sistema_voltar_pai(&s)==SIS_SEM_ALTERACAO);
    for(int t=1;t<=4;t++) { char n[20]; snprintf(n,sizeof n,"tipo%d",t); OK(sistema_criar_arquivo(&s,n,(TipoArquivo)t)); }
    s.proximo_id=UINT64_MAX; OK(sistema_criar_arquivo(&s,"ultimo",TIPO_BINARIO)); assert(arquivo(&s,"ultimo")->id==UINT64_MAX); assert(sistema_criar_arquivo(&s,"esgotado",TIPO_BINARIO)==SIS_ID_ESGOTADO);
    invariantes(&s); sistema_liberar(&s); sistema_liberar(&s); Sistema zero={0}; assert(!memcmp(&s,&zero,sizeof s));
}
static void permissoes(void) {
    Sistema s={0}; OK(sistema_inicializar(&s)); OK(sistema_criar_arquivo(&s,"p",TIPO_PROGRAMA));
    const char *mascaras[]={"640","000","004","020","100","777"};
    const char *usuarios[]={"ana","bruno","carla"};
    for(size_t m=0;m<6;m++) {
        OK(sistema_trocar_usuario(&s,"ana")); OK(sistema_alterar_permissoes(&s,"p",mascaras[m]));
        for(size_t u=0;u<3;u++) {
            OK(sistema_trocar_usuario(&s,usuarios[u])); unsigned bits=(unsigned)(mascaras[m][u]-'0');
            Arquivo antes=*arquivo(&s,"p"); Disco d=s.disco; unsigned char b[1024]; size_t n;
            Resultado r=sistema_ler_arquivo(&s,"p",b,sizeof b,&n); assert(r==((bits&4)?SIS_OK:SIS_PERMISSAO_NEGADA));
            if(!(bits&4)) assert(!memcmp(&antes,arquivo(&s,"p"),sizeof antes));
            antes=*arquivo(&s,"p"); r=sistema_escrever_arquivo(&s,"p",(unsigned char *)"oi",2); assert(r==((bits&2)?SIS_OK:SIS_PERMISSAO_NEGADA));
            if(!(bits&2)) { assert(!memcmp(&antes,arquivo(&s,"p"),sizeof antes)); assert(!memcmp(&d,&s.disco,sizeof d)); }
            r=sistema_executar_arquivo(&s,"p"); assert(r==((bits&1)?SIS_OK:SIS_PERMISSAO_NEGADA));
            r=sistema_copiar_arquivo(&s,"p","/","c"); assert(r==((bits&4)?SIS_OK:SIS_PERMISSAO_NEGADA));
            if(r==SIS_OK) OK(sistema_excluir_arquivo(&s,"c"));
            if(u) {
                antes=*arquivo(&s,"p");
                assert(sistema_alterar_permissoes(&s,"p","777")==SIS_PERMISSAO_NEGADA);
                assert(sistema_mover_arquivo(&s,"p","/","outro")==SIS_PERMISSAO_NEGADA);
                assert(sistema_excluir_arquivo(&s,"p")==SIS_PERMISSAO_NEGADA);
                assert(!memcmp(&antes,arquivo(&s,"p"),sizeof antes));
            }
            invariantes(&s);
        }
    }
    OK(sistema_trocar_usuario(&s,"ana")); const char *ruins[]={"888","64","0644","abc"};
    for(size_t i=0;i<4;i++) { Arquivo a=*arquivo(&s,"p"); assert(sistema_alterar_permissoes(&s,"p",ruins[i])==SIS_ARGUMENTO_INVALIDO); assert(!memcmp(&a,arquivo(&s,"p"),sizeof a)); }
    OK(sistema_criar_arquivo(&s,"texto",TIPO_CARACTERE)); OK(sistema_alterar_permissoes(&s,"texto","777")); assert(sistema_executar_arquivo(&s,"texto")==SIS_TIPO_INCORRETO);
    char oct[4],sim[10]; OK(permissao_formatar(0754,oct,4,sim,10)); assert(!strcmp(oct,"754")&&!strcmp(sim,"rwxr-xr--"));
    sistema_liberar(&s);
}
static void capacidade(void) {
    Sistema s={0}; unsigned char dados[1024]; memset(dados,'a',sizeof dados); OK(sistema_inicializar(&s));
    char nome[20]; for(int i=0;i<8;i++) { snprintf(nome,sizeof nome,"f%d",i); OK(sistema_criar_arquivo(&s,nome,TIPO_BINARIO)); OK(sistema_escrever_arquivo(&s,nome,dados,sizeof dados)); }
    assert(!disco_blocos_livres(&s.disco)); OK(sistema_criar_arquivo(&s,"vazio",TIPO_CARACTERE));
    Disco d=s.disco; Arquivo a=*arquivo(&s,"f0"); uint64_t id=s.proximo_id;
    assert(sistema_escrever_arquivo(&s,"vazio",dados,1)==SIS_DISCO_CHEIO);
    assert(sistema_copiar_arquivo(&s,"f0","/","copia")==SIS_DISCO_CHEIO);
    assert(s.proximo_id==id&&!memcmp(&d,&s.disco,sizeof d)&&!memcmp(&a,arquivo(&s,"f0"),sizeof a));
    OK(sistema_escrever_arquivo(&s,"f0",dados,1024)); OK(sistema_escrever_arquivo(&s,"f0",dados,64)); assert(disco_blocos_livres(&s.disco)==15);
    OK(sistema_excluir_arquivo(&s,"f0")); OK(sistema_escrever_arquivo(&s,"vazio",dados,1024)); assert(arquivo(&s,"vazio")->blocos[0]==0);
    invariantes(&s); sistema_liberar(&s); OK(sistema_inicializar(&s));
    for(int i=0;i<32;i++) { snprintf(nome,sizeof nome,"d%d",i); OK(sistema_criar_pasta(&s,nome)); snprintf(nome,sizeof nome,"f%d",i); OK(sistema_criar_arquivo(&s,nome,TIPO_NUMERICO)); }
    OK(sistema_criar_pasta(&s,"extra")); OK(sistema_criar_arquivo(&s,"arquivo_extra",TIPO_NUMERICO));
    OK(sistema_mover_arquivo(&s,"f0","/","renomeado")); OK(sistema_entrar_pasta(&s,"d0")); OK(sistema_criar_arquivo(&s,"origem",TIPO_CARACTERE)); Arquivo *origem=arquivo(&s,"origem");
    OK(sistema_mover_arquivo(&s,"origem","/","novo")); while(s.atual->pai) { OK(sistema_voltar_pai(&s)); } assert(arquivo(&s,"novo")==origem); OK(sistema_entrar_pasta(&s,"d0"));
    for(int i=1;i<16;i++) { OK(sistema_criar_pasta(&s,"nivel")); OK(sistema_entrar_pasta(&s,"nivel")); }
    assert(sistema_criar_pasta(&s,"excesso")==SIS_PROFUNDIDADE_MAXIMA); invariantes(&s); sistema_liberar(&s);
}
int main(void) { basicos(); permissoes(); capacidade(); puts("Todos os testes de API aprovados."); }

