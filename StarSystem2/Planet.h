#pragma once
#include "Circle.h"

class StarSystem;

class Planet :
	public Circle
{
	friend StarSystem;
public:
	Planet();
	~Planet();

	void init(float distance, float p_size, F4 color);
	void stop();

	void update();
	void normalise();

	bool isActive();
	F2 getPositionXY();
	F2 getPositionRF();
	float getSpeed();
	void setSpeed(float p_speed);

private:

	bool exist;
	float distance;
	float angle;
	float p_size;
	float p_speed;
};

