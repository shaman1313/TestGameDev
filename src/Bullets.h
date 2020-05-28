#pragma once
#include <Utils\FPoint.h>


class Bullet
{
public:
	Bullet() = delete;
	Bullet(int dmg, FPoint start, float angle, float speed, std::string type);
	~Bullet();

	void SetDamage(int dmg);
	FPoint GetPosition(float time) const;
	virtual void CalculateTrajectory(FPoint start, float angle);
	virtual void Collide() = 0;
	int GetDamage() const;
	std::string GetType() const;
	
private:
	
	int _damage;
	FPoint _startPoint;
	FPoint _finishPoint;
	float _angle;
	float _maxSplineLength;
	float _speed;
	TimedSpline<FPoint> _bulletTrajectory;
	const std::string _type;
};

class CannonBall : public Bullet {
public:
	CannonBall(int dmg, FPoint start, float angle, float speed, std::string type);
	~CannonBall();
	void Collide() override;
};

class Bomb : public Bullet {
public:
	Bomb(int dmg, FPoint start, float angle, float speed, std::string type);
	~Bomb();
	void Collide() override;
};