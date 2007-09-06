/*!
    \file   Tokenizer.cpp
    \brief

    Copyright (c) 2002-2007 Higepon.
    All rights reserved.
    License=MIT/X License

    \author  Higepon
    \version $Revision$
    \date   create:2007/07/14 update:$Date$
*/
#include "Tokenizer.h"

using namespace util;
using namespace monash;

Tokenizer::Tokenizer(const String& input) : input_(input), postion_(0), lineno_(1)
{
}

Tokenizer::~Tokenizer()
{
}

char Tokenizer::getChar()
{
    if (input_.size() <= postion_) return EOF;
    int c = input_[postion_];
    if (c == '\n') lineno_++;
    postion_++;
    return c;

}

void Tokenizer::unGetChar()
{
    postion_--;
    if (postion_ < 0) postion_ = 0;
}

Token Tokenizer::nextToken()
{
    int c;
    Token token;
    int minus = 1;

once_more:
    c = getChar();
    switch(c)
    {
    case '(':
        token.type = Token::LEFT_PAREN;
        token.lineno = lineno_;
        return token;
    case ')':
        token.type = Token::RIGHT_PAREN;
        token.lineno = lineno_;
        return token;
    }
    if (c == '-')
    {
        char next = getChar();
        if (isdigit(next))
        {
            c = next;
            minus = -1;
        }
        else
        {
            unGetChar();
        }
    }
    if (isspace(c)) goto once_more;
    if (c == ';')
    {
        for (;;)
        {
            c = getChar();
            if (c == '\n') goto once_more;
        }
    }
    if (isdigit(c))
    {
        int n = 0;
        do {
            n = n * 10 + c - '0';
            c = getChar();
        } while (isdigit(c));
        unGetChar();
        token.type = Token::NUMBER;
        token.value = n * minus;
        token.lineno = lineno_;
        return token;
    }
    if (c == '\"')
    {
        String str("");
        for (;;)
        {
            c = getChar();

            if (c == '\\')
            {
                char next = getChar();
                if (next == 'n') // \n = \ + n
                {
                    str += '\n';
                    continue;
                }
                else if (next == '\"')
                {
                    str += "\"";
                    continue;
                }
                else
                {
                    unGetChar();
                }
            }
            if (c == '\"')
            {
                break;
            }

            str += c;
        }
        token.text = str;
        token.type = Token::STRING;
        token.lineno = lineno_;
        return token;
    }
    else
    {
        String str("");
        str += c;
        for (;;)
        {

            c = getChar();
            if (isspace(c) || c == '(' || c == ')' || c== '\'')
            {
                unGetChar();
                break;
            }
            str += c;
        }
        token.type = Token::IDENTIFIER;
        token.text = str;
        token.lineno = lineno_;
        return token;
    }
}