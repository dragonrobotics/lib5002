#include <vector>
#include "trajectory.h"
#include "AccelerateFunctions.h"
#include "Integrator.h"

namespace frc2016 {

const double boulder_diameter = .254; // m
const double boulder_mass = .294835; // kg
const double boulder_drag_coeff = 0.47;
const double wheel_mass = 0; // TODO: fill this in
const double boulder_iner_moment = 0;
const double wheel_iner_moment = 0;
const double timestep = 0.01;

const std::vector<AccelerateFunction*> forceComp = {
  new spinLiftAcc(boulder_diameter / 2, boulder_mass),
  new dragAcc(boulder_diameter, boulder_mass, boulder_drag_coeff),
  new gravAcc() }; 

PhysState frc2016_initalize(double avel1, double avel2, double initElev) {
  double ballAngVel = (wheel_iner_moment * (avel1+avel2)) / boulder_iner_moment;
  double ballLinVel = ( (wheel_iner_moment * ((avel1+avel2) * (avel1+avel2))) - (boulder_iner_moment * ballAngVel * ballAngVel) ) / boulder_mass;
  
  PhysState out;
  out.angvel = ballAngVel;
  out.linvel.x = ballLinVel;
  out.linvel.y = 0;
  out.pos.y = initElev;
  
  return out;
}

trajectory frc2016_simulate(PhysState initState) {
  PhysState cur = initState;
  double curTime = 0;
  trajectory data;
  
  while(cur.pos.y >= 0) {
    data.addPoint(curTime, cur);
    cur = integrate(cur, curTime, timestep, forceComp);
    curTime += timestep;
  }
  data.addPoint(curTime, cur);
  
  return data;
}

}
