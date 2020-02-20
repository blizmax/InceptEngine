#ifndef ACTOR_COMPONENT_H
#define ACTOR_COMPONENT_H


class Actor;

class ActorComponent
{
public:
	ActorComponent();
	ActorComponent(Actor* owner);
	Actor* getComponentOwner();

protected:
	Actor* m_owner;
};

#endif