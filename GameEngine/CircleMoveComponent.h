#pragma once
#include "Component.h"
#include "Vector2.h"

class CircleMoveComponent : Component
{
public:
	CircleMoveComponent(Actor* ownerP, int updateOrderP = 100); // By default, update before other components
	CircleMoveComponent() = delete;
	CircleMoveComponent(const CircleMoveComponent&) = delete;
	CircleMoveComponent& operator=(const CircleMoveComponent&) = delete;

	void updatePosition(float dt);
	void accelerate(Vector2 accelerationP);
	Vector2 getCurrentPosition() const { return currentPosition; }
	void setCurrentPosition(Vector2 positionP);

protected:
	Vector2 acceleration;
	Vector2 currentPosition;
	Vector2 lastPosition;
};