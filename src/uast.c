#include <inttypes.h>

#include <wist/uast.h>

/* === PROTOTYPES === */

static void uast_print_patt_indent(UAstPatt *patt, int indent);
static void uast_print_expr_indent(UAstExpr *expr, int indent);
static void uast_print_decl_indent(UAstDecl *decl, int indent);

static void expr_destroy(UAstExpr *expr);

/* === PUBLIC FUNCTIONS === */

#define WIST_OBJPOOL_TYPE UAstExpr
#define WIST_OBJPOOL_FUN_PREFIX uast_expr_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstExpr
#define WIST_OBJPOOL_DESTRUCTOR expr_destroy
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstPatt
#define WIST_OBJPOOL_FUN_PREFIX uast_patt_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstPatt
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstDecl
#define WIST_OBJPOOL_FUN_PREFIX uast_decl_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstDecl
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

void
uast_create(UAst *out)
{
    uast_expr_pool_create(&out->exprs);
    uast_patt_pool_create(&out->patts);
    uast_decl_pool_create(&out->decls);
}

void
uast_destroy(UAst *uast)
{
    uast_expr_pool_destroy(&uast->exprs);
    uast_patt_pool_destroy(&uast->patts);
    uast_decl_pool_destroy(&uast->decls);

}

UAstExpr *
uast_create_expr(UAst *uast)
{
    return uast_expr_pool_alloc(&uast->exprs);
}

const char *
uast_expr_type_to_string[] =
{
    [UAST_EXPR_VAR] = "Var",
    [UAST_EXPR_INT] = "Int",
    [UAST_EXPR_APP] = "Application",
    [UAST_EXPR_PAREN] = "Paren",
    [UAST_EXPR_LAM] = "Lambda",
};

const char *
uast_patt_type_to_string[] = 
{
    [UAST_PATT_VAR] = "Var",
    [UAST_PATT_WILDCARD] = "Wildcard",
};

const char *
uast_decl_type_to_string[] =
{
    [UAST_DECL_BIND] = "Bind",
};

void
uast_print_expr(UAstExpr *expr)
{
    uast_print_expr_indent(expr, 0);
    printf("\n");
}

void
uast_print_patt(UAstPatt *patt)
{
    uast_print_patt_indent(patt, 0);
    printf("\n");
}

void 
uast_print_decl(UAstDecl *decl)
{
    uast_print_decl_indent(decl, 0);
    printf("\n");
}

UAstExpr *
uast_create_var_expr(UAst *uast,
                     WistSpan loc,
                     WistSym *sym)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_VAR;
    ret->loc = loc;
    ret->var = sym;
    return ret;
}

UAstExpr *
uast_create_int_expr(UAst *uast, 
                     WistSpan loc, 
                     int64_t i)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_INT;
    ret->loc = loc;
    ret->i = i;
    return ret;
}

UAstExpr *
uast_create_app_expr(UAst *uast,
                     WistSpan loc, 
                     UAstExpr *fun, 
                     UAstExpr **args,
                     size_t nargs)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_APP;
    ret->loc = loc;
    ret->app.fun = fun;
    ret->app.args = args;
    ret->app.nargs = nargs;
    return ret;
}

UAstExpr *
uast_create_paren_expr(UAst *uast, 
                       WistSpan loc, 
                       UAstExpr *subexpr)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_PAREN;
    ret->loc = loc;
    ret->paren = subexpr;
    return ret;
}

UAstExpr *
uast_create_lam_expr(UAst *uast, 
                     WistSpan loc, 
                     UAstPatt **patts, 
                     size_t npatts, 
                     UAstExpr *body)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_LAM;
    ret->loc = loc;
    ret->lam.body = body;
    ret->lam.patts = patts;
    ret->lam.npatts = npatts;
    return ret;
}


UAstPatt *
uast_create_patt(UAst *uast)
{
    return uast_patt_pool_alloc(&uast->patts);
}

UAstPatt *
uast_create_wildcard_patt(UAst *uast, 
                          WistSpan loc)
{
    UAstPatt *patt = uast_create_patt(uast);
    patt->t = UAST_PATT_WILDCARD;
    patt->loc = loc;
    return patt;
}

UAstPatt *
uast_create_var_patt(UAst *uast, 
                     WistSpan loc, 
                     WistSym *var)
{
    UAstPatt *patt = uast_create_patt(uast);
    patt->t = UAST_PATT_VAR;
    patt->loc = loc;
    patt->var = var;
    return patt;
}

UAstDecl *
uast_create_decl(UAst *uast)
{
    return uast_decl_pool_alloc(&uast->decls);
}

UAstDecl *
uast_create_bind_decl(UAst *uast, 
                      WistSpan loc, 
                      WistSym *name, 
                      UAstPatt **patts, 
                      size_t npatts, 
                      UAstExpr *body)
{
    UAstDecl *decl = uast_create_decl(uast);
    decl->t = UAST_DECL_BIND;
    decl->loc = loc;
    decl->bind.name = name;
    decl->bind.args = patts;
    decl->bind.nargs = npatts;
    decl->bind.body = body;
    return decl;
}
                                

/* === PRIVATE FUNCTIONS === */

static void 
uast_print_decl_indent(UAstDecl *decl, 
                       int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
    
    printf("Decl_%s", uast_decl_type_to_string[decl->t]);
    switch (decl->t)
    {
        case UAST_DECL_BIND: {
            printf(" : '%.*s'", (int) decl->bind.name->str.len,
                   (char *) decl->bind.name->str.str);
            for (size_t i = 0; i < decl->bind.nargs; i++)
            {
                printf("\n");
                uast_print_patt_indent(decl->bind.args[i], indent + 1);
            }
            printf("\n");
            uast_print_expr_indent(decl->bind.body, indent + 1);
            break;
        }   
    }
}

static void 
uast_print_patt_indent(UAstPatt *patt, 
                       int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
    
    printf("Patt_%s", uast_patt_type_to_string[patt->t]);
    switch (patt->t)
    {
        case UAST_PATT_VAR:
            printf(" : '%.*s'", (int) patt->var->str.len, patt->var->str.str);
            break;
        case UAST_PATT_WILDCARD:
            break;
    }
}

static void
uast_print_expr_indent(UAstExpr *expr, 
                       int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
    printf("Expr_%s", uast_expr_type_to_string[expr->t]);
    switch (expr->t) {
    case UAST_EXPR_VAR:
        printf(" : '%.*s'", (int) expr->var->str.len,
               (char *) expr->var->str.str);
        break;
    case UAST_EXPR_INT:
        printf(" : %" PRId64, expr->i);
        break;
    case UAST_EXPR_PAREN:
        printf("\n");
        uast_print_expr_indent(expr->paren, indent + 1);
        break;
    case UAST_EXPR_APP:
    {
        printf("\n");
        uast_print_expr_indent(expr->app.fun, indent + 1);
        for (size_t i = 0; i < expr->app.nargs; i++)
        {
            printf("\n");
            uast_print_expr_indent(expr->app.args[i], indent + 1);
        }
        break;
    }
    case UAST_EXPR_LAM:
    {
        for (size_t i = 0; i < expr->lam.npatts; i++)
        {
            printf("\n");
            uast_print_patt_indent(expr->lam.patts[i], indent + 1);
        }
        printf("\n");
        uast_print_expr_indent(expr->lam.body, indent + 1);
        break;
    }
    }
}

static void 
expr_destroy(UAstExpr *expr)
{
    if (expr->t == UAST_EXPR_APP)
    {
        WIST_FREE(expr->app.args);
    }
}
