#ifndef WIST_UAST_H
#define WIST_UAST_H

#include <wist/span.h>
#include <wist/sym.h>

typedef enum
{
    UAST_PATT_VAR,
    UAST_PATT_WILDCARD,
    UAST_PATT_ERR,
} UAstPattType;

typedef struct
{
    UAstPattType t;
    WistSpan loc;
    union
    {
        WistSym *var;
    };
} UAstPatt;

typedef enum
{
    UAST_EXPR_INT,
    UAST_EXPR_VAR,
    UAST_EXPR_APP,
    UAST_EXPR_PAREN,
    UAST_EXPR_LAM,
    UAST_EXPR_ERR,
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
        struct
        {
            UAstPatt **patts;
            size_t npatts;
            struct UAstExpr *body;
        } lam;
    };
} UAstExpr;

typedef enum
{
    UAST_DECL_BIND,
    UAST_DECL_ERR,
} UAstDeclType;

typedef struct
{
    UAstDeclType t;
    WistSpan loc;
    union
    {
        struct
        {
            WistSym *name;
            UAstPatt **args;
            size_t nargs;
            UAstExpr *body;
        } bind;
    };
} UAstDecl;

#define WIST_OBJPOOL_TYPE UAstExpr
#define WIST_OBJPOOL_FUN_PREFIX uast_expr_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstExpr
#define WIST_OBJPOOL_HEADER
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstPatt
#define WIST_OBJPOOL_FUN_PREFIX uast_patt_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstPatt
#define WIST_OBJPOOL_HEADER
#include <wist/objpool.c.h>

#define WIST_OBJPOOL_TYPE UAstDecl
#define WIST_OBJPOOL_FUN_PREFIX uast_decl_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstDecl
#define WIST_OBJPOOL_HEADER
#include <wist/objpool.c.h>

typedef struct
{
    UAstExprPool exprs;
    UAstPattPool patts;
    UAstDeclPool decls;
    UAstDecl **roots;
    size_t ndecls;
} UAst;

void uast_create(UAst *uast);
void uast_destroy(UAst *uast);

UAstExpr *uast_create_expr(UAst *uast);
UAstExpr *uast_create_var_expr(UAst *uast, WistSpan loc, WistSym *sym);
UAstExpr *uast_create_int_expr(UAst *uast, WistSpan loc, int64_t i);
UAstExpr *uast_create_app_expr(UAst *uast, WistSpan loc, UAstExpr *fun, UAstExpr **args, size_t nargs);
UAstExpr *uast_create_paren_expr(UAst *uast, WistSpan loc, UAstExpr *subexpr);
UAstExpr *uast_create_lam_expr(UAst *uast, WistSpan loc, UAstPatt **patts, size_t npatts, UAstExpr *body);
UAstExpr *uast_create_err_expr(UAst *uast, WistSpan err);

UAstPatt *uast_create_patt(UAst *uast);
UAstPatt *uast_create_err_patt(UAst *uast, WistSpan loc);
UAstPatt *uast_create_wildcard_patt(UAst *uast, WistSpan loc);
UAstPatt *uast_create_var_patt(UAst *uast, WistSpan loc, WistSym *var);

UAstDecl *uast_create_decl(UAst *uast);
UAstDecl *uast_create_bind_decl(UAst *uast, WistSpan loc, WistSym *name, 
                                UAstPatt **patts, size_t npatts, UAstExpr *body);
UAstDecl *uast_create_err_decl(UAst *uast, WistSpan loc);
             
void uast_print_expr(UAstExpr *expr);
void uast_print_patt(UAstPatt *patt);
void uast_print_decl(UAstDecl *decl);

#endif /* WIST_UAST_H */
