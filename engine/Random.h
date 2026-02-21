//
// Created by d on 6/21/25.
//

#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include "utils/Types.h"

namespace Plunksna {

class Random {
public:
    Random() noexcept : m_gen(m_device())
    {}

    explicit Random(u32 seed) noexcept : m_gen(seed)
    {}

    //get random integer type, inclusive min and max
    template <typename TInt1, typename TInt2>
    auto randomInt(TInt1 low, TInt2 high)
    {
        using TCommon = std::common_type_t<TInt1, TInt2>;
        std::uniform_int_distribution<TCommon> distribution(static_cast<TCommon>(low), static_cast<TCommon>(high));
        return distribution(m_gen);
    }

    //get random float type, inclusive min and max
    template <typename TReal1, typename TReal2>
    auto randomReal(TReal1 low, TReal2 high)
    {
        using TCommon = std::common_type_t<TReal1, TReal2>;
        std::uniform_real_distribution<TCommon> distribution(static_cast<TCommon>(low), static_cast<TCommon>(high));
        return distribution(m_gen);
    }

    //return random unit vector, if 1 d, returns a vector of random value from -1,1;
    template <glm::length_t Dimension, typename T>
    glm::vec<Dimension, T> randomVector()
    {
        static_assert(Dimension >= 1, "Vector must not be 1D or greater!");
        static_assert(std::is_floating_point_v<T>, "Vector must be a float!");

        if constexpr (Dimension == 1)
            return glm::vec<1, T>(randomReal(-1.0, 1.0));

        glm::vec<Dimension, T> vector;

        for (glm::length_t i = 0; i < Dimension; i++)
            vector[i] = randomReal(static_cast<T>(-1.0), static_cast<T>(1.0));

        return glm::normalize(vector);
    }

private:
    std::random_device m_device;
    std::mt19937 m_gen;
};

#ifdef NDEBUG
inline static Random g_random{};
#else
inline static Random g_random{0};
#endif

} // Plunksna

#endif //RANDOM_H
