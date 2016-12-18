#pragma once

#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <vector>

#include "maths.hpp"

class NeuralNetwork
{
public:

    static float activate(float value)
    {
        return std::tanh(value);
    }

    static float activateAlternative(float value)
    {
        return 1.0 - tanh(value) * tanh(value);
    }

    static float activateStrict(float value)
    {
        if(value == 0.f)
            return 0.f;

        return (value > 0.f) ? 1.f : -1.f;
    }

    struct Neuron
    {
        std::vector<float> weights;
        float value;
        float gradient;
    };

    using Neurons = std::vector<Neuron>;

    Neurons& getInputs()
    {
        return neurons.front();
    }

    Neurons& getOutputs()
    {
        return neurons.back();
    }

    Neurons& getLayer(uint32_t layerIndex)
    {
        return neurons[layerIndex];
    }

    uint32_t getLayerCount()
    {
        return neurons.size();
    }

    uint32_t getLayerSize(uint32_t layerIndex)
    {
        return getLayer(layerIndex).size();
    }

    void addLayer(size_t neuronCount)
    {
        if(neurons.empty())
        {
            neurons.emplace_back(neuronCount);
            return;
        }

        size_t inputCount = neurons.back().size();

        neurons.emplace_back(neuronCount);

        for(auto& neuron : neurons.back())
            neuron.weights.resize(inputCount + 1);
    }

    template<typename...Args>
    void addLayer(size_t arg, Args...args)
    {
        addLayer(arg);
        addLayer(args...);
    }

    void activate()
    {
        for(int x = 1; x < neurons.size(); x++)
        {
            Neurons& current = neurons[x];
            const Neurons& previous = neurons[x - 1];

            for(auto& neuron : current)
            {
                assert(neuron.weights.size() == previous.size() + 1); // +1 for hidden weight

                neuron.value = 0.f;
                for(int x = 0; x < previous.size(); x++)
                    neuron.value += previous[x].value * neuron.weights[x];

                neuron.value += neuron.weights.back();

                neuron.value = activate(neuron.value);
            }
        }
    }

    template<typename Container>
    void activate(const Container& inputs)
    {
        auto& inputLayer = getInputs();

        assert(inputs.size() == inputLayer.size());

        for(int x = 0; x < inputs.size(); x++)
            inputLayer[x].value = inputs[x];

        activate();
    }

    void backpropagation(const std::vector<float>& correctOutputs, float learningRate = 0.3f)
    {
        assert(correctOutputs.size() == getOutputs().size());

        std::vector<std::vector<float>> gradiants;
        gradiants.resize(getLayerCount() - 1);

        for(int x = 0; x < correctOutputs.size(); x++)
        {
            Neuron& neuron = getOutputs()[x];

            float delta = correctOutputs[x] - neuron.value;
            neuron.gradient = delta * activateAlternative(neuron.value);
        }

        for(int x = getLayerCount() - 2; x > 0; x--)
        {
            const Neurons& previousLayer = getLayer(x + 1);
            Neurons& currentLayer = getLayer(x);

            for(int y = 0; y < currentLayer.size(); y++)
            {
                Neuron& neuron = currentLayer[y];

                float dow = 0.f;

                for(const auto& previousNeuron : previousLayer)
                    dow += previousNeuron.weights[y] * previousNeuron.gradient;

                neuron.gradient = dow * activateAlternative(neuron.value);
            }
        }

        for(size_t x = 1; x < getLayerCount(); x++)
        {
            auto& previousLayer = getLayer(x - 1);
            auto& layer = getLayer(x);
            for(size_t y = 0; y < layer.size(); y++)
            {
                auto& neuron = layer[y];
                for(size_t z = 0; z < previousLayer.size(); z++)
                    neuron.weights[z] += learningRate * neuron.gradient * previousLayer[z].value;

                neuron.weights.back() += learningRate * neuron.gradient;
            }
        }
    }

    void mutate(float mutationRate, float alpha)
    {
        for(auto& layer : neurons)
        {
            for(auto& neuron : layer)
            {
                for(auto& weight : neuron.weights)
                {
                    if(probability(mutationRate))
                        weight += rand(-alpha, alpha);
                }
            }
        }
    }

    void mutate(float mutationRate, float min, float max)
    {
        for(auto& layer : neurons)
        {
            for(auto& neuron : layer)
            {
                for(auto& weight : neuron.weights)
                {
                    if(probability(mutationRate))
                        weight = rand(min, max);
                }
            }
        }
    }

    void init(float min = -1.f, float max = 1.f)
    {
        for(auto& layer : neurons)
        {
            for(auto& neuron : layer)
            {
                for(auto& weight : neuron.weights)
                    weight = rand(min, max);
            }
        }
    }

    NeuralNetwork() = default;

    template<typename...Args>
    NeuralNetwork(Args...args)
    {
        addLayer(args...);
    }


//private:
    std::vector<Neurons> neurons;
};
