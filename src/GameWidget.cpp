#include "stdafx.h"
#include "GameWidget.h"
#include "Targets.h"
#include "Bullets.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include "bla_vector.h"


class GameOverMessageFunc : public MessageFunc {
public:
	GameOverMessageFunc() = default;
	~GameOverMessageFunc() {}
	void AcceptMessage(const Message& message) override {
		if (message.getData() == "press") {
			Core::guiManager.getLayer("GameLayer")->getWidget("GameWidget")->AcceptMessage(Message("", "restart"));
		}
	}
};

GameWidget::GameWidget(const std::string& name, rapidxml::xml_node<>* elem)
	: Widget(name)
	, _score(0)
	, _cannonTimer(0)
	, _targetTimer(0)
	, _eff(NULL)
	, _effExplosion(NULL)
	, _effShot(NULL)
	, _cannonScale(0.25)
	, _cannonPosition(890)
	, _currentTime(0)
{
	Init();
}

void GameWidget::Init()
{
	

	_gameOn = true;
	// устанавливаем текстуры
	_aimTexture = Core::resourceManager.Get<Render::Texture>("aim");
	_targetYellowTexture = Core::resourceManager.Get<Render::Texture>("targetYellow");
	_targetRedTexture = Core::resourceManager.Get<Render::Texture>("targetRed");
	_cannonballTexture = Core::resourceManager.Get<Render::Texture>("cannonball");
	_cannonbombTexture = Core::resourceManager.Get<Render::Texture>("cannonbomb");
	_backgroundTexture = Core::resourceManager.Get<Render::Texture>("background");
	_cannonTexture = Core::resourceManager.Get<Render::Texture>("cannon");
	_clockTexture = Core::resourceManager.Get<Render::Texture>("clock");
	_cannonballPictActiveTexture = Core::resourceManager.Get<Render::Texture>("cannonballPictActive");
	_cannonballPictDisActiveTexture = Core::resourceManager.Get<Render::Texture>("cannonballPictDisActive");
	_bombPictActiveTexture = Core::resourceManager.Get<Render::Texture>("bombPictActive");
	_bombPictDisActiveTexture = Core::resourceManager.Get<Render::Texture>("bombPictDisActive");
	// еще несколько констант
	// точка поворота пушки
	_cannonPivotAxisPosition_x = _cannonPosition + _cannonTexture->Width() * _cannonScale / 2;
	// максимальная длина траектории - диагональ экрана
	// нужно для того, чтобы скорость не зависела от угла выстрела
	// если учитывать только видимую траекторию,
	// то при малых углах траетория будет большая (соответственно скорость маленькая) 
	// а при больших (около 90 градусов) - траектория очень маленькая а скорость высокая.
	// А так траектория всегда одного размера и скорость одинаковая при любых углах
	_maxSplineLength = sqrt(pow(Render::device.Height(), 2) + pow(Render::device.Width(), 2));
	//
	_currentBulletType = "cannonball";

	// считываем параметры игры из файла
	IO::InputStreamPtr inStream = Core::fileSystem.OpenRead("input.txt");
	if (inStream != nullptr) {

		std::vector<uint8_t> v;
		inStream->ReadAllBytes(v);

		// начало каждой строки
		auto ItLineBegin = v.begin();
		// знак равно
		auto ItParamEnd = v.begin();
		// пока не конец документа
		while (ItLineBegin != v.end()) {

			std::string param = "";
			std::string value = "";
			// ищем знак равно
			ItParamEnd = std::find(ItLineBegin, v.end(), '=');
			// конец строки
			auto EndLineIt = std::find(ItParamEnd, v.end(), '\n');
			// от начала строки до знака равно - название параметра
			for (auto It = ItLineBegin; It != ItParamEnd; It++) {
				param += *It;
			}
			// со следующего после знака равно до конца строки - значение
			for (auto It = ++ItParamEnd; It != EndLineIt; ++It) {
				value += *It;
			}

			if (param == "CountTarget") {
				_targetCount = atoi(value.data());
			}
			else if (param == "Speed") {
				_cannonSpeed = atoi(value.data());
			}
			else if (param == "Time") {
				_roundTime = atoi(value.data());
			}
			// переход на следующую строку
			if (EndLineIt != v.end()) {
				ItLineBegin = ++EndLineIt;
			}
			else {
				break;
			}
			
		}

	}
	
	inStream.reset();

	

	// количество красных мишеней в процентах
	float redRate = 0.25f;
	// в числах
	int redNumber = _targetCount * redRate;

	//  заполняем контейнер с мишенями
	for (int i = 0; i < _targetCount; ++i) {
		// выбираем угол
		
		float angle = (0.0 + i * 15.0) / (180.0 / 3.14);
		// создаем мишень, оборачиваем в умный указатель, кладем в контейнер
		if (i < redNumber) {
			_targets.insert(std::move(std::make_unique<RedTarget>(200, FPoint(512, 100), angle, _targetRedTexture->Width() * _cannonScale, _targetRedTexture->Height() * _cannonScale, "Red")));
		}
		else {
			_targets.insert(std::move(std::make_unique<YellowTarget>(100, FPoint(512, 100), angle, _targetYellowTexture->Width() * _cannonScale, _targetYellowTexture->Height() * _cannonScale, "Yellow")));
		}
		
	}
	// обьект орудия
	// в конструкторе - время перезарядки
	_cannon = std::move(std::make_unique<Cannon>(3));

}

