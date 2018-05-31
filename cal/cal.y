%{
#include <iostream>
using namespace std;

//-----------------------
//    Extern variables
//-----------------------
int yylex (void);
int calparse();
extern "C" int calwrap();

//-----------------------
//    static functoins
//-----------------------
static void calerror(const char *str)
{
   cerr << "Error: " << str << endl;
}

%}

%token NUMBER

%%
lines
    :
    | lines expression '\n' { cout << $2 << endl; }
    ;
expression
    : factor { $$ = $1}
    | expression '+' term { $$ = $1 + $3; }
    | expression '-' term { $$ = $1 - $3; }
    ;
term
    : factor { $$ = $1; }
    | term '*' factor { $$ = $1 * $3; }
    | term '/' factor { $$ = $1 / $3; }
    ;
factor
    : factor { $$ = $1; }
    | group { $$ = $1; }
    ;
group
    : factor { $$ = $1; }
    | group { $$ = $1; }
    ;

%%

int main()
{
   calparse();
   return 0;
}

