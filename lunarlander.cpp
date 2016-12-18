#include "lunarlander.h"

#include "maths.hpp"

LunarLander::LunarLander()
{

}

void LunarLander::setup(sf::Vector2f size, uint32_t agentCount)
{
    landSize = size;

    landerShape.setSize({10, 10});
    landerShape.setOrigin({5, 5});

//    agents.resize(agentCount);
}

void LunarLander::nextGeneration()
{
    landscape.clear();

    landscape.push_back({-50, 0});
    landscape.push_back({0, landSize.y * 0.7f});

    const float landingSize = 100;

    float landingPos = rand(0, landSize.x - landingSize);

    bool landingPlaced = false;

    float angle = 90;

    float minY = 100.f;
    float maxY = landSize.y;

    while(landscape.back().x < landSize.x)
    {
        if(!landingPlaced && landscape.back().x > landingPos)
        {
            landingPlaced = true;
            landingIndex = landscape.size() - 1;
            landingPoint = landscape.back() + sf::Vector2f(landingSize/2.f, 0);
            landscape.push_back(landscape.back() + sf::Vector2f(landingSize, 0));
            continue;
        }

        auto angle = rand(10.f, 170.f);
//        auto angle = rand(50.f, 130.f);
//        angle += rand(25.f, -25.f);

        angle = std::min(angle, 180.f);
        angle = std::max(angle, 0.f);

        auto length = rand(10.f, 100.f);

        landscape.push_back(landscape.back() + angleToVector(angle) * length);

        float& y = landscape.back().y;
        y = std::max(y, minY);
        y = std::min(y, maxY);
    }

    landscape.back().x = landSize.x;
    landscape.push_back({landSize.x + 50, 0});

    Agent defaultAgent;

    defaultAgent.velocity = {};
    defaultAgent.position = {landSize.x / 2.f, 50.f};
    defaultAgent.rotation = 0;
    defaultAgent.thrust = 0;

    defaultAgent.score = 10000;

    for(auto& input : defaultAgent.inputs)
        input = 0;

    for(auto& output : defaultAgent.outputs)
        output = 0;

    defaultAgent.running = true;

    for(auto& agent : agents)
        agent = defaultAgent;
}

void LunarLander::finishGeneration()
{
    for(auto& agent : agents)
    {
        float distToLanding = lenght(landingPoint - agent.position);
        float velocity = lenght(agent.velocity);

        agent.score -= velocity * 10;
        agent.score -= agent.rotation * 100;
        agent.score -= distToLanding * 1000;
    }
}

bool LunarLander::step()
{
    rays.clear();

    bool running = false;

    for(auto& agent : agents)
    {
        running |= agent.running;

        if(!agent.running)
        {
            if(renderTarget != nullptr)
            {
                landerShape.setFillColor(sf::Color::Green);
                landerShape.setPosition(agent.position);
                landerShape.setRotation(agent.rotation);
                renderAgent(agent);
            }
            continue;
        }

//        std::cout << agent.score << std::endl;

        agent.score -= 1;

        bool thrusting = agent.inputs[I_THRUST] > 0.5f;
        bool rotating = std::abs(agent.inputs[I_ROTATION]) > 0.5f;

        if(thrusting)
        {
            agent.score -= 10;
//            agent.velocity += angleToVector(agent.rotation) * THRUST_FACTOR * stepFactor;
            agent.velocity += std::abs(agent.inputs[I_THRUST]) * angleToVector(agent.rotation) * THRUST_FACTOR * stepFactor;
        }

        if(rotating)
        {
//            agent.rotation += (agent.inputs[I_ROTATION] > 0.f ? 1.f : -1.f) * ROTATION_FACTOR * stepFactor;
            agent.rotation += agent.inputs[I_ROTATION] * ROTATION_FACTOR * stepFactor;
        }

        agent.velocity += gravity * stepFactor;
        agent.position += agent.velocity * stepFactor;

        landerShape.setPosition(agent.position);
        landerShape.setRotation(agent.rotation);

        sf::Vector2f rayPos = agent.position;

        size_t outputIndex = 0;
        for(auto angle : angles)
        {
            sf::Vector2f rayDir = angleToVector(agent.rotation + angle);
            float min = 9999999.f;
            sf::Vector2f prev = landscape.front();
            for(auto& point : landscape)
            {
                sf::Vector2f segment = point;
                sf::Vector2f segmentSize = prev - point;
                float t1 = rayVsSegment(rayPos, rayDir, segment, segmentSize);
                if(t1 != -1.f && t1 < min)
                    min = t1;

                prev = point;
            }

            min = std::min(min, RAY_RADIUS);

            agent.outputs[outputIndex++] = 1.f - (min / RAY_RADIUS);

            if(renderTarget && displayRays)
            {
                rays.append(rayPos);
                rays.append(rayPos + rayDir * min);
            }
        }

        agent.outputs[O_ROTATION] = ((int)agent.rotation % 360) / 360.f;

        agent.outputs[O_VELOCITY] = lenght(agent.velocity) / 100.f;
        agent.outputs[O_VELOCITY_X] = agent.velocity.x / 100.f;
        agent.outputs[O_VELOCITY_Y] = agent.velocity.y / 100.f;

        sf::Vector2f landingDir = normalize(landingPoint - agent.position);
        float landingDist = std::min(lenght(landingPoint - agent.position), RAY_RADIUS);

        agent.outputs[O_LANDING_DIST] = landingDist / RAY_RADIUS;
        agent.outputs[O_LANDING_DIR] = vectorToAngle(landingDir) / 180.f;
        agent.outputs[O_LANDING_DIR_X] = landingDir.x;
        agent.outputs[O_LANDING_DIR_Y] = landingDir.y;

        std::array<sf::Vector2f, 4> landerPoints;
        landerPoints[0] = landerShape.getTransform() * landerShape.getPoint(0);
        landerPoints[1] = landerShape.getTransform() * landerShape.getPoint(1);
        landerPoints[2] = landerShape.getTransform() * landerShape.getPoint(2);
        landerPoints[3] = landerShape.getTransform() * landerShape.getPoint(3);

        std::array<sf::Vector2f, 3> axes;
        axes[1] = normalize(perpandicular(landerPoints[0] - landerPoints[1]));
        axes[2] = normalize(perpandicular(landerPoints[0] - landerPoints[3]));

        auto prev = landscape.front();
        float landerMin;
        float landerMax;
        bool collide = false;
        sf::Vector2f collisionPoint;
        for(auto& point : landscape)
        {
            if(prev == point)
                continue;

            float segMin;
            float segMax;

            axes[0] = perpandicular(normalize(prev - point));

            bool overlap = true;
            for(auto& axi : axes)
            {
                projectOntoAxis(std::array<sf::Vector2f, 2>{prev, point}, axi, segMin, segMax);
                projectOntoAxis(landerPoints, axi, landerMin, landerMax);

                if(!(landerMin <= segMax && landerMax >= segMin))
                {
                    overlap = false;
                    break;
                }
            }

            if(overlap)
            {
                collide = true;
                collisionPoint = point;
                break;
            }

            prev = point;
        }

        if(collide)
        {
            agent.running = false;

            bool onLanding = collisionPoint == landscape[landingIndex + 1];

            if(onLanding)
                agent.score += 10000;
            else
                agent.score -= 1000;

        }
        else
            landerShape.setFillColor(sf::Color::White);

        renderAgent(agent);
    }

    renderGeneration();

    return running;
}

void LunarLander::renderGeneration()
{
    if(renderTarget == nullptr || computerVision)
        return;

    sf::VertexArray land;
    land.setPrimitiveType(sf::PrimitiveType::LinesStrip);
    for(size_t x = 0; x < landscape.size(); x++)
    {
        land.append({landscape[x]});

        if(x == landingIndex)
        {
            land.append({landscape[x], sf::Color::Green});
            land.append({landscape[x + 1], sf::Color::Green});
        }
    }

    if(displayRays)
    {
        rays.setPrimitiveType(sf::PrimitiveType::Lines);
        renderTarget->draw(rays);
    }

    renderTarget->draw(land);
}

void LunarLander::renderAgent(const Agent& agent)
{
    if(renderTarget == nullptr)
        return;

    if(displayRays)
    {
        sf::Vector2f landingDir = normalize(landingPoint - agent.position);
        rays.append({agent.position, sf::Color::Blue});
        rays.append({agent.position + landingDir * 30.f, sf::Color::Blue});
    }

    renderTarget->draw(landerShape);
}

