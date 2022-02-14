#include <stdlib.h>
#include <inttypes.h>

#include <wist/parser.h>
#include <wist/index.h>

#define INIT_ARG_ARR 2
#define INIT_PATT_ARR 1
#define INIT_FUN_ARG_ARR 2
#define INIT_DECLS_ARR 8
#define INIT_TUPLE_ARR 2
/* === PROTOTYPES === */

static UAstExpr *parse_fexpr(WistParser *parser, UAst *ast);
static UAstExpr *parse_aexpr(WistParser *p, UAst *ast);
static UAstExpr *parse_lexpr(WistParser *p, UAst *ast);
static UAstExpr *parse_expr(WistParser *p, UAst *ast);

static UAstType *parse_type(WistParser *p, UAst *uast);
static UAstType *parse_atype(WistParser *p, UAst *ast);

static UAstPatt *parse_patt(WistParser *p, UAst *ast);

static UAstDecl *parse_decl(WistParser *p, UAst *ast);

static void bump(WistParser *parser);
static bool expect(WistParser *parser, WistTokenType t);
static bool expect_with_tok(WistParser *parser, WistTokenType t, WistToken *out);
static bool check(WistParser *parser, WistTokenType t);
static bool check_with_span(WistParser *parser, WistTokenType t, WistSpan *span);
static WistError *err_on_span(WistParser *parser, WistErrorCode code, 
                              const char *msg);

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
    size_t adecls = INIT_DECLS_ARR;
    uast->roots = WIST_NEW_ARR(UAstDecl *, adecls);
    uast->ndecls = 0;
    while (!check(parser, WIST_TOK_EOF))
    {    
        if (uast->ndecls + 1 >= adecls)
        {
            adecls *= 2;
            uast->roots = WIST_REALLOC(UAstDecl *, uast->roots, adecls);
        }
        uast->roots[uast->ndecls++] = parse_decl(parser, uast);
        if (uast->roots[uast->ndecls - 1]->t == UAST_DECL_ERR)
        {
            bump(parser);
        }
    }    
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
    while (check(p, WIST_TOK_NL_SCOLON))
        ; /* DO NOTHING */
    UAstDecl *decl;
    WistToken sym_tok;
    if (!expect_with_tok(p, WIST_TOK_SYM, &sym_tok))
    {
        err_on_span(p, WIST_ERROR_EXPECTED_DECL, 
                    "expected declaration, found %s"); 
        return uast_create_err_decl(ast, p->tok.loc);
    }

    if (check(p, WIST_TOK_COLON))
    {
        UAstType *type = parse_type(p, ast);
        WistSpan loc = wist_combine_span(p->lex->spans, sym_tok.loc, type->loc);
        decl = uast_create_type_decl(ast, loc, sym_tok.sym, type); 
    }
    else {
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
            if (args[nargs-1]->t == UAST_PATT_ERR)
            {
                return uast_create_err_decl(ast, p->tok.loc);
            }
        }
    
        if (aargs != nargs && nargs != 0)
        {
            args = WIST_REALLOC(UAstPatt *, args, nargs);
        }

        UAstExpr *body = parse_expr(p, ast);
    
        WistSpan loc = wist_combine_span(p->lex->spans, sym_tok.loc, body->loc);
        decl = uast_create_bind_decl(ast, loc, sym_tok.sym, args, nargs, body);
    }
    
    if (!check(p, WIST_TOK_NL_SCOLON) && !check(p, WIST_TOK_SCOLON) && !check(p, WIST_TOK_EOF))
    {
        err_on_span(p, WIST_ERROR_EXPECTED_END_OF_DECL, 
                    "expected newline or ';' after declaration, found %s");
        return uast_create_err_decl(ast, p->tok.loc);
    }
    
    return decl;
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
        case WIST_TOK_LPAREN:
        {
            WistSpan first_paren = p->tok.loc;
            bump(p);
            UAstPatt *patt = parse_patt(p, ast);
            WistToken last_tok;
            if (check(p, WIST_TOK_COMMA))
            {
                UAstPatt **patts = WIST_NEW_ARR(UAstPatt *, INIT_TUPLE_ARR);
                size_t apatts = INIT_TUPLE_ARR;
                size_t npatts = 1;
                patts[0] = patt;
                patts[npatts++] = parse_patt(p, ast);
                
                while (check(p, WIST_TOK_COMMA))
                {
                    if (npatts + 1 >= apatts)
                    {
                        apatts *= 2;
                        patts = WIST_REALLOC(UAstPatt *, patts, apatts);
                    }
                    patts[npatts++] = parse_patt(p, ast);
                }
                if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_tok))
                {
                    err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER,
                            "expected closing delimiter ')', found %s");
                    return uast_create_err_patt(ast, p->tok.loc);
                }
                WistSpan span = wist_combine_span(p->lex->spans, first_paren, 
                                                  last_tok.loc);
                return uast_create_tuple_patt(ast, span, patts, npatts);
            }
            if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_tok))
            {
                err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER,
                            "expected closing delimiter ')', found %s");
                return uast_create_err_patt(ast, p->tok.loc);
            }
            WistSpan span = wist_combine_span(p->lex->spans, first_paren, 
                                              last_tok.loc);
            return uast_create_paren_patt(ast, span, patt);
        }
        case WIST_TOK_UNDERSCORE:
            patt = uast_create_wildcard_patt(ast, p->tok.loc);
            bump(p);
            return patt;
        default: {
            err_on_span(p, WIST_ERROR_EXPECTED_PATTERN, 
                        "expected pattern, found %s");
            patt = uast_create_err_patt(ast, p->tok.loc);
            bump(p);
            return patt;
        }
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
        if (check(p, WIST_TOK_COMMA))
        {
            UAstExpr **exprs = WIST_NEW_ARR(UAstExpr *, INIT_TUPLE_ARR);
            size_t aexprs = INIT_TUPLE_ARR;
            size_t nexprs = 1;
            exprs[0] = expr;
            
            exprs[nexprs++] = parse_expr(p, ast);
            while (check(p, WIST_TOK_COMMA))
            {
                if (nexprs + 1 >= aexprs)
                {
                    aexprs *= 2;
                    exprs = WIST_REALLOC(UAstExpr *, exprs, aexprs);
                }
                
                exprs[nexprs++] = parse_expr(p, ast);
            }
            WistToken last_tok;
            if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_tok))
            {
                err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER,
                            "expected closing delimiter ')', found %s");
                return uast_create_err_expr(ast, p->tok.loc);
            }
            WistSpan full_span = wist_combine_span(p->lex->spans, first_paren, 
                                                   last_tok.loc);
            return uast_create_tuple_expr(ast, full_span, exprs, nexprs);
        }
        WistToken last_tok;
        if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_tok))
        {
            err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER, 
                        "expected closing delimiter ')', found %s");
            return uast_create_err_expr(ast, p->tok.loc);
        }
        WistSpan last_paren = last_tok.loc;
        WistSpan full_span = wist_combine_span(p->lex->spans, first_paren, last_paren);
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
    if (fun == NULL)
    {
        err_on_span(p, WIST_ERROR_EXPECTED_EXPR, "expected expression, found %s");
        return uast_create_err_expr(ast, p->tok.loc);
    }
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
    
    WistSpan span = wist_combine_span(p->lex->spans, fun->loc, 
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
    
    if (!expect(p, WIST_TOK_ARROW))
    {
        err_on_span(p, WIST_ERROR_EXPECTED_LAMBDA_ARROW, 
                    "expected '->' after lambda parameters, found %s");
        return uast_create_err_expr(ast, p->tok.loc);
    }
    if (npatts != apatts)
    {
        patts = WIST_REALLOC(UAstPatt *, patts, npatts);
    }
    
    UAstExpr *body = parse_expr(p, ast);
    
    WistSpan span = wist_combine_span(p->lex->spans, slash_loc, body->loc);
    UAstExpr *expr = uast_create_lam_expr(ast, span, patts, npatts, body);
    return expr;
}

