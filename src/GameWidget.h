#pragma once
#include <vector>
#include <memory>
#include <set>

#include "Bullets.h"
#include "Targets.h"
#include "Cannon.h"

///
/// Виджет - основной визуальный элемент на экране.
/// Он отрисовывает себя, а также может содержать другие виджеты.
///
class GameWidget : public GUI::Widget
{
public:
	GameWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;
	void Update(float dt) override;
	
	void AcceptMessage(const Message& message) override;
	
	bool MouseDown(const IPoint& mouse_pos) override;
	void MouseMove(const IPoint& mouse_pos) override;
	void MouseUp(const IPoint& mouse_pos) override;

	void KeyPressed(int keyCode) override;
	void CharPressed(int unicodeChar) override;

	void Switchbullets();
	void DrawBullet();
	void DrawTargets();

private:
	void Init();
	void Restart();

private:
	// таймер движения ядра
	float _cannonTimer;
	// таймер движения мишени
	float _targetTimer;
	// количество мишеней
	int _targetCount;
	// скорость мишеней
	int _cannonSpeed;
	// длительность раунда
	float _roundTime;
	float _currentTime;

	// диагональ экрана в пикселях
	// нужно для вычисления траекторий движения мишеней
	float _maxSplineLength;
	
	// текстуры
	// прицел
	Render::Texture* _aimTexture;
	// мишень
	Render::Texture* _targetYellowTexture;
	Render::Texture* _targetRedTexture;
	// фон
	Render::Texture* _backgroundTexture;
	// пушка
	Render::Texture* _cannonTexture;
	// пушечное ядро
	Render::Texture* _cannonballTexture;
	// пушечная бомба
	Render::Texture* _cannonbombTexture;

	// масштаб пушки (а также ядра и мишени)
	float _cannonScale;

	// точка на оси х - где рисуется пушка
	int _cannonPosition;

	// координаты точки поворота пушки (где-то центр толстой части пушки)
	int _cannonPivotAxisPosition_x;
	int _cannonPivotAxisPosition_y;
	
	// текстуры интерфейса
	// часы
	Render::Texture* _clockTexture;
	// текстуры выбора снарядов
	Render::Texture* _cannonballPictActiveTexture;
	Render::Texture* _cannonballPictDisActiveTexture;
	Render::Texture* _bombPictActiveTexture;
	Render::Texture* _bombPictDisActiveTexture;

	// контейнер с эффектами
	EffectsContainer _effCont;
	//
	ParticleEffectPtr _eff;
	ParticleEffectPtr _effShot;
	// снаряд	
	std::unique_ptr<Bullet> _bullet = nullptr;
	// контейнер с мишенями
	std::set<std::unique_ptr<Target>> _targets;
	
	std::unique_ptr<Cannon> _cannon;
	//  счет игры
	int _score;

	bool _gameOn = false;
	IPoint _lastMouse_pos = IPoint();
	ParticleEffectPtr _effExplosion;

	std::string _currentBulletType;
	
};
