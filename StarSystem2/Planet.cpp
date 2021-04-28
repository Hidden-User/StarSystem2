#include "Planet.h"

Planet::Planet()
{
	this->p_speed = 10.0f;
	this->angle = 0.0f;
}

Planet::~Planet()
{
}

void Planet::init(float distance, float p_size, F4 color)
{
	this->distance = distance;
	this->p_size = p_size;
	this->circleInit(this->distance, this->p_size, color);
}

void Planet::stop()
{
	this->exist = false;
	this->Circle::stop();
}

void Planet::update()
{
	F2 old_;
	F2 new_;
	if (!this->exist) {
		return;
	}

	this->lock();

	this->normalise();

	float buff = ((this->p_speed * this->getDeltaTime()) / 1000.0f) / this->distance;

	old_ = this->getPositionXY();

	this->angle += buff;

	new_ = this->getPositionXY();

	this->move(new_.x - old_.x, new_.y - old_.y);

	this->unlock();

	this->draw();

}

void Planet::normalise()
{
	if (this->angle > DirectX::XM_2PI) {
		while (this->angle > DirectX::XM_2PI)
		{
			this->angle -= DirectX::XM_2PI;
		}
	}
	if (this->angle < -DirectX::XM_2PI) {
		while (this->angle < -DirectX::XM_2PI)
		{
			this->angle += DirectX::XM_2PI;
		}
	}
}

bool Planet::isActive()
{
	return this->exist;
}

F2 Planet::getPositionXY()
{
	return { cosf(this->angle) * this->distance, sinf(this->angle) * this->distance };
}

F2 Planet::getPositionRF()
{
	return { this->distance, this->angle };
}

float Planet::getSpeed()
{
	return this->p_speed;
}

void Planet::setSpeed(float p_speed)
{
	this->p_speed = p_speed;
}
