#include "Simulation.h"
#include "Renderer.h"

int main() {
    Simulation sim(500);  // start small
    Renderer renderer(800, 600);

    while (true) {
        sim.update(0.1f);
        renderer.render(sim);
    }

    return 0;
}
