#pragma once
#include <SFML/Graphics.hpp>
#include "Simulation.h"

class Renderer {
public:
    Renderer(int width, int height);
    void render(Simulation& sim);

private:
    sf::RenderWindow window;
}