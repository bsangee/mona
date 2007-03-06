#include "procedures/Procedure.h"
#include "primitive_procedures.h"
using namespace util;
using namespace std;
using namespace monash;

PROCEDURE(NumberP, "number?")
{
    ARGC_SHOULD_BE(1);
    RETURN_BOOLEAN(ARGV(0)->isNumber());
}

PROCEDURE(Plus, "+")
{
    Objects* as = Kernel::listOfValues(arguments, env);
    int total = 0;
    for (int i = 0; i < as->size(); i++)
    {
        Object* o = as->get(i);
        if (o->isNumber())
        {
            Number* n = (Number*)o;
            total += n->value();
        }
    }
    return new Number(total);
}

PROCEDURE(Minus, "-")
{
    ARGC_SHOULD_BE_GT(0);
    CAST(ARGV(0), Number, n);
    if (ARGC == 1) return new Number(-1 * n->value());

    int total = n->value();
    for (int i = 1; i < as->size(); i++)
    {
        Object* o = as->get(i);
        if (o->isNumber())
        {
            Number* nn = (Number*)o;
            total -= nn->value();
        }
    }
    return new Number(total);
}

PROCEDURE(NumberToString, "number->string")
{
    ARGC_SHOULD_BE(1);
    CAST(ARGV(0), Number, n);
    char buf[32];
    snprintf(buf, 32, "%d", n->value());
    return new SString(buf, n->lineno());
}

PROCEDURE(NumberEqual, "=")
{
    ARGC_SHOULD_BE_GT(1);
    CAST_RETURN_FALSE(ARGV(0), Number, n0);
    for (int i = 1; i < as->size(); i++)
    {
        CAST_RETURN_FALSE(as->get(i), Number, n);
        if (n->value() != n0->value()) return SCM_FALSE;
    }
    return SCM_TRUE;
}


PROCEDURE(NumberGt, ">")
{
    ARGC_SHOULD_BE_GT(1);
    CAST_RETURN_FALSE(ARGV(0), Number, prev);
    for (int i = 1; i < as->size(); i++)
    {
        CAST_RETURN_FALSE(as->get(i), Number, n);
        if (prev->value() <= n->value()) return SCM_FALSE;
        prev = n;
    }
    return SCM_TRUE;
}

PROCEDURE(NumberLt, "<")
{
    ARGC_SHOULD_BE_GT(1);
    CAST_RETURN_FALSE(ARGV(0), Number, prev);
    for (int i = 1; i < as->size(); i++)
    {
        CAST_RETURN_FALSE(as->get(i), Number, n);
        if (prev->value() >= n->value()) return SCM_FALSE;
        prev = n;
    }
    return SCM_TRUE;
}

PROCEDURE(NumberGe, ">=")
{
    ARGC_SHOULD_BE_GT(1);
    CAST_RETURN_FALSE(ARGV(0), Number, prev);
    for (int i = 1; i < as->size(); i++)
    {
        CAST_RETURN_FALSE(as->get(i), Number, n);
        if (prev->value() < n->value()) return SCM_FALSE;
        prev = n;
    }
    return SCM_TRUE;
}

PROCEDURE(NumberLe, "<=")
{
    ARGC_SHOULD_BE_GT(1);
    CAST_RETURN_FALSE(ARGV(0), Number, prev);
    for (int i = 1; i < as->size(); i++)
    {
        CAST_RETURN_FALSE(as->get(i), Number, n);
        if (prev->value() > n->value()) return SCM_FALSE;
        prev = n;
    }
    return SCM_TRUE;
}

PROCEDURE(Multiply, "*")
{
    Objects* as = Kernel::listOfValues(arguments, env);
    int total = 1;
    for (int i = 0; i < as->size(); i++)
    {
        Object* o = as->get(i);
        if (o->isNumber())
        {
            Number* n = (Number*)o;
            total *= n->value();
        }
    }
    return new Number(total);
}

PROCEDURE(Divide, "/")
{
    ARGC_SHOULD_BE_GT(0);
    CAST(ARGV(0), Number, n);
    int total = n->value();
    for (int i = 1; i < as->size(); i++)
    {
        Object* o = as->get(i);
        if (o->isNumber())
        {
            Number* nn = (Number*)o;
            if (nn->value() == 0)
            {
                RAISE_ERROR(nn->lineno(), "devide by zero")
                return NULL;
            }
            total /= nn->value();
        }
    }
    return new Number(total);
}

