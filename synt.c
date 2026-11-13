/**
 * @file synt.c
 * @author Prof. Ivairton M. Santos - UFMT - Computacao
 * @brief Codificacao do modulo do analisador sintatico
 * @version 0.4
 * @date 2022-02-04
 * @note last modified 12.11.2026 by João Paulo
 */

// Inclusao do cabecalho
#include "synt.h"
#include "gen.h"
#include "struct_compiler.h"
#include "symbols.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variaveis globais
type_token *lookahead;
extern type_symbol_table_variables global_symbol_table_variables;
extern type_symbol_table_string symbol_table_string;
extern char output_file_name[MAX_CHAR];
extern FILE *output_file;

/**
 * @brief Verifica se o proximo caracter (a frente) na cadeia eh o esperado
 *
 * @param token_tag (int) codigo do token a ser verificado
 * @return int true/false
 */
int match(int token_tag)
{
    if (lookahead->tag == token_tag)
    {
        lookahead = getToken(); // Pega o proximo token por meio do lexico
        return true;
    }
    printf("[ERRO] Entrada processada: '%s' - tag esperada: '%d'\n",
           lookahead->lexema, token_tag);
    return false;
}

/**
 * @brief Regra de derivacao inicial
 */
int program(void)
{
    gen_preambule(); // Temporariamente cria um preambulo adicional que permite o
                     // uso das funcoes scanf e printf
    declarations();
    gen_preambule_code(); // Chamada do gerador de codigo para escrita do
                          // cabecalho da secao de codigo
    // #BEGIN implementar nao sei comoo
    match(BEGIN); // suponho que seja assim

    statements();
    // #END implementar não sei como
    match(END);
    // #codigo das funções implementar tambem

    gen_epilog_code();
    while(func_code());

    gen_data_section(); // Chamada do gerador de codigo para declaracao de
                        // variaveis
    return true;
}

int func_code(void){
    int func_type, n_param = 0, type_param;
    char func_name[MAX_CHAR];
    type_symbol_table_func_entry *search_func_symbol;

    func_type = lookahead->tag;
    if (func_type == INT || func_type == FLOAT || func_type == CHAR || func_type == STRING){
        match(func_type);
        if(lookahead->tag == ID){
            strcpy(func_name, lookahead->lexema); 
            search_func_symbol = sym_func_find(func_name);
            if (search_func_symbol != NULL){
                match(ID);
                if(lookahead->tag == OPEN_PAR){
                    match(OPEN_PAR);
                    while(lookahead->tag != CLOSE_PAR){
                        type_param = lookahead->tag;
                        if (type_param != INT && type_param != FLOAT && type_param != CHAR && type_param != STRING){
                                printf("[ERRO] TIPO DE VARIAVEL NA TS DECLARADA DESCONHECIDA.\n");
                                return false;
                        }else{

                            match(type_param);
                            if(lookahead->tag == ID){
                                type_symbol_table_entry *search_symbol = sym_find(lookahead->lexema, &global_symbol_table_variables);
                                if (search_symbol != NULL){
                                    if(search_symbol->type == type_param){
                                        match(ID);
                                        if(lookahead->tag == COMMA){
                                            match(COMMA);         
                                        }else if (lookahead->tag != CLOSE_PAR){     
                                            printf("[ERRO] lexema processado: '%s', tag processada '%d'\n", lookahead->lexema, lookahead->tag);
                                            return false;   
                                        }
                                    }else{
                                        printf("[ERRO] Tipo de parametro não coincide com o prototipo.");
                                        return false;
                                    }
                                }else{
                                    printf("[ERRO] Variavel '%s' do parametro da funçao '%s' não existe! Não esqueça de declara-la!\n",lookahead->lexema, func_name);
                                    return false;
                                }
                            }else{
                                printf("[ERRO] Esperado um ID para o tipo de parametro '%d' na funçao '%s' ",type_param, func_name);
                                return false;
                            }
                        }
                        n_param++;
                        if(n_param > search_func_symbol->n_params){
                            printf("[ERRO] Quantidade de parametros não coincide. Esperado '%d', processado'%d'\n",search_func_symbol->n_params, n_param);
                            return false;
                        }    
                    }
                    if(n_param != search_func_symbol->n_params){
                        printf("[ERRO] Quantidade de parametros não coincide. Esperado '%d', processado'%d'\n",search_func_symbol->n_params, n_param);
                        return false;
                    }
                    match(CLOSE_PAR);
                    if(lookahead->tag == BEGIN){
                        match(BEGIN);
                        gen_label(search_func_symbol->label);
                        //ALGO QUE REPRESENTE O CODIGO ACHO QUE STATEMENTS
                        statements();

                        match(END);

                        gen_jr_ra();
                        return true;
                    }
                }else{
                    printf("[ERRO] Esperado abre chaves \"(\"\n");
                    return false;
                }
            }else{
                printf("[ERRO] Declaraçâo de funçao nao prototipada\n");
                return false;
            }

        }else{
            printf("[ERRO] Esperado o nome da função");
            return false;
        }
    }else if (lookahead->tag == ENDTOKEN) { 
        return false; 
    }else{
        printf("[ERRO] Tipo da função não existente\n");
        return false;
    }
}

