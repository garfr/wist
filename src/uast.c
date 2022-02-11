#include <inttypes.h>

#include <wist/uast.h>

/* === PROTOTYPES === */

static void uast_print_expr_indent(UAstExpr *expr, int indent);
static void expr_destroy(UAstExpr *expr);

/* === PUBLIC FUNCTIONS === */

#define WIST_OBJPOOL_TYPE UAstExpr
#define WIST_OBJPOOL_FUN_PREFIX uast_expr_pool
#define WIST_OBJPOOL_TYPE_PREFIX UAstExpr
#define WIST_OBJPOOL_IMPLEMENTATION
#define WIST_OBJPOOL_DESTRUCTOR expr_destroy
#include <wist/objpool.c.h>

void
uast_create(UAst *out)
{
    uast_expr_pool_create(&out->exprs);
}

void
uast_destroy(UAst *uast)
{
    uast_expr_pool_destroy(&uast->exprs);
}

UAstExpr *
uast_create_expr(UAst *uast)
{
    return uast_expr_pool_alloc(&uast->exprs);
}

const char *
uast_type_to_string[] =
{
    [UAST_EXPR_VAR] = "Var",
    [UAST_EXPR_INT] = "Int",
    [UAST_EXPR_APP] = "Application",
};

void
uast_print_expr(UAstExpr *expr)
{
    uast_print_expr_indent(expr, 0);
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
uast_create_paren_expr(UAst *uast, WistSpan loc, UAstExpr *subexpr);

/* === PRIVATE FUNCTIONS === */

static void
uast_print_expr_indent(UAstExpr *expr, int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }
    printf("%s", uast_type_to_string[expr->t]);
    switch (expr->t) {
    case UAST_EXPR_VAR:
        printf(" : '%.*s'", (int) expr->var->str.len,
               (char *) expr->var->str.str);
        break;
    case UAST_EXPR_INT:
        printf(" : %" PRId64, expr->i);
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
