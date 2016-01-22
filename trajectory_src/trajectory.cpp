#include <vector>
#include <utility>
#include <algorithm>

#include "trajectory.h"
#include "Integrator.h"

static bool cmpTrajPtHeight( trajPt p1, trajPt p2) {
        return (p1.second.pos.y < p2.second.pos.y);
};

trajPt trajectory::findMaxHeight() {
        std::vector<trajPt>::iterator maxElem = std::max_element(points.begin(), points.end(), cmpTrajPtHeight);

        return *maxElem;
}
