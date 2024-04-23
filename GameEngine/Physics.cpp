#include "Physics.h"
#include "Window.h"
#include "Actor.h"
#include "CircleActor.h"
#include "Game.h"
#include <algorithm>
#include <iostream>
using namespace std;

void Physics::addCircle(CircleMoveComponent* circle)
{
	circles.push_back(circle);
}

void Physics::removeCircle(CircleMoveComponent* circle)
{
	auto iter = std::find(begin(circles), end(circles), circle);
	circles.erase(iter);
}

void Physics::setBottomBorder(float bottomBorderP)
{
	bottomBorder = bottomBorderP;
}

void Physics::setLeftBorder(float leftBorderP)
{
	leftBorder = leftBorderP;
}

void Physics::setRightBorder(float rightBorderP)
{
	rightBorder = rightBorderP;
}

void Physics::setTopBorder(float topBorderP)
{
	topBorder = topBorderP;
}

void Physics::computePhysics(float dt)
{
	deltaTime = dt;
	const float substepDt = getStepDeltaTime();
	for (size_t i = substeps; i > 0; i--)
	{
		// Apply gravity to circles
		applyGravity();

		// Apply constraints to circles
		applyConstraint();

		// Solve collisions between circles
		solveCollisions();

		// Update positions of circles
		updatePositions(substepDt);
	}
}

void Physics::updatePositions(float dt)
{
	for (auto circle : circles)
	{
		circle->updatePosition(dt);
	}
}

void Physics::applyGravity()
{
	for (auto circle : circles)
	{
		circle->accelerate(gravity);
	}
}

void Physics::applyConstraint()
{
	for (auto circle : circles)
	{
		if (circle->getCurrentPosition().y > bottomBorder - circle->getRadius())
		{
			circle->setCurrentPosition(Vector2(circle->getCurrentPosition().x, bottomBorder - circle->getRadius()));
			Vector2 velocity = Vector2(circle->getCurrentPosition().x, bottomBorder - circle->getRadius()) - circle->getCurrentPosition();
			if (velocity.length() > 3.0f) {
				//cout << velocity.length() << endl;
				circle->setVelocity(Vector2(0.0f, 0.0f), getStepDeltaTime());
			}
		}

		if (circle->getCurrentPosition().y < topBorder + circle->getRadius())
		{
			//circle->setCurrentPosition(Vector2(circle->getCurrentPosition().x, bottomBorder + circle->getRadius()));
		}

		if (circle->getCurrentPosition().x < leftBorder + circle->getRadius())
		{
			circle->setCurrentPosition(Vector2(leftBorder + circle->getRadius(), circle->getCurrentPosition().y));
			Vector2 velocity = Vector2(leftBorder + circle->getRadius(), circle->getCurrentPosition().y) - circle->getCurrentPosition();
			if (velocity.length() > 3.0f) {
				//cout << velocity.length() << endl;
				circle->setVelocity(Vector2(0.0f, 0.0f), getStepDeltaTime());
			}
		}

		if (circle->getCurrentPosition().x > rightBorder - circle->getRadius())
		{
			circle->setCurrentPosition(Vector2(rightBorder - circle->getRadius(), circle->getCurrentPosition().y));
			Vector2 velocity = Vector2(rightBorder - circle->getRadius(), circle->getCurrentPosition().y) - circle->getCurrentPosition();
			if (velocity.length() > 3.0f) {
				//cout << velocity.length() << endl;
				circle->setVelocity(Vector2(0.0f, 0.0f), getStepDeltaTime());
			}
		}
	}
}

void Physics::solveCollisions()
{
	circlesToRemove.clear();
	const int size = circles.size();
	for (size_t i = 0; i < size; i++)
	{
		CircleMoveComponent* lCircle = circles[i];
		for (size_t k = i + 1; k < size; k++)
		{
			CircleMoveComponent* rCircle = circles[k];
			const Vector2 collisionDir = lCircle->getCurrentPosition() - rCircle->getCurrentPosition();
			const float distance = collisionDir.length();

			const float radiusSum = lCircle->getRadius() + rCircle->getRadius();

			if (distance < radiusSum)
			{
				const Vector2 normal = collisionDir / distance;
				const float delta = radiusSum - distance;

				Vector2 lVelocity = (lCircle->getCurrentPosition() + normal * delta / 2.0f) - lCircle->getCurrentPosition();
				Vector2 rVelocity = (rCircle->getCurrentPosition() - normal * delta / 2.0f) - rCircle->getCurrentPosition();


				lCircle->setCurrentPosition(lCircle->getCurrentPosition() + normal * delta / 2.0f);
				rCircle->setCurrentPosition(rCircle->getCurrentPosition() - normal * delta / 2.0f);

				if (lVelocity.length() > 3.0f || rVelocity.length() > 3.0f)
				{
					cout << lVelocity.length() << " " << rVelocity.length() << endl;
					lCircle->setVelocity(Vector2(0.0f, 0.0f), getStepDeltaTime());
					rCircle->setVelocity(Vector2(0.0f, 0.0f), getStepDeltaTime());
				}
				checkSameFruits(lCircle, rCircle);
			}
		}
	}
	mergeFruits();
}

bool Physics::checkSameFruits(CircleMoveComponent* lCircle, CircleMoveComponent* rCircle)
{
	CircleActor* lCircleActor = static_cast<CircleActor*>(&lCircle->getOwner());
	CircleActor* rCircleActor = static_cast<CircleActor*>(&rCircle->getOwner());
	if (lCircleActor->getFruit().getRadius() == rCircleActor->getFruit().getRadius())
	{
		if (std::find(circlesToRemove.begin(), circlesToRemove.end(), lCircle) == circlesToRemove.end() && std::find(circlesToRemove.begin(), circlesToRemove.end(), rCircle) == circlesToRemove.end()) {
			circlesToRemove.push_back(lCircle);
			circlesToRemove.push_back(rCircle);
			return true;
		}
	}
	return false;
}

void Physics::mergeFruits()
{
	if (circlesToRemove.size() == 0)return;

	CircleActor* fruitActor = static_cast<CircleActor*>(&circlesToRemove[0]->getOwner());
	Fruit fruit = fruitActor->getFruit();
	Fruit nextFruit;
	auto iter = std::find(Fruits::fruitList.begin(), Fruits::fruitList.end(), fruit);
	if (iter != Fruits::fruitList.end() && std::next(iter) != Fruits::fruitList.end()) {
		nextFruit = *(std::next(iter));
	}

	Game& game = circlesToRemove[0]->getOwner().getGame();

	Vector2 position = circlesToRemove[0]->getCurrentPosition();
	Vector2 position2 = circlesToRemove[1]->getCurrentPosition();
	Vector2 spawnPos = position + (position2 - position)/2;
	spawnPos.y -= circlesToRemove[0]->getRadius();

	for (size_t i = 0; i < circlesToRemove.size(); i++)
	{
		circlesToRemove[i]->getOwner().setState(Actor::ActorState::Dead);
		removeCircle(circlesToRemove[i]);
	}
	if (nextFruit.getRadius() != 0.0f) {
		game.spawnFruit(Fruit(nextFruit), spawnPos);
	}
}

void Physics::setObjectVelocity(CircleMoveComponent& circle, Vector2 velocityP) const
{
	circle.setVelocity(velocityP, getStepDeltaTime());
}