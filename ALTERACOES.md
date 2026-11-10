# Registro das alteracoes do compilador

As linhas indicadas referem-se aos arquivos no estado atual. Em “linhas novas”,
as faixas incluem tanto codigo acrescentado quanto linhas que substituem
implementacoes anteriores.

## Resumo por arquivo

| Arquivo | Linhas/areas modificadas | Motivo |
|---|---|---|
| `gen.c` | 17, 55, 68-69, 78-120, 170-179, 203, 277, 284, 316-318, 416 | Corrigir instrucoes NASM, gerar acesso/atribuicao, chamadas e retorno; corrigir labels booleanos e dados globais. |
| `gen.h` | 26-30 | Declarar as novas rotinas de geracao de codigo. |
| `symbols.c` | 52-63, 151-173, 255-288 | Corrigir ponteiro da TS; armazenar label/implementacao e exibir a TSF legivelmente. |
| `symbols.h` | 60-68 | Adicionar label e estado de implementacao à entrada de funcao. |
| `synt.c` | 24, 48-311, 319-900, 909-1097, 1113-1161 | Propagar erros, separar declaracoes, analisar chamadas/expressoes e implementar corpos de funcao; controlar criacao e falha do `.asm`. |
| `synt.h` | 46-49 | Declarar as funcoes de declaracao e implementacao de funcao. |
| `run_tests.sh` | 1-155 | Automatizar os casos de sucesso, falha e verificacao de assembly solicitados. |
| `lex.c` | Sem alteracoes | A analise lexica existente ja reconhece os tokens necessarios. |

## Linhas novas ou substituidas, por arquivo

### `gen.c`

**Linhas:** 17, 55, 68-69, 81-120, 170, 173, 176, 179, 203, 284, 318, 416.

- Buffer de caminho ampliado; multiplicacao e divisao emitidas com instrucoes
  validas (`imul`, `cqo`, `idiv rbx`).
- Adicionados `gen_id`, `gen_string_value`, `gen_assign`, `gen_call` e `gen_ret`
  para carregar variaveis, atribuir valores, chamar funcoes e retornar.
- Numerais float usam representacao NASM; variaveis globais passam a reservar
  dados coerentes com seus tipos.
- Label booleano recebe prefixo inicializado e a comparacao termina em newline.
- Escrita de string usa o valor armazenado na variavel.

### `gen.h`

**Linhas novas:** 26-30. Prototipos para as rotinas de leitura, atribuicao,
chamada e retorno adicionadas ao gerador.

### `symbols.c`

**Linhas:** 52-63, 151-173, 255-288.

- `sym_declare` retorna a entrada inserida, em vez de uma posicao alem dela.
- `sym_func_declare` preenche `func_<nome>` e inicializa `implemented` em zero;
  a mensagem de limite termina em newline.
- `printSTFunc` imprime nomes de tipos, label, parametros em linhas separadas.

**Linhas novas:** 158-160, 173, 258, 265-273, 275, 279-288.

### `symbols.h`

**Linhas novas:** 65-66. Campos `label` e `implemented` na estrutura da TSF.

### `synt.c`

**Linhas novas ou substituidas:**
24, 42, 50, 53-55, 59-60, 62-64, 66-69, 71-78, 90-93, 100, 102-139,
141-142, 144, 146-147, 149-157, 159-160, 162-164, 166-168, 170-171,
173, 175-180, 183-190, 192-204, 207-311, 313, 338, 660-900, 912-915,
949-952, 959, 962-963, 965-968, 972, 975-976, 978-981, 992-997,
1011-1014, 1021, 1024-1025, 1027-1030, 1034, 1037-1038, 1040-1043,
1045-1050, 1077, 1081-1097, 1133-1137, 1139-1143, 1150-1152, 1158, 1161.

- `program` e `declarations` propagam `ERROR`, verificam `ENDTOKEN`, geram a
  seção `.data` uma única vez e executam `func_code` após o epílogo do main.
- `declaration_var` e `declaration_func` separam as regras; impedem colisões
  entre tabelas, validam vírgulas/limites e registram parâmetros como globais.
- `func_code` verifica protótipo, tipo e assinatura, impede implementação
  duplicada e emite label, comandos e `ret`.
- `statement` trata comandos inválidos como erro; implementa atribuição por
  expressão, comparação booleana e chamada com quantidade/tipos validados,
  argumentos delimitados por vírgula e atribuição aos parâmetros globais.
- `B`, `E`, `ER`, `T`, `TR` e `F` propagam erros e aceitam variáveis em
  expressões; `F` rejeita identificadores ausentes da TS.
- `main` usa `snprintf`, verifica abertura do arquivo e, em falha, fecha e
  remove o `.asm`; sucesso retorna `EXIT_SUCCESS`.

### `synt.h`

**Linhas novas:** 46-47 e 49. Prototipos de `declaration_var`,
`declaration_func` e `func_code`.

### `run_tests.sh`

**Linhas novas:** 1-155. Script gera fontes em `/tmp`, verifica protótipo,
chamada e implementação, rejeita os casos inválidos, aplica timeout ao caso
sem `)` e testa atribuição aritmética e `if`. Se `nasm` estiver disponível,
tenta montar o assembly de sucesso.

## Validacao registrada

- Compilacao: `gcc -Wall -o compiler lex.c synt.c gen.c symbols.c`.
- Script: 15 casos aprovados (`OK`); `nasm` nao estava instalado no ambiente.
