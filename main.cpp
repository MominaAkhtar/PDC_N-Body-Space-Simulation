// #include <SFML/Graphics.hpp>
// #include <vector>
// #include <cmath>
// #include <omp.h>
// #include <iostream>

// using namespace std;

// const float G = 400.0f; 
// const float DT = 0.01f; 
// const int NUM_BODIES = 1000; 

// struct Body {
//     sf::Vector2f pos;
//     sf::Vector2f vel;
//     float mass;
//     float radius;
//     sf::Color color;
//     bool active = true; 
// };

// void updatePhysics(vector<Body>& bodies, sf::Vector2f mousePos, bool mouseActive) {
//     int n = bodies.size();
//     #pragma omp parallel for
//     for (int i = 0; i < n; i++) {
//         if (!bodies[i].active) continue;
//         sf::Vector2f force(0, 0);
//         if (mouseActive) {
//             sf::Vector2f d = mousePos - bodies[i].pos;
//             float distSq = d.x * d.x + d.y * d.y + 500.0f;
//             force += (G * 10000.0f * bodies[i].mass / distSq) * (d / sqrt(distSq));
//         }
//         for (int j = 0; j < n; j++) {
//             if (i == j || !bodies[j].active) continue;
//             sf::Vector2f d = bodies[j].pos - bodies[i].pos;
//             float distSq = d.x * d.x + d.y * d.y + 150.0f;
//             float dist = sqrt(distSq);
//             if (dist < (bodies[i].radius + bodies[j].radius)) {
//                 if (bodies[i].mass >= bodies[j].mass) {
//                     #pragma omp critical
//                     {
//                         if (bodies[j].active) {
//                             bodies[i].vel = (bodies[i].mass * bodies[i].vel + bodies[j].mass * bodies[j].vel) / (bodies[i].mass + bodies[j].mass);
//                             bodies[i].mass += bodies[j].mass;
//                             bodies[i].radius = sqrt(bodies[i].radius * bodies[i].radius + bodies[j].radius * bodies[j].radius);
//                             bodies[j].active = false;
//                         }
//                     }
//                 }
//             }
//             force += (G * bodies[i].mass * bodies[j].mass / distSq) * (d / dist);
//         }
//         bodies[i].vel += (force / bodies[i].mass) * DT;
//     }
//     for (int i = 0; i < n; i++) {
//         if (bodies[i].active) bodies[i].pos += bodies[i].vel * DT;
//     }
// }

// int main() {
//     sf::RenderWindow window(sf::VideoMode({1400, 900}), "Ethereal N-Body Galaxy");
//     window.setFramerateLimit(60);

//     sf::RenderTexture trailTexture;
//     if (!trailTexture.resize({1400, 900})) return -1;
//     trailTexture.clear(sf::Color::Black);

//     vector<Body> bodies;
//     sf::Vector2f center(700, 450);

//     // Central Star (Brighter, White-Yellow core)
//     bodies.push_back({center, {0, 0}, 8000.0f, 15.0f, sf::Color(255, 255, 200)});

//     for (int i = 1; i < NUM_BODIES; i++) {
//         float r = 80.0f + (rand() % 600);
//         float angle = (rand() % 360) * 3.14159f / 180.0f;
//         float v_mag = sqrt((G * 8000.0f) / r); 
        
//         // Initial colors: Deep violets and cyans
//         sf::Color startColor = (rand() % 2 == 0) ? sf::Color(150, 50, 255, 180) : sf::Color(50, 150, 255, 180);

//         bodies.push_back({
//             center + sf::Vector2f(cos(angle) * r, sin(angle) * r),
//             sf::Vector2f(-sin(angle) * v_mag, cos(angle) * v_mag),
//             static_cast<float>(rand() % 10 + 1),
//             1.8f,
//             startColor
//         });
//     }

//     while (window.isOpen()) {
//         while (const std::optional event = window.pollEvent()) {
//             if (event->is<sf::Event::Closed>()) window.close();
//         }

//         sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
//         bool rightClick = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
//         bool leftClick = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

