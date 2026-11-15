#include "sistema.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    Sistema s={0}; assert(sistema_inicializar(&s)==SIS_OK);
    assert(s.quantidade_usuarios==3);
    assert(sistema_criar_usuario(&s,"diego",GRUPO_ALUNOS)==SIS_OK);
    assert(s.usuario_atual==0 && s.usuarios[3].id==4);
    assert(sistema_criar_arquivo(&s,"ana",TIPO_CARACTERE)==SIS_OK);
    assert(sistema_alterar_permissoes(&s,"ana","640")==SIS_OK);
    assert(sistema_trocar_usuario(&s,"diego")==SIS_OK);
    unsigned char b[10]; size_t n;
    assert(sistema_ler_arquivo(&s,"ana",b,sizeof b,&n)==SIS_OK);
    assert(sistema_escrever_arquivo(&s,"ana",NULL,0)==SIS_PERMISSAO_NEGADA);
    assert(sistema_criar_arquivo(&s,"novo",TIPO_CARACTERE)==SIS_OK);
    const Arquivo *a; assert(sistema_consultar_arquivo(&s,"novo",&a)==SIS_OK);
    assert(a->dono_id==4 && a->grupo==GRUPO_ALUNOS);
    assert(sistema_criar_usuario(&s,"eva",GRUPO_VISITANTES)==SIS_OK);
    assert(sistema_trocar_usuario(&s,"eva")==SIS_OK);
    assert(sistema_ler_arquivo(&s,"ana",b,sizeof b,&n)==SIS_PERMISSAO_NEGADA);
    Sistema antes=s;
    assert(sistema_criar_usuario(&s,"diego",GRUPO_ALUNOS)==SIS_NOME_DUPLICADO);
    assert(sistema_criar_usuario(&s,"   ",GRUPO_ALUNOS)==SIS_NOME_INVALIDO);
    assert(sistema_criar_usuario(&s,"x",(Grupo)3)==SIS_ARGUMENTO_INVALIDO);
    assert(sistema_criar_usuario(&s,NULL,GRUPO_ALUNOS)==SIS_ARGUMENTO_INVALIDO);
    assert(!memcmp(&s,&antes,sizeof s));
    char nome[65]; memset(nome,'a',63); nome[63]=0;
    assert(sistema_criar_usuario(&s,nome,GRUPO_ALUNOS)==SIS_OK);
    nome[63]='a'; nome[64]=0;
    assert(sistema_criar_usuario(&s,nome,GRUPO_ALUNOS)==SIS_NOME_INVALIDO);
    while(s.quantidade_usuarios<MAX_USUARIOS) {
        snprintf(nome,sizeof nome,"u%zu",s.quantidade_usuarios);
        assert(sistema_criar_usuario(&s,nome,GRUPO_ALUNOS)==SIS_OK);
    }
    antes=s; assert(sistema_criar_usuario(&s,"extra",GRUPO_ALUNOS)==SIS_USUARIOS_CHEIO);
    assert(!memcmp(&s,&antes,sizeof s));
    sistema_liberar(&s); puts("Cadastro e permissoes de novos usuarios aprovados.");
}
