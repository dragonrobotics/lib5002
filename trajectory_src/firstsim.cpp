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
const double motor_k = 12.0 / 18000.0;
const double motor_r = 17.14;

const double mount_launch_angle = 0;

/*
 * speed = (voltage_in / motor_k) - (torque / motor_k^2)resistance
 */

const std::vector<AccelerateFunction*> forceComp = {
  new spinLiftAcc(boulder_diameter / 2, boulder_mass),
  new dragAcc(boulder_diameter, boulder_mass, boulder_drag_coeff),
  new gravAcc() }; 

PhysState frc2016_initalize(double avel1, double avel2, double initElev) {
  double ballAngVel = (wheel_iner_moment * (avel1+avel2)) / boulder_iner_moment;
  double ballLinVel = ( (wheel_iner_moment * ((avel1+avel2) * (avel1+avel2))) - (boulder_iner_moment * ballAngVel * ballAngVel) ) / boulder_mass;
  
  PhysState out;
  out.angvel = ballAngVel;
  out.linvel.x = ballLinVel * cos(mount_launch_angle);
  out.linvel.y = ballLinVel * sin(mount_launch_angle);
  out.pos.y = initElev;
  
  return out;
}

PhysState frc_2016_initialize_motorVbusOut(double m1, double m2, double initElev) {
	double avel1 = (12*m1) / motor_k;
	double avel2 = (12*m2) / motor_k;
	return frc_2016_initialize(avel1, avel2, initElev);
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