void GameWidget::Draw()
{	
	
	// Рисуем фон
	_backgroundTexture->Draw();

	// Рисуем пушку, направление привязывем к указателю мыши
	IPoint mouse_pos;
	if (_gameOn) {
		mouse_pos = Core::mainInput.GetMousePos();
		_lastMouse_pos = mouse_pos;
	}
	else 
	{
		mouse_pos = _lastMouse_pos;
	}
			
	// Рассчитываем угол наклона
	// Это острый угол прямоугольного треугольника:
	//    катет а - отрезок соединяющий указатель мыши и проекцию указателя на ось х
	//    катет б - отрезок соединяющий проекцию указателя на ось х и проекцию точки поворота пушки, которая лежит на оси х
	//	  гипотенуза - отрезок между указателем мыши и проекцией точки поворота пушки

	float a, b, c;
	a = mouse_pos.y;
	b = _cannonPivotAxisPosition_x - mouse_pos.x;
	c = sqrt(pow(a, 2) + pow(b, 2));

	// согласно теореме косинусов
	float cannonAngle = acos(b/c);
	// переводим в градусы
	cannonAngle = cannonAngle * (180.0 / 3.14);

	Render::device.PushMatrix();
	// выставляем позицию пушки
	Render::device.MatrixTranslate(_cannonPosition, 0.0, 0);
	// масштабируем пушку
	Render::device.MatrixScale(_cannonScale, _cannonScale, 1.0f);
	// смещаем точку поворота
	Render::device.MatrixTranslate(320.0, 220.0, 0);
	// поворачиваем пушку
	Render::device.MatrixRotate(math::Vector3(0, 0, 1), (math::clamp(5.0, 85.0, 90.0 - cannonAngle)));
	// возвращаем начало координат
	Render::device.MatrixTranslate(-320.0, -220.0, 0);
		
	_cannonTexture->Draw();
	Render::device.PopMatrix();
	
	// если пушка на перезарядке
	// показываем это
	if (!_cannon->IsReady(_currentTime)) {
		Render::BindFont("arialRed");
		Render::PrintString(FPoint(940.0, _cannonTexture->Height()*_cannonScale), std::string("{font size=35}") + "RELOAD" + std::string("{}"), 1.0f, CenterAlign, BottomAlign);

	}
	
	// рисуем прицел
	Render::device.PushMatrix();
	Render::device.MatrixTranslate((float)mouse_pos.x-62, (float)mouse_pos.y-62, 0);	// 62 - размер прицела / 2     125/2        чтобы сместить центр прицела в позицию указателя мыши
	_aimTexture->Draw();
	Render::device.PopMatrix();
		
	// рисуем снаряд в полете
	DrawBullet();
	
	// рисуем мишень(ни)
	DrawTargets();
	
	// рисуем эффекты
	_effCont.Draw();

	// рисуем таймер
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(880.0, 600.0, 0);
	Render::device.MatrixScale(0.75, 0.75, 0);
	_clockTexture->Draw();
	Render::device.PopMatrix();
		
	if (_roundTime > 6) {
		Render::BindFont("arial");
	}
	else {
		Render::BindFont("arialRed");
	}
	
	Render::PrintString(FPoint(980.0, 550.0), std::string("{font size=45}") + utils::lexical_cast((int)_roundTime) + std::string("{}"), 1.0f, CenterAlign, BottomAlign);
	
	// рисуем счет 
	Render::BindFont("arial");
	Render::PrintString(FPoint(512.0, 725.0), std::string("{font size=45}") + "Score: " + utils::lexical_cast((int)_score) + std::string("{}"), 1.0f, CenterAlign, BottomAlign);

	// рисуем значки снарядов
	if (_currentBulletType == "cannonball") {
		// ядро - активно, бомба нет
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(900 + 20, 360.0, 0);
		Render::device.MatrixScale(0.3, 0.3, 1.0f);
		_bombPictDisActiveTexture->Draw();
		Render::device.PopMatrix();

		Render::device.PushMatrix();
		Render::device.MatrixTranslate(900 - 35, 260.0, 0);
		Render::device.MatrixScale(0.4, 0.4, 1.0f);
		_cannonballPictActiveTexture->Draw();
		Render::device.PopMatrix();

	}
	else if (_currentBulletType == "bomb") {
		// наоборот
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(900 + 20, 360.0, 0);
		Render::device.MatrixScale(0.3, 0.3, 1.0f);
		_cannonballPictDisActiveTexture->Draw();
		Render::device.PopMatrix();

		Render::device.PushMatrix();
		Render::device.MatrixTranslate(900 + -35, 260.0, 0);
		Render::device.MatrixScale(0.4, 0.4, 1.0f);
		_bombPictActiveTexture->Draw();
		Render::device.PopMatrix();

	}
	
}

void GameWidget::Update(float dt)
{
	// Если игра закончена
	// показываем это
	if (!_gameOn) {
		Core::guiManager.getLayer("GameLayer")->getWidget("GameOverWidget")->setVisible(true);
		Core::guiManager.getLayer("GameLayer")->getWidget("Again")->setVisible(true);
		return;
	}
	// если игра не закончена убираем с экрана
	Core::guiManager.getLayer("GameLayer")->getWidget("GameOverWidget")->setVisible(false);
	Core::guiManager.getLayer("GameLayer")->getWidget("Again")->setVisible(false);

	//
	// Обновим контейнер с эффектами
	//
	_effCont.Update(dt);

	//
	// dt - значение времени в секундах, прошедшее от предыдущего кадра.
	// Оно может принимать разные значения, в зависимости от производительности системы
	// и сложности сцены.
	//
	// Для того, чтобы наша анимация зависела только от времени, и не зависела от
	// производительности системы, мы должны рассчитывать её от этого значения.
	//
	// Увеличиваем наш таймер с удвоенной скоростью.
	//
	
	
	// если время вышло или все мишени уничтожены
	// заканчиваем игру
	if (_roundTime <= 0 || _targetCount == 0) {
		Core::guiManager.getLayer("GameLayer")->messageFunc = new GameOverMessageFunc();
		_gameOn = false;
		return;
	}
	

	// обновление таймеров
	_cannonTimer += dt * 2;
	_targetTimer += dt * 2;
	_roundTime -= dt;
	_currentTime += dt;
	//
	// Зацикливаем таймер в диапазоне (0, 2п).
	// Это нужно делать для предотвращения получения некорректных значений,
	// если вдруг переполнится разрядная сетка (float переполнился) или задержка
	// от предыдущего кадра была слишкой большой (система тормози-и-ит).
	//
	// Диапазон значений выбран равным (0, 2п), потому что мы используем это значение
	// для расчёта синуса, и большие значения будут просто периодически повторять результат.
	//
	while (_cannonTimer > 2 * math::PI)
	{
		_cannonTimer -= 2 * math::PI;
	}
	while (_targetTimer > 2 * math::PI)
	{
		_targetTimer -= 2 * math::PI;
	}

	
	//  рассчитываем столкновения между снарядом и мишенями
	if (_bullet) {
		// радиусы обьектов
		float targetR = 120.0 * _cannonScale;
		float cannonBallR = 120.0 * _cannonScale;
		// дистанция столкновения между центрами обьектов
		float criticalDistance = targetR + cannonBallR;

		// сюда будем класть итераторы на мишени (елементы контейнера) которые нужно удалить после столкновений
		std::vector<std::set<std::unique_ptr<Target>>::iterator> del;
		
		FPoint bulletPos = _bullet->GetPosition(_cannonTimer);
				
		// проходим по всем мишеням
		for (auto& target : _targets) {
			
			FPoint targetPos = target->GetPosition(_targetTimer);
			// расстояние между центрами мишени и снаряда
			float distance = targetPos.GetDistanceTo(bulletPos);
			
			// если есть попадание
			if (distance <= criticalDistance) {
				_bullet->Collide(); // временная заглушка для изменения в будущем если понадобится поменять поведения снаряда при столкновении

				// для ядра
				if (_bullet->GetType() == "cannonball") {
					// добавляем эффект
					_effExplosion = _effCont.AddEffect("Explosion", targetPos);
					// наносим урон
					target->Damage(_bullet->GetDamage());
					// если здоровье меньше нуля - мишень уничтожена
					// заносим ее в контейнер для удаления
					if (target->GetHealth() <= 0) {
						auto It = _targets.find(target);
						del.push_back(It);
						_score++;
						_targetCount--;

					}
				} // для бомбы
				else if (_bullet->GetType() == "bomb") {
					_effExplosion = _effCont.AddEffect("bombExplosion", targetPos);
					// проходим по всем мишеням и находим все в радиусе поражения (200 рх)
					for (auto& target2 : _targets) {
						FPoint targetPos = target2->GetPosition(_targetTimer);
						float distance = targetPos.GetDistanceTo(bulletPos);
						if (distance <= 200) {
							// урон обратнопропорциональный расстоянию
							target2->Damage(_bullet->GetDamage() * (200 - distance) / 200);
						}
						if (target2->GetHealth() <= 0) {
							auto It = _targets.find(target2);
							del.push_back(It);
							_score++;
							_targetCount--;

						}
					}
				}
			
				_bullet.reset();
				break;
			}
		}
		// удаляем уничтоженые мишени
		if (!del.empty()) {
			
			for (auto& It : del) {
				_targets.erase(It);
			}
		}

	}
	
	// рассчитываем столкновения между мишенями
	// поскольку снаряды разлетаются из одной точки, предоставим 3 секунды, 
	//чтобы они разлетелись иначе они будут детектировать столкновения
	if (_currentTime < 3.0) {
		return;
	}

	// пробегаемся по всем мишеням и проверяем уникальные пары, не проверяя мишени 0 1 и 1 0 во второй раз
	// это позволит снизить сложность алгоритма с N^2 до N^2 / 2
	// Но все таки, это не подходит для большого количества мишеней, поскольку
	// количество всевозможных пар из N обьектов вычисляется через факториалы
	//						  N!
	//		С(2, N) = -------------------
	//					2! * (N - 2) !


	for (auto it1 = _targets.begin(); it1 != _targets.end(); ++it1) {
		auto it2 = it1;
		for (++it2; it2 != _targets.end(); ++it2) {
			// точки в центрах двух мишеней
			// вектор из этих точек является нормалью к прямой, от которой будут отталкиваться мишени
			FPoint one = (*it1)->GetPosition(_targetTimer) + FPoint(_targetYellowTexture->Width() * _cannonScale / 2, _targetYellowTexture->Height() * _cannonScale / 2);
			FPoint two = (*it2)->GetPosition(_targetTimer) + FPoint(_targetYellowTexture->Width() * _cannonScale / 2, _targetYellowTexture->Height() * _cannonScale / 2);
			float distance = one.GetDistanceTo(two);
			// 120 - радиус мишени
			if (distance <= 120.0 * _cannonScale * 2) {
				// получаем вектора движения мишеней
				auto pos1 = (*it1)->GetStartFinishPos();
				bla::Vector2<float> v1(pos1.first, pos1.second);
				
				auto pos2 = (*it2)->GetStartFinishPos();
				bla::Vector2<float> v2(pos2.first, pos2.second);
				
				// 1st
				// ищем отраженный вектор

				// нормаль к прямой отражения
				auto normal = bla::Vector2<float>(one, two);
				// исходный вектор
				auto l = v1;
				// отраженный вектор
				// ищем так:
				//			  l dot normal
				//	r = l - ------------------ dot normal dot 2
				//			normal dot normal

				auto r = l-(normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0f));
				// получаем угол наклона к оси x
				float angle = r.Angle(bla::Vector2<float>(1024.0f, 0.0f));

				// коректируем позицию, переставляя мишень с начальной точки новой траектории чуть дальше
				// чтобы дистанция увеличилась и не детектировалось новое столкновение
				auto pos = (*it1)->GetPosition(_targetTimer) +FPoint(r.x, r.y) * (((120.0 * _cannonScale) - (distance / 2) + 1) / _maxSplineLength);
				// устанавливаем новую позицию
				(*it1)->SetNewStartPosition(pos, angle, _targetTimer);

				// 2nd target
				l = v2;
				r = l - (normal.Dot(l.Dot(normal) / normal.Dot(normal)).Dot(2.0));
				angle = r.Angle(bla::Vector2<float>(1024.0, 0.0));
				pos = (*it2)->GetPosition(_targetTimer) +FPoint(r.x, r.y) * (((120.0 * _cannonScale) - (distance / 2) + 1) / _maxSplineLength);
				(*it2)->SetNewStartPosition(pos, angle, _targetTimer);
			}
		}
	
	}

}

bool GameWidget::MouseDown(const IPoint &mouse_pos)
{
	// клик праой кнопки - смена типа снаряда
	if (Core::mainInput.GetMouseRightButton())
	{
		Switchbullets();
	}
	else
	{	// нажатие на левую кнопку мыши
		// выстрел

		// перезаряжена ли пушка?
		if (!_cannon->IsReady(_currentTime)) {
			return false;
		}
		
		// рассчитываем угол траектории снаряда
		// из прямоугольного треуголька
		float a, b, c;
		a = mouse_pos.y - _cannonballTexture->Width() * _cannonScale / 2;
		b = _cannonPivotAxisPosition_x - mouse_pos.x - _cannonballTexture->Height() * _cannonScale / 2;
		c = sqrt(pow(a, 2) + pow(b, 2));

		// согласно теореме косинусов
		float angle = acos(b / c);

		// снаряд будем начинать рисовать при вылете из ствола пушки
		FPoint bulletPos;
		// константы - это координаты на краю ствола пушки
		bulletPos.x = 215.0 * _cannonScale;
		bulletPos.y = 640.0 * _cannonScale;

		// теперь корректируем эти координаты согласно наклону пушки
		auto shiftP = FPoint(bulletPos.x, 0.0);
		bulletPos -= shiftP;
		bulletPos.Rotate(math::clamp(0.09, 1.48, 1.48 - angle));
		bulletPos += shiftP;
		bulletPos.x += _cannonPosition;
		// добавляем обьект снаряда
		if (_currentBulletType == "cannonball") {
			_bullet = std::make_unique<CannonBall>(150, bulletPos, math::clamp(0.09f, 1.48f, angle), _cannonSpeed, _currentBulletType);
		}
		else if (_currentBulletType == "bomb") {
			_bullet = std::make_unique<Bomb>(500, bulletPos, math::clamp(0.09f, 1.48f, angle), _cannonSpeed, _currentBulletType);
		}
		// запускаем перезарядку орудия
		_cannon->Reload(_currentTime);
		// и добавляем эффекты шлейфа и выстрела
		_effShot = _effCont.AddEffect("shot", bulletPos);
		_eff = _effCont.AddEffect("Iskra");
		// обнуляем таймер снаряда, чтобы он начал отрисовку вначале своей траектории а не где попало
		_cannonTimer = 0;


	}
	
	return false;
}