static UAstType *
parse_type(WistParser *p, 
           UAst *ast)
{
    UAstType *lhs;
    lhs = parse_atype(p, ast);
    if (lhs == NULL)
    {
        err_on_span(p, WIST_ERROR_EXPECTED_TYPE, "expected type, found %s");
        lhs = uast_create_err_type(ast, p->tok.loc);
        bump(p);
        return lhs;
    }
    
    if (check(p, WIST_TOK_ARROW))
    {
        UAstType *rhs = parse_type(p, ast);
        WistSpan span = wist_combine_span(p->lex->spans, lhs->loc, rhs->loc);
        return uast_create_fun_type(ast, span, lhs, rhs);
    }
    
    return lhs;
}

static UAstType *
parse_atype(WistParser *p, 
            UAst *ast)
{
    UAstType *type;
    switch (p->tok.t)
    {
        case WIST_TOK_SYM:
            type = uast_create_var_type(ast, p->tok.loc,  p->tok.sym);
            bump(p);
            return type;
        case WIST_TOK_LPAREN: {
            WistSpan first_paren = p->tok.loc;
            bump(p);
            UAstType *inner = parse_type(p, ast);
            WistToken last_paren;
            if (check(p, WIST_TOK_COMMA))
            {
                UAstType **types = WIST_NEW_ARR(UAstType *, INIT_TUPLE_ARR);
                size_t atypes = INIT_TUPLE_ARR;
                size_t ntypes = 1;
                
                types[0] = inner;
                
                types[ntypes++] = parse_type(p, ast);
                
                while (check(p, WIST_TOK_COMMA))
                {
                    if (ntypes + 1 >= atypes)
                    {
                        atypes *= 2;
                        types = WIST_REALLOC(UAstType *, types, atypes);
                    }
                    
                    types[ntypes++] = parse_type(p, ast);
                }
                
                if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_paren))
                {
                    err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER,
                                "expected closing delimiter ')', found %s");
                    return uast_create_err_type(ast, p->tok.loc);
                }
                
                WistSpan loc = wist_combine_span(p->lex->spans, 
                                        first_paren, last_paren.loc);
                return uast_create_tuple_type(ast, loc, types, ntypes);
            }
            if (!expect_with_tok(p, WIST_TOK_RPAREN, &last_paren))
            {
                err_on_span(p, WIST_ERROR_EXPECTED_CLOSE_DELIMITER, 
                            "expected closing delimiter ')', found %s");
                return uast_create_err_type(ast, p->tok.loc);
            }
            WistSpan span = wist_combine_span(p->lex->spans, first_paren, 
                                              last_paren.loc);
            return uast_create_paren_type(ast, span, inner);
        }
        default:
            return NULL;
    }    
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

static WistError *
err_on_span(WistParser *parser, 
            WistErrorCode code, 
            const char *msg)
{
    WistError *err = wist_add_error(parser->err);
    WistStrRef ref = index_get_span(parser->err->index, parser->err->spans, 
                                    &parser->tok.loc);
    WistMultiSpan mspan = wist_multispan_create(1, parser->tok.loc);
    wist_fill_error(err, WIST_ERROR_ERR, code, wist_format(msg, &ref), mspan);
    return err;
}
