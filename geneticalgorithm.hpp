#pragma once

#include <functional>

#include "neuralnetwork.h"

template<typename EnvironmentType>
class GeneticAlgorithm
{
public:
    GeneticAlgorithm(EnvironmentType& environment, size_t agentCount = 15) : environment(environment), agentCount(agentCount)
    {
        const auto inputCount = EnvironmentType::OUTPUT_COUNT;
        const auto ouputCount = EnvironmentType::INPUT_COUNT;

        anns.assign(agentCount, {inputCount, inputCount * 3, inputCount * 2, inputCount, ouputCount* 2, ouputCount});

        for(auto& ann : anns)
            ann.init();

        environment.agents.resize(agentCount);
        environment.nextGeneration();
    }

    size_t rouletteSelection(int64_t totalScore)
    {
        int64_t current = 0;
        auto selected = rand((int64_t)0, totalScore);
        for(size_t x = 0; x < agentCount; x++)
        {
            current += environment.agents[x].score;

            if(current >= selected)
                return x;
        }
        return agentCount - 1;
    }

    bool step()
    {
        bool done = currentStep++ >= maxStepCount;
        done |= !environment.step();

        for(size_t x = 0; x < agentCount; x++)
        {
            auto& ann = anns[x];
            auto& agent = environment.agents[x];

            ann.activate(agent.outputs);

            for(size_t y = 0; y < agent.outputs.size(); y++)
                agent.inputs[y] = ann.getOutputs()[y].value;
        }

        if(done)
        {
            currentStep = 0;
            environment.finishGeneration();

            maxScore = std::numeric_limits<int32_t>::min();
            minScore = std::numeric_limits<int32_t>::max();
            size_t maxScoreIndex = 0;

            int64_t totalScore = 0;

            for(size_t x = 0; x < agentCount; x++)
            {
                auto& agent = environment.agents[x];
                totalScore += agent.score;
                if(agent.score > maxScore)
                {
                    maxScore = agent.score;
                    maxScoreIndex = x;
                }

                if(agent.score < minScore)
                    minScore = agent.score;
            }

            averageScore = totalScore / (int64_t)agentCount;

            for(auto& agent : environment.agents)
                std::cout << "FINAL SCORE: " << agent.score << std::endl;

            std::cout << "MIN SCORE: " << minScore << std::endl;
            std::cout << "MAX SCORE: " << maxScore << std::endl;
            std::cout << "AVG SCORE: " << averageScore << std::endl;

            if(minScore < 0)
            {
                for(auto& agent : environment.agents)
                    agent.score -= minScore;

                totalScore -= minScore * agentCount;
            }

            std::vector<NeuralNetwork> newAnns(agentCount);

            //Save Best
//            newAnns[0] = anns[maxScoreIndex];

            //Replicate Best
//            newAnns[1] = anns[maxScoreIndex];
//            newAnns[2] = anns[maxScoreIndex];
//            newAnns[3] = anns[maxScoreIndex];

            for(size_t x = 0; x < agentCount; x++)
            {
                newAnns[x] = anns[rouletteSelection(totalScore)];
                newAnns[x].mutate(mutationRate, mutationAlpha);
            }
//
//            for(size_t x = 1; x < agentCount; x++)

            anns = newAnns;

            environment.nextGeneration();

            return true;
        }

        return false;
    }

    int32_t maxScore;
    int32_t minScore;
    int32_t averageScore;

private:
    EnvironmentType& environment;

    size_t agentCount;

    size_t currentStep = 0;
    size_t maxStepCount = 2000;

    float mutationRate = 0.1f;
    float mutationAlpha = 0.2f;

    std::vector<NeuralNetwork> anns;
};