/**
 * @brief Regra de derivacao para declaracoes
 */
int declarations(void)
{
    while (declaration())
        ; // Laco para processamento continuo das declaracoes
    return false;
}

/**
 * @brief Regra de derivacao declaracao
 * @return int true/false
 */
int declaration(void)
{
    type_symbol_table_entry *search_symbol;
    type_symbol_table_func_entry *search_func_symbol;
    int ok1, ok2;
    char var_name[MAX_CHAR];
    int var_type;

    // Verifica o tipo da variavel
    var_type = lookahead->tag;
    if (var_type == INT || var_type == FLOAT || var_type == CHAR || var_type == STRING)
    {
        match(var_type);
        strcpy(var_name, lookahead->lexema);
        ok1 = match(ID); // Verifica ID

        // Declaracao de VARIAVEL
        if (lookahead->tag == SEMICOLON)
        {                                                                       // Verifica se eh declaracao de variavel
            search_symbol = sym_find(var_name, &global_symbol_table_variables); // Busca na TS de variaveis
            if (search_symbol != NULL)
            {
                printf("[ERRO] Variavel '%s' ja declarada.\n", var_name);
                return false;
            }
            else
            {
                sym_declare(var_name, var_type, 0, &global_symbol_table_variables);
                ok2 = match(SEMICOLON); // Verifica se ; vem a seguir
                return ok1 && ok2;
            }
        } // Declaracao de FUNCAO
        else if (lookahead->tag == OPEN_PAR)
        { // Verifica se eh declaracao de funcao
            search_func_symbol = sym_func_find(var_name);
            if (search_func_symbol != NULL)
            {
                printf("[ERRO] Funcao '%s' ja declarada.\n", var_name);
                return false;
            }
            else
            {
                match(OPEN_PAR);
                // Constroi lista de parametros
                type_symbol_table_entry parameters[MAX_PARAMS];
                int n_params;
                n_params = 0;
                char p_name[MAX_CHAR];
                int p_type;

                while (lookahead->tag != CLOSE_PAR)
                {
                    p_type = lookahead->tag;
                    if (p_type == INT || p_type == FLOAT || p_type == CHAR || p_type == STRING)
                    {
                        match(p_type);
                        strcpy(p_name, lookahead->lexema);
                        match(ID);
                        parameters[n_params].type = p_type;
                        strcpy(parameters[n_params].name, p_name);
                        sym_declare(parameters[n_params].name, parameters[n_params].type, NULL, &global_symbol_table_variables); // ADICIONAR PARAMETROS NA TS DE VARIAVEISs *stv)
                        n_params++;
                        if (lookahead->tag == COMMA)
                        {
                            match(COMMA);
                            if ((lookahead->tag != INT) && (lookahead->tag != FLOAT) && (lookahead->tag != CHAR) && (lookahead->tag != STRING))
                            {
                                printf("[ERRO] Erro na declaracao de parametro da funcao. Obrigatorio um tipo de dado apos a virgula.\n");
                                return false;
                            }
                        }
                    }
                    else
                    {
                        printf("[ERRO] Declaracao de parametro incorreta. Esperado um tipo de dado.\n");
                        return false;
                    }
                }

                sym_func_declare(var_name, var_type, parameters, n_params);
                ok2 = match(CLOSE_PAR) && match(SEMICOLON);
                return ok1 && ok2;
            }
        }
        else
        { // Caso contrario eh erro
            printf("[ERRO] Era esperado um ';' ou '(' para reconhecer declaracao de variavel, ou de funcao.\n");
            return false;
        }
    }
    else if (lookahead->tag == ENDTOKEN || lookahead->tag == BEGIN || lookahead->tag == END)
    { // o begin esta aqui pois depois que acaba as declarações o lookahead.tag é begin, então para não dar erro colocamos ele aqiu
        // Verifica se fim de arquivo
        return false;
    }
    else
    {
        printf("[ERRO] Tipo desconhecido: %d %s.\n", lookahead->tag, lookahead->lexema);
        return false;
    }
}

