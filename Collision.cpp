#include "Collision.h"
#include "Actor.h"
CapsuleCollision::CapsuleCollision(Actor* owner, glm::vec4 A, glm::vec4 B, float r)
{
	m_owner = owner;
	m_A = A;
	m_B = B;
	m_R = r;
}

CapsuleCollision::~CapsuleCollision()
{
}



float pointDistance(glm::vec3 a, glm::vec3 b)
{
    return glm::distance(a, b);
}

#include <algorithm>
#include <set>

bool CapsuleCollision::operator==(const CapsuleCollision& other)
{
    auto a0 = glm::vec3(m_owner->getActorTransformation() * m_A);
    auto b0 = glm::vec3(m_owner->getActorTransformation() * m_B);
    auto a1 = glm::vec3(other.m_owner->getActorTransformation() * other.m_A);
    auto b1 = glm::vec3(other.m_owner->getActorTransformation() * other.m_B);
    float r = m_R + other.m_R;
    auto w0 = a0 - a1;
    auto u = b0 - a0;
    auto v = b1 - a1;
    auto a = glm::dot(u, u);
    auto b = glm::dot(u, v);
    auto c = glm::dot(v, v);
    auto d = glm::dot(u, w0);
    auto e = glm::dot(v, w0);
    auto s = (b * e - c * d) / (a * c - b * b);
    auto t = (a * e - b * d) / (a * c - b * b);


    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        auto distance = pointDistance(a0 + s * u, a1 + t * v);
        return distance < r;
    }
    else
    {
        std::set<float> comparison = { pointDistance(a0, a1), pointDistance(a0, b1), pointDistance(b0, a1), pointDistance(b0, b1) };
        auto distance = *comparison.begin();

        return distance < r;
    }
}
