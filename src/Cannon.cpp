#include "stdafx.h"
#include "Cannon.h"

Cannon::Cannon(float reloadTime):
	_reloadTime(reloadTime)
{
}

Cannon::~Cannon()
{
}

bool Cannon::IsReady(float time) {
	return time - _lastReload > _reloadTime;
}

void Cannon::Reload(float time) {
	_lastReload = time;
}