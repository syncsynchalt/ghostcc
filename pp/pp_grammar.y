%{
#include "ast.h"
#include "pp_lex.h"
extern char *yytext;
extern void yyerror(const char *s);

ast_node *combine_binary(ast_node *n1, ast_node *n2, ast_node *n3)
{
    n2->left = n1;
    n2->right = n3;
    return n2;
}

ast_node *combine_unary(ast_node *n1, ast_node *n2)
{
    n1->left = n2;
    return n1;
}

void f(ast_node *n)
{
    // free an orphaned node
    free(n->s);
    free(n);
}

%}

// TODO xxx fixme, move tokens here?
%token ID 700
%token NUM 701
%token STR 702
%token CHA 703
%token AND_OP 708
%token OR_OP 709
%token EQ_OP 710
%token NE_OP 711
%token LE_OP 712
%token GE_OP 713
%token LEFT_OP 706
%token RIGHT_OP 707

%start constant_expression

%%

primary_expression
    : ID
    | NUM
    | STR
    | CHA
    | '(' constant_expression ')'                           { $$ = $2; f($1); f($3); }
    ;

postfix_expression
    : primary_expression
    | pp_function_expression
    ;

unary_expression
    : postfix_expression
    | unary_operator unary_expression                       { $$ = combine_unary($1, $2); }
    ;

unary_operator
    : '+'
    | '-'
    | '~'
    | '!'
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression '*' unary_expression        { $$ = combine_binary($1, $2, $3); }
    | multiplicative_expression '/' unary_expression        { $$ = combine_binary($1, $2, $3); }
    | multiplicative_expression '%' unary_expression        { $$ = combine_binary($1, $2, $3); }
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression     { $$ = combine_binary($1, $2, $3); }
    | additive_expression '-' multiplicative_expression     { $$ = combine_binary($1, $2, $3); }
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression          { $$ = combine_binary($1, $2, $3); }
    | shift_expression RIGHT_OP additive_expression         { $$ = combine_binary($1, $2, $3); }
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression            { $$ = combine_binary($1, $2, $3); }
    | relational_expression '>' shift_expression            { $$ = combine_binary($1, $2, $3); }
    | relational_expression LE_OP shift_expression          { $$ = combine_binary($1, $2, $3); }
    | relational_expression GE_OP shift_expression          { $$ = combine_binary($1, $2, $3); }
    ;

equality_expression
    : relational_expression
    | equality_expression '=' relational_expression         { $$ = combine_binary($1, $2, $3); }
    | equality_expression EQ_OP relational_expression       { $$ = combine_binary($1, $2, $3); }
    | equality_expression NE_OP relational_expression       { $$ = combine_binary($1, $2, $3); }
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression                { $$ = combine_binary($1, $2, $3); }
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression            { $$ = combine_binary($1, $2, $3); }
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression       { $$ = combine_binary($1, $2, $3); }
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression     { $$ = combine_binary($1, $2, $3); }
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression        { $$ = combine_binary($1, $2, $3); }
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' conditional_expression ':' conditional_expression
                { $2->left = $1; $2->right = $4; $4->left = $3; $4->right = $5; $$ = $2; }
    ;

pp_function_expression
    : ID '(' pp_arguments ')'                   { $$ = combine_binary($1, $2, $3); f($4); }
    ;

pp_arguments
    : ID
    | ID ',' pp_arguments                       { $1->right = $3; $$ = $1; f($2); }
    ;

constant_expression
    : conditional_expression                    { pp_parse_result = $1; }
    ;

%%
