/**
 * @file symbols.c
 * @author Prof. Ivairton M. Santos - UFMT - Computacao
 * @brief Modulo do gerador da tabela de simbolos
 * @version 0.1
 * @date 2022-02-04
 * @note last modified 12.11.2026 by deuteragonistDev João Paulo Alves
 */
#include "symbols.h"
#include "gen.h"
#include <stdio.h>


// extern int nstringconsts;
// int sympos = 0;
// int nstringconsts = 0;
// int symfuncspos = 0; //Sera usado posteriormente
// int stack_pos = 0;

// Variaveis globais
type_symbol_table_variables global_symbol_table_variables;
type_symbol_table_string symbol_table_string;
// type_symbol_function symfuncs[MAX_FUNCS];
type_symbol_table_func symbol_table_func; // implementar eu acho

/**
 * @brief Busca pelo simbolo na tabela de variaveis
 *
 * @param char*
 * @param type_symbol_table_variables*
 * @return type_symbol_table_entry*
 */
type_symbol_table_entry *sym_find(char *s, type_symbol_table_variables *stv)
{
  int i;
  type_symbol_table_entry *symbol = NULL;
  for (i = 0; i < stv->n_variables; i++)
  {
    if (strcmp(stv->variable[i].name, s) == 0)
    {
      symbol = &(stv->variable[i]);
    }
  }
  return symbol;
}

/**
 * @brief Insere um novo simbolo na tabela de variaveis
 *
 * @param name
 * @param type
 * @param addr
 * @return type_symbol*
 */
type_symbol_table_entry *sym_declare(char *name, int type, void *value, type_symbol_table_variables *stv)
{
  if (stv->n_variables < MAX_SYMBOLS)
  { // Verifica se eh possivel declarar mais variavel
    int n_var;
    n_var = stv->n_variables;
    stv->variable[n_var].type = type;
    stv->variable[n_var].value = value;
    strcpy(stv->variable[n_var].name, name);
    stv->n_variables++;
    n_var = stv->n_variables; // ainda não sei pra que serve mas eu tive que desmontar
    return &(stv->variable[n_var]);
  }
  else
  {
    printf("[ERRO] Limite de declaracao (quantidade) de variaveis atingido.\n");
    return NULL;
  }
}

/**
 * @brief Busca pelo simbolo na tabela de STRINGS
 *
 * @param char*
 * @return type_symbol_table_string_entry*
 */
type_symbol_table_string_entry *sym_string_find(char *s)
{
  int i;
  type_symbol_table_string_entry *str = NULL;
  for (i = 0; i < symbol_table_string.n_strings; i++)
  {
    if (strcmp(symbol_table_string.string[i].value, s) == 0)
    {
      str = &(symbol_table_string.string[i]);
    }
  }
  return str;
}

/**
 * @brief Insere um novo simbolo na tabela de STRINGS
 *
 * @param char*
 * @return type_symbol_table_string_entry*
 */
type_symbol_table_string_entry *sym_string_declare(char *s)
{
  static int str_label_count = 0;

  if (symbol_table_string.n_strings < MAX_STRINGS)
  {
    int n_str;
    char str_name[MAX_TOKSZ];
    sprintf(str_name, "str%d", str_label_count);
    n_str = symbol_table_string.n_strings;

    strcpy(symbol_table_string.string[n_str].value, s);
    strcpy(symbol_table_string.string[n_str].name, str_name);
    str_label_count++;
    symbol_table_string.n_strings++;
    return &(symbol_table_string.string[n_str]);
  }
  else
  {
    printf("[ERRO] Numero maximo de strings declaradas alcancado!\n");
    return NULL;
  }
}

/**
 * @brief Busca pelo simbolo na tabela de simbolos de funcoes
 *
 * @return type_symbol_func_entry
 */
type_symbol_table_func_entry *sym_func_find(char *f_name)
{
  int i;
  type_symbol_table_func_entry *func;
  func = NULL;

  for (i = 0; i < symbol_table_func.n_funcs; i++)
  {
    if (strcmp(symbol_table_func.funcs[i].name, f_name) == 0)
    {
      func = &symbol_table_func.funcs[i];
    }
  }
  return func;
}

/**
 * @brief Insere um novo simbolo na tabela de funcoes.
 *
 * @param name nome da funcao
 * @param type tipo da funcao
 * @param params lista de parametros
 * @param nparams quantidade de parametros
 * @return type_symbol_function_entry* posicao de memoria da funcao inserida
 */
type_symbol_table_func_entry *sym_func_declare(char *name, int type, type_symbol_table_entry params[MAX_PARAMS], int n_params)
{
  int i, pos;
  if (symbol_table_func.n_funcs < MAX_FUNCS) // verifica se fda pra criar mais funçao
  {
    pos = symbol_table_func.n_funcs; //recebe a posiçao livre (que tambem significa que é a ultima)
    strcpy(symbol_table_func.funcs[pos].name, name); //copia o nome para a tabela de funcoes
    symbol_table_func.funcs[pos].type = type; //copia o tipo para a tabela de funcoes
    for (i = 0; i < n_params; i++)
    {
      strcpy(symbol_table_func.funcs[pos].params[i].name, params[i].name); //copia os parametros para a tabela de funções com base no numero de parametros "i"
      symbol_table_func.funcs[pos].params[i].type = params[i].type; //copia o tipo dos parametros para a tabela de funções 
    }
    symbol_table_func.funcs[pos].n_params = n_params;
    symbol_table_func.n_funcs++;
    sprintf(symbol_table_func.funcs[pos].label, "label%d", pos);
    gen_label_name(symbol_table_func.funcs[pos].label); // AGORA GERA A LABEL GENERICA PARA A FUNÇÂO 
    return &symbol_table_func.funcs[pos];
  }
  else
  {
    printf("[ERRO] Limite de declaracao de funcoes atingido.");
  }
  return NULL;
}

/**
 * @brief Inicializa tabela de simbolos de variaveis globais
 *
 * @param stv
 */
void initSymbolTableVariables(type_symbol_table_variables *stv)
{
  int i;
  stv->n_variables = 0;
  for (i = 0; i < MAX_SYMBOLS; i++)
  {
    strcpy(stv->variable[i].name, "");
    stv->variable[i].type = -1;
    stv->variable[i].value = 0;
  }
  stv->next = NULL;
}

/**
 * @brief Inicializa tabela de simbolos de strings
 *
 */
void initSymbolTableString()
{
  int i;
  symbol_table_string.n_strings = 0;
  for (i = 0; i < MAX_SYMBOLS; i++)
  {
    strcpy(symbol_table_string.string[i].name, "");
    strcpy(symbol_table_string.string[i].value, "");
  }
}

/**
 * @brief Inicializa tabela de simbolos de funcoes
 *
 */
void initSymbolTableFunc()
{
  int i;
  symbol_table_func.n_funcs = 0;
  for (i = 0; i < MAX_FUNCS; i++)
  {
    strcpy(symbol_table_func.funcs[i].name, "");
    symbol_table_func.funcs[i].type = 0;
    symbol_table_func.funcs[i].n_params = 0;
  }
}

/// Para DEBUG - Impressao da tabela de simbolos de variaveis
void printSTVariables(type_symbol_table_variables *stv)
{
  int i, n_var;
  printf("TABELA DE SIMBOLOS DE VARIAVEIS:\n");
  n_var = stv->n_variables;
  printf("Numero de variaveis = %d\n", n_var);
  for (i = 0; i < n_var; i++)
  {
    printf("\tvariavel[%d].name:%s\n", i, stv->variable[i].name);
    printf("\tvariavel[%d].type:%d\n", i, stv->variable[i].type);
  }
}

void printSTString()
{
  int i, n_str;
  printf("TABELA DE SIMBOLOS DE STRINGS:\n");
  n_str = symbol_table_string.n_strings;
  printf("Numero de strings = %d\n", n_str);
  for (i = 0; i < n_str; i++)
  {
    printf("\tstring[%d].name:%s\n", i, symbol_table_string.string[i].name);
    printf("\tstring[%d].value:%s\n", i, symbol_table_string.string[i].value);
  }
}

/// Para DEBUG - Impressao da tabela de simbolos de funcoes
void printSTFunc()
{
  int i, j, n_func;
  printf("TABELA DE SIMBOLOS DE FUNCOES:\n");
  n_func = symbol_table_func.n_funcs;
  printf("Numero de funcoes cadastradas = %d\n", n_func);
  for (i = 0; i < n_func; i++)
  {
    printf("\tFuncao[%d].name: %s\n", i, symbol_table_func.funcs[i].name);
    printf("\tFuncao[%d].tipo: %d\n", i, symbol_table_func.funcs[i].type);
    printf("\tFuncao[%d].n_param: %d\n", i, symbol_table_func.funcs[i].n_params);
    printf("\tFuncao[%d].parametros:\n", i);
    for (j = 0; j < symbol_table_func.funcs[i].n_params; j++)
    {
      printf("\t\ttipo: %d", symbol_table_func.funcs[i].params[j].type);
      printf("\t\tnome: %s", symbol_table_func.funcs[i].params[j].name);
    }
  }
}