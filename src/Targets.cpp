#include "stdafx.h"
#include "Targets.h"
#include "bla_vector.h"


Target::Target(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type):
	_health(health),
	_startPosition(start),
	_angle(angle),
	_dtime(0),
	_sizeW(sizeWidth),
	_sizeH(sizeHeight),
	_type(type)
{	// ������������ ����� ����������
	_maxSplineLength = sqrt(pow(Render::device.Height(), 2) + pow(Render::device.Width(), 2));
	CalculateTrajectory();
}

Target::~Target()
{
}

void Target::SetNewStartPosition(FPoint start, float angle, float time)
{
	_dtime = time;// -0.05;
	_startPosition = start;
	_angle = angle;
	CalculateTrajectory();
}

void Target::Damage(int dmg)
{
	_health -= dmg;
}

int Target::GetHealth() {
	return _health;
}

float Target::GetAngle() {
	return _angle;
}

std::string Target::GetType()
{
	return _type;
}

void Target::CalculateTrajectory()
{
	
	_targetTrajectory.Clear();
	_targetTrajectory.addKey(0.0, _startPosition);
	
	// ������� �������� ��������� ������ �� �������� �������������
	float x_n = _startPosition.x + _maxSplineLength * sin(math::PI / 2 + _angle);
	float y_n = _startPosition.y - _maxSplineLength * cos(math::PI / 2 + _angle);
	
	_finishPosition = FPoint(x_n, y_n);
	_targetTrajectory.addKey(1.0, _finishPosition);
	_targetTrajectory.CalculateGradient();
	
}

FPoint Target::GetPosition(float time) {
	
	
	// ����� ������� ������������� �� ������ ����� �����
	// ����� ����� ����� ����� ������ ������ ���������� � ������ ���������� � �� ���������
	float timeShift;
	// ������� ������� �� �������, ���� ������ �� ��� �������, �� ���������� ������� �������
	if (time - _dtime >= 0)
	{
		timeShift = time - _dtime;
	} // ����� ��������� �������� ����� �������
	else
	{
		timeShift = time + math::PI * 2 - _dtime;
	}


	FPoint targetPosition = _targetTrajectory.getGlobalFrame(math::clamp(0.0f, 1.0f, timeShift / 6.0f));

	float angle = _angle;
	// ���� ������ ������������ � �������
	// �������� �� ��� ��� �� �����
	if (targetPosition.x > Render::device.Width() - _sizeW ||
		targetPosition.x < 0 ||
		targetPosition.y > Render::device.Height() - _sizeH ||
		targetPosition.y < 0) {

		// ����� �� ������ ������� 
		if (targetPosition.x > Render::device.Width() - _sizeW) {

			bla::Vector2<float> v(_startPosition, _finishPosition);
			bla::Vector2<float> d(0, 768);


			auto normal = d.Normal();
			auto l = v;
			auto r = l - (normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0f));
			angle = r.Angle(bla::Vector2<float>(1024, 0));

			targetPosition.x = Render::device.Width() - _sizeW - 1;

		}// ����� �� ����� ������� (�� �� �����)
		else if (targetPosition.x < 0)
		{
			bla::Vector2<float> v(_startPosition, _finishPosition);
			bla::Vector2<float> d(0, 768);


			auto normal = d.Normal();
			auto l = v;
			auto r = l - (normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0f));
			angle = r.Angle(bla::Vector2<float>(1024, 0));

			targetPosition.x = 1;
		}  // ����� �� ������� ������� 
		else if (targetPosition.y > Render::device.Height() - _sizeH) {
			bla::Vector2<float> v(_startPosition, _finishPosition);
			bla::Vector2<float> d(1024, 0);


			auto normal = d.Normal();
			auto l = v;
			auto r = l - (normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0f));
			angle = -(r.Angle(bla::Vector2<float>(1024, 0)));

			targetPosition.y = Render::device.Height() - _sizeH - 1;

		}// ����� �� ������ ������� (�� �� ��� � ����� �� �������)
		else if (targetPosition.y < 0) {
			bla::Vector2<float> v(_startPosition, _finishPosition);
			bla::Vector2<float> d(1024, 0);


			auto normal = d.Normal();
			auto l = v;
			auto r = l - (normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0f));
			angle = r.Angle(bla::Vector2<float>(1024, 0));

			targetPosition.y = 1;
		}

		// ������������� � ���������� ����� ����������

		SetNewStartPosition(targetPosition, angle, time);

		// �������� ������ ����� ����� ���������
		targetPosition = _targetTrajectory.getGlobalFrame(0.0f);
	}


	//if (targetPosition.x > Render::device.Width() - _sizeW ||
	//	targetPosition.x < 0 ||
	//	targetPosition.y > Render::device.Height() - _sizeH ||
	//	targetPosition.y < 0) {

	//	// ����� �� ������ ������� 
	//	if (targetPosition.x > Render::device.Width() - _sizeW) {
	//		// ������ ���� ����
	//		angle *= -1.0;
	//		// ����������� �����������
	//		angle = angle + math::PI;
	//		// ����������� ��������� ������� (���� ����� �� �������)
	//		targetPosition.x = Render::device.Width() - _sizeW - 1;

	//	}// ����� �� ����� ������� (�� �� �����)
	//	else if (targetPosition.x < 0)
	//	{
	//		angle *= -1.0;
	//		angle = angle + math::PI;
	//		targetPosition.x = 1;
	//	}  // ����� �� ������� ������� 
	//	else if (targetPosition.y > Render::device.Height() - _sizeH) {
	//		// ������ ���� ����
	//		angle *= -1.0;
	//		// ����������� ��������� ������� (���� ����� �� �������)
	//		targetPosition.y = Render::device.Height() - _sizeH - 1;

	//	}// ����� �� ������ ������� (�� �� ��� � ����� �� �������)
	//	else if (targetPosition.y < 0) {
	//		angle *= -1.0;
	//		targetPosition.y = 1;
	//	}

	//	// ������������� � ���������� ����� ����������
	//	
	//	SetNewStartPosition(targetPosition, angle, time);
	//
	//	// �������� ������ ����� ����� ���������
	//	targetPosition = _targetTrajectory.getGlobalFrame(0.0f);
	//}
	
	return targetPosition;

}

std::pair<FPoint, FPoint> Target::GetStartFinishPos() {
	return std::make_pair(_startPosition, _finishPosition);
}

FPoint Target::GetVector() {
	return FPoint(_targetTrajectory.getGlobalFrame(1) - _targetTrajectory.getGlobalFrame(0));
}

// �� ������������ �� ����� �������������� ��� ��������� �������� �������� (������� �������� ��� ��������)
void Target::SetHealth(int hp) {
	_health = hp;
}

// Yellow
YellowTarget::YellowTarget(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type): Target(health, start, angle, sizeWidth, sizeHeight, type)
{

}

YellowTarget::~YellowTarget() {}


// Red
RedTarget::RedTarget(int health, FPoint start, float angle, float sizeWidth, float sizeHeight, const char* type): Target(health, start, angle, sizeWidth, sizeHeight, type) {

}
RedTarget::~RedTarget() {}