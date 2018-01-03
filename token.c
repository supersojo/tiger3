#include "token.h"

namespace tiger {



char * kTokenString[]={
"kToken_ID",
"kToken_NUM",
"kToken_STR",
"kToken_LPAR",/* ( */
"kToken_RPAR",/* ) */
"kToken_LBRA",/* { */
"kToken_RBRA",/* } */
"kToken_LSQB",/* [ */
"kToken_RSQB",/* ] */

/* add more tokens */
"kToken_ADD",
"kToken_SUB",
"kToken_MUL",
"kToken_DIV",
"kToken_ASSIGN",
"kToken_COMMA",
"kToken_COLON",
"kToken_SEMICOLON",
"kToken_DOT",

"kToken_EQUAL",
"kToken_NOTEQUAL",
"kToken_LT",
"kToken_GT",
"kToken_LE",
"kToken_GE",

"kToken_AND",
"kToken_OR",
"kToken_NOT",


/* keywords */
"type",
"array",
"of",
"var",
"for",
"to",
"while",
"do",
"if",
"then",
"else",
"function",
"end",
"let",
"in",
"nil",

"kToken_EOT",/* the end of token stream */
"kToken_Unknown",
"kToken_Unused"
};

} // namespace tiger