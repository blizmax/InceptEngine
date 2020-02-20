#include "ActorComponent.h"

ActorComponent::ActorComponent()
{
	m_owner = nullptr;
}

ActorComponent::ActorComponent(Actor* owner)
{
	m_owner = owner;
}

Actor* ActorComponent::getComponentOwner()
{
	return m_owner;
}
