#include <vector>
#include <tuple>

class trajectory {
  std::vector< std::pair<double, PhysState> > points;
  
  points.iterator iterator() { return points.begin(); }
  void addPoint(double t, PhysState st) { points.push_back( std::make_pair(t, st) ); }
};
