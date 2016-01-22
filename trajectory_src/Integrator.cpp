/*
 * Integrator.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: Tatantyler
 */

#include "Integrator.h"
#include "AccelerateFunctions.h"
#include <vector>

PhysDeriv derive(
		PhysState const& curState,
		PhysDeriv const& curDeriv,
		double time,
		double delta,
		std::vector<AccelerateFunction*> forceLift) {

	PhysState tmpState;

	/* Temporary integration for calculation of acceleration functions. */
	tmpState.pos = curState.pos + (curDeriv.linvel * delta);
	tmpState.linvel = curState.linvel + (curDeriv.linacc * delta);
	tmpState.rot = curState.rot + (curDeriv.angvel * delta);
	tmpState.angvel = curState.angvel + (curDeriv.angacc * delta);

	PhysDeriv newDeriv;

	newDeriv.linvel = curState.linvel;
	newDeriv.angvel = curState.angvel;

	for(std::vector<AccelerateFunction*>::iterator i = forceLift.begin();i != forceLift.end();++i) {
		(*(*i))(tmpState, newDeriv, time, delta);
	}

	return newDeriv;
}


PhysState integrate(PhysState const& curState, double time, double delta, std::vector<AccelerateFunction*> accFunc) {
	PhysDeriv a = derive(curState, PhysDeriv(), time, 0.0, accFunc);
	PhysDeriv b = derive(curState, a, time+(delta/2), delta/2, accFunc);
	PhysDeriv c = derive(curState, b, time+(delta/2), delta/2, accFunc);
	PhysDeriv d = derive(curState, c, time+delta, delta, accFunc);

	vec2D dPos = (a.linvel + ((b.linvel+c.linvel)*2.0) + d.linvel) / 6.0;
	vec2D dVel = (a.linacc + ((b.linacc+c.linacc)*2.0) + d.linacc) / 6.0;
	double dRot = (a.angvel + ((b.angvel + c.angvel)*2.0) + d.angvel) / 6.0;
	double dAngVel = (a.angacc + ((b.angacc + c.angacc)*2.0) + d.angacc) / 6.0;

	PhysState result;
	result.pos = curState.pos + (dPos * delta);
	result.linvel = curState.linvel + (dVel * delta);
	result.rot = curState.rot + (dRot * delta);
	result.angvel = curState.angvel + (dAngVel * delta);

	return result;
}
