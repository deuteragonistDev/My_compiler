/**
 * @file synt.c
 * @author Prof. Ivairton M. Santos - UFMT - Computacao
 * @brief Codificacao do modulo do analisador sintatico
 * @version 0.4
 * @date 2022-02-04
 * @note last modified 2026-10-29 by João Paulo
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
extern char output_file_name[256];
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
    return ERROR;
}

/**
 * @brief Regra de derivacao inicial
 */
int program(void)
{
    int status;
    gen_preambule(); // Temporariamente cria um preambulo adicional que permite o
                     // uso das funcoes scanf e printf
    status = declarations();
    if (status == ERROR)
        return ERROR;
    gen_preambule_code(); // Chamada do gerador de codigo para escrita do
                          // cabecalho da secao de codigo
    // #BEGIN implementar nao sei comoo
    if (!match(BEGIN))
        return ERROR;

    status = statements();
    if (status == ERROR)
        return ERROR;
    // #END implementar não sei como
    if (!match(END))
    {
        return ERROR;
    }
    gen_epilog_code();
    status = func_code();
    if (status == ERROR)
        return ERROR;
    if (lookahead->tag != ENDTOKEN)
    {
        printf("[ERRO] Texto inesperado apos o programa.\n");
        return ERROR;
    }

    gen_data_section(); // Chamada do gerador de codigo para declaracao de
                        // variaveis
    return true;
}

/**
 * @brief Regra de derivacao para declaracoes
 */
int declarations(void)
{
    int status;
    while ((status = declaration()) == true)
        ;
    return status == ERROR ? ERROR : true;
}

/**
 * @brief Regra de derivacao declaracao
 * @return int true/false
 */
static int is_type_tag(int tag)
{
    return tag == INT || tag == FLOAT || tag == CHAR || tag == STRING;
}

int declaration_var(char *name, int type)
{
    if (sym_find(name, &global_symbol_table_variables) != NULL ||
        sym_func_find(name) != NULL)
    {
        printf("[ERRO] Nome '%s' ja declarado.\n", name);
        return ERROR;
    }
    if (!match(SEMICOLON) ||
        sym_declare(name, type, 0, &global_symbol_table_variables) == NULL)
        return ERROR;
    return true;
}

int declaration_func(char *name, int type)
{
    type_symbol_table_entry parameters[MAX_PARAMS];
    int n_params = 0;
    int param_type;
    char param_name[MAX_TOKSZ];

    if (sym_find(name, &global_symbol_table_variables) != NULL ||
        sym_func_find(name) != NULL)
    {
        printf("[ERRO] Nome '%s' ja declarado.\n", name);
        return ERROR;
    }
    if (!match(OPEN_PAR))
        return ERROR;

    if (lookahead->tag != CLOSE_PAR)
    {
        for (;;)
        {
            if (!is_type_tag(lookahead->tag))
            {
                printf("[ERRO] Esperado tipo de parametro.\n");
                return ERROR;
            }
            if (n_params >= MAX_PARAMS)
            {
                printf("[ERRO] Limite de parametros excedido.\n");
                return ERROR;
            }
            param_type = lookahead->tag;
            if (!match(param_type) || lookahead->tag != ID)
                return ERROR;
            strcpy(param_name, lookahead->lexema);
            if (!match(ID))
                return ERROR;
            if (sym_find(param_name, &global_symbol_table_variables) != NULL ||
                sym_func_find(param_name) != NULL ||
                strcmp(param_name, name) == 0)
            {
                printf("[ERRO] Nome de parametro '%s' ja declarado.\n", param_name);
                return ERROR;
            }
            parameters[n_params].type = param_type;
            strcpy(parameters[n_params].name, param_name);
            n_params++;

            if (lookahead->tag == COMMA)
            {
                if (!match(COMMA) || !is_type_tag(lookahead->tag))
                {
                    printf("[ERRO] Esperado parametro apos a virgula.\n");
                    return ERROR;
                }
                continue;
            }
            if (lookahead->tag != CLOSE_PAR)
            {
                printf("[ERRO] Esperada virgula entre parametros.\n");
                return ERROR;
            }
            break;
        }
    }
    if (!match(CLOSE_PAR) || !match(SEMICOLON))
        return ERROR;

    for (int i = 0; i < n_params; i++)
    {
        if (sym_declare(parameters[i].name, parameters[i].type, 0,
                        &global_symbol_table_variables) == NULL)
            return ERROR;
    }
    if (sym_func_declare(name, type, parameters, n_params) == NULL)
        return ERROR;
    return true;
}

