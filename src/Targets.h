#pragma once
#include <Utils\FPoint.h>

class Target
{
public:
	Target() = delete;
	Target(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type);
	~Target();
	void SetNewStartPosition(FPoint start, float angle, float time);

	void Damage(int dmg);
	int GetHealth();
	FPoint GetPosition(float time);
	float GetAngle();
	std::string GetType();
	FPoint GetVector();
	std::pair<FPoint, FPoint> GetStartFinishPos();

protected:
	void SetHealth(int hp);
	virtual void CalculateTrajectory();
private:

	float _sizeH;
	float _sizeW;
	int _health;
	float _angle;
	float _maxSplineLength;
	FPoint _startPosition;
	FPoint _finishPosition;
	TimedSpline<FPoint> _targetTrajectory;
	float _dtime;
	std::string _type;
};


class YellowTarget : public Target {
public:
	YellowTarget(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type);
	~YellowTarget();
private:
	
};

class RedTarget : public Target {
public:
	RedTarget(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type);
	~RedTarget();
};