#include "sistema.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void) {
    Sistema s={0}; assert(sistema_inicializar(&s)==SIS_OK);
    char nome[64], caminho[TAM_CAMINHO], barra[TAM_CAMINHO]; memset(nome,'n',63); nome[63]=0;
    for(int i=0;i<16;i++) { assert(sistema_criar_pasta(&s,nome)==SIS_OK); assert(sistema_entrar_pasta(&s,nome)==SIS_OK); }
    assert(sistema_caminho_atual(&s,caminho,sizeof caminho)==SIS_OK); assert(strlen(caminho)==1024);
    strcpy(barra,caminho); strcat(barra,"/");
    const Diretorio *d=NULL; assert(diretorio_resolver(&s,barra,&d)==SIS_OK); assert(d==s.atual);
    char pequeno[2]="!"; assert(sistema_caminho_atual(&s,pequeno,sizeof pequeno)==SIS_BUFFER_INSUFICIENTE); assert(!strcmp(pequeno,"!"));
    assert(sistema_criar_pasta(&s,"excesso")==SIS_PROFUNDIDADE_MAXIMA);
    assert(sistema_criar_arquivo(&s,"f",TIPO_CARACTERE)==SIS_OK); assert(sistema_escrever_arquivo(&s,"f",(unsigned char *)"abc",3)==SIS_OK);
    const Arquivo *a; assert(sistema_consultar_arquivo(&s,"f",&a)==SIS_OK); Arquivo antes=*a; Disco disco=s.disco;
    size_t l=123; unsigned char b[2]={42,43};
    assert(sistema_ler_arquivo(&s,"f",b,sizeof b,&l)==SIS_BUFFER_INSUFICIENTE); assert(l==123&&b[0]==42&&b[1]==43);
    assert(sistema_mover_arquivo(&s,"f","/ausente","novo")==SIS_NAO_ENCONTRADO);
    assert(sistema_copiar_arquivo(&s,"f","/ausente","novo")==SIS_NAO_ENCONTRADO);
    assert(sistema_escrever_arquivo(&s,"f",(unsigned char *)"a\nb",3)==SIS_ARGUMENTO_INVALIDO);
    assert(!memcmp(&antes,a,sizeof antes)&&!memcmp(&disco,&s.disco,sizeof disco));
    while(s.atual->pai) { assert(sistema_voltar_pai(&s)==SIS_OK); }
    assert(sistema_criar_arquivo(&s,"f",TIPO_CARACTERE)==SIS_OK);
    assert(sistema_criar_arquivo(&s,"F",TIPO_CARACTERE)==SIS_OK);
    sistema_liberar(&s); puts("Caminho maximo, buffers e falhas adicionais aprovados.");
}