//         if (leftClick) {
//             bodies.push_back({mousePos, {15.0f, -15.0f}, 100.0f, 4.0f, sf::Color::White});
//         }

//         updatePhysics(bodies, mousePos, rightClick);

//         // Visual enhancement: Darker fade for "sharper" trails
//         sf::RectangleShape fade(sf::Vector2f(1400, 900));
//         fade.setFillColor(sf::Color(0, 0, 0, 25)); 
//         trailTexture.draw(fade);

//         for (auto& b : bodies) {
//             if (!b.active) continue;

//             // DYNAMIC COLORING BASED ON SPEED
//             float speedSq = b.vel.x * b.vel.x + b.vel.y * b.vel.y;
//             float speed = sqrt(speedSq);
            
//             // Map speed to color (Blue -> Magenta -> White)
//             sf::Color drawColor;
//             if (speed > 150.0f) drawColor = sf::Color(255, 255, 255); // White hot
//             else if (speed > 80.0f) drawColor = sf::Color(255, 100, 255); // Magenta
//             else drawColor = b.color; // Original Blue/Violet

//             // THE SOFT GLOW EFFECT
//             sf::CircleShape shape(b.radius);
//             shape.setOrigin({b.radius, b.radius});
//             shape.setPosition(b.pos);
//             shape.setFillColor(drawColor);
            
//             // Use Additive Blending for the stars
//             trailTexture.draw(shape, sf::BlendAdd);

//             // MASSIVE BLOOM FOR LARGE BODIES
//             if (b.mass > 5000.0f) {
//                 for (int i = 1; i <= 3; i++) {
//                     sf::CircleShape glow(b.radius * (1.0f + i * 1.5f));
//                     glow.setOrigin({glow.getRadius(), glow.getRadius()});
//                     glow.setPosition(b.pos);
//                     glow.setFillColor(sf::Color(255, 200, 100, 30 / i));
//                     trailTexture.draw(glow, sf::BlendAdd);
//                 }
//             }
//         }
//         trailTexture.display();

//         window.clear();
//         window.draw(sf::Sprite(trailTexture.getTexture()));
//         window.display();
//     }
//     return 0;
// }

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <omp.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// Simulation Constants
const float G = 450.0f; 
const float DT = 0.008f; 
const int NUM_BODIES = 2000; // High density for stress testing

struct Body {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float mass;
    float radius;
    sf::Color color;
    bool active = true; 
};

// Physics Engine: Parallelized N-Body Interaction
void updatePhysics(vector<Body>& bodies, sf::Vector2f mousePos, bool mouseActive) {
    int n = static_cast<int>(bodies.size());
    
    #pragma omp parallel for schedule(dynamic, 64)
    for (int i = 0; i < n; i++) {
        if (!bodies[i].active) continue;
        sf::Vector2f force(0, 0);

        // Right-Click: Artificial Singularity (Black Hole)
        if (mouseActive) {
            sf::Vector2f d = mousePos - bodies[i].pos;
            float distSq = d.x * d.x + d.y * d.y + 800.0f;
            force += (G * 25000.0f * bodies[i].mass / distSq) * (d / sqrt(distSq));
        }

        for (int j = 0; j < n; j++) {
            if (i == j || !bodies[j].active) continue;
            
            sf::Vector2f d = bodies[j].pos - bodies[i].pos;
            float distSq = d.x * d.x + d.y * d.y + 120.0f; // Gravitational Softening
            float dist = sqrt(distSq);
            
            // Collision & Inelastic Merging
            if (dist < (bodies[i].radius + bodies[j].radius)) {
                if (bodies[i].mass >= bodies[j].mass) {
                    #pragma omp critical
                    {
                        if (bodies[j].active) {
                            // Momentum Transfer: p_total = m1v1 + m2v2
                            bodies[i].vel = (bodies[i].mass * bodies[i].vel + bodies[j].mass * bodies[j].vel) / (bodies[i].mass + bodies[j].mass);
                            bodies[i].mass += bodies[j].mass;
                            // Volume Conservation: r_new = sqrt(r1^2 + r2^2)
                            bodies[i].radius = sqrt(bodies[i].radius * bodies[i].radius + bodies[j].radius * bodies[j].radius);
                            bodies[j].active = false;
                        }
                    }
                }
            }
            force += (G * bodies[i].mass * bodies[j].mass / distSq) * (d / dist);
        }
        bodies[i].vel += (force / bodies[i].mass) * DT;
    }

    for (int i = 0; i < n; i++) {
        if (bodies[i].active) bodies[i].pos += bodies[i].vel * DT;
    }
}

int main() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    
    sf::RenderWindow window(sf::VideoMode({1600, 900}), "Cosmic Architect: HPC N-Body Simulation", sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    sf::RenderTexture renderTex;
    if (!renderTex.resize({1600, 900})) return -1;
    renderTex.setSmooth(true);

    // Vertex Array for Background Stars (Performance Optimized)
    sf::VertexArray stars(sf::PrimitiveType::Points, 4000);
    for (int i = 0; i < 4000; i++) {
        stars[i].position = {(float)(rand() % 1600), (float)(rand() % 900)};
        int lum = rand() % 130 + 50;
        stars[i].color = sf::Color(lum, lum, lum, 100);
    }

    vector<Body> bodies;
    sf::Vector2f center(800, 450);

    // Initial Solar System Configuration
    bodies.push_back({center, {0, 0}, 15000.0f, 22.0f, sf::Color::White}); // Supermassive Core
    
    for (int i = 1; i < NUM_BODIES; i++) {
        float r = 100.0f + (rand() % 650);
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float v_mag = sqrt((G * 15000.0f) / r); // Keplerian Orbital Velocity
        
        bodies.push_back({
            center + sf::Vector2f(cos(angle) * r, sin(angle) * r),
            sf::Vector2f(-sin(angle) * v_mag, cos(angle) * v_mag),
            (float)(rand() % 8 + 1), 1.5f, sf::Color::White
        });
    }

    sf::Clock fpsClock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        updatePhysics(bodies, mPos, sf::Mouse::isButtonPressed(sf::Mouse::Button::Right));

        // Draw Sequence
        sf::RectangleShape dim({1600, 900});
        dim.setFillColor(sf::Color(0, 0, 5, 25)); // Motion Blur Effect
        renderTex.draw(dim);
        renderTex.draw(stars);

        int activeCount = 0;
        for (auto& b : bodies) {
            if (!b.active) continue;
            activeCount++;

            // Velocity-Based Color Mapping (Heatmap)
            float speedSq = b.vel.x*b.vel.x + b.vel.y*b.vel.y;
            sf::Color heat;
            if (speedSq > 15000) heat = sf::Color(255, 255, 255);      // White Hot
            else if (speedSq > 8000) heat = sf::Color(100, 255, 255);  // Cyan
            else if (speedSq > 3000) heat = sf::Color(180, 100, 255);  // Violet
            else heat = sf::Color(80, 80, 220);                        // Deep Blue

            sf::CircleShape core(b.radius);
            core.setOrigin({b.radius, b.radius});
            core.setPosition(b.pos);
            core.setFillColor(heat);
            renderTex.draw(core, sf::BlendAdd);

            if (b.mass > 8000.0f) { // Core Bloom
                sf::CircleShape glow(b.radius * 3.0f);
                glow.setOrigin({glow.getRadius(), glow.getRadius()});
                glow.setPosition(b.pos);
                glow.setFillColor(sf::Color(255, 250, 200, 30));
                renderTex.draw(glow, sf::BlendAdd);
            }
        }

        renderTex.display();
        window.clear();
        window.draw(sf::Sprite(renderTex.getTexture()));
        
        // HUD Update
        float fps = 1.f / fpsClock.restart().asSeconds();
        stringstream hud;
        hud << "G-ENGINE | FPS: " << (int)fps << " | ACTIVE: " << activeCount;
        window.setTitle(hud.str());
        
        window.display();
    }
    return 0;
}
