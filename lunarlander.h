#pragma once

#include <iostream>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class LunarLander
{
    public:
        enum INPUTS{I_THRUST, I_ROTATION, INPUT_COUNT};
        enum OUTPUTS{
//                        O_RAY_0, O_RAY_1, O_RAY_2, O_RAY_3, O_RAY_4,
                        O_RAY_0, O_RAY_1, O_RAY_2,
//                        O_RAY_0,
                        O_ROTATION,
                        O_VELOCITY, O_VELOCITY_X, O_VELOCITY_Y,
                        O_LANDING_DIR, O_LANDING_DIST, O_LANDING_DIR_X, O_LANDING_DIR_Y,
                        OUTPUT_COUNT
                    };


//        const static std::array<float, 5> angles = {180, 160, -160, 140, -140};
//        const std::array<float, 1> angles = {180};
        const std::array<float, 3> angles = {{180, 150, -150}};

        struct Agent
        {
            sf::Vector2f velocity;
            sf::Vector2f position;
            float rotation;
            float thrust;

            int32_t score = 100000;

            std::array<float, INPUT_COUNT> inputs;
            std::array<float, OUTPUT_COUNT> outputs;

            bool running = true;
        };

        LunarLander();

        void setup(sf::Vector2f size, uint32_t agentCount = 10);

        void nextGeneration();
        void finishGeneration();
        bool step();

        void renderGeneration();
        void renderAgent(const Agent& agent);

        float stepFactor = 1.f/60.f;

        const float THRUST_FACTOR = 30;
//        const float ROTATION_FACTOR = 90;
        const float ROTATION_FACTOR = 80;

        const float RAY_RADIUS = 500;

        const sf::Vector2f gravity = {0, 10};

        std::vector<Agent> agents;

        std::vector<sf::Vector2f> landscape;
        sf::Vector2f landSize;
        sf::Vector2f landingPoint;
        size_t landingIndex;

        sf::RectangleShape landerShape;
        sf::VertexArray rays;

        bool computerVision = false;
        bool displayRays = false;
        sf::RenderTarget* renderTarget = nullptr;
};