void GameWidget::MouseMove(const IPoint &mouse_pos)
{

}

void GameWidget::MouseUp(const IPoint &mouse_pos)
{
	
}

void GameWidget::AcceptMessage(const Message& message)
{
	//
	// Виджету могут посылаться сообщения с параметрами.
	//

	const std::string& publisher = message.getPublisher();
	const std::string& data = message.getData();
	if (data == "restart") {
		
		Restart();
		
	}
}

void GameWidget::KeyPressed(int keyCode)
{
	//
	// keyCode - виртуальный код клавиши.
	// В качестве значений для проверки нужно использовать константы VK_.
	//

	if (keyCode == VK_A) {
		// Реакция на нажатие кнопки A
	}
}

void GameWidget::CharPressed(int unicodeChar)
{
	//
	// unicodeChar - Unicode код введённого символа
	//

	if (unicodeChar == L'а') {
		// Реакция на ввод символа 'а'
	}
}

void GameWidget::Switchbullets() {

	if (_currentBulletType == "cannonball") {
		_currentBulletType = "bomb";
	} else if (_currentBulletType == "bomb") {
		_currentBulletType = "cannonball";
	}

	if (_cannon) {
		_cannon->Reload(_currentTime);
	}
}

void GameWidget::DrawBullet() {
	// удаляем шлейф если снаряд уничтожен
	if (!_bullet) {
		if (_eff)
		{
			_eff->Finish();
			_eff = NULL;
		}
		return;
	}
	
	// размер снаряда
	// для обоих типов он почти одинаков
	float offset_w = _cannonballTexture->Width() * _cannonScale;
	float offset_h = _cannonballTexture->Height() * _cannonScale;

	FPoint currentPosition = _bullet->GetPosition(_cannonTimer);
	// если снаряд улетел за границы экрана
	if (currentPosition.x <= -offset_w || currentPosition.y > 768) {
		// удаляем его из памяти
		_bullet.reset();
		// удаляем эффект шлейфа
		if (_eff)
		{
			_eff->Finish();
			_eff = NULL;
		}
		return;
	}
	// если снаряд жив
	// смещаем эффект вслед за ним
	if (_eff) {
		_eff->posX = currentPosition.x + offset_w/2;
		_eff->posY = currentPosition.y + offset_h/2;
	}

	
	// отрисовываем снаряд в зависимости от его типа
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(currentPosition.x, currentPosition.y, 0);
	Render::device.MatrixScale(_cannonScale, _cannonScale, 0);
	if (_bullet->GetType() == std::string("cannonball")) {
		_cannonballTexture->Draw();
	} else if (_bullet->GetType() == std::string("bomb")) {
		_cannonbombTexture->Draw();
	}
	Render::device.PopMatrix();
		
}

void GameWidget::DrawTargets() {

	float time = _targetTimer;

	for (auto& target : _targets) {
		// для каждой мишени:
		// получаем текущую позицию мишени
		FPoint targetPosition = target->GetPosition(time);

		// рисуем траекторию
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(targetPosition.x, targetPosition.y, 0);
		Render::device.MatrixScale(_cannonScale, _cannonScale, 0);
		if (target->GetType() == "Red") {
			_targetRedTexture->Draw();
		}
		else if (target->GetType() == "Yellow") {
			_targetYellowTexture->Draw();
		}
		
		Render::device.PopMatrix();

	}

}

void GameWidget::Restart() {
	_score = 0;
	_cannonTimer = 0;
	_targetTimer = 0;
	_eff = NULL;
	_effExplosion = NULL;
	_cannonScale = 0.25;
	_cannonPosition = 890;
	_currentTime = 0;

	_targets.clear();
	_bullet.reset();
	_effCont.KillAllEffects();

	Init();
}