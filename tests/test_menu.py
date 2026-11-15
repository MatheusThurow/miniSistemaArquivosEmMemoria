import subprocess

def run(lines):
    p = subprocess.run(['./simulador'], input=lines, text=True, capture_output=True)
    assert p.returncode == 0, p.stderr
    assert 'Sistema liberado.' in p.stdout
    return p.stdout

s = run('abc\n99\n' + '9'*2000 + '\n2\n' + 'n'*63 + '\n3\n' + 'n'*63 + '\n5\nf\n4\n6\nf\n' + 'a'*1024 + '\n7\nf\n6\nf\n' + 'b'*1025 + '\n7\nf\n15\n0\n')
assert s.count('a'*1024) == 2
assert 'Entrada longa demais' in s and 'Opcao invalida' in s
s = run('2\na\n3\na\n2\nb\n3\nb\n5\np\n4\n6\np\n' + 'x'*150 + '\n8\np\n/\nc\n9\np\n/a\nmovido\n4\n4\n12\nc\n100\n14\nc\n13\nbruno\n7\nc\n13\nana\n10\nc\n15\n3\na\n15\n0\n')
assert '22 bytes uteis' in s and '64 bytes uteis' in s
assert 'Permissao negada' in s and 'Execucao simulada' in s
for lines in ['', '5\n', '5\nf\n', '2\na\n3\na\n5\nf\n2\n6\nf\nabc', '2\n'+'x'*64+'\n2\nok\n1\n0\n']:
    run(lines)
print('Testes de menu, sessao integrada, limites e EOF aprovados.')

s = run('16\ndiego\n1\n13\ndiego\n5\nmeu\n2\n11\nmeu\n16\neva\n2\n13\neva\n6\nmeu\nnegado\n16\ndiego\n1\n16\ninvalido\n3\n0\n')
assert 'diego/alunos' in s and 'diego (alunos)' in s and 'eva (visitantes)' in s
assert 'Permissao negada' in s and 'Nome duplicado' in s and 'Argumento invalido' in s
run('16\nnovo\n')
print('Menu de cadastro e troca de novos usuarios aprovado.')

entradas = ''.join(f'2\npasta{i}\n5\narquivo{i}\n2\n' for i in range(40))
s = run(entradas + '1\n')
assert '[pasta] pasta39' in s and 'arquivo39 | ID=40' in s
print('Menu com mais de 32 arquivos/subpastas e EOF aprovado.')


s = run('2\na\n3\n/a\n3\na\n2\nb\n3\na/b\n3\nb\n4\n4\n4\n0\n')
assert 'Nome invalido' in s and 'Pasta atual: /a/b' in s
assert 'Sem alteracao' in s and 'Nome da subpasta:' in s
assert 'nome ou absoluto' not in s
print('Navegacao apenas por nome, recusa de caminhos e retorno ao pai aprovados.')
