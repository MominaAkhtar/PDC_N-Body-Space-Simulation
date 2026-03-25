#include "Renderer.h"

Renderer::Renderer(int width, int height)
    : window(sf::VideoMode(width, height), "N-Body Simulation") {}

void Renderer::render(Simulation& sim) {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        for (auto& b : sim.getBodies()) {
            sf::CircleShape circle(2);
            circle.setPosition(b.x, b.y);
            circle.setFillColor(sf::Color::White);
            window.draw(circle);
        }

        window.display();
    }
}
