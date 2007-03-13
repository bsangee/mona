#include "QuoteFilter.h"

#include <ctype.h>

using namespace util;
using namespace monash;

QuoteFilter::QuoteFilter()
{
}

QuoteFilter::~QuoteFilter()
{
}

String QuoteFilter::filter(const String& text)
{
    postion_ = 0;
    input_ = text;
    input_.replace("#(", "(vector "); // ugly
    String ret;
    char c;
    for (;;)
    {
        c = getChar();
        if (c == EOF)
        {
            break;
        }
        else if (c == ';')
        {
            ret += c;
            for (;;)
            {
                c = getChar();
                ret += c;
                if (c == '\n') break;
            }
        }
        else if (c == '\"')
        {
            ret += c;
            for (;;)
            {
                c = getChar();
                ret += c;
                if (c == '\"') break;
            }
        }
        else if (c == '\'')
        {
            String quoteString = getQuoteString(0);
            ret += "(quote " + quoteString + ")";
        }
        else
        {
            ret += c;
        }
    }
    return ret;
}

String QuoteFilter::getQuoteString(uint32_t paren)
{
    String ret;
    char c;
    for (;;)
    {
        c = getChar();
        if (c == EOF)
        {
            break;
        }
        else if (c == '(')
        {
            ret += c;
            ret += getQuoteString(paren + 1);
        }
        else if (c == ')')
        {
            ret += c;
            break;
        }
        else if (isspace(c) && paren == 0)
        {
            break;
        }
        else if (isspace(c) && paren != 0)
        {
            ret += c;
        }
        else
        {
            ret += c;
        }
    }
    return ret;
}

char QuoteFilter::getChar()
{
    if (input_.size() <= postion_) return EOF;
    int c = input_[postion_];
    postion_++;
    return c;

}