/**
 * @brief Regra de derivacao para comandos
 */
int statements(void)
{
    // while (statement()); // processa enquanto houver comandos
    int sucess;
    do
    {
        sucess = statement();
    } while (sucess == true);
    if (sucess == ERROR)
        return ERROR;
    else
        return true;
}

/**
 * @brief Regra de derivacao que processa os comandos
 *
 * @return int true/false
 */
int statement(void)
{
    char lexeme_of_id[MAX_CHAR];
    type_symbol_table_entry *search_symbol;
    type_symbol_table_string_entry *gen_string;
    int ok1, ok2, type;
    char string_value[MAX_CHAR];

    if (lookahead->tag == READ)
    {
        match(READ);
        strcpy(lexeme_of_id, lookahead->lexema);
        ok1 = match(ID);
        search_symbol = sym_find(lexeme_of_id, &global_symbol_table_variables);
        if (search_symbol != NULL)
        {
            type = search_symbol->type;
            gen_read(lexeme_of_id, type);
            ok2 = match(SEMICOLON);
            return ok1 && ok2;
        }
        else
        {
            printf("[ERRO] Simbolo desconhecido (Variavel nao declarada): %s\n", lexeme_of_id);
            return false;
        }
    }
    else if (lookahead->tag == WRITE)
    {
        match(WRITE);
        if (lookahead->tag == STRING)
        {
            strcpy(string_value, lookahead->lexema);
            // TODO: verificar se o valor da constante ja foi incluida na tabela de simbolos
            gen_string = sym_string_declare(string_value);
            match(STRING);
            if (gen_string != NULL)
            {
                strcpy(lexeme_of_id, gen_string->name);
                gen_write(lexeme_of_id, STRING);
                match(SEMICOLON);
                return true;
            }
        }
        else if (lookahead->tag == ID)
        { // Verifica se uso de funcao ou variavel
            strcpy(lexeme_of_id, lookahead->lexema);
            match(ID);
            search_symbol = sym_find(lexeme_of_id, &global_symbol_table_variables);
            if (search_symbol != NULL)
            {
                type = search_symbol->type;
                gen_write(lexeme_of_id, type);
                match(SEMICOLON);
                return true;
            }
            else
            {
                printf("[ERRO] Simbolo desconhecido (Variavel nao declarada): %s\n", lexeme_of_id);
                return false;
            }
        }
    }
    else if (lookahead->tag == IF)
    {
        char label_else[MAX_CHAR];
        char label_end[MAX_CHAR];
        gen_label_name(label_else);
        gen_label_name(label_end);

        match(IF);
        match(OPEN_PAR);
        B(); // Expressao booleana
        gen_cond_jump(label_else);
        match(CLOSE_PAR);

        match(BEGIN);
        statements();
        match(END);

        gen_incond_jump(label_end);
        gen_label(label_else);

        // Verifica se ocorre um ELSE
        if (lookahead->tag == ELSE)
        {
            match(ELSE);
            match(BEGIN);
            statements();
            match(END);
        }
        gen_label(label_end);
        return true;
    }
    else if (lookahead->tag == WHILE)
    {
        char label_begin[MAX_CHAR];
        char label_end[MAX_CHAR];
        gen_label_name(label_begin);
        gen_label_name(label_end);

        match(WHILE);
        match(OPEN_PAR);
        gen_label(label_begin);
        B(); // Expressao booleana
        gen_cond_jump(label_end);
        match(CLOSE_PAR);
        match(BEGIN);
        statements();
        match(END);
        gen_incond_jump(label_begin);
        gen_label(label_end);
        return true;
        // A partir daqui é minha implementação !
    }

    
    /**
    * @note : esta parte da função reconeche atribuição de valor para variaveis ja inclusas na tabela de simbolos
    */
    else if (lookahead->tag == ID)
    {
        char id_name[MAX_TOKSZ];
        // TODO:
        // Verifica se a variavel ID está na TS
        strcpy(id_name, lookahead->lexema);
        match(ID);
        if (lookahead->tag == ASSIGN)
        {

            search_symbol = sym_find(id_name, &global_symbol_table_variables);
            if (search_symbol != NULL)
            {
                // Se sim, confirma se o proximo token é ASSIGN

                ok1 = match(ASSIGN);
                type = search_symbol->type;
                // Se sim, verifica se o proximo token é INT, FLOAT, CHAR ou STRING
                if (lookahead->tag == NUM_INT)
                {
                    //     Verifica se o tipo do token é compativel com o tipo da variavel na TS
                    if (type == INT)
                    {
                        //     atribui o valor para a variavel na TS
                        search_symbol->value = lookahead->lexema;
                        match(NUM_INT);
                        ok2 = match(SEMICOLON);
                    }
                    else
                    {
                        // Se nao, gera erro e retorna falso
                        printf("[ERRO] TIPO %d NAO COINCIDE COM A VARIAVEL NA TS: Esperado %d\n", lookahead->tag, type);
                        return false;
                    }
                    return ok1 && ok2;
                }
                else if (lookahead->tag == NUM_FLOAT)
                {
                    //     Verifica se o tipo do token é compativel com o tipo da variavel na TS
                    if (type == FLOAT)
                    {
                        //     atribui o valor para a variavel na TS
                        search_symbol->value = lookahead->lexema;
                        match(NUM_FLOAT);
                        ok2 = match(SEMICOLON);
                    }
                    else
                    {
                        // Se nao, gera erro e retorna falso
                        printf("[ERRO] TIPO %d NAO COINCIDE COM A VARIAVEL NA TS: Esperado %d\n", lookahead->tag, type);
                        return false;
                    }
                    return ok1 && ok2;
                }
                else if (lookahead->tag == STRING || lookahead->tag == CHAR)
                {
                    if (lookahead->tag == type)
                    {
                        //     atribui o valor para a variavel na TS
                        strcpy(string_value, lookahead->lexema);
                        gen_string = sym_string_declare(string_value);
                        search_symbol->value = gen_string->value;
                        match(type);
                        ok2 = match(SEMICOLON);
                        // se tudo deu certo, retorna verdadeiro
                    }
                    else
                    {
                        // Se nao, gera erro e retorna falso
                        printf("[ERRO] TIPO %d NAO COINCIDE COM A VARIAVEL NA TS: Esperado %d\n", lookahead->tag, type);
                        return false;
                    }
                    return ok1 && ok2;
                }
                else
                {
                    printf("[ERRO] TIPO DE VARIAVEL NÃO RECONHECIDA PELO COMPILADOR\n");
                    return false;
                }
            }
            else
            {
                printf("[ERRO] VARIAVEL NÃO DECLARADA: %s\n", id_name);
                return false;
            }

            // ESTA SEÇÂO ESTA MOVIDA À GAMBIARRAS!! NADA AQUI DEVE SER MOVIDO UMA LINHA
            // este aqui deve ser o gosto de dev que pega sistema legado
        }
        else if (lookahead->tag == OPEN_PAR)
        {
            type_symbol_table_func_entry *function;
            function = sym_func_find(id_name);
            if (function != NULL)
            {
                match(OPEN_PAR);
                // Monta lista de parametros
                type_token params[MAX_PARAMS];
                int cont_params;
                cont_params = 0;

                while (lookahead->tag != CLOSE_PAR)
                {
                    if (lookahead->tag == ID)
                    {
                        type_symbol_table_entry *search_symbol = sym_find(lookahead->lexema, &global_symbol_table_variables);
                        if (search_symbol != NULL)
                        {
                            if (search_symbol->type != INT && search_symbol->type != FLOAT && search_symbol->type != CHAR && search_symbol->type != STRING)
                            {
                                printf("[ERRO] TIPO DE VARIAVEL NA TS DECLARADA DESCONHECIDA.\n");
                                return false;
                            }
                            params[cont_params].tag = search_symbol->type;
                            if (search_symbol->type == CHAR || search_symbol->type == STRING)
                            {
                                strcpy(params[cont_params].lexema, lookahead->lexema);
                            }
                            else if (search_symbol->type == INT)
                            {
                                if (search_symbol->value != NULL && search_symbol->value != 0)
                                {
                                    params[cont_params].value = atoi(search_symbol->value);
                                }
                                else
                                    params[cont_params].value = 0;
                            }
                            else
                            {
                                if (search_symbol->value != NULL && search_symbol->value != 0)
                                {
                                    params[cont_params].value = atof(search_symbol->value);
                                }
                                else
                                    params[cont_params].value = 0;
                            }
                            cont_params++;
                        }
                        else
                        {
                            printf("[ERRO] VARIAVEL '%s', Não declarada na TS\n", lookahead->lexema);
                            return false;
                        }
                    }
                    else if (lookahead->tag == NUM_INT)
                    {
                        params[cont_params].tag = INT; // para facilitar a comparação depois
                        params[cont_params].value = atoi(lookahead->lexema);
                        cont_params++;
                    }
                    else if (lookahead->tag == NUM_FLOAT)
                    {
                        params[cont_params].tag = FLOAT; // para facilitar a comparação depois
                        params[cont_params].value = atof(lookahead->lexema);
                        cont_params++;
                    }
                    else if (lookahead->tag == CHAR || lookahead->tag == STRING)
                    {
                        params[cont_params].tag = lookahead->tag;
                        sym_string_declare(lookahead->lexema); // declara a string na tabela de strings e gera o codigo
                        strcpy(params[cont_params].lexema, lookahead->lexema);
                        cont_params++;
                    }
                    match(lookahead->tag);
                    if (cont_params > function->n_params)
                    {
                        printf("A função espera %d argumentos mas esta recebendo mais do que o esperado.\n ", function->n_params);
                        return false;
                    }
                    if (params[(cont_params - 1)].tag != function->params[(cont_params - 1)].type)
                    {
                        printf("[ERRO] TIPO DE ARGUMENTO INVALIDO PARA FUNÇÂO DECLARADA. ESPERADO: %d, PROCESSADO: %d\n", function->params[(cont_params - 1)].type, params[(cont_params - 1)].tag);
                        return false;
                    }
                    if (lookahead->tag == COMMA)
                    {
                        match(COMMA);
                    }
                }
                if (cont_params != function->n_params)
                {
                    printf("Quantidade de Parametros/argumentos incorreto. Esperado: %d, Processado: %d\n", function->n_params, cont_params);
                    return false;
                }
                match(CLOSE_PAR);
                match(SEMICOLON);
                gen_jal_label_name(function->label); // chama a geração de codigo jal para a função chamada
                return true;
                // TODO: Verifica se lista de parametros eh compativel com declaracao
            }
            else
            {
                printf("[ERRO] Tentativa de chamada de funcao nao declarada,\n");
                return false;
            }
        }
        //     Gera codigo de atribuicao
    }
    else if ((lookahead->tag == ENDTOKEN) || (lookahead->tag == END))
    {
        return false;
    }
    else
    {
        printf("[ERRO] Comando desconhecido.\nTag=%d; Lexema=%s\n", lookahead->tag,
               lookahead->lexema);
        return false;
    }
}

