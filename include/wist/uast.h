#ifndef WIST_UAST_H
#define WIST_UAST_H

#include <wist/span.h>
#include <wist/sym.h>

typedef enum
{
    UAST_PATT_VAR,
    UAST_PATT_WILDCARD,
    UAST_PATT_TUPLE,
    UAST_PATT_ERR,
    UAST_PATT_PAREN,
} UAstPattType;

typedef struct UAstPatt
{
    UAstPattType t;
    WistSpan loc;
    union
    {
        struct
        {
            struct UAstPatt **patts;
            size_t npatts;
        } tuple;
        struct UAstPatt *paren;
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
    UAST_EXPR_TUPLE,
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
        struct
        {
            struct UAstExpr **exprs;
            size_t nexprs;
        } tuple;
    };
} UAstExpr;

typedef enum
{
    UAST_TYPE_VAR,
    UAST_TYPE_FUN,
    UAST_TYPE_ERR,
    UAST_TYPE_PAREN,
    UAST_TYPE_TUPLE,
} UAstTypeType;

typedef struct UAstType
{
    UAstTypeType t;
    WistSpan loc;
    union
    {
        struct
        {
            struct UAstType **types;
            size_t ntypes;
        } tuple;
        struct UAstType *paren;
        WistSym *var;
        struct
        {
            struct UAstType *in;
            struct UAstType *out;
        } fun;
    };
} UAstType;

typedef enum
{
    UAST_DECL_BIND,
    UAST_DECL_TYPE,
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
        struct
        {
            WistSym *name;
            UAstType *type;
        } type;
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

#define WIST_OBJPOOL_TYPE UAstType
#define WIST_OBJPOOL_FUN_PREFIX uast_type_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstType
#define WIST_OBJPOOL_HEADER
#include <wist/objpool.c.h>

typedef struct
{
    UAstExprPool exprs;
    UAstPattPool patts;
    UAstDeclPool decls;
    UAstTypePool types;
    UAstDecl **roots;
    size_t ndecls;
} UAst;

void uast_create(UAst *uast);
void uast_destroy(UAst *uast);

UAstExpr *uast_create_expr(UAst *uast);
UAstExpr *uast_create_var_expr(UAst *uast, WistSpan loc, WistSym *sym);
UAstExpr *uast_create_int_expr(UAst *uast, WistSpan loc, int64_t i);
UAstExpr *uast_create_app_expr(UAst *uast, WistSpan loc, UAstExpr *fun, 
                               UAstExpr **args, size_t nargs);
UAstExpr *uast_create_paren_expr(UAst *uast, WistSpan loc, UAstExpr *subexpr);
UAstExpr *uast_create_lam_expr(UAst *uast, WistSpan loc, UAstPatt **patts, 
                               size_t npatts, UAstExpr *body);
UAstExpr *uast_create_tuple_expr(UAst *uast, WistSpan loc, UAstExpr **exprs, 
                                 size_t nexprs);
UAstExpr *uast_create_err_expr(UAst *uast, WistSpan err);

UAstPatt *uast_create_patt(UAst *uast);
UAstPatt *uast_create_err_patt(UAst *uast, WistSpan loc);
UAstPatt *uast_create_wildcard_patt(UAst *uast, WistSpan loc);
UAstPatt *uast_create_tuple_patt(UAst *uast, WistSpan loc, UAstPatt **patts, size_t npatts);
UAstPatt *uast_create_var_patt(UAst *uast, WistSpan loc, WistSym *var);
UAstPatt *uast_create_paren_patt(UAst *uast, WistSpan loc, UAstPatt *patt);

UAstType *uast_create_type(UAst *uast);
UAstType *uast_create_var_type(UAst *uast, WistSpan loc, WistSym *var);
UAstType *uast_create_fun_type(UAst *uast, WistSpan loc, UAstType *in, 
                               UAstType *out);
UAstType *uast_create_paren_type(UAst *uast, WistSpan loc, UAstType *subtype);
UAstType *uast_create_tuple_type(UAst *uast, WistSpan loc, UAstType **types, 
                                 size_t ntypes);
UAstType *uast_create_err_type(UAst *uast, WistSpan loc);

UAstDecl *uast_create_decl(UAst *uast);
UAstDecl *uast_create_bind_decl(UAst *uast, WistSpan loc, WistSym *name, 
                                UAstPatt **patts, size_t npatts, 
                                UAstExpr *body);
UAstDecl *uast_create_type_decl(UAst *uast, WistSpan loc, WistSym *name, 
                                UAstType *type);
UAstDecl *uast_create_err_decl(UAst *uast, WistSpan loc);
             
         
void uast_print_expr(UAstExpr *expr);
void uast_print_patt(UAstPatt *patt);
void uast_print_decl(UAstDecl *decl);
void uast_print_type(UAstType *type);

#endif /* WIST_UAST_H */
