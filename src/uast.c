#include <inttypes.h>

#include <wist/uast.h>

/* === PROTOTYPES === */

static void uast_print_patt_indent(UAstPatt *patt, int indent);
static void uast_print_expr_indent(UAstExpr *expr, int indent);
static void uast_print_decl_indent(UAstDecl *decl, int indent);
static void uast_print_type_indent(UAstType *type, int indent);

static void expr_destroy(UAstExpr *expr);
static void type_destroy(UAstType *type);
static void patt_destroy(UAstPatt *patt);
static void decl_destroy(UAstDecl *decl);

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
#define WIST_OBJPOOL_DESTRUCTOR patt_destroy
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstDecl
#define WIST_OBJPOOL_FUN_PREFIX uast_decl_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstDecl
#define WIST_OBJPOOL_DESTRUCTOR decl_destroy
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstType
#define WIST_OBJPOOL_FUN_PREFIX uast_type_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstType
#define WIST_OBJPOOL_DESTRUCTOR type_destroy
#define WIST_OBJPOOL_IMPLEMENTATION
#include <wist/objpool.c.h>

void
uast_create(UAst *out)
{
    uast_expr_pool_create(&out->exprs);
    uast_patt_pool_create(&out->patts);
    uast_decl_pool_create(&out->decls);
    uast_type_pool_create(&out->types);

}

void
uast_destroy(UAst *uast)
{
    uast_expr_pool_destroy(&uast->exprs);
    uast_patt_pool_destroy(&uast->patts);
    uast_type_pool_destroy(&uast->types);
    uast_decl_pool_destroy(&uast->decls);
    WIST_FREE(uast->roots);
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
    [UAST_EXPR_ERR] = "Err",
    [UAST_EXPR_TUPLE] = "Tuple",
};

const char *
uast_patt_type_to_string[] = 
{
    [UAST_PATT_VAR] = "Var",
    [UAST_PATT_WILDCARD] = "Wildcard",
    [UAST_PATT_ERR] = "Err",
    [UAST_PATT_TUPLE] = "Tuple",
    [UAST_PATT_PAREN] = "Paren",
};

const char *
uast_decl_type_to_string[] =
{
    [UAST_DECL_BIND] = "Bind",
    [UAST_DECL_TYPE] = "Type",
    [UAST_DECL_ERR] = "Err",
};

