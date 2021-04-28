#pragma once
#include "Planet.h"
class StarSystem :
	public Circle
{
public:
	StarSystem();
	~StarSystem();

	void init(unsigned countOfPlanet);
	void update();
	void stop();
	void modifyPlanetSpeed(unsigned index, float speed, unsigned type = 0u);

private:

	Planet** planets;
	std::thread** __planets;
	unsigned countOfPlanet;
};

