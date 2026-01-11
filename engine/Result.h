//
// Created by d on 1/5/26.
//

#ifndef ERROR_H
#define ERROR_H

namespace Plunksna {

enum class Result
{
    //generic
    SUCCESS = 0,
    ERROR,

    //ecs
    ECS_NULL_ENTITY = 1000,
    ECS_NULL_INDEX,
    ECS_REMOVE_COMP_NO_STORE,
    ECS_EMPTY_SWAP_REMOVED
};

constexpr bool fail(Result r)
{
    return r != Result::SUCCESS;
}

}

#endif //ERROR_H
