%option noyywrap
%option nounput
%option noinput
%{

  // ['][^']*[']  {
  //                string tmp(yytext);
  //                yylval.s = new string(tmp.substr(1, tmp.size() - 2));
  //                return string_token;
  // 	           } /* ' */
#include <iostream>
#include <cstdlib>
#include <cctype>

using namespace std;

#include <string>
#include "ast.hpp"
#include <vector>

#include "parser.tab.hpp"

void stringToLower(string &s)
{
  for (auto &c: s)
      c = tolower(c);
}

%}

%%

begin            return begin_token;
end              return end_token;
write            return write_token;
read             return read_token;
mod              return mod_token;
if               return if_token;
then             return then_token;
else             return else_token;
while            return while_token;
do               return do_token;
var              return var_token;

[0-9]+  {
          yylval.x = atof(yytext);
          return num_token;
        }

0[xX][0-9a-fA-F]+  {
                     yylval.x = atof(yytext);
                     return num_token;
                   }

[a-zA-Z_][a-zA-Z_0-9]* {
                         yylval.s = new string(yytext);
			                   stringToLower(*(yylval.s));
                         return id_token;
                       }

[<>().;+,/:=\-*]     return *yytext;
[ \n\t]            {  }
.  {
     cerr << "Lexical error!\n" << *yytext << endl;
     exit(EXIT_FAILURE);
   }

%%
