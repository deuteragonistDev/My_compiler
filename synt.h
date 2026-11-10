/**
 * @file synt.h
 * @author Prof. Ivairton M. Santos - UFMT - Computacao
 * @brief Modulo do analisado sintatico
 * @version 0.4
 * @date 2022-02-04
 * @note last modified 2026.10.29 by DeuteragonistDev João Paulo
 */

#ifndef _SYNT_H_
#define _SYNT_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen.h"
#include "lex.h"
#include "struct_compiler.h"
#include "symbols.h"

/*
 * Gramatica:
 *      program -> declarations  statements
 *      declarations-> declaration declarations | [vazio]
 *      declaration -> int id;
 *      statements --> statement statements | [vazio]
 *      statement  --> read_stmt | write_stmt | if_stmt | if_else_stmt | while_stmt | assign_stmt
 *      read_stmt  --> read id;
 *      write_stmt --> write id;
 *      if_stmt    --> if '(' rel_expr ')' begin statements end [ else begin statements end  ]  (ideal, ainda por fazer)
 *      if_stmt    --> if '(' E ')' begin statements end [ else begin statements end ] (implementado)
 *      assign_stmt -> id '=' E;
 *      rel_expr   --> id rel_operator E
 *      rel_operator -->  '<' | '>' | '<=' | '>=' | '==' | '!='
 *      E          --> expressões com numeros inteiros, cujo resultado final fica na pilha.
 */

// Prototipos
int match(int token_tag);
int program(void);
int declarations(void);
int statements(void);
int declaration(void);
int declaration_var(char *name, int type);
int declaration_func(char *name, int type);
int statement(void);
int func_code(void);

int B(void);
int boolOperator(int *);

int E();
int ER();
int T();
int TR();
int F();

int main(int argc, char *argv[]);

#endif //_SYNT_H_