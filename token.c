#include "token.h"

namespace tiger {



char * kTokenString[]={
(char*)"kToken_ID",
(char*)"kToken_NUM",
(char*)"kToken_STR",
(char*)"kToken_LPAR",/* ( */
(char*)"kToken_RPAR",/* ) */
(char*)"kToken_LBRA",/* { */
(char*)"kToken_RBRA",/* } */
(char*)"kToken_LSQB",/* [ */
(char*)"kToken_RSQB",/* ] */

/* add more tokens */
(char*)"kToken_ADD",
(char*)"kToken_SUB",
(char*)"kToken_MUL",
(char*)"kToken_DIV",
(char*)"kToken_ASSIGN",
(char*)"kToken_COMMA",
(char*)"kToken_COLON",
(char*)"kToken_SEMICOLON",
(char*)"kToken_DOT",

(char*)"kToken_EQUAL",
(char*)"kToken_NOTEQUAL",
(char*)"kToken_LT",
(char*)"kToken_GT",
(char*)"kToken_LE",
(char*)"kToken_GE",

(char*)"kToken_AND",
(char*)"kToken_OR",
(char*)"kToken_NOT",


/* keywords */
(char*)"type",
(char*)"array",
(char*)"of",
(char*)"var",
(char*)"for",
(char*)"to",
(char*)"while",
(char*)"do",
(char*)"if",
(char*)"then",
(char*)"else",
(char*)"function",
(char*)"end",
(char*)"let",
(char*)"break",
(char*)"in",
(char*)"nil",

(char*)"kToken_EOT",/* the end of token stream */
(char*)"kToken_Unknown",
(char*)"kToken_Unused"
};

} // namespace tiger