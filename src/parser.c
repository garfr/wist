#include <stdlib.h>

#include <wist/parser.h>

#define INIT_ARG_ARR 2
#define INIT_PATT_ARR 1
#define INIT_FUN_ARG_ARR 2

/* === PROTOTYPES === */

static UAstExpr *parse_fexpr(WistParser *parser, UAst *ast);
static UAstExpr *parse_aexpr(WistParser *p, UAst *ast);
static UAstExpr *parse_lexpr(WistParser *p, UAst *ast);
static UAstExpr *parse_expr(WistParser *p, UAst *ast);

static UAstPatt *parse_patt(WistParser *p, UAst *ast);

static UAstDecl *parse_decl(WistParser *p, UAst *ast);

static void bump(WistParser *parser);
static bool expect(WistParser *parser, WistTokenType t);
static bool expect_with_tok(WistParser *parser, WistTokenType t, WistToken *out);
static bool check(WistParser *parser, WistTokenType t);
static bool check_with_span(WistParser *parser, WistTokenType t, WistSpan *span);

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
    uast->root = parse_decl(parser, uast);
}

void
wist_parser_destroy(WistParser *parser)
{
    (void) parser;
    return;
}

/* === PRIVATE FUNCTIONS === */

static UAstDecl *
parse_decl(WistParser *p, UAst *ast)
{
    WistToken sym_tok;
    expect_with_tok(p, WIST_TOK_SYM, &sym_tok);

    UAstPatt **args = WIST_NEW_ARR(UAstPatt *, INIT_FUN_ARG_ARR);

    size_t nargs = 0;
    size_t aargs = INIT_FUN_ARG_ARR;

    while (!check(p, WIST_TOK_EQ))
    {
        if (nargs + 1 > aargs)
        {
            aargs *= 2;
            args = WIST_REALLOC(UAstPatt *, args, INIT_FUN_ARG_ARR);
        }
    
        args[nargs++] = parse_patt(p, ast);
    }
    
    if (aargs != nargs && nargs != 0)
    {
        args = WIST_REALLOC(UAstPatt *, args, nargs);
    }

    UAstExpr *body = parse_expr(p, ast);
    WistSpan loc = wist_combine_span(&p->lex->spans, sym_tok.loc, body->loc);
    return uast_create_bind_decl(ast, loc, sym_tok.sym, args, nargs, body);
}

static UAstPatt *
parse_patt(WistParser *p, 
           UAst *ast)
{
    UAstPatt *patt;
    switch (p->tok.t)
    {
        case WIST_TOK_SYM:
            patt = uast_create_var_patt(ast, p->tok.loc, p->tok.sym);
            bump(p);
            return patt;
        case WIST_TOK_UNDERSCORE:
            patt = uast_create_wildcard_patt(ast, p->tok.loc);
            bump(p);
            return patt;
        default:
            printf("Error: not pattern.\n");
            exit(EXIT_FAILURE);
            return NULL;
    }
}

static UAstExpr *
parse_expr(WistParser *p, 
           UAst *ast)
{
    return parse_lexpr(p, ast);
}

static UAstExpr *
parse_aexpr(WistParser *p, 
            UAst *ast)
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
        expect(p, WIST_TOK_RPAREN);
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
parse_fexpr(WistParser *p, 
            UAst *ast)
{
    UAstExpr *fun, *arg;
    fun = parse_aexpr(p, ast);
    UAstExpr **args = WIST_NEW_ARR(UAstExpr *, INIT_ARG_ARR);
    size_t nargs = 0;
    size_t aargs = INIT_ARG_ARR;
    while ((arg = parse_aexpr(p, ast)) != NULL)
    {
        if (nargs + 1 > aargs)
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

static UAstExpr *
parse_lexpr(WistParser *p, 
            UAst *ast)
{
    WistSpan slash_loc;
    if (!check_with_span(p, WIST_TOK_BSLASH, &slash_loc))
    {
        return parse_fexpr(p, ast);
    }
    
    UAstPatt **patts = WIST_NEW_ARR(UAstPatt *, INIT_PATT_ARR);
    size_t apatts = INIT_PATT_ARR;
    size_t npatts = 0;
    patts[npatts++] = parse_patt(p, ast);
    
    while (check(p, WIST_TOK_COMMA))
    {
        if (npatts + 1 > apatts)
        {
            apatts *= 2;
            patts = WIST_REALLOC(UAstPatt *, patts, apatts);
        }
        patts[npatts++] = parse_patt(p, ast);
    }
    
    expect(p, WIST_TOK_ARROW);
    if (npatts != apatts)
    {
        patts = WIST_REALLOC(UAstPatt *, patts, npatts);
    }
    
    UAstExpr *body = parse_expr(p, ast);
    
    WistSpan span = wist_combine_span(&p->lex->spans, slash_loc, body->loc);
    UAstExpr *expr = uast_create_lam_expr(ast, span, patts, npatts, body);
    return expr;
}

static void
bump(WistParser *parser)
{
    parser->tok = wist_lexer_next(parser->lex);
}

static bool 
expect(WistParser *parser, 
       WistTokenType t)
{
    if (parser->tok.t == t)
    {
        bump(parser);
        return true;
    }
    printf("Error expected: %d\n", t);
    exit(EXIT_FAILURE);
    return false;
}

static bool 
expect_with_tok(WistParser *parser,
                  WistTokenType t, 
                  WistToken *out)
{
    if (parser->tok.t == t)
    {
        *out = parser->tok;
        bump(parser);
        return true;
    }
    printf("Error expected: %d\n", t);
    exit(EXIT_FAILURE);
    return false;
}

static bool 
check(WistParser *parser, 
      WistTokenType t)
{
    if (parser->tok.t == t)
    {
        bump(parser);

        return true;
    }
    return false;
}

static bool 
check_with_span(WistParser *parser, 
                WistTokenType t, 
                WistSpan *span)
{
    if (parser->tok.t == t)
    {
        *span = parser->tok.loc;
        bump(parser);
        return true;
    }
    return false;
}
