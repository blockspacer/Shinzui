#include <catch2/catch.hpp>
#include "mocks/entity.hpp"
#include "mocks/server.hpp"

#include <map/cell.hpp>
#include <map/map-cluster/cluster.hpp>
#include <map/map.hpp>


SCENARIO("Map cells can be fetched once created", "[map]") {
    GIVEN("A map with one cell and a non-updating entity") {
        TestServer server(12345);
        Map& map = *server.map();

        Entity e(0);
        map.addTo(0, 0, e.asDefault(), nullptr);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations(0);

        REQUIRE(map.size() == 7);
        // REQUIRE(map.scheduledSize() == 0);

        WHEN("the main cell is required") {
            Cell* cell = map.get(0, 0);

            THEN("cell is not nullptr") {
                REQUIRE(cell != nullptr);
            }

            THEN("cell has a ring with radius 1") {
                auto ring = cell->ring(1);

                REQUIRE(ring.size() == 6);
                for (int i = 0; i < 6; ++i)
                {
                    REQUIRE(ring[i] != nullptr);
                }
            }

            THEN("cell has not a ring with radius 2") {
                auto ring = cell->ring(2);

                REQUIRE(ring.size() == 12);
                for (int i = 0; i < 12; ++i)
                {
                    REQUIRE(ring[i] == nullptr);
                }
            }
        }
    }

    GIVEN("A map with one cell and an updating sibling") {
        TestServer server(12345);
        Map& map = *server.map();

        Entity e(0); e.forceUpdater();

        map.addTo(0, 0, e.asDefault(), nullptr);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations(0);

        REQUIRE(map.size() == 7);
        // REQUIRE(map.scheduledSize() == 0);

        WHEN("the main cell is required") {
            Cell* cell = map.get(0, 0);

            THEN("cell is not nullptr") {
                REQUIRE(cell != nullptr);
            }

            THEN("cell has a ring with radius 1") {
                auto ring = cell->ring(1);

                REQUIRE(ring.size() == 6);
                for (int i = 0; i < 6; ++i)
                {
                    REQUIRE(ring[i] != nullptr);
                }
            }

            THEN("cell has not a ring with radius 2") {
                auto ring = cell->ring(2);

                REQUIRE(ring.size() == 12);
                for (int i = 0; i < 12; ++i)
                {
                    REQUIRE(ring[i] == nullptr);
                }
            }
        }
    }
}
