#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include "maths.hpp"
#include "lunarlander.h"
#include "neuralnetwork.h"
#include "geneticalgorithm.hpp"

struct InputKey
{
    static constexpr auto ANY_KEY = sf::Keyboard::KeyCount;

    size_t index;
    sf::Keyboard::Key key;
    float value = 1;
    enum Type {SET, ADD, MULTIPLY} type = SET;
};

int main()
{
    seed(time(NULL));

    {
    struct TrainSet
    {
        std::vector<float> inputs;
        std::vector<float> outputs;
    };

    std::vector<TrainSet> trainSets =
    {
        {
            {-1, -1},
            {-1}
        },

        {
            {1, 1},
            {-1}
        },

        {
            {1, -1},
            {1}
        },

        {
            {-1, 1},
            {1}
        }
    };

    NeuralNetwork ann(2, 2, 1);
    ann.init();

    std::vector<NeuralNetwork> anns;

//    for(int x = 0; x < 10; x++)
    for(int x = 0; x < 1; x++)
    {
        anns.emplace_back(2, 2, 1);
        anns.back().init();
        anns.back().mutate(0.1f, 0.5f);
    }

    int currentSet = 0;
//    while(true)
    while(false)
    {
        std::vector<float> fitness;

//        auto& trainSet = trainSets[currentSet];
//
//        currentSet++;
//        currentSet %= trainSets.size();

        for(NeuralNetwork& network : anns)
        {
            float fit = 0.f;

            for(const auto& trainSet : trainSets)
            {
                network.activate(trainSet.inputs);
                network.backpropagation(trainSet.outputs, 0.15f);

//                for(auto& neuron : network.getOutputs())
//                    neuron.value = NeuralNetwork::activateStrict(neuron.value);

//                std::cout << trainSet.outputs[0] << " : " << network.getOutputs()[0].value << std::endl;
                for(int x = 0; x < trainSet.outputs.size(); x++)
//                    fit += trainSet.outputs[x] == network.getOutputs()[x].value;
                    fit -= std::abs(trainSet.outputs[x] - network.getOutputs()[x].value);
            }

            fitness.push_back(fit);
        }

        float maxFitness = -9999999999999.f;
        size_t maxIndex = 0;

        for(int x = 0; x < fitness.size(); x++)
        {
//            std::cout << fitness[x] << std::endl;
            if(maxFitness < fitness[x])
            {
                maxFitness = fitness[x];
                maxIndex = x;
            }
        }

        ann = anns[maxIndex];
        for(auto& network : anns)
        {
            network = ann;
//            network.mutate(0.1f, -1.f, 1.f);
//            network.mutate(0.1f, 0.5f);
        }
        anns[0] = ann;

        std::cout << (int)maxFitness << std::endl;
        if(maxFitness > -1)
            break;
    }
//    return 0;
    }


    bool maxSpeed = false;

    LunarLander lander;

    sf::RenderWindow window(sf::VideoMode(700, 500), "Lander");
    window.setFramerateLimit(1.f/lander.stepFactor);

    ImGui::SFML::Init(window);

    lander.setup((sf::Vector2f)window.getSize());
    lander.renderTarget = &window;
    lander.nextGeneration();

    GeneticAlgorithm<LunarLander> ga(lander, 30);

//    lander.agents[1].position += sf::Vector2f(100, 0);

    std::vector<InputKey> keys;

    keys.push_back({LunarLander::I_THRUST, InputKey::ANY_KEY, 0});
    keys.push_back({LunarLander::I_THRUST, sf::Keyboard::Up, 1});

    keys.push_back({LunarLander::I_ROTATION, InputKey::ANY_KEY, 0});
    keys.push_back({LunarLander::I_ROTATION, sf::Keyboard::Left, -1});
    keys.push_back({LunarLander::I_ROTATION, sf::Keyboard::Right, 1});

    auto view = window.getDefaultView();
    view.zoom(1.1f);
    window.setView(view);

    std::vector<float> maxFitness;
    std::vector<float> minFitness;
    std::vector<float> averageFitness;

    sf::Clock deltaClock;
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::C)
                    lander.computerVision = ! lander.computerVision;
                if(event.key.code == sf::Keyboard::G)
                    lander.nextGeneration();
                if(event.key.code == sf::Keyboard::R)
                    lander.displayRays = !lander.displayRays;
                if(event.key.code == sf::Keyboard::T)
                {
                    if(lander.renderTarget != nullptr)
                        lander.renderTarget = nullptr;
                    else
                        lander.renderTarget = &window;

                }
                if(event.key.code == sf::Keyboard::I)
                {
//                    for(auto)
//                    if(lander.renderTarget != nullptr)
//                        lander.renderTarget = nullptr;
//                    else
//                        lander.renderTarget = &window;

                }
                if(event.key.code == sf::Keyboard::V)
                {
                    maxSpeed = !maxSpeed;
                    window.setFramerateLimit(maxSpeed ? 99999999999 : 1.f / lander.stepFactor);
                }
            }
        }

//        for(const auto& input : keys)
//        {
//            if(input.key == InputKey::ANY_KEY || sf::Keyboard::isKeyPressed(input.key))
//            {
//                for(auto& agent : lander.agents)
//                {
//                    if(input.type == InputKey::SET)
//                        agent.inputs[input.index] = input.value;
//                    if(input.type == InputKey::ADD)
//                        agent.inputs[input.index] += input.value;
//                    if(input.type == InputKey::MULTIPLY)
//                        agent.inputs[input.index] *= input.value;
//                }
//            }
//        }

        if(lander.renderTarget != nullptr)
        {
            ImGui::SFML::Update(deltaClock.restart());
            window.clear();
        }

//        float ms = deltaClock.restart().asMilliseconds();
//        std::cout << ms << std::endl;

//        lander.step();
        if(ga.step())
        {
            const int maxGenHistory = 100;

            maxFitness.push_back((int32_t)(float)ga.maxScore);
            minFitness.push_back((int32_t)(float)ga.minScore);
            averageFitness.push_back((int32_t)(float)ga.averageScore);

            if(maxFitness.size() >= maxGenHistory)
            {
                maxFitness.erase(maxFitness.begin(), maxFitness.begin() + maxGenHistory / 2);
                minFitness.erase(minFitness.begin(), minFitness.begin() + maxGenHistory / 2);
                averageFitness.erase(averageFitness.begin(), averageFitness.begin() + maxGenHistory / 2);
            }
        }

//        sf::VertexArray speeds;
//        speeds.setPrimitiveType(sf::PrimitiveType::Quads);
//        speeds.append(sf::Vector2f(50, 100));
//        speeds.append(sf::Vector2f(50, 110));
//        speeds.append(sf::Vector2f(50 + lander.velocity.x, 110));
//        speeds.append(sf::Vector2f(50 + lander.velocity.x, 100));
//
//        speeds.append(sf::Vector2f(50, 120));
//        speeds.append(sf::Vector2f(50, 130));
//        speeds.append(sf::Vector2f(50 + lander.velocity.y, 130));
//        speeds.append(sf::Vector2f(50 + lander.velocity.y, 120));

//        window.draw(speeds);

        if(lander.renderTarget != nullptr)
        {
            ImGui::Begin("Fitness History");
            if(maxFitness.size())
            {
                ImGui::PlotLines("Max", maxFitness.data(), maxFitness.size(), 0, NULL, -200000, 200000);
                ImGui::PlotLines("Min", minFitness.data(), minFitness.size(), 0, NULL, -200000, 200000);
                ImGui::PlotLines("Average", averageFitness.data(), averageFitness.size(), 0, NULL, -200000, 200000);
            }
//            ImGui::PlotHistogram("Fitness", maxFitness.data(), maxFitness.size());
            ImGui::End();
        }

        if(lander.renderTarget != nullptr)
        {
            ImGui::Render();
            window.display();
        }
    }
}
