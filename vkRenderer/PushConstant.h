//
// Created by d on 2/22/26.
//

#ifndef PUSHCONSTANT_H
#define PUSHCONSTANT_H

#include "Utils/Types.h"

namespace Plunksna {

struct PushConstant
{
    alignas(4) u32 instanceIndex;
};

}


#endif //PUSHCONSTANT_H
