/*
 * AccelerateFunctions.h
 *
 *  Created on: Jan 18, 2016
 *      Author: Tatantyler
 */

#pragma once
#include "PhysObjects.h"
#include <cmath>

const double pi = 3.14159265358;
const double pi_squared = pi*pi;

class AccelerateFunction {
	public:
	virtual void accelerate(PhysState& curstate, PhysDeriv& outderiv, double time, double delta)=0;
	void operator()(PhysState& curstate, PhysDeriv& outderiv, double time, double delta) {
		return accelerate(curstate, outderiv, time, delta);
	}
};

class gravAcc : public AccelerateFunction {
	constexpr static double gravity = 9.81;

	void accelerate(PhysState& curstate, PhysDeriv& outderiv, double time, double delta) {
		outderiv.linacc.y -= gravity;
	}
};

class dragAcc : public AccelerateFunction {
	double mass;

	double dragForceCoefficient;

	constexpr static double airDensity = 1.225;

	void accelerate(PhysState& curstate, PhysDeriv& outderiv, double time, double delta) {
		vec2D accDrag = (curstate.linvel.square() * dragForceCoefficient) / mass;

		outderiv.linacc.x = outderiv.linacc.x - ( (outderiv.linacc.x > 0) ? accDrag.x : -accDrag.x );
		outderiv.linacc.y = outderiv.linacc.y - ( (outderiv.linacc.y > 0) ? accDrag.y : -accDrag.y );
	}

public:
	dragAcc(double diameter, double m, double dragCoefficient) {
		dragForceCoefficient = (((pi * (diameter*diameter)) / 4) * dragCoefficient * airDensity) / 2.0;
		mass = m;
	}
};

class spinLiftAcc : public AccelerateFunction {
	constexpr static double airDensity = 1.225;
	double r3;
	double mass;

	void accelerate(PhysState& curstate, PhysDeriv& outderiv, double time, double delta) {
		// L = (4/3)(4(pi^2)(r^3)*rot*density*vel), rot = revs. per second
		vec2D accLift = (curstate.linvel * airDensity * (curstate.angvel / 360.0) * r3 * pi_squared * 16) / 3;
		accLift = accLift / mass;

		outderiv.linacc = outderiv.linacc + accLift;
	}

public:
	spinLiftAcc(double r, double m) {
		r3 = r*r*r;
		mass = m;
	}
};
