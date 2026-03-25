#include "Simulation.h"
#include <cmath>
#include <cstdlib>

Simulation::Simulation(int numBodies) {
    for (int i = 0; i < numBodies; i++) {
        Body b;
        b.x = rand() % 800;
        b.y = rand() % 600;
        b.vx = 0;
        b.vy = 0;
        b.mass = rand() % 10 + 1;
        bodies.push_back(b);
    }
}

std::vector<Body>& Simulation::getBodies() {
    return bodies;
}

void Simulation::update(float dt) {
    computeForces(dt);

    for (auto& b : bodies) {
        b.x += b.vx * dt;
        b.y += b.vy * dt;
    }
}

void Simulation::computeForces(float dt) {
    for (size_t i = 0; i < bodies.size(); i++) {
        float fx = 0, fy = 0;

        for (size_t j = 0; j < bodies.size(); j++) {
            if (i == j) continue;

            float dx = bodies[j].x - bodies[i].x;
            float dy = bodies[j].y - bodies[i].y;
            float dist = sqrt(dx * dx + dy * dy) + 0.01f;

            float force = G * bodies[i].mass * bodies[j].mass / (dist * dist);

            fx += force * dx / dist;
            fy += force * dy / dist;
        }

        bodies[i].vx += fx / bodies[i].mass * dt;
        bodies[i].vy += fy / bodies[i].mass * dt;
    }
}