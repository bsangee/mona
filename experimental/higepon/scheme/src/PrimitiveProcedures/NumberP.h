#ifndef __NUMBER_P_H__
#define __NUMBER_P_H__

#include "PrimitiveProcedure.h"
#include "Number.h"

namespace monash {

class NumberP : public PrimitiveProcedure
{
public:
    NumberP();
    virtual ~NumberP();

    virtual std::string toString();
    virtual Object* eval(Environment* env);
    virtual Object* apply(Objects* arguments);
};

}; // namespace monash

#endif // __NUMBER_P_H__