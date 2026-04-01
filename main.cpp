#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <deque>
#include <sstream>
#include <iostream>
#include <omp.h>

using namespace std;

enum Stage { NEBULA, COLLAPSE, PROTOSTAR, ACCRETION, DIFFERENTIATION, CHAOTIC };

const float G = 450.f;
const float DT = 0.008f;
const int NUM_BODIES = 2000;

struct Body {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float mass;
    float radius;
    bool active = true;
    sf::Color color;
    deque<sf::Vector2f> trail;
    float collisionGlow = 0.f;
};

Stage getStage(float time) {
    if(time < 20.f) return NEBULA;
    if(time < 50.f) return COLLAPSE;
    if(time < 100.f) return PROTOSTAR;
    if(time < 150.f) return ACCRETION;
    if(time < 250.f) return DIFFERENTIATION;
    return CHAOTIC;
}

string stageName(Stage s) {
    switch(s) {
        case NEBULA: return "Stage 1: Nebula Cloud Rotating...";
        case COLLAPSE: return "Stage 2: Disk Formation Rotating!";
        case PROTOSTAR: return "Stage 3: Protostar Forming (Sun Igniting!)";
        case ACCRETION: return "Stage 4: Planet Formation & Merging!";
        case DIFFERENTIATION: return "Stage 5: Planets Forming (Inner/Outer)";
        case CHAOTIC: return "Stage 6: Orbital Chaos & Stability";
    }
    return "";
}

// Draw glowing rings for sun
void drawSunGlow(sf::RenderTexture &tex, sf::Vector2f pos, float radius, Stage stage) {
    if(stage < PROTOSTAR) return;
    
    float intensity = 1.0f;
    if(stage == ACCRETION) intensity = 1.5f;
    else if(stage >= DIFFERENTIATION) intensity = 2.0f;
    
    // Multiple glow layers
    for(int i = 0; i < 8; i++) {
        sf::CircleShape ring(radius * (1.0f + i*0.5f));
        ring.setOrigin({ring.getRadius(), ring.getRadius()});
        ring.setPosition(pos);
        uint8_t alpha = uint8_t(max(0.f, 150.f - i*20.f) * intensity);
        
        if(stage == PROTOSTAR) {
            ring.setFillColor(sf::Color(255, 200, 100, alpha));
        } else if(stage == ACCRETION) {
            ring.setFillColor(sf::Color(255, 150, 80, alpha));
        } else {
            ring.setFillColor(sf::Color(255, 180, 60, alpha));
        }
        tex.draw(ring, sf::BlendAdd);
    }
    
    // Core bright sphere
    sf::CircleShape core(radius);
    core.setOrigin({radius, radius});
    core.setPosition(pos);
    if(stage == PROTOSTAR) core.setFillColor(sf::Color(255, 255, 200, 255));
    else core.setFillColor(sf::Color(255, 200, 100, 255));
    tex.draw(core, sf::BlendAdd);
}

// Draw orbital trails for large bodies
void drawTrails(sf::RenderTexture &tex, const vector<Body>& bodies) {
    for(const auto &b : bodies) {
        if(!b.active || b.mass < 30.f || b.trail.size() < 2) continue;
        
        sf::VertexArray trail(sf::PrimitiveType::LineStrip);
        for(size_t i = 0; i < b.trail.size(); i++) {
            uint8_t alpha = uint8_t((i / (float)b.trail.size()) * 200);
            sf::Vertex v;
            v.position = b.trail[i];
            
            // Trail color based on orbit size/position
            if(b.mass > 100.f) {
                v.color = sf::Color(100, 200, 255, alpha);  // Cyan for planets
            } else {
                v.color = sf::Color(200, 150, 100, alpha);  // Brown for smaller
            }
            trail.append(v);
        }
        tex.draw(trail);
    }
}

void updatePhysics(vector<Body>& bodies, sf::Vector2f center, Stage stage) {
    for(auto &b : bodies) {
        if(!b.active) continue;
        if(stage == NEBULA) {
        sf::Vector2f toCenter = center - b.pos;
        float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);

        if(dist > 10.f) {
            // Circular motion around center
            sf::Vector2f tangent(-toCenter.y, toCenter.x);
            tangent /= dist;
            b.vel = tangent * 0.25f;  // faster rotation

            // Gentle drift toward center
            b.vel += toCenter / dist * 0.01f;

            // Add small random jitter for liveliness
            b.vel.x += ((rand() % 100) - 50) * 0.001f;
            b.vel.y += ((rand() % 100) - 50) * 0.001f;
        }

            // Optional: slight position oscillation to simulate "vibration"
            b.pos.x += sin(dist + b.pos.y * 0.1f) * 0.2f;
            b.pos.y += cos(dist + b.pos.x * 0.1f) * 0.2f;
}
        else if(stage == COLLAPSE) {
            sf::Vector2f toCenter = center - b.pos;
            float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
            if(dist > 30.f) {
                sf::Vector2f tangent(-toCenter.y, toCenter.x);
                tangent = tangent / dist;
                b.vel = tangent * sqrt(5500.f / (dist + 15.f));
                
                // Create 3D perspective: tilt disk and add depth
                float angle = atan2(b.pos.y - center.y, b.pos.x - center.x);
                float yOffset = cos(angle) * dist * 0.15f;  // Tilt effect
                b.pos.y = center.y + yOffset - (b.pos.y - center.y) * 0.88f;
            }
        } 
        else if(stage == PROTOSTAR) {
            sf::Vector2f toCenter = center - b.pos;
            float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
            if(dist > 20.f) {
                sf::Vector2f radial = toCenter / dist;
                sf::Vector2f tangent(-toCenter.y, toCenter.x);
                if(dist > 1.f) tangent /= dist;
                b.vel = radial * (2500.f / (dist*dist)) + tangent * 0.3f;
            }
        }
        else if(stage == ACCRETION) {
            sf::Vector2f toCenter = center - b.pos;
            float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
            if(dist > 30.f) {
                sf::Vector2f tangent(-toCenter.y, toCenter.x);
                tangent = tangent / dist;
                b.vel = tangent * sqrt(3500.f / (dist + 25.f));
                
                // Gentle inward drift
                sf::Vector2f radial = toCenter / dist;
                b.vel += radial * (300.f / (dist*dist)) * 0.2f;
            }
        }
        // else if(stage == DIFFERENTIATION) {
        //     sf::Vector2f toCenter = center - b.pos;
        //     float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
        //     if(dist > 30.f && b.mass < 8000.f) {
        //         sf::Vector2f tangent(-toCenter.y, toCenter.x);
        //         tangent = tangent / dist;
        //         b.vel = tangent * sqrt(4000.f / (dist + 20.f));
        //     }
            
        //     // Add trails for large bodies
        //     if(b.mass > 50.f) {
        //         b.trail.push_back(b.pos);
        //         if(b.trail.size() > 150) b.trail.pop_front();
        //     }
        // }
        else if(stage == DIFFERENTIATION) {
            sf::Vector2f toCenter = center - b.pos;
            float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);

            if(dist > 50.f && b.mass < 8000.f) { // avoid too close
                sf::Vector2f tangent(-toCenter.y, toCenter.x);
                tangent /= dist;

                // proper circular orbit velocity
                float vOrbit = sqrt(G * 15000.f / dist);
                b.vel = tangent * vOrbit; // set orbit
            }

            // maintain trails
            if(b.mass > 50.f) {
                b.trail.push_back(b.pos);
                if(b.trail.size() > 150) b.trail.pop_front();
            }
        }
        else if(stage == CHAOTIC) {
            sf::Vector2f toCenter = center - b.pos;
            float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
            if(dist > 30.f && b.mass < 8000.f) {
                sf::Vector2f tangent(-toCenter.y, toCenter.x);
                tangent = tangent / dist;
                float speed = sqrt(4000.f / (dist + 20.f));
                speed += (rand()%100 - 50) * 0.015f;
                b.vel = tangent * speed;
            }
            
            // Trails for chaos
            if(b.mass > 50.f) {
                b.trail.push_back(b.pos);
                if(b.trail.size() > 120) b.trail.pop_front();
            }
        }

        b.pos += b.vel * DT;
        b.collisionGlow = max(0.f, b.collisionGlow - DT * 15.f);

        // Wrap boundaries
        if(b.pos.x < 0.f) b.pos.x += 1200.f;
        if(b.pos.x > 1200.f) b.pos.x -= 1200.f;
        if(b.pos.y < 0.f) b.pos.y += 700.f;
        if(b.pos.y > 700.f) b.pos.y -= 700.f;
    }
}

// void resolveCollisions(vector<Body>& bodies, Stage stage) {
//     if(stage < ACCRETION) return;
    
//     int n = bodies.size();
//     for(int i = 0; i < n; i++) {
//         if(!bodies[i].active) continue;
//         for(int j = i + 1; j < n; j++) {
//             if(!bodies[j].active) continue;
//             sf::Vector2f d = bodies[j].pos - bodies[i].pos;
//             float dist = sqrt(d.x*d.x + d.y*d.y);
//             if(dist < bodies[i].radius + bodies[j].radius) {
//                 int big = (bodies[i].mass >= bodies[j].mass) ? i : j;
//                 int small = (big == i) ? j : i;

//                 bodies[big].vel = (bodies[big].vel * bodies[big].mass + bodies[small].vel * bodies[small].mass) / (bodies[big].mass + bodies[small].mass);
//                 bodies[big].mass += bodies[small].mass;
//                 bodies[big].radius = sqrt(bodies[big].radius*bodies[big].radius + bodies[small].radius*bodies[small].radius);
//                 bodies[big].collisionGlow = 1.0f;
//                 bodies[small].active = false;
//             }
//         }
//     }
// }

void resolveCollisions(vector<Body>& bodies, Stage stage) {
    if(stage < ACCRETION) return;

    int n = bodies.size();
    for(int i = 0; i < n; i++) {
        if(!bodies[i].active) continue;

        // Special merge with sun (assume sun is bodies[0])
        if(i != 0) {
            sf::Vector2f dSun = bodies[i].pos - bodies[0].pos;
            float distSun = sqrt(dSun.x*dSun.x + dSun.y*dSun.y);
            if(distSun < bodies[0].radius) {
                // Merge with sun
                bodies[0].vel = (bodies[0].vel * bodies[0].mass + bodies[i].vel * bodies[i].mass) / (bodies[0].mass + bodies[i].mass);
                bodies[0].mass += bodies[i].mass;
                bodies[0].radius = sqrt(bodies[0].radius*bodies[0].radius + bodies[i].radius*bodies[i].radius);
                bodies[0].collisionGlow = 1.0f;
                bodies[i].active = false;
                continue; // no need to check other collisions for this body
            }
        }

        // Regular collisions between other bodies
        for(int j = i + 1; j < n; j++) {
            if(!bodies[j].active) continue;
            sf::Vector2f d = bodies[j].pos - bodies[i].pos;
            float dist = sqrt(d.x*d.x + d.y*d.y);
            if(dist < bodies[i].radius + bodies[j].radius) {
                int big = (bodies[i].mass >= bodies[j].mass) ? i : j;
                int small = (big == i) ? j : i;

                bodies[big].vel = (bodies[big].vel * bodies[big].mass + bodies[small].vel * bodies[small].mass) / (bodies[big].mass + bodies[small].mass);
                bodies[big].mass += bodies[small].mass;
                bodies[big].radius = sqrt(bodies[big].radius*bodies[big].radius + bodies[small].radius*bodies[small].radius);
                bodies[big].collisionGlow = 1.0f;
                bodies[small].active = false;
            }
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1200, 700)), "N-Body Solar System Formation");
    window.setFramerateLimit(60);
    
    sf::RenderTexture renderTex(sf::Vector2u(1200, 700));
    renderTex.setSmooth(true);

    // Better star field with varying brightness
    sf::VertexArray stars(sf::PrimitiveType::Points, 3000);
    for(int i = 0; i < 3000; i++) {
        stars[i].position = {(float)(rand() % 1200), (float)(rand() % 700)};
        int lum = rand() % 180 + 40;
        int brightness = rand() % 160 + 60;
        stars[i].color = sf::Color(brightness, brightness, brightness, lum);
    }

    vector<Body> bodies;
    sf::Vector2f center(600, 350);
    
    // Central sun
    bodies.push_back({center, {0,0}, 15000.f, 28.f, true, sf::Color(255, 200, 100)});

    // Initialize particles - better distribution
    for(int i = 1; i < NUM_BODIES; i++) {
        float r = 80.f + rand() % 480;
        float angle = (rand() % 360) * 3.14159f / 180.f;
        float mass = float(rand() % 12 + 1);
        
        bodies.push_back({
            center + sf::Vector2f(cos(angle) * r, sin(angle) * r),
            {(rand() % 100 - 50) * 0.002f, (rand() % 100 - 50) * 0.002f},
            mass,
            0.8f + mass * 0.15f,  // Size based on mass
            true,
            sf::Color(180, 180, 200, 120)
        });
    }

    sf::Clock fpsClock, simClock;

    while(window.isOpen()) {
        while(auto eventOpt = window.pollEvent()) {
            auto event = *eventOpt;
            if(event.is<sf::Event::Closed>()) window.close();
        }

        float simTime = simClock.getElapsedTime().asSeconds();
        Stage stage = getStage(simTime);

        // Update colors based on stage with better gradients
        for(auto &b : bodies) {
            if(!b.active || b.mass > 8000.f) continue;

            if(stage == NEBULA) {
                // Nebula: blue-purple-gray
                int hue = rand() % 60 + 150;
                b.color = sf::Color(150 + hue/20, 150 + hue/40, 180, 100);
            } 
            else if(stage == COLLAPSE) {
                // Disk: VIVID bright color gradient by radius WITH 3D depth
                sf::Vector2f toCenter = center - b.pos;
                float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
                
                // Depth shading: particles at top of disk (higher Y) are dimmer
                float depthShade = 1.0f - abs(b.pos.y - center.y) / 200.f;
                depthShade = max(0.5f, depthShade);  // Keep minimum visibility
                
                if(dist < 100.f) {
                    b.color = sf::Color(uint8_t(100*depthShade), uint8_t(180*depthShade), 255, 255);
                } else if(dist < 160.f) {
                    b.color = sf::Color(uint8_t(120*depthShade), 255, uint8_t(200*depthShade), 255);
                } else if(dist < 240.f) {
                    b.color = sf::Color(uint8_t(200*depthShade), 255, uint8_t(100*depthShade), 255);
                } else if(dist < 320.f) {
                    b.color = sf::Color(255, uint8_t(220*depthShade), uint8_t(60*depthShade), 255);
                } else if(dist < 400.f) {
                    b.color = sf::Color(255, uint8_t(120*depthShade), uint8_t(80*depthShade), 255);
                } else {
                    b.color = sf::Color(255, uint8_t(80*depthShade), uint8_t(120*depthShade), 255);
                }
            }
            else if(stage == PROTOSTAR) {
                // Bright white/yellow as spiraling in
                uint8_t bright = uint8_t(200 + rand()%55);
                b.color = sf::Color(bright, bright, uint8_t(bright*0.9f), 180);
            }
            else if(stage == ACCRETION) {
                // Merging bodies: orange/red
                if(b.mass > 40.f) {
                    b.color = sf::Color(255, 150 + rand()%50, 80, 220);
                } else {
                    b.color = sf::Color(220, 200, 180, 160);
                }
            }
            else if(stage == DIFFERENTIATION) {
                // Inner/outer planet colors
                sf::Vector2f toCenter = center - b.pos;
                float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
                if(dist < 280.f) {
                    // Inner: brown/red (rocky)
                    b.color = sf::Color(200, 100, 60, 240);
                } else {
                    // Outer: blue (icy/gas)
                    b.color = sf::Color(80, 160, 240, 240);
                }
            }
            else if(stage == CHAOTIC) {
                // Mix of colors for chaotic field
                sf::Vector2f toCenter = center - b.pos;
                float dist = sqrt(toCenter.x*toCenter.x + toCenter.y*toCenter.y);
                if(dist < 250.f) {
                    b.color = sf::Color(200, 100, 50, 200);
                } else {
                    b.color = sf::Color(100, 150, 220, 200);
                }
            }
        }

        updatePhysics(bodies, center, stage);
        resolveCollisions(bodies, stage);

        // Draw scene
        sf::RectangleShape dim({1200, 700});
        dim.setFillColor(sf::Color(5, 5, 15, 40));  // Motion blur fade
        renderTex.draw(dim);
        renderTex.draw(stars);

        // Draw orbital trails
        drawTrails(renderTex, bodies);

        // Draw disk ring guides for Stage 2 visualization
        if(stage == COLLAPSE) {
            vector<float> ringRadii = {100.f, 160.f, 240.f, 320.f, 400.f};
            vector<sf::Color> ringColors = {
                sf::Color(100, 180, 255, 120),
                sf::Color(120, 255, 200, 120),
                sf::Color(200, 255, 100, 120),
                sf::Color(255, 220, 60, 120),
                sf::Color(255, 80, 120, 120)
            };
            
            for(size_t i = 0; i < ringRadii.size(); i++) {
                // Draw ring outline
                sf::CircleShape ringGuide(ringRadii[i]);
                ringGuide.setOrigin({ringRadii[i], ringRadii[i]});
                ringGuide.setPosition(center);
                ringGuide.setFillColor(sf::Color::Transparent);
                ringGuide.setOutlineThickness(2.f);
                ringGuide.setOutlineColor(ringColors[i]);
                renderTex.draw(ringGuide);
            }
        }

        // Draw all particles with glow
        int activeCount = 0;
        for(auto &b : bodies) {
            if(!b.active) continue;
            activeCount++;

            // Collision glow effect
            if(b.collisionGlow > 0.f && b.mass > 15.f) {
                for(int gi = 0; gi < 3; gi++) {
                    sf::CircleShape glow(b.radius * (1.5f + gi*0.8f));
                    glow.setOrigin({glow.getRadius(), glow.getRadius()});
                    glow.setPosition(b.pos);
                    uint8_t glowAlpha = uint8_t(b.collisionGlow * (200 - gi*60));
                    glow.setFillColor(sf::Color(255, 180, 60, glowAlpha));
                    renderTex.draw(glow, sf::BlendAdd);
                }
            }

            // Main particle with small glow
            if(b.mass > 20.f) {
                sf::CircleShape glow(b.radius * 1.3f);
                glow.setOrigin({glow.getRadius(), glow.getRadius()});
                glow.setPosition(b.pos);
                sf::Color glowColor = b.color;
                glowColor.a = uint8_t(glowColor.a * 0.5f);
                glow.setFillColor(glowColor);
                renderTex.draw(glow, sf::BlendAdd);
            }

            // Core particle
            sf::CircleShape particle(b.radius);
            particle.setOrigin({b.radius, b.radius});
            particle.setPosition(b.pos);
            particle.setFillColor(b.color);
            renderTex.draw(particle, sf::BlendAdd);
        }

        // Draw sun with dramatic glow
        drawSunGlow(renderTex, center, 28.f, stage);

        renderTex.display();
        window.clear(sf::Color(0, 0, 10));
        window.draw(sf::Sprite(renderTex.getTexture()));

        // HUD
        float fps = 1.f / fpsClock.restart().asSeconds();
        stringstream hud;
        hud << stageName(stage) << " | FPS: " << (int)fps << " | Bodies: " << activeCount << " | Time: " << (int)simTime << "s";
        window.setTitle(hud.str());

        window.display();
    }

    return 0;
}

