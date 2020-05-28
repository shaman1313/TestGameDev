#pragma once

class Cannon
{
public:
	Cannon(float reloadTime);
	~Cannon();
	bool IsReady(float time);
	void Reload(float time);
	

private:
	float _reloadTime;
	float _lastReload;
};

