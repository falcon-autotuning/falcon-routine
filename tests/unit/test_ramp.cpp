#include <gtest/gtest.h>

#include <falcon-core/math/Point.hpp>
#include <falcon-core/math/Quantity.hpp>
#include <falcon-core/math/Vector.hpp>

#include <falcon-core/physics/device_structures/Connection.hpp>

#include <falcon-core/physics/units/SymbolUnit.hpp>

using falcon_core::math::Point;
using falcon_core::math::PointSP;
using falcon_core::math::Quantity;
using falcon_core::physics::device_structures::Connection;
using falcon_core::physics::device_structures::ConnectionSP;
using falcon_core::physics::units::SymbolUnit;

namespace {

PointSP
make_point(std::initializer_list<std::pair<std::string, double>> values) {
  auto point = std::make_shared<Point>();

  for (const auto &[name, value] : values) {
    point->insert(Connection::ScreeningGate(name),
                  std::make_shared<Quantity>(value, SymbolUnit::Volt()));
  }

  return point;
}

} // namespace

TEST(RampPlanningTest, PointLookupWorksForExistingConnection) {
  auto point = make_point({
      {"S1", 0.0},
  });

  auto s1 = Connection::ScreeningGate("S1");

  EXPECT_NO_THROW({
    auto q = point->at(s1);

    ASSERT_NE(q, nullptr);
    EXPECT_DOUBLE_EQ(q->value(), 0.0);
  });
}

TEST(RampPlanningTest, PointLookupThrowsForMissingConnection) {
  auto point = make_point({
      {"S2", 0.0},
  });

  auto s1 = Connection::ScreeningGate("S1");

  EXPECT_THROW(point->at(s1), std::exception);
}

TEST(RampPlanningTest, VectorCanBeConstructedForMatchingDomain) {
  auto start = make_point({
      {"S1", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
  });

  EXPECT_NO_THROW({ falcon_core::math::Vector sweep(start, end); });
}

TEST(RampPlanningTest, VectorAllowsMissingConnections) {
  auto start = make_point({});
  auto end = make_point({
      {"S1", 0.5},
  });

  EXPECT_NO_THROW({ falcon_core::math::Vector(start, end); });
}

TEST(RampPlanningTest, PrincipalAxisSingleDimension) {
  auto start = make_point({
      {"S1", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
  });

  falcon_core::math::Vector sweep(start, end);

  EXPECT_NO_THROW({
    auto axis = sweep.principle_connection();

    auto bounds = sweep.at(axis);

    ASSERT_NE(bounds, nullptr);

    EXPECT_DOUBLE_EQ(bounds->first()->value(), 0.0);

    EXPECT_DOUBLE_EQ(bounds->second()->value(), 0.5);
  });
}

TEST(RampPlanningTest, PrincipalAxisChoosesLargestSweep) {
  auto start = make_point({
      {"S1", 0.0},
      {"S2", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
      {"S2", 2.0},
  });

  falcon_core::math::Vector sweep(start, end);

  auto axis = sweep.principle_connection();

  auto bounds = sweep.at(axis);

  ASSERT_NE(bounds, nullptr);

  double delta = std::abs(bounds->second()->value() - bounds->first()->value());

  EXPECT_DOUBLE_EQ(delta, 2.0);
}

TEST(RampPlanningTest, RampDurationMatchesLargestSweep) {
  auto start = make_point({
      {"S1", 0.0},
      {"S2", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
      {"S2", 2.0},
  });

  falcon_core::math::Vector sweep(start, end);

  auto axis = sweep.principle_connection();

  auto bounds = sweep.at(axis);

  constexpr double ramp_rate = 0.1;

  const double total_time =
      std::abs(std::abs(bounds->first()->value() - bounds->second()->value()) /
               ramp_rate);

  EXPECT_DOUBLE_EQ(total_time, 20.0);
}

TEST(RampPlanningTest, RuntimeLikeStartPointAndSubsetEndPoint) {
  auto start = make_point({
      {"S1", 0.0},
      {"S2", 0.0},
      {"S3", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
  });

  EXPECT_NO_THROW({ falcon_core::math::Vector sweep(start, end); });
}

TEST(RampPlanningTest, EndPointConnectionsExistInStartPoint) {
  auto start = make_point({
      {"S1", 0.0},
      {"S2", 0.0},
      {"S3", 0.0},
  });

  auto end = make_point({
      {"S1", 0.5},
  });

  std::cerr << "before connections()\n";

  auto connections = end->connections();

  ASSERT_NE(connections, nullptr);

  std::cerr << "connection count = " << connections->size() << "\n";

  size_t idx = 0;

  for (const auto &connection : *connections) {
    std::cerr << "connection " << idx++ << "\n";

    ASSERT_NE(connection, nullptr);

    std::cerr << "ptr = " << connection.get() << "\n";

    std::cerr << "before start lookup\n";

    auto start_value = start->at(connection);

    std::cerr << "after start lookup\n";

    ASSERT_NE(start_value, nullptr);

    std::cerr << "before end lookup\n";

    auto end_value = end->at(connection);

    std::cerr << "after end lookup\n";

    ASSERT_NE(end_value, nullptr);
  }
}
TEST(RampPlanningTest, IncreasingFlagMatchesVoltageDirection) {
  auto start = make_point({
      {"S1", 0.0},
  });

  auto end = make_point({
      {"S1", 1.0},
  });

  auto connection = Connection::ScreeningGate("S1");

  bool increasing =
      end->at(connection)->value() > start->at(connection)->value();

  EXPECT_TRUE(increasing);
}

TEST(RampPlanningTest, DecreasingFlagMatchesVoltageDirection) {
  auto start = make_point({
      {"S1", 1.0},
  });

  auto end = make_point({
      {"S1", 0.0},
  });

  auto connection = Connection::ScreeningGate("S1");

  bool increasing =
      end->at(connection)->value() > start->at(connection)->value();

  EXPECT_FALSE(increasing);
}