/**
 * @brief Regra de derivação que analiza expressoes booleanas
 *        no padrao 'id op_rel expr'
 *
 */
////// TODO
int B()
{
    int operator;
    char lexema_of_id[MAX_CHAR];
    strcpy(lexema_of_id, lookahead->lexema);
    match(ID);
    // gen_id(lexema_of_id);
    if (boolOperator(&operator))
    {
        E();
        gen_bool(operator);
        return true;
    }
    else
    {
        return false;
    }
}

int boolOperator(int *operator)
{
    int lookahead_tag;
    lookahead_tag = lookahead->tag;
    *operator = -1;

    if (lookahead_tag == EQUAL ||
        lookahead_tag == NE ||
        lookahead_tag == LT ||
        lookahead_tag == GT ||
        lookahead_tag == LE ||
        lookahead_tag == GE)
    {
        *operator = lookahead_tag;
        match(lookahead_tag);
        return true;
    }
    else
    {
        printf("[ERRO] Operador relacional experado.\n");
        return false;
    }
}

//------
/* Funções que representam a gramatica que reconhece expressoes aritmeticas */
/* Elaborada nas primeiras aulas */
int E()
{
    int b1, b2;
    b1 = T();
    if (b1)
        b2 = ER();
    return b1 && b2;
}

int ER()
{
    if (lookahead->tag == '+')
    {
        int b1, b2;
        match('+');
        b1 = T();
        genAdd();
        if (b1)
            b2 = ER();
        return b1 && b2;
    }
    else if (lookahead->tag == '-')
    {
        int b1, b2;
        match('-');
        b1 = T();
        genSub();
        if (b1)
            b2 = ER();
        return b1 && b2;
        // } else if (lookahead -> tag == ')') {
        //     return true;
        // } else if (lookahead -> tag == ENDTOKEN) {
        //     return true;
        // } else if (lookahead -> tag == '*') {
        //     return true;
        // } else if (lookahead -> tag == '/') {
        //     return true;
        // OU PODEMOS FAZER EM UM UNICO ELSE IF.
    }
    else if (lookahead->tag == ')' || lookahead->tag == '*' || lookahead->tag == '/' || lookahead->tag == ENDTOKEN)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int T()
{
    int b1, b2;
    b1 = F();
    if (b1)
        b2 = TR();
    return b1 && b2;
}

int TR()
{
    if (lookahead->tag == '*')
    {
        int b1, b2;
        match('*');
        b1 = F();
        genMult();
        if (b1)
            b2 = TR();
        return b1 && b2;
    }
    else if (lookahead->tag == '/')
    {
        int b1, b2;
        match('/');
        b1 = F();
        genDiv();
        if (b1)
            b2 = TR();
        return b1 && b2;
    }
    else if (lookahead->tag == ')')
    {
        return true;
    }
    else if (lookahead->tag == ENDTOKEN)
    { // EOF
        return true;
    }
    else if (lookahead->tag == '+')
    {
        return true;
    }
    else if (lookahead->tag == '-')
    {
        return true;
    }
    else
    {
        return false;
    }
}

int F()
{
    if (lookahead->tag == '(')
    {
        int b1, b2;
        match('(');
        b1 = E();
        if (b1)
            b2 = match(')');
        return b1 && b2;
    }
    else if (lookahead->tag == NUM_INT || lookahead->tag == NUM_FLOAT)
    {
        int b1, num = lookahead->tag; // num é uma gambiarra pois serve para aceitar tanto float quanto int, assim ele só pula o numero
        char lexema[MAX_TOKEN];
        strcpy(lexema, lookahead->lexema);
        b1 = match(num); // utiliza-se da gambiarra aqui.
        genNum(lexema);
        return b1;
    }
    else
    {
        return false;
    }
}
/*fim da importação */
//------

//--------------------- MAIN -----------------------

/**
 * @brief Funcao principal (main) do compilador
 *
 * @return int
 */
int main(int argc, char *argv[])
{
    int sucess;

    // Inicializa a tabela de simbolo global
    initSymbolTableVariables(&global_symbol_table_variables);
    initSymbolTableString();
    initSymbolTableFunc();

    // Verifica a passagem de parametro
    if (argc != 2)
    {
        printf("[ERRO]\n\tÉ necessário informar um arquivo de entrada (código) "
               "como parâmetro.\n\n");
        exit(EXIT_FAILURE);
    }

    initLex(argv[1]);       // Carrega codigo
    lookahead = getToken(); // Inicializacao do lookahead

    strcpy(output_file_name, argv[1]);
    strcat(output_file_name, ".asm");
    output_file = fopen(output_file_name, "w+");

    sucess = program(); // Chamada da derivacao/funcao inicial da gramatica

    if (sucess != true)
    {
        printf("\t[ERRO] FALHA NA COMPILACAO DO PROGRAMA\n\n");
    }
    else
    {
        printSTString();
        printSTVariables(&global_symbol_table_variables);
        printSTFunc();
        fclose(output_file);
    }
    return 1;
}