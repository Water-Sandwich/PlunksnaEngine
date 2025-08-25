#include <engine/Random.h>
#include "TestIncludes.h"

TEST_CASE( "random is within inclusive bounds", "[random]" ) {
    SECTION("int inclusivity test") {
        auto min = 999;
        auto max = -999;
        for (int i = 0; i < 10000; i++) {
            auto num = g_random.randomInt(0, 100);
            if (num < min)
                min = num;
            if (num > max)
                max = num;
        }

        REQUIRE(min == 0);
        REQUIRE(max == 100);
    }

    SECTION("real inclusivity test") {
        auto min = 1.f;
        auto max = 0.f;
        for (int i = 0; i < 10000; i++) {
            auto num = g_random.randomReal(0.f, 1.f);
            if (num < min)
                min = num;
            if (num > max)
                max = num;
        }

        REQUIRE(min <= 0.001);
        REQUIRE(max >= 0.999f);
    }
}