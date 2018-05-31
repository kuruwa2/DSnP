%{
#include <cstdlib>
#include "calYacc.hpp"

using namespace std;

extern "C" {
   int calwrap() { return 1; }
}
void calerror(char *);

%}


%%
[0~9]+"."[0~9]+ {
    yylval = atoi(yytext);
    return NUMBER;
}
[0~9]+ {
    yylval = atoi(yytexy);
    return NUMBER;
}
[ \t] ;
[-+()=/*\n] { return *yytext; }
. { calerror("invalid character"); }

%%
int yywrap()
{
    return 1;
}