int declaration(void)
{
    int type;
    char name[MAX_TOKSZ];

    if (lookahead->tag == BEGIN)
        return false;
    if (!is_type_tag(lookahead->tag))
    {
        printf("[ERRO] Tipo desconhecido: %d %s.\n", lookahead->tag, lookahead->lexema);
        return ERROR;
    }
    type = lookahead->tag;
    if (!match(type) || lookahead->tag != ID)
        return ERROR;
    strcpy(name, lookahead->lexema);
    if (!match(ID))
        return ERROR;
    if (lookahead->tag == SEMICOLON)
        return declaration_var(name, type);
    if (lookahead->tag == OPEN_PAR)
        return declaration_func(name, type);
    printf("[ERRO] Esperado ';' ou '(' apos a declaracao.\n");
    return ERROR;
}

int func_code(void)
{
    while (lookahead->tag != ENDTOKEN)
    {
        int function_type;
        int param_count = 0;
        char function_name[MAX_TOKSZ];
        type_symbol_table_func_entry *function;

        if (!is_type_tag(lookahead->tag))
        {
            printf("[ERRO] Implementacao de funcao invalida.\n");
            return ERROR;
        }
        function_type = lookahead->tag;
        if (!match(function_type) || lookahead->tag != ID)
            return ERROR;
        strcpy(function_name, lookahead->lexema);
        if (!match(ID))
            return ERROR;
        function = sym_func_find(function_name);
        if (function == NULL)
        {
            printf("[ERRO] Funcao '%s' nao possui prototipo.\n", function_name);
            return ERROR;
        }
        if (function->type != function_type)
        {
            printf("[ERRO] Tipo de implementacao incompativel para '%s'.\n",
                   function_name);
            return ERROR;
        }
        if (!match(OPEN_PAR))
            return ERROR;
        if (lookahead->tag != CLOSE_PAR)
        {
            for (;;)
            {
                if (!is_type_tag(lookahead->tag) ||
                    param_count >= MAX_PARAMS ||
                    param_count >= function->n_params)
                {
                    printf("[ERRO] Assinatura invalida para '%s'.\n",
                           function_name);
                    return ERROR;
                }
                if (lookahead->tag != function->params[param_count].type)
                {
                    printf("[ERRO] Tipo de parametro incompativel para '%s'.\n",
                           function_name);
                    return ERROR;
                }
                if (!match(lookahead->tag) || lookahead->tag != ID ||
                    !match(ID))
                    return ERROR;
                param_count++;
                if (lookahead->tag == COMMA)
                {
                    if (!match(COMMA) || !is_type_tag(lookahead->tag))
                        return ERROR;
                    continue;
                }
                if (lookahead->tag != CLOSE_PAR)
                {
                    printf("[ERRO] Esperada virgula entre parametros.\n");
                    return ERROR;
                }
                break;
            }
        }
        if (param_count != function->n_params)
        {
            printf("[ERRO] Quantidade de parametros incompativel para '%s'.\n",
                   function_name);
            return ERROR;
        }
        if (!match(CLOSE_PAR) || !match(BEGIN))
            return ERROR;
        if (function->implemented)
        {
            printf("[ERRO] Funcao '%s' implementada mais de uma vez.\n",
                   function_name);
            return ERROR;
        }
        function->implemented = 1;
        gen_label(function->label);
        if (statements() == ERROR || !match(END))
            return ERROR;
        gen_ret();
    }
    return true;
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
int statement_legacy(void)
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
            // este aqui deve ser a gosto de dev que pega sistema legado
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
    return ERROR;
}

static int expression_type = INT;

int statement(void)
{
    char name[MAX_TOKSZ];
    type_symbol_table_entry *symbol;
    type_symbol_table_string_entry *string_symbol;
    int status;

    if (lookahead->tag == END || lookahead->tag == ENDTOKEN)
        return false;

    if (lookahead->tag == READ)
    {
        if (!match(READ) || lookahead->tag != ID)
            return ERROR;
        strcpy(name, lookahead->lexema);
        if (!match(ID) || !match(SEMICOLON))
            return ERROR;
        symbol = sym_find(name, &global_symbol_table_variables);
        if (symbol == NULL)
        {
            printf("[ERRO] Variavel nao declarada: %s\n", name);
            return ERROR;
        }
        gen_read(name, symbol->type);
        return true;
    }

    if (lookahead->tag == WRITE)
    {
        if (!match(WRITE))
            return ERROR;
        if (lookahead->tag == STRING)
        {
            string_symbol = sym_string_declare(lookahead->lexema);
            if (string_symbol == NULL || !match(STRING) || !match(SEMICOLON))
                return ERROR;
            gen_write(string_symbol->name, STRING);
            return true;
        }
        if (lookahead->tag != ID)
            return ERROR;
        strcpy(name, lookahead->lexema);
        if (!match(ID) || !match(SEMICOLON))
            return ERROR;
        symbol = sym_find(name, &global_symbol_table_variables);
        if (symbol == NULL)
        {
            printf("[ERRO] Variavel nao declarada: %s\n", name);
            return ERROR;
        }
        gen_write(name, symbol->type);
        return true;
    }

    if (lookahead->tag == IF || lookahead->tag == WHILE)
    {
        int is_while = lookahead->tag == WHILE;
        char label_start[MAX_CHAR], label_end[MAX_CHAR];
        if (!match(lookahead->tag) || !match(OPEN_PAR))
            return ERROR;
        if (is_while)
        {
            gen_label_name(label_start);
            gen_label_name(label_end);
            gen_label(label_start);
        }
        if (B() != true)
            return ERROR;
        if (!match(CLOSE_PAR))
            return ERROR;
        if (is_while)
            gen_cond_jump(label_end);
        else
        {
            gen_label_name(label_end);
            gen_cond_jump(label_end);
        }
        if (!match(BEGIN) || statements() == ERROR || !match(END))
            return ERROR;
        if (is_while)
        {
            gen_incond_jump(label_start);
            gen_label(label_end);
        }
        else
            gen_label(label_end);
        return true;
    }

    if (lookahead->tag == ID)
    {
        strcpy(name, lookahead->lexema);
        if (!match(ID))
            return ERROR;
        if (lookahead->tag == OPEN_PAR)
        {
            type_symbol_table_func_entry *function = sym_func_find(name);
            int count = 0;
            if (function == NULL)
            {
                printf("[ERRO] Funcao nao declarada: %s\n", name);
                return ERROR;
            }
            if (!match(OPEN_PAR))
                return ERROR;
            while (lookahead->tag != CLOSE_PAR)
            {
                int argument_type;
                if (lookahead->tag == ENDTOKEN || count >= MAX_PARAMS ||
                    count >= function->n_params)
                {
                    printf("[ERRO] Quantidade invalida de argumentos para '%s'.\n", name);
                    return ERROR;
                }

                if (lookahead->tag == CHAR || lookahead->tag == STRING)
                {
                    argument_type = lookahead->tag;
                    string_symbol = sym_string_declare(lookahead->lexema);
                    if (string_symbol == NULL)
                        return ERROR;
                    if (argument_type == STRING)
                        gen_string_value(string_symbol->name);
                    else
                    {
                        char char_value[MAX_TOKEN];
                        snprintf(char_value, sizeof(char_value), "%u",
                                 (unsigned char)lookahead->lexema[1]);
                        genNum(char_value);
                    }
                    if (!match(argument_type))
                        return ERROR;
                }
                else if (lookahead->tag == ID || lookahead->tag == NUM_INT ||
                         lookahead->tag == NUM_FLOAT || lookahead->tag == OPEN_PAR)
                {
                    if (E() != true)
                    {
                        printf("[ERRO] Expressao de argumento invalida.\n");
                        return ERROR;
                    }
                    argument_type = expression_type;
                }
                else
                {
                    printf("[ERRO] Argumento invalido para '%s'.\n", name);
                    return ERROR;
                }

                if (argument_type != function->params[count].type)
                {
                    printf("[ERRO] Tipo de argumento invalido para '%s'.\n", name);
                    return ERROR;
                }
                gen_assign(function->params[count].name,
                           function->params[count].type);
                count++;

                if (lookahead->tag == COMMA)
                {
                    if (!match(COMMA) || lookahead->tag == CLOSE_PAR ||
                        lookahead->tag == ENDTOKEN)
                    {
                        printf("[ERRO] Esperado argumento apos a virgula.\n");
                        return ERROR;
                    }
                    continue;
                }
                if (lookahead->tag != CLOSE_PAR)
                {
                    printf("[ERRO] Esperada virgula entre argumentos.\n");
                    return ERROR;
                }
            }
            if (count != function->n_params)
            {
                printf("[ERRO] Quantidade invalida de argumentos para '%s'.\n", name);
                return ERROR;
            }
            if (!match(CLOSE_PAR) || !match(SEMICOLON))
                return ERROR;
            gen_call(function->label);
            return true;
        }
        if (lookahead->tag != ASSIGN)
            return ERROR;
        symbol = sym_find(name, &global_symbol_table_variables);
        if (symbol == NULL)
        {
            printf("[ERRO] Variavel nao declarada: %s\n", name);
            return ERROR;
        }
        if (!match(ASSIGN))
            return ERROR;
        if (symbol->type == CHAR || symbol->type == STRING)
        {
            if (lookahead->tag != symbol->type)
            {
                printf("[ERRO] Tipo de atribuicao incompativel.\n");
                return ERROR;
            }
            string_symbol = sym_string_declare(lookahead->lexema);
            if (string_symbol == NULL || !match(symbol->type))
                return ERROR;
            symbol->value = string_symbol->value;
            if (symbol->type == STRING)
                gen_string_value(string_symbol->name);
            else
            {
                char char_value[MAX_TOKEN];
                snprintf(char_value, sizeof(char_value), "%u",
                         (unsigned char)string_symbol->value[1]);
                genNum(char_value);
            }
            gen_assign(name, symbol->type);
        }
        else
        {
            status = E();
            if (status != true)
                return ERROR;
            if (expression_type != symbol->type)
            {
                printf("[ERRO] Tipo de expressao incompativel com '%s'.\n", name);
                return ERROR;
            }
            gen_assign(name, symbol->type);
        }
        if (!match(SEMICOLON))
            return ERROR;
        return true;
    }

    printf("[ERRO] Comando desconhecido.\nTag=%d; Lexema=%s\n",
           lookahead->tag, lookahead->lexema);
    return ERROR;
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
    if (E() != true || !boolOperator(&operator) || E() != true)
        return ERROR;
    gen_bool(operator);
    return true;
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
    if (b1 != true)
        return ERROR;
    b2 = ER();
    return b2 == true ? true : ERROR;
}

int ER()
{
    if (lookahead->tag == '+')
    {
        int b1, b2, previous_type = expression_type;
        match('+');
        b1 = T();
        if (previous_type == FLOAT || expression_type == FLOAT)
            expression_type = FLOAT;
        genAdd();
        if (b1 != true)
            return ERROR;
        b2 = ER();
        return b2 == true ? true : ERROR;
    }
    else if (lookahead->tag == '-')
    {
        int b1, b2, previous_type = expression_type;
        match('-');
        b1 = T();
        if (previous_type == FLOAT || expression_type == FLOAT)
            expression_type = FLOAT;
        genSub();
        if (b1 != true)
            return ERROR;
        b2 = ER();
        return b2 == true ? true : ERROR;
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
    else if (lookahead->tag == ')' || lookahead->tag == ',' ||
             lookahead->tag == SEMICOLON || lookahead->tag == ENDTOKEN ||
             lookahead->tag == EQUAL || lookahead->tag == NE ||
             lookahead->tag == LT || lookahead->tag == GT ||
             lookahead->tag == LE || lookahead->tag == GE ||
             lookahead->tag == '+' || lookahead->tag == '-')
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
    if (b1 != true)
        return ERROR;
    b2 = TR();
    return b2 == true ? true : ERROR;
}

int TR()
{
    if (lookahead->tag == '*')
    {
        int b1, b2, previous_type = expression_type;
        match('*');
        b1 = F();
        if (previous_type == FLOAT || expression_type == FLOAT)
            expression_type = FLOAT;
        genMult();
        if (b1 != true)
            return ERROR;
        b2 = TR();
        return b2 == true ? true : ERROR;
    }
    else if (lookahead->tag == '/')
    {
        int b1, b2, previous_type = expression_type;
        match('/');
        b1 = F();
        if (previous_type == FLOAT || expression_type == FLOAT)
            expression_type = FLOAT;
        genDiv();
        if (b1 != true)
            return ERROR;
        b2 = TR();
        return b2 == true ? true : ERROR;
    }
    else if (lookahead->tag == ')' || lookahead->tag == ',' ||
             lookahead->tag == SEMICOLON || lookahead->tag == ENDTOKEN ||
             lookahead->tag == EQUAL || lookahead->tag == NE ||
             lookahead->tag == LT || lookahead->tag == GT ||
             lookahead->tag == LE || lookahead->tag == GE ||
             lookahead->tag == '+' || lookahead->tag == '-')
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
        expression_type = num == NUM_INT ? INT : FLOAT;
        genNum(lexema);
        return b1;
    }
    else if (lookahead->tag == ID)
    {
        char name[MAX_TOKSZ];
        type_symbol_table_entry *symbol;
        strcpy(name, lookahead->lexema);
        symbol = sym_find(name, &global_symbol_table_variables);
        if (symbol == NULL)
        {
            printf("[ERRO] Variavel nao declarada: %s\n", name);
            return ERROR;
        }
        if (!match(ID))
            return ERROR;
        expression_type = symbol->type;
        gen_id(name);
        return true;
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

    if (snprintf(output_file_name, sizeof(output_file_name), "%s.asm", argv[1]) >= (int)sizeof(output_file_name))
    {
        printf("[ERRO] Caminho do arquivo de saida muito longo.\n");
        return EXIT_FAILURE;
    }
    output_file = fopen(output_file_name, "w+");
    if (output_file == NULL)
    {
        perror("[ERRO] Nao foi possivel criar o arquivo .asm");
        return EXIT_FAILURE;
    }

    sucess = program(); // Chamada da derivacao/funcao inicial da gramatica

    if (sucess != true)
    {
        printf("\t[ERRO] FALHA NA COMPILACAO DO PROGRAMA\n\n");
        fclose(output_file);
        remove(output_file_name);
        return EXIT_FAILURE;
    }
    else
    {
        printSTString();
        printSTVariables(&global_symbol_table_variables);
        printSTFunc();
        fclose(output_file);
    }
    return 0;
}