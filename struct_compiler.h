/**
 * @file struct_compiler.h
 * @author Prof. Ivairton M. Santos - UFMT - Computacao
 * @brief Modulo para definicao de constantes e estruturas de dados do Sistema
 *        Compilador de C Simplificado
 * @version 0.3
 * @date 2021-12-09
 * @note last modified: 2026-10-29 by João Paulo
 */

#ifndef _STRUCT_COMPILER_
#define _STRUCT_COMPILER_

// Definicao de constantes para todo o compilador
#define MAX_CHAR 32
#define MAX_TOKEN 32

#define true 1  // por conveniencia, para facilitar leitura de codigo
#define false 0 // por conveniencia, para facilitar leitura de codigo

// Definicao dos codigos dos tokens
#define ENDTOKEN '\0'
#define ERROR -1
// #define NUM 1 //assim que implementar para float devo retirar esta linha!
#define NUM_INT 1
#define NUM_FLOAT 2

#define PLUS '+'
#define MINUS '-'
#define MULT '*'
#define DIV '/'
#define OPEN_PAR '('
#define CLOSE_PAR ')'
#define SEMICOLON ';'
#define DOUBLE_QUOTES '"'
#define COMMA ','

// KeyWords
#define ID 256
#define IF 257    // implementar // ok
#define ELSE 258  // implementar // ok
#define THEN 259  // implementar // não sei se é pra implementar não...
#define WHILE 260 // implementar // ok
#define DO 261    // implementar // também não sei...
#define READ 262
#define WRITE 263
#define INT 264
#define STRING 265
#define FLOAT 266
#define CHAR 267
#define BEGIN 268 // ok
#define END 269   // ok

// Operadores
#define ASSIGN '='
#define EQUAL 270 // ==
#define NE 271    // !=
#define GE 272    // >=
#define LE 273    // <=
#define GT '>'
#define LT '<'

// Definicao da estrutura de dados 'token'
struct st_token
{
    int tag;
    char lexema[MAX_CHAR];
    int value;
};
typedef struct st_token type_token;

#endif //_STRUCT_COMPILER_