#include <stdlib.h>

#include <wist/parser.h>

#define INIT_ARG_ARR 2
/* === PROTOTYPES === */

static UAstExpr *parse_fexpr(WistParser *parser, UAst *ast);
static UAstExpr *parse_aexpr(WistParser *p, UAst *ast);
static UAstExpr *parse_expr(WistParser *p, UAst *ast);

static void bump(WistParser *parser);

/* === PUBLIC FUNCTIONS === */

void
wist_parser_create(WistParser *out,
                   WistLexer *lexer,
                   WistErrorEngine *err)
{
    out->lex = lexer;
    out->err = err;
}

void
wist_parser_parse(WistParser *parser,
                  UAst *uast)
{
    bump(parser);
    uast->root = parse_expr(parser, uast);
}

void
wist_parser_destroy(WistParser *parser)
{
    (void) parser;
    return;
}

/* === PRIVATE FUNCTIONS === */

static UAstExpr *
parse_expr(WistParser *p, 
           UAst *ast)
{
    return parse_fexpr(p, ast);
}

static UAstExpr *
parse_aexpr(WistParser *p, UAst *ast)
{
    UAstExpr *expr;
    switch (p->tok.t)
    {
    case WIST_TOK_SYM:
        expr = uast_create_var_expr(ast, p->tok.loc, p->tok.sym);
        bump(p);
        return expr;
    case WIST_TOK_INT:
        expr = uast_create_int_expr(ast, p->tok.loc, p->tok.i);
        bump(p);
        return expr;
    case WIST_TOK_LPAREN: {
        WistSpan first_paren = p->tok.loc;
        bump(p);
        UAstExpr *expr = parse_expr(p, ast);
        if (p->tok.t != WIST_TOK_RPAREN)
        {
            printf("Error: expected closing paren.\n");
            return expr;
        }
        WistSpan last_paren = p->tok.loc;
        bump(p);
        WistSpan full_span = wist_combine_span(&p->lex->spans, first_paren, last_paren);
        return uast_create_paren_expr(ast, full_span, expr);
    }
    default:
        return NULL;
    }
    
}

static UAstExpr *
parse_fexpr(WistParser *p, UAst *ast)
{
    UAstExpr *fun, *arg;
    fun = parse_aexpr(p, ast);
    UAstExpr **args = WIST_NEW_ARR(UAstExpr *, INIT_ARG_ARR);
    size_t nargs = 0;
    size_t aargs = INIT_ARG_ARR;
    while ((arg = parse_aexpr(p, ast)) != NULL)
    {
        if (nargs + 1 < aargs)
        {
            aargs *= 2;
            args = WIST_REALLOC(UAstExpr *, args, aargs);
        }
        args[nargs++] = arg;
    }
    if (nargs == 0)
    {
        return fun;
    }
    if (aargs != nargs)
    {
        args = WIST_REALLOC(UAstExpr *, args, nargs);
    }
    
    WistSpan span = wist_combine_span(&p->lex->spans, fun->loc, 
                                      args[nargs - 1]->loc);
    return uast_create_app_expr(ast, span, fun, args, nargs);
}

static void
bump(WistParser *parser)
{
    parser->tok = wist_lexer_next(parser->lex);
}
