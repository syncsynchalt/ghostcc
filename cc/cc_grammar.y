%{
// Jeff Lee, 1985
#include "ast.h"
#include "die.h"
#include "cc_lex.h"
extern char *yytext;
extern void yyerror(const char *s);

ast_node *new_node(node_type t)
{
    ast_node *n = calloc(1, sizeof(*n));
    n->type = t;
    return n;
}

ast_node *new_token(token_type t)
{
    ast_node *n = new_node(NODE_OTHER);
    n->tok_type = t;
    return n;
}

ast_node *combine_binary(ast_node *left, ast_node *node, ast_node *right)
{
    node->left = left;
    node->right = right;
    return node;
}

ast_node *combine_unary(ast_node *node, ast_node *left)
{
    return combine_binary(left, node, NULL);
}

#define f free_ast_node

void f2(ast_node *n1, ast_node *n2)
{
    free_ast_node(n1);
    free_ast_node(n2);
}


%}

// TODO xxx fixme, move token.h here?
%token AUTO 600
%token BREAK 601
%token CASE 602
%token CHAR 603
%token CONST 604
%token CONTINUE 605
%token DEFAULT 606
%token DO 607
%token DOUBLE 608
%token ELSE 609
%token ENUM 610
%token EXTERN 611
%token FLOAT 612
%token FOR 613
%token GOTO 614
%token IF 615
%token INT 616
%token LONG 617
%token REGISTER 618
%token RETURN 619
%token SHORT 620
%token SIGNED 621
%token SIZEOF 622
%token STATIC 623
%token STRUCT 624
%token SWITCH 625
%token TYPEDEF 626
%token UNION 627
%token UNSIGNED 628
%token VOID 629
%token VOLATILE 630
%token WHILE 631

%token ID 700
%token NUM 701
%token STR 702
%token CHA 703

%token ELLIPSIS 704
%token PTR_OP 705
%token LEFT_OP 706
%token RIGHT_OP 707

%token AND_OP 708
%token OR_OP 709
%token EQ_OP 710
%token NE_OP 711
%token LE_OP 712
%token GE_OP 713
%token INC_OP 714
%token DEC_OP 715

%token ADD_ASSIGN 800
%token SUB_ASSIGN 801
%token MUL_ASSIGN 802
%token DIV_ASSIGN 803
%token LEFT_ASSIGN 804
%token RIGHT_ASSIGN 805
%token MOD_ASSIGN 806
%token AND_ASSIGN 807
%token OR_ASSIGN 808
%token XOR_ASSIGN 809

%token TYPE_NAME 810

%start translation_unit

%expect 1   /* ignore the "dangling else" shift/reduce conflict, ref. https://www.quut.com/c/ANSI-C-grammar-FAQ.html */
%%

primary_expression
	: ID
	| NUM
	| STR
	| CHA
	| '(' expression ')'                                    { $$ = $2; f2($1, $3); }
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'    { $$ = combine_binary($1, new_node(NODE_SUBSCRIPT), $3); f2($2, $4); }
	| postfix_expression '(' ')'                            { $$ = combine_binary($1, $2, NULL); f($3); }
	| postfix_expression '(' argument_expression_list ')'   { $$ = combine_binary($1, $2, $3); f($4); }
	| postfix_expression '.' ID                             { $$ = combine_binary($1, $2, $3); }
	| postfix_expression PTR_OP ID                          { $$ = combine_binary($1, $2, $3); }
	| postfix_expression INC_OP                             { $$ = combine_binary(NULL, $2, $1); }
	| postfix_expression DEC_OP                             { $$ = combine_binary(NULL, $2, $1); }
	;

/* function call arguments */
argument_expression_list
	: assignment_expression                                 { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| argument_expression_list ',' assignment_expression    { $$ = $1; add_to_ast_list($$, $3); f($2); }
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression                               { $$ = combine_binary($1, $2, NULL); }
	| DEC_OP unary_expression                               { $$ = combine_binary($1, $2, NULL); }
	| unary_operator cast_expression                        { $$ = combine_unary($1, $2); }
	| SIZEOF unary_expression                               { $$ = combine_unary($1, $2); }
	| SIZEOF '(' type_name ')'                              { $$ = combine_unary($1, $3); f2($2, $4); }
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression             { $$ = combine_binary($2, new_node(NODE_CAST), $4); f2($1, $3); }
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression         { $$ = combine_binary($1, $2, $3); }
	| multiplicative_expression '/' cast_expression         { $$ = combine_binary($1, $2, $3); }
	| multiplicative_expression '%' cast_expression         { $$ = combine_binary($1, $2, $3); }
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
	| inclusive_or_expression '|' exclusive_or_expression   { $$ = combine_binary($1, $2, $3); }
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
	| logical_or_expression '?' expression ':' conditional_expression
	                { $$ = combine_binary($1, $2, combine_binary($3, $4, $5)); }
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression    { $$ = combine_binary($1, $2, $3); }
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

/* assignment [, assignment ...] */
expression
	: assignment_expression
	| expression ',' assignment_expression                  { $$ = combine_binary($1, $2, $3); }
	;

constant_expression
	: conditional_expression
	;

/* variable(s) declaration */
declaration
	: declaration_specifiers ';'
	            {
	                $$ = combine_binary($1, new_node(NODE_DECL), NULL);
	                f($2);
	                check_for_typedef($$);
                }
	| declaration_specifiers init_declarator_list ';'
	            {
	                $$ = combine_binary($1, new_node(NODE_DECL), $2);
	                f($3);
	                check_for_typedef($$);
                }
	;

/* const/auto/register/volatile/etc, type, and pointer-ness */
declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers        { $$ = combine_unary($1, $2); }
	| type_specifier
	| type_specifier declaration_specifiers                 { $$ = combine_unary($1, $2); }
	| type_qualifier
	| type_qualifier declaration_specifiers                 { $$ = combine_unary($1, $2); }
	;

/* list of name [= initializer] */
init_declarator_list
	: init_declarator                               { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| init_declarator_list ',' init_declarator      { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

/* name [= initializer] */
init_declarator
	: declarator                    { $$ = combine_binary($1, new_token('='), NULL); }
	| declarator '=' initializer    { $$ = combine_binary($1, $2, $3); }
	;

storage_class_specifier
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union ID '{' struct_declaration_list '}'        { $$ = combine_binary($2, $1, $4); f2($3, $5); }
	| struct_or_union '{' struct_declaration_list '}'           { $$ = combine_binary(NULL, $1, $3); f2($2, $4); }
	| struct_or_union ID                                        { $$ = combine_binary($2, $1, NULL); }
	;

struct_or_union
	: STRUCT
	| UNION
	;

/* the list of members in a struct */
struct_declaration_list
	: struct_declaration                            { $$ = new_node(NODE_STRUCT_MEMBERS); add_to_ast_list($$, $1); }
	| struct_declaration_list struct_declaration    { add_to_ast_list($1, $2); $$ = $1; }
	;

/* one item in a struct member list (might be one type and multiple named fields) */
struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'   { $$ = combine_binary($1, $2, NULL); f($3); }
	;

/* type and type-flavors (const / volatile) */
specifier_qualifier_list
	: type_specifier specifier_qualifier_list       { $$ = combine_unary($1, $2); }
	| type_specifier
	| type_qualifier specifier_qualifier_list       { $$ = combine_unary($1, $2); }
	| type_qualifier
	;

/* list of names of field names / bitfield info */
struct_declarator_list
	: struct_declarator                             { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| struct_declarator_list ',' struct_declarator  { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

/* field name / bitfield info */
struct_declarator
	: declarator
	| ':' constant_expression               { $$ = combine_binary(NULL, new_node(NODE_BITFIELD), $2); f($1); }
	| declarator ':' constant_expression    { $$ = combine_binary($1, new_node(NODE_BITFIELD), $3); f($2); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}'          { $$ = combine_binary(NULL, $1, $3); f2($2, $4); }
	| ENUM ID '{' enumerator_list '}'       { $$ = combine_binary($2, $1, $4); f2($3, $5); }
	| ENUM ID                               { $$ = combine_binary($2, $1, NULL); }
	;

enumerator_list
	: enumerator                            { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| enumerator_list ',' enumerator        { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

enumerator
	: ID
	| ID '=' constant_expression            { $$ = combine_unary($1, $3); f($2); }
	;

/* const/volatile */
type_qualifier
	: CONST
	| VOLATILE
	;

/* [pointer] name */
declarator
	: pointer direct_declarator
	| direct_declarator
	;

/* name and possible decoration such as array size or function pointer info */
direct_declarator
	: ID
	| '(' declarator ')'                                { $$ = combine_unary($1, $2); f($3); }
	| direct_declarator '[' constant_expression ']'     { $$ = combine_binary($1, $2, $3); f($4); }
	| direct_declarator '[' ']'                         { $$ = combine_binary($1, $2, NULL); f($3); }
	| direct_declarator '(' parameter_type_list ')'     { $$ = combine_binary($1, $2, $3); f($4); }
	| direct_declarator '(' identifier_list ')'         { $$ = combine_binary($1, $2, $3); f($4); }
	| direct_declarator '(' ')'                         { $$ = combine_binary($1, $2, NULL); f($3); }
	;

pointer
	: '*'
	| '*' type_qualifier_list           { $$ = combine_binary($2, $1, NULL); }
	| '*' pointer                       { $$ = combine_binary(NULL, $1, $2); }
	| '*' type_qualifier_list pointer   { $$ = combine_binary($2, $1, $3); }
	;

/* const/volatile */
type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier    { $$ = combine_unary($1, $2); }
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS                   { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

parameter_list
	: parameter_declaration                         { $$ = new_node(NODE_PARAM_LIST); add_to_ast_list($$, $1); }
	| parameter_list ',' parameter_declaration      { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

/* "int foo" */
parameter_declaration
	: declaration_specifiers declarator             { $$ = combine_binary(NULL, $1, $2); }
	/* "int (*foo)(int, char x)" */
	| declaration_specifiers abstract_declarator    { $$ = combine_binary(NULL, $1, $2); }
	/* "int *" */
	| declaration_specifiers
	;

identifier_list
	: ID                        { $$ = new_node(NODE_ID_LIST); add_to_ast_list($$, $1); }
	| identifier_list ',' ID    { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

/* something that can be used in sizeof() or a cast */
type_name
	/* type */
	: specifier_qualifier_list
	/* function pointer */
	| specifier_qualifier_list abstract_declarator   { $$ = combine_binary($1, new_node(NODE_ABSTRACT_TYPE), $2); }
	;

/* a function pointer, e.g. "int (*foo)(int x, int, char *s)" */
abstract_declarator
	: pointer                               { $$ = combine_unary(new_node(NODE_ABSTRACT), $1); }
	| direct_abstract_declarator            { $$ = combine_unary(new_node(NODE_ABSTRACT), $1); }
	| pointer direct_abstract_declarator    { $$ = combine_unary(new_node(NODE_ABSTRACT), combine_unary($1, $2)); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'                               { $$ = $2; f2($1, $3); }
	| '[' ']'                                                   { $$ = $1; f($2); }
	| '[' constant_expression ']'                               { $$ = combine_unary($1, $2); f($3); }
	| direct_abstract_declarator '[' ']'                        { $$ = combine_binary($1, $2, NULL); f($3); }
	| direct_abstract_declarator '[' constant_expression ']'    { $$ = combine_binary($1, $2, $3); f($4); }
	| '(' ')'                                                   { $$ = NULL; f2($1, $2); }
	| '(' parameter_type_list ')'                               { $$ = $2; f2($1, $3); }
	| direct_abstract_declarator '(' ')'                        { $$ = combine_unary($2, $1); f($3); }
	| direct_abstract_declarator '(' parameter_type_list ')'    { $$ = combine_binary($1, $2, $3); f($4); }
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'          { $$ = $2; f2($1, $3); }
	| '{' initializer_list ',' '}'      { $$ = $2; f2($1, $3); f($4); }
	;

initializer_list
	: initializer                       { $$ = new_node(NODE_INIT_LIST); add_to_ast_list($$, $1); }
	| initializer_list ',' initializer  { $$ = $1; add_to_ast_list($1, $3); f($2); }
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: ID ':' statement                          { $$ = combine_unary($1, $3); f($2); }
	| CASE constant_expression ':' statement    { $$ = combine_binary($2, $1, $4); f($3); }
	| DEFAULT ':' statement                     { $$ = combine_unary($1, $3); f($2); }
	;

/* braced list of statements */
/* todo: consider allowing decls amidst other statements */
compound_statement
	: '{' '}'
            { $$ = combine_binary(NULL, new_node(NODE_COMPOUND_STATEMENT), NULL); f2($1, $2); }
	| '{' statement_list '}'
	        { $$ = combine_binary(NULL, new_node(NODE_COMPOUND_STATEMENT), $2); f2($1, $3); }
	| '{' declaration_list '}'
	        { $$ = combine_binary($2, new_node(NODE_COMPOUND_STATEMENT), NULL); f2($1, $3); }
	| '{' declaration_list statement_list '}'
	        { $$ = combine_binary($2, new_node(NODE_COMPOUND_STATEMENT), $3); f2($1, $4); }
	;

declaration_list
	: declaration                       { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| declaration_list declaration      { add_to_ast_list($1, $2); $$ = $1; }
	;

statement_list
	: statement                     { $$ = new_node(NODE_LIST); add_to_ast_list($$, $1); }
	| statement_list statement      { $$ = $1; add_to_ast_list($$, $2); }
	;

expression_statement
	: ';'                   { $$ = NULL; f($1); }
	| expression ';'        { $$ = $1; f($2); }
	;

selection_statement
	: IF '(' expression ')' statement                       { $$ = combine_binary($3, $1, $5); f2($2, $4); }
	| IF '(' expression ')' statement ELSE statement        {
	                                                            $$ = combine_binary($3, $1, combine_binary($5, $6, $7));
                                                                f2($2, $4);
                                                            }
	| SWITCH '(' expression ')' statement                   { $$ = combine_binary($3, $1, $5); f2($2, $4); }
	;

iteration_statement
	: WHILE '(' expression ')' statement                { $$ = combine_binary($3, $1, $5); f2($2, $4); }
	| DO statement WHILE '(' expression ')' ';'         { $$ = combine_binary($5, $1, $2); f2($3, $4); f2($6, $7); }
	| FOR '(' expression_statement expression_statement ')' statement
            {
                $$ = combine_binary(new_node(NODE_LIST), $1, $6);
                add_to_ast_list($$->left, $3);
                add_to_ast_list($$->left, $4);
                f2($2, $5);
            }
	| FOR '(' expression_statement expression_statement expression ')' statement
            {
                $$ = combine_binary(new_node(NODE_LIST), $1, $7);
                add_to_ast_list($$->left, $3);
                add_to_ast_list($$->left, $4);
                add_to_ast_list($$->left, $5);
                f2($2, $6);
            }
	;

jump_statement
	: GOTO ID ';'                   { $$ = combine_unary($1, $2); f($3); }
	| CONTINUE ';'                  { $$ = $1; f($2); }
	| BREAK ';'                     { $$ = $1; f($2); }
	| RETURN ';'                    { $$ = $1; f($2); }
	| RETURN expression ';'         { $$ = combine_unary($1, $2); f($3); }
	;

translation_unit
	: external_declaration                      {
	                                                $$ = new_node(NODE_TRANSLATION_UNIT);
	                                                add_to_ast_list($$, $1);
	                                                cc_parse_result = $$;
                                                }
	| translation_unit external_declaration     {
                                                    $$ = $1;
                                                    add_to_ast_list($1, $2);
	                                                cc_parse_result = $$;
                                                }
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
	    { die("Un-prototyped function declarations not supported"); }
	| declaration_specifiers declarator compound_statement
	    { $$ = combine_binary($1, new_node(NODE_FUNCTION), $2); add_to_ast_list($$, $3); }
	| declarator declaration_list compound_statement
	    { die("Un-prototyped function declarations not supported"); }
	| declarator compound_statement
	    { $$ = combine_binary(NULL, new_node(NODE_FUNCTION), $1); add_to_ast_list($$, $2); }
	;

%%
