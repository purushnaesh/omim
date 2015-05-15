#pragma once

#include "std/set.hpp"
#include "std/shared_ptr.hpp"
#include "std/string.hpp"
#include "std/utility.hpp"
#include "std/vector.hpp"

#include "routing/osrm_router.hpp"

/*
 * These tests are developed to simplify routing integration tests writing.
 * You can use the interface bellow however you want but there are some hints.
 * 1. Most likely you want to use GetAllMaps() to get a reference to OsrmRouterComponents.
 *    It loads all the maps from directories Platform::WritableDir()
 *    and Platform::ResourcesDir() only once and then reuse it.
 *    Use LoadMaps() only if you want to test something on a special map set.
 * 2. Loading maps and calculating routes is a time consumption process.
 *    Do this only if you really need it.
 * 3. If you want to check that a turn is absent you have two options
 *    - use GetTurnByPoint(...).TestNotValid();
 *    - or use TestTurnCount.
 * 4. The easiest way to gather all the information for writing an integration test is
 *    - to put a break point in OsrmRouter::CalculateRouteImpl;
 *    - to make a route with MapWithMe desktop application;
 *    - to get all necessary parameters and result of the route calculation;
 *    - to place them into the test you're writing.
 * 5. The recommended way for naming tests for a route from one place to another one is
 *    <Country><City><Street1><House1><Street2><House2><Test time. TurnTest or RouteTest for the
 * time being>
 * 6. It's a good idea to use short routes for testing turns. The thing is geometry of long routes
 *    could be changes for one dataset (osrm version) to another one.
 *    The shorter route the less chance it'll be changed.
 */
using namespace routing;
using namespace turns;

typedef pair<shared_ptr<Route>, OsrmRouter::ResultCode> TRouteResult;

namespace integration
{
  class OsrmRouterComponents;

  void TestOnlineCrosses(m2::PointD const & startPoint, m2::PointD const & finalPoint,
                         vector<string> const & expected,
                         OsrmRouterComponents & routerComponents);

  OsrmRouterComponents & GetAllMaps();
  shared_ptr<OsrmRouterComponents> LoadMaps(vector<string> const & mapNames);
  TRouteResult CalculateRoute(OsrmRouterComponents const & routerComponents,
                              m2::PointD const & startPoint, m2::PointD const & startDirection,
                              m2::PointD const & finalPoint);

  void TestTurnCount(Route const & route, uint32_t expectedTurnCount);

  /// Testing route length.
  /// It is used for checking if routes have expected(sample) length.
  /// A created route will pass the test iff
  /// expectedRouteLength - expectedRouteLength * relativeError  <= route->GetDistance()
  /// && expectedRouteLength + expectedRouteLength * relativeError >= route->GetDistance()
  void TestRouteLength(Route const & route, double expectedRouteLength,
                       double relativeError = 0.01);
  void CalculateRouteAndTestRouteLength(OsrmRouterComponents const & routerComponents,
                                        m2::PointD const & startPoint,
                                        m2::PointD const & startDirection,
                                        m2::PointD const & finalPoint, double expectedRouteLength,
                                        double relativeError = 0.07);

  class TestTurn
  {
    friend TestTurn GetNthTurn(Route const & route, uint32_t turnNumber);
    friend TestTurn GetTurnByPoint(Route const & route, m2::PointD const & approximateTurnPoint,
                                   double inaccuracy);

    m2::PointD const m_point;
    TurnDirection const m_direction;
    uint32_t const m_roundAboutExitNum;
    bool const m_isValid;

    TestTurn()
        : m_point({0., 0.}),
          m_direction(TurnDirection::NoTurn),
          m_roundAboutExitNum(0),
          m_isValid(false)
    {
    }
    TestTurn(m2::PointD const & pnt, TurnDirection direction, uint32_t roundAboutExitNum)
        : m_point(pnt),
          m_direction(direction),
          m_roundAboutExitNum(roundAboutExitNum),
          m_isValid(true)
    {
    }

  public:
    const TestTurn & TestValid() const;
    const TestTurn & TestNotValid() const;
    const TestTurn & TestPoint(m2::PointD  const & expectedPoint, double inaccuracyMeters = 3.) const;
    const TestTurn & TestDirection(TurnDirection expectedDirection) const;
    const TestTurn & TestOneOfDirections(set<TurnDirection> const & expectedDirections) const;
    const TestTurn & TestRoundAboutExitNum(uint32_t expectedRoundAboutExitNum) const;
  };

  /// Extracting appropriate TestTurn if any. If not TestTurn::isValid() returns false.
  /// inaccuracy is set in meters.
  TestTurn GetNthTurn(Route const & route, uint32_t turnNumber);
  TestTurn GetTurnByPoint(Route const & route, m2::PointD const & approximateTurnPoint,
                          double inaccuracyMeters = 3.);
}
