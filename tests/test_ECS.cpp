//
// Created by d on 8/19/25.
//

#include "TestIncludes.h"
#include "ecs/ComponentStore.h"
#include "ecs/Filter.h"
#include "engine/Components.h"


TEST_CASE("Component Store test", "[ECS]")
{
    SECTION("Add/Remove")
    {
        ComponentStore<glm::vec2> cs(2);
        Entity e = 0;

        REQUIRE(cs.count() == 0);

        cs.add(e, 5,5);
        REQUIRE(cs.count() == 1);

        glm::vec2 v = *cs.get(e);
        glm::vec2 v2 = {5,5};
        REQUIRE(v == v2);

        cs.add(100, 9,9);
        cs.add(101, 9,9);
        REQUIRE(cs.count() == 3);

        v = *cs.get(e);
        REQUIRE(v == v2);

        cs.remove(e);
        REQUIRE(cs.count() == 2);

        v = *cs.get(100);
        v2 = {9,9};
        REQUIRE(v == v2);
    }

    SECTION("Resize operations")
    {
        ComponentStore<glm::vec2> cs(0);
        REQUIRE(cs.capacity() == 0);

        cs.add(0, 0, 0);
        REQUIRE(cs.capacity() >= 1);

        cs.remove(0);
        REQUIRE(cs.capacity() >= 0);

        cs.reserve(2);
        REQUIRE(cs.capacity() >= 2);

        SECTION("Moving objects after resize")
        {
            Entity e = 0;
            glm::vec2 a = {5,5};
            cs.add(e, 5,5);
            auto start = cs.getStart();

            for (int i = 1; i < 256; i++) {
                cs.add(i, i,i);
            }

            auto distance = cs.offsetAfterMove();
            auto newStart = cs.getStart();

            REQUIRE(*cs.get(e) == a);
            REQUIRE(newStart - start == distance);

        }
    }
}

TEST_CASE("Filter test", "[ECS]")
{
    Filter<glm::vec2, int> filter(nullptr, 0, 512);
    ComponentStore<glm::vec2> csv;
    ComponentStore<int> csi;

    REQUIRE(!filter.foreachDefault());
    REQUIRE(filter.count() == 0);

    Entity e = 0;
    csi.add(e, 5);
    csv.add(e, 5,5);

    REQUIRE(csi.count() == 1);
    REQUIRE(csv.count() == 1);

    REQUIRE(filter.add(e, csv.get(e), csi.get(e)));
    REQUIRE(filter.count() == 1);

    auto function = [](glm::vec2& vec, int& i)
    {
        vec.x++; vec.y++; i++;
    };

    REQUIRE(filter.setFunction(function));
    REQUIRE(filter.foreachDefault());

    REQUIRE(*csi.get(e) == 6);
    REQUIRE(*csv.get(e) == glm::vec2(6,6));

}

TEST_CASE("Registry test", "[ECS]")
{

}