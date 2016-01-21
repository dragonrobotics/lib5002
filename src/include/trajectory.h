#pragma once

#include <vector>
#include <tuple>
#include <algorithm>
#include "Integrator.h"

typedef std::pair<double, PhysState> trajPt;

class trajectory {
  std::vector<trajPt> points;
  
  points.iterator iterator() { return points.begin(); }
  void addPoint(double t, PhysState st) { points.push_back( std::make_pair(t, st) ); }
  void addPoint(trajPt pt) { points.push_back(pt); }
  trajPt findMaxHeight();
  double getDistance() { return points.back().second.pos.x; }
  double getFlightTime() { return points.back().first; }
};
