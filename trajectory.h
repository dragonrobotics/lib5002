#pragma once

#include <vector>
#include <tuple>
#include <algorithm>
#include "Integrator.h"

class trajectory {
  std::vector< std::pair<double, PhysState> > points;
  
  points.iterator iterator() { return points.begin(); }
  void addPoint(double t, PhysState st) { points.push_back( std::make_pair(t, st) ); }
  std::pair<double, PhysState> findMaxHeight();
  double getDistance() { return points.back()[1].pos.x; }
  double getFlightTime() { return points.back()[0]; }
};