const char *
uast_type_type_to_string[] =
{
    [UAST_TYPE_FUN] = "Fun",
    [UAST_TYPE_VAR] = "Var",
    [UAST_TYPE_ERR] = "Err",
    [UAST_TYPE_PAREN] = "Paren",
    [UAST_TYPE_TUPLE] = "Tuple",
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

void 
uast_print_type(UAstType *type)
{
    uast_print_type_indent(type, 0);
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
uast_create_err_expr(UAst *uast, 
                     WistSpan loc)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_ERR;
    ret->loc = loc;
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

UAstExpr *
uast_create_tuple_expr(UAst *uast, 
                       WistSpan loc, 
                       UAstExpr **exprs, 
                       size_t nexprs)
{
    UAstExpr *ret = uast_create_expr(uast);
    ret->t = UAST_EXPR_TUPLE;
    ret->loc = loc;
    ret->tuple.exprs = exprs;
    ret->tuple.nexprs = nexprs;
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

UAstPatt *
uast_create_tuple_patt(UAst *uast, 
                       WistSpan loc, 
                       UAstPatt **patts, 
                       size_t npatts)
{
    UAstPatt *patt = uast_create_patt(uast);
    patt->t = UAST_PATT_TUPLE;
    patt->loc = loc;
    patt->tuple.patts = patts;
    patt->tuple.npatts = npatts;
    return patt;
}

UAstPatt *
uast_create_paren_patt(UAst *uast, 
                       WistSpan loc,
                       UAstPatt *subpatt)
{
    UAstPatt *patt = uast_create_patt(uast);
    patt->t = UAST_PATT_PAREN;
    patt->loc = loc;
    patt->paren = subpatt;
    return patt;
}

UAstPatt *
uast_create_err_patt(UAst *uast, 
                     WistSpan loc)
{
    UAstPatt *patt = uast_create_patt(uast);
    patt->t = UAST_PATT_ERR;
    patt->loc = loc;
    return patt;
}

UAstType *
uast_create_type(UAst *uast)
{
    return uast_type_pool_alloc(&uast->types);
}

UAstType *
uast_create_var_type(UAst *uast, 
                     WistSpan loc, 
                     WistSym *var)
{
    UAstType *type = uast_create_type(uast);
    type->t = UAST_TYPE_VAR;
    type->loc = loc;
    type->var = var;
    return type;
}

UAstType *
uast_create_fun_type(UAst *uast, 
                     WistSpan loc, 
                     UAstType *in, 
                     UAstType *out)
{
    UAstType *type = uast_create_type(uast);
    type->t = UAST_TYPE_FUN;
    type->loc = loc;
    type->fun.in = in;
    type->fun.out = out;
    return type;
}

UAstType *
uast_create_tuple_type(UAst *uast, 
                       WistSpan loc, 
                       UAstType **types, 
                       size_t ntypes)
{
    UAstType *type = uast_create_type(uast);
    type->t = UAST_TYPE_TUPLE;
    type->loc = loc;
    type->tuple.types = types;
    type->tuple.ntypes = ntypes;
    return type;
}

UAstType *
uast_create_err_type(UAst *uast, 
                     WistSpan loc)
{
    UAstType *type = uast_create_type(uast);
    type->t = UAST_TYPE_ERR;
    type->loc = loc;
    return type;
}

UAstType *
uast_create_paren_type(UAst *uast, 
                       WistSpan loc,
                       UAstType *subtype)
{
    UAstType *type = uast_create_type(uast);
    type->t = UAST_TYPE_PAREN;
    type->loc = loc;
    type->paren = subtype;
    return type;
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

UAstDecl *
uast_create_type_decl(UAst *uast, 
                      WistSpan loc, 
                      WistSym *name, 
                      UAstType *type)
{
    UAstDecl *decl = uast_create_decl(uast);
    decl->t = UAST_DECL_TYPE;
    decl->loc = loc;
    decl->type.name = name;
    decl->type.type = type;
    return decl;
}
                
UAstDecl *
uast_create_err_decl(UAst *uast, 
                 WistSpan loc)
{
    UAstDecl *decl = uast_create_decl(uast);
    decl->t = UAST_DECL_ERR;
    decl->loc = loc;
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
        case UAST_DECL_TYPE:
            printf(" : '%.*s'\n", (int) decl->type.name->str.len, 
                   (const char *) decl->type.name->str.str);
            uast_print_type_indent(decl->type.type, indent + 1);
            break;
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
        case UAST_PATT_PAREN:
            printf("\n");
            uast_print_patt_indent(patt->paren, indent + 1);
            break;
        case UAST_PATT_TUPLE:
        {
            for (size_t i = 0; i < patt->tuple.npatts; i++)
            {
                printf("\n");
                uast_print_patt_indent(patt->tuple.patts[i], indent + 1);
            }
            break;
        }
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
    case UAST_EXPR_TUPLE: {
        for (size_t i = 0; i < expr->tuple.nexprs; i++)
        {
            printf("\n");
            uast_print_expr_indent(expr->tuple.exprs[i], indent + 1);
        }
        break;
    }
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
uast_print_type_indent(UAstType *type, 
                       int indent)

{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
    printf("Type_%s", uast_type_type_to_string[type->t]);
    switch (type->t)
    {
        case UAST_TYPE_PAREN:
            printf("\n");
            uast_print_type_indent(type->paren, indent + 1);
            break;
        case UAST_TYPE_VAR:
            printf(" : '%.*s'", (int) type->var->str.len, 
                   (const char *) type->var->str.str);
            break;
        case UAST_TYPE_TUPLE: 
        {
            for (size_t i = 0; i < type->tuple.ntypes; i++)
            {
                printf("\n");
                uast_print_type_indent(type->tuple.types[i], indent + 1);
            }
            break;
        }
        case UAST_TYPE_FUN:
            printf("\n");
            uast_print_type_indent(type->fun.in, indent + 1);
            printf("\n");
            uast_print_type_indent(type->fun.out, indent + 1);
            break;
        case UAST_TYPE_ERR:
            break;
    }
}

static void 
expr_destroy(UAstExpr *expr)
{
    switch (expr->t)
    {
        case UAST_EXPR_APP:
            WIST_FREE(expr->app.args);
            break;
        case UAST_EXPR_LAM:
            WIST_FREE(expr->lam.patts);
            break;
        case UAST_EXPR_TUPLE:
            WIST_FREE(expr->tuple.exprs);
            break;
    }
}
static void 
type_destroy(UAstType *type)
{
    switch (type->t)
    {
        case UAST_TYPE_TUPLE:
            WIST_FREE(type->tuple.types);
            break;
    }
}

static void 
patt_destroy(UAstPatt *patt)
{
    switch (patt->t)
    {
        case UAST_PATT_TUPLE:
            WIST_FREE(patt->tuple.patts);
            break;
    }
}

static void 
decl_destroy(UAstDecl *decl)
{
    switch (decl->t)
    {
        case UAST_DECL_BIND:
            WIST_FREE(decl->bind.args);
            break;
    }
}

