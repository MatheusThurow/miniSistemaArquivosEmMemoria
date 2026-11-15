#include "sistema.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static int falhar_memoria, falhar_relogio;
void *__real_calloc(size_t,size_t);
void *__wrap_calloc(size_t n,size_t t) { return falhar_memoria ? NULL : __real_calloc(n,t); }
time_t __wrap_time(time_t *p) { time_t t=falhar_relogio ? (time_t)-1 : 1000; if(p)*p=t; return t; }
int main(void) {
    Sistema s={0}, zero={0}; falhar_memoria=1;
    assert(sistema_inicializar(&s)==SIS_SEM_MEMORIA); assert(!memcmp(&s,&zero,sizeof s));
    falhar_memoria=0; assert(sistema_inicializar(&s)==SIS_OK);
    falhar_memoria=1; assert(sistema_criar_pasta(&s,"a")==SIS_SEM_MEMORIA); assert(sistema_criar_arquivo(&s,"f",TIPO_PROGRAMA)==SIS_SEM_MEMORIA);
    falhar_memoria=0; assert(sistema_criar_arquivo(&s,"f",TIPO_PROGRAMA)==SIS_OK);
    assert(sistema_alterar_permissoes(&s,"f","777")==SIS_OK);
    Sistema antes=s; Arquivo f=*s.atual->arquivos[0];
    falhar_memoria=1; assert(sistema_copiar_arquivo(&s,"f","/","c")==SIS_SEM_MEMORIA); falhar_memoria=0;
    falhar_relogio=1; unsigned char b[10]; size_t l=999;
    assert(sistema_criar_arquivo(&s,"c",TIPO_BINARIO)==SIS_ERRO_RELOGIO);
    assert(sistema_escrever_arquivo(&s,"f",(unsigned char *)"oi",2)==SIS_ERRO_RELOGIO);
    assert(sistema_ler_arquivo(&s,"f",b,sizeof b,&l)==SIS_ERRO_RELOGIO); assert(l==999);
    assert(sistema_executar_arquivo(&s,"f")==SIS_ERRO_RELOGIO);
    assert(sistema_copiar_arquivo(&s,"f","/","c")==SIS_ERRO_RELOGIO);
    assert(!memcmp(&s,&antes,sizeof s)); assert(!memcmp(s.atual->arquivos[0],&f,sizeof f));
    sistema_liberar(&s); puts("Falhas injetadas de alocacao e relogio aprovadas.");
}
