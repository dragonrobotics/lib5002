/*
 * Integrator.h
 *
 *  Created on: Jan 18, 2016
 *      Author: Sebastian Mobo
 */

#pragma once
#include "PhysObjects.h"
#include "AccelerateFunctions.h"
#include <vector>

PhysState integrate(PhysState const& curState, double time, double delta, std::vector<AccelerateFunction*> accFunc);
