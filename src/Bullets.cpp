#include "stdafx.h"
#include "Bullets.h"

//Bullet::Bullet() :
//	_damage(0)
//{
//}

Bullet::Bullet(int dmg, FPoint start, float angle, float speed, std::string type) :
	_damage(dmg),
	_startPoint(start),
	_angle(angle),
	_speed(speed),
	_type(type)
{
	// макс длина траектории
	_maxSplineLength = sqrt(pow(Render::device.Height(), 2) + pow(Render::device.Width(), 2));
	CalculateTrajectory(_startPoint, _angle);
}

Bullet::~Bullet() {

}

// пока не используется, но может при переменной величине урона (например если будут использоваться усилители)
void Bullet::SetDamage(int dmg) 
{
	_damage = dmg;
}

FPoint Bullet::GetPosition(float time) const
{
	FPoint currentPosition = _bulletTrajectory.getGlobalFrame(math::clamp(0.0f, 1.0f, time / (_maxSplineLength/_speed)));
	return currentPosition;
}

void Bullet::CalculateTrajectory(FPoint start, float angle)
{
	_bulletTrajectory.addKey(0.0, _startPoint);
	_finishPoint.x = _startPoint.x - _maxSplineLength * cos(angle);
	_finishPoint.y = _maxSplineLength * sin(angle);
	_bulletTrajectory.addKey(1.0, _finishPoint);
	_bulletTrajectory.CalculateGradient();
}

int Bullet::GetDamage() const 
{
	return _damage;
}

std::string Bullet::GetType() const {
	return _type;
}

CannonBall::CannonBall(int dmg, FPoint start, float angle, float speed, std::string type): Bullet(dmg, start, angle, speed, type) {}
CannonBall::~CannonBall() {}

void CannonBall::Collide() {

}

Bomb::Bomb(int dmg, FPoint start, float angle, float speed, std::string type) : Bullet(dmg, start, angle, speed, type) {}

Bomb::~Bomb() {}

void Bomb::Collide() {

}