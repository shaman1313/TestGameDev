#include "stdafx.h"
#include "GameOverWidget.h"



GameOverWidget::GameOverWidget(const std::string& name, rapidxml::xml_node<>* elem)	: Widget(name)
{
	Init();
}

void GameOverWidget::Init()
{
	_gameOverTexture = Core::resourceManager.Get<Render::Texture>("gameOverTexture");
	
}

void GameOverWidget::Draw()
{
	// затеняем основную область экрана
	Render::device.SetTexturing(false);
	Render::BeginColor(Color(0, 0, 0, 128));
	Render::DrawRect(0, 0, Render::device.Width(), Render::device.Height());
	Render::EndColor();
	Render::device.SetTexturing(true);
	// рисуем заставку
	Render::device.PushMatrix();
	Render::device.MatrixTranslate(Render::device.Width()/2 - _gameOverTexture->Width()/2, Render::device.Height() / 2 - _gameOverTexture->Height() / 2, 0);
	_gameOverTexture->Draw();
	Render::device.PopMatrix();
}

void GameOverWidget::Update(float dt)
{
}

void GameOverWidget::AcceptMessage(const Message& message)
{

	
}

bool GameOverWidget::MouseDown(const IPoint& mouse_pos)
{
	return false;
}

void GameOverWidget::MouseMove(const IPoint& mouse_pos)
{
}

void GameOverWidget::MouseUp(const IPoint& mouse_pos)
{
}

void GameOverWidget::KeyPressed(int keyCode)
{
}

void GameOverWidget::CharPressed(int unicodeChar)
{
}
