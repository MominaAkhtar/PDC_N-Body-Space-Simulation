#pragma once
#include <vector>
#include "Body.h"

class Simulation {
public:
    Simulation(int numBodies);

    void update(float dt);
    std::vector<Body>& getBodies();

private:
    std::vector<Body> bodies;
    const float G = 0.1f;

    void computeForces(float dt);
};