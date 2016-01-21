/*
 * AccelerateFunctions.h
 *
 *  Created on: Jan 18, 2016
 *      Author: Tatantyler
 */

#pragma once
#include "Integrator.h"
#include <cmath>

class AccelerateFunction {
	virtual void accelerate(PhysState const& curstate, PhysDeriv& outderiv, double time, double delta)=0;
	void operator()(PhysState const& curstate, PhysDeriv& outderiv, double time, double delta) {
		return accelerate(curstate, outderiv, time, delta);
	}
};

class gravAcc : public AccelerateFunction {
	static double gravity = 9.81;

	void accelerate(PhysState const& curstate, PhysDeriv& outderiv, double time, double delta) {
		outDeriv.linacc.y -= gravity;
	}

	void gravAcc() {};
};

class dragAcc : public AccelerateFunction {
	double mass;

	double dragForceCoefficient;

	static double airDensity = 1.225;

	void accelerate(PhysState const& curstate, PhysDeriv& outderiv, double time, double delta) {
		vec2D accDrag = (curstate.linvel.square() * dragForceCoefficient) / mass;

		outderiv.linacc.x = outderiv.linacc.x - ( (outderiv.linacc.x > 0) ? accDrag.x : -accDrag.x );
		outderiv.linacc.y = outderiv.linacc.y - ( (outderiv.linacc.y > 0) ? accDrag.y : -accDrag.y );
	}

	void dragAcc(double crossSection, double m, double dragCoefficient) {
		dragForceCoefficient = (crossSection * dragCoefficient * airDensity) / 2.0;
		mass = m;
	}
};

class spinLiftAcc : public AccelerateFunction {
	static double airDensity = 1.225;
	double r3;
	double mass;

	void accelerate(PhysState const& curstate, PhysDeriv& outderiv, double time, double delta) {
		// L = (4/3)(4(pi^2)(r^3)*rot*density*vel), rot = revs. per second
		vec2D accLift = (curstate.linvel * density * (curstate.angvel / 360.0) * r3 * (PI*PI) * 16) / 3;
		accLift = accLift / mass;

		outderiv.linacc = outderiv.linacc + accLift;
	}

	void spinLiftAcc(double r, double m) {
		r3 = r*r*r;
		mass = m;
	}
};
