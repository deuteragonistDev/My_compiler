int valor;
float taxa;
string curso;
char conceito;
int soma(int x, int y);
int exibir(string msg, char nota);

begin
    read valor;
    valor = 10;
    taxa = 3.14;
    curso = "Computacao";
    conceito = 'A';

    if (valor == 10)
    begin
        write curso;
    end
    else
    begin
        write "Outro";
    end

    while (valor < 15)
    begin
        valor = 15;
    end

    soma(valor, 20);
    exibir(curso, conceito);
    write valor;
end

int soma(int x, int y)
begin
    x = 30;
    write x;
end

int exibir(string msg, char nota)
begin
    write msg;
    write nota;
end