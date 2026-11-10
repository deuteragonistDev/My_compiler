#!/usr/bin/env bash
set -u

tmpdir=$(mktemp -d /tmp/my_compiler_tests.XXXXXX) || exit 1
trap 'rm -rf "$tmpdir"' EXIT
failures=0

compiler=${COMPILER:-}
if [ -z "$compiler" ]; then
    if [ -x ./compiler ]; then
        compiler=./compiler
    elif command -v gcc >/dev/null 2>&1; then
        if ! gcc -Wall -o "$tmpdir/compiler" lex.c synt.c gen.c symbols.c \
            >/dev/null 2>&1; then
            printf 'FALHOU\n'
            exit 1
        fi
        compiler="$tmpdir/compiler"
    elif [ -x ./compiler.exe ]; then
        compiler=./compiler.exe
    else
        printf 'FALHOU\n'
        exit 1
    fi
fi

check_result() {
    if [ "$1" -eq 0 ]; then
        printf 'OK\n'
    else
        printf 'FALHOU\n'
        failures=$((failures + 1))
    fi
}

run_success() {
    name=$1
    source=$2
    file="$tmpdir/$name.cc"
    printf '%s\n' "$source" > "$file"
    rm -f "$file.asm"
    "$compiler" "$file" >/dev/null 2>&1
    result=$?
    if [ "$result" -ne 0 ] || [ ! -f "$file.asm" ]; then
        check_result 1
        return
    fi
    check_result 0
}

run_failure() {
    name=$1
    source=$2
    timed=$3
    file="$tmpdir/$name.cc"
    printf '%s\n' "$source" > "$file"
    rm -f "$file.asm"
    if [ "$timed" = yes ]; then
        timeout 3 "$compiler" "$file" >/dev/null 2>&1
    else
        "$compiler" "$file" >/dev/null 2>&1
    fi
    result=$?
    if [ "$result" -ne 0 ] && [ "$result" -ne 124 ] && [ ! -f "$file.asm" ]; then
        check_result 0
    else
        check_result 1
    fi
}

function_source='int a;
int soma(int x, int y);
begin
soma(1, 2);
end
int soma(int x, int y) begin
a = x + y;
end'
printf '%s\n' "$function_source" > "$tmpdir/prototype.cc"
rm -f "$tmpdir/prototype.cc.asm"
"$compiler" "$tmpdir/prototype.cc" >/dev/null 2>&1
result=$?
asm="$tmpdir/prototype.cc.asm"
if [ "$result" -eq 0 ] && [ -f "$asm" ] &&
    grep -Fq 'call func_soma' "$asm" &&
    grep -Fq 'ret' "$asm" &&
    [ "$(grep -c 'section .data' "$asm")" -eq 1 ] &&
    grep -Eq '^x: ' "$asm" && grep -Eq '^y: ' "$asm" &&
    [ "$(grep -n 'int 0x80' "$asm" | head -n 1 | cut -d: -f1)" -lt \
      "$(grep -n '^func_soma:' "$asm" | cut -d: -f1)" ]; then
    if command -v nasm >/dev/null 2>&1; then
        nasm -f elf64 "$asm" -o "$tmpdir/prototype.o" >/dev/null 2>&1
        check_result $?
    else
        check_result 0
    fi
else
    check_result 1
fi

run_failure missing_function 'begin
missing(1);
end' no
run_failure too_few_arguments 'int soma(int x, int y);
begin
soma(1);
end' no
run_failure too_many_arguments 'int soma(int x, int y);
begin
soma(1, 2, 3);
end' no
run_failure wrong_argument_type 'int soma(int x);
begin
soma("x");
end' no
run_failure duplicate_prototype 'int soma();
int soma();
begin
end' no
run_failure implementation_without_prototype 'begin
end
int soma() begin
end' no
run_failure duplicate_implementation 'int soma();
begin
end
int soma() begin
end
int soma() begin
end' no
run_failure missing_argument_comma 'int soma(int x, int y);
begin
soma(1 2);
end' no
run_failure trailing_argument_comma 'int soma(int x, int y);
begin
soma(1, 2,);
end' no
run_failure missing_closing_parenthesis 'int soma(int x, int y);
begin
soma(1, 2' yes
run_failure duplicate_parameter_name 'int x;
int soma(int x);
begin
end' no
run_failure missing_parameter_comma 'int soma(int x int y);
begin
end' no

run_success arithmetic_assignment 'int a;
begin
a = 1 + 2;
end'
run_success boolean_if 'int a;
begin
if (a > 0) begin
a = 1;
end
end'

if ! command -v nasm >/dev/null 2>&1; then
    printf 'nasm nao encontrado\n'
fi
exit "$failures"
