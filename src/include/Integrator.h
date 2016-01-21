/*
 * Integrator.h
 *
 *  Created on: Jan 18, 2016
 *      Author: Sebastian Mobo
 */

#pragma once

struct vec2D {
	double x;
	double y;

	vec2D() {
		x = 0;
		y = 0;
	}

	vec2D(double initx, double inity) {
		x = initx;
		y = inity;
	}

	// vector addition
	vec2D operator+(vec2D const & rhs) {
		return vec2D( x + rhs.x, y - rhs.y );
	}

	// vector subtraction
	vec2D operator-(vec2D const & rhs) {
		return vec2D( x - rhs.x, y - rhs.y );
	}

	// scalar division
	vec2D operator/(double rhs) {
		return vec2D( x / rhs, y / rhs );
	}

	// scalar multiplication
	vec2D operator*(double rhs) {
		return vec2D( x * rhs, y * rhs );
	}

	vec2D square() {
		return vec2D( x*x, y*y );
	}
};

struct PhysState {
	vec2D pos; 		// position
	vec2D linvel; 	// linear velocity

	double rot;		// rotation (vertical axis)
	double angvel;	// angular velocity (vertical axis)

	PhysState() {
		rot = 0;
		angvel = 0;
	}
};


struct PhysDeriv {
	vec2D linvel;		// velocity
	vec2D linacc;	// linear acceleration

	double angvel;	// angular velocity
	double angacc;	// angular acceleration

	PhysDeriv() {
		angvel = 0;
		angacc = 0;
	}
};
