#ifndef WIST_UAST_H
#define WIST_UAST_H

#include <wist/span.h>
#include <wist/sym.h>

typedef enum
{
    UAST_EXPR_INT,
    UAST_EXPR_VAR,
    UAST_EXPR_APP,
    UAST_EXPR_PAREN,
} UAstExprType;

typedef struct UAstExpr
{
    UAstExprType t;
    WistSpan loc;
    union
    {
        WistSym *var;
        int64_t i;
        struct
        {
            struct UAstExpr *fun;
            struct UAstExpr **args;
            size_t nargs;
        } app;
        struct UAstExpr *paren;
    };
} UAstExpr;

#define WIST_OBJPOOL_TYPE UAstExpr
#define WIST_OBJPOOL_FUN_PREFIX uast_expr_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstExpr
#define WIST_OBJPOOL_HEADER
#include <wist/objpool.c.h>

typedef struct
{
    UAstExprPool exprs;
    UAstExpr *root;
} UAst;

void uast_create(UAst *uast);
void uast_destroy(UAst *uast);

UAstExpr *uast_create_expr(UAst *uast);
UAstExpr *uast_create_var_expr(UAst *uast, WistSpan loc, WistSym *sym);
UAstExpr *uast_create_int_expr(UAst *uast, WistSpan loc, int64_t i);
UAstExpr *uast_create_app_expr(UAst *uast, WistSpan loc, UAstExpr *fun, UAstExpr **args, size_t nargs);
UAstExpr *uast_create_paren_expr(UAst *uast, WistSpan loc, UAstExpr *subexpr);

void uast_print_expr(UAstExpr *expr);

#endif /* WIST_UAST_H */
