//
// Created by d on 6/21/25.
//

#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace Plunksna {

class Random {
public:
    Random() noexcept
        : m_gen(m_device())
    {}

    //get random integer type
    template <typename TInt1, typename TInt2>
    auto randomInt(TInt1 low, TInt2 high)
    {
        using TCommon = std::common_type_t<TInt1, TInt2>;
        std::uniform_int_distribution<TCommon> distribution(static_cast<TCommon>(low), static_cast<TCommon>(high));
        return distribution(m_gen);
    }

    //get random float type
    template <typename TReal1, typename TReal2>
    auto randomReal(TReal1 low, TReal2 high)
    {
        using TCommon = std::common_type_t<TReal1, TReal2>;
        std::uniform_real_distribution<TCommon> distribution(static_cast<TCommon>(low), static_cast<TCommon>(high));
        return distribution(m_gen);
    }

private:
    std::random_device m_device;
    std::mt19937 m_gen;
};

inline static Random g_random{};

} // Plunksna

#endif //RANDOM_H
