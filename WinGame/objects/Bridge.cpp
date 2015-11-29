﻿
#include "Bridge.h"

int Bridge::_matrixIndex[2][MAX_WAVE * 2] =
{
	{ 3, 0, 3, 0, 3, 0, 3, 0 },
	{ 1, 4, 4, 4, 4, 4, 4, 5 },
};

Bridge::Bridge(GVector2 postion) : BaseObject(eID::BRIDGE)
{
	_sprite = SpriteManager::getInstance()->getSprite(eID::BRIDGE);
	_sprite->setScale(SCALE_FACTOR);
	_transform = new Transformable();
	_transform->setPosition(postion);

}
void Bridge::init()
{
	_stopwatch = new StopWatch();

	_explode = new QuadExplose(_transform->getPosition());
	_explode->init();
	_wave = 0;
	_stopwatch->restart();
	_listComponent["CollisionBody"] = new CollisionBody(this);

	for (int i = 0; i < 2; i++)
	{
		this->privateIndex[i] = new int[MAX_WAVE * 2];
	}
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < MAX_WAVE * 2; j++)
		{
			this->privateIndex[i][j] = _matrixIndex[i][j];
		}
	}
}
void Bridge::update(float deltatime)
{
	if (this->getStatus() == eStatus::DESTROY)
		return;
	if (this->getStatus() == eStatus::BURST)
		this->burst(deltatime);
	else
	{
		auto playscene = (PlayScene*) SceneManager::getInstance()->getCurrentScene();
		this->trackBill(playscene->getBill());
	}
	for (auto component : _listComponent)
	{
		component.second->update(deltatime);
	}
}

void Bridge::burst(float deltatime)
{
	if (this->getStatus() == eStatus::DESTROY)
		return;
	privateIndex[0][0 + _wave * 2] =
		privateIndex[0][1 + _wave * 2] =
		privateIndex[1][0 + _wave * 2] =
		privateIndex[1][1 + _wave * 2] = -1;
	if (_explode->getStatus() == NORMAL)
		_explode->update(deltatime);
	if (_explode->getStatus() == DESTROY)
	{
		_explode->release();
		if (_stopwatch->isStopWatch(DELAYTIME))
		{
			_wave++;
			GVector2 pos = this->getPosition();
			pos.x += this->_sprite->getFrameWidth() * this->_sprite->getScale().x * _wave;
			_explode = new QuadExplose(pos);
			_explode->init();
			_stopwatch->restart();
			if (_wave == MAX_WAVE)
			{
				_explode->setStatus(eStatus::DESTROY);
				this->setStatus(eStatus::DESTROY);
			}
		}
	}
}

void Bridge::draw(LPD3DXSPRITE spritehandle, Viewport* viewport)
{
	if (this->getStatus() == eStatus::DESTROY)
		return;
	GVector2 posrender;
	// Thuật toán vẽ giống cách vẽ từng pixel trên console.
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < MAX_WAVE * 2; j++)
		{
			if (privateIndex[i][j] == -1)
				continue;
			posrender = this->_transform->getPosition();
			posrender.x += j * this->getSprite()->getFrameWidth();
			posrender.y -= i * this->getSprite()->getFrameHeight();
			_sprite->setPosition(posrender);
			_sprite->setIndex(privateIndex[i][j]);
			_sprite->render(spritehandle, viewport);
		}
	}
	if (this->getStatus() == eStatus::BURST)
	{
		_explode->draw(spritehandle, viewport);
	}

}
void Bridge::release()
{

}
void Bridge::setPosition(GVector2 position)
{
	this->_transform->setPosition(position);
}
GVector2 Bridge::getPosition()
{
	return	this->_transform->getPosition();
}
RECT Bridge::getBounding()
{
	RECT rect = {0,0,0,0};
	if (this->getStatus() == eStatus::DESTROY || _wave == MAX_WAVE - 1)
	{
		return rect;
	}

	int framewidth = this->_sprite->getFrameWidth();
	int frameheight = this->_sprite->getFrameHeight();
	//rect.left = this->getPosition().x - (framewidth > 1);					// framewidth /2 là origin(Anchor).
	//rect.bottom = this->getPosition().y - frameheight - (frameheight > 1);
	//rect.right = rect.left + (framewidth * MAX_WAVE) < 1;					// Nhân 2 vì cách 2 hình có 1 vụ nổ.
	////rect.top = rect.bottom + frameheight < 1 - frameheight > 1;				// < 1 là nhân 2; > 1 là chia 2. Trừ đi frameheight / 2 đẻ bằng với land

	//if (this->getStatus() == eStatus::BURST)
	//{
	//	rect.left += (_wave + 1) * (framewidth < 1);
	//}
	rect.left = this->getPosition().x - framewidth / 2;					// framewidth /2 là origin(Anchor).
	rect.bottom = this->getPosition().y - frameheight - frameheight / 2;
	rect.right = rect.left + framewidth * 2 * MAX_WAVE;					// Nhân 2 vì cách 2 hình có 1 vụ nổ.
	rect.top = rect.bottom + frameheight * 2;

	if (this->getStatus() == eStatus::BURST)
	{
		rect.left += (_wave + 1) * framewidth * 2;
	}
	return rect;
}

void Bridge::trackBill(Bill* bill)
{
	RECT billBound = bill->getBounding();
	RECT bridgeBound = this->getBounding();

	if(billBound.right >= bridgeBound.left)
		this->setStatus(eStatus::BURST);
}
Bridge::~Bridge()
{
}
Bridge::QuadExplose::QuadExplose(GVector2 position) : BaseObject(eID::QUADEXPLODE)
{
	_transform = new Transformable();
	this->_transform->setPosition(position);
}
void Bridge::QuadExplose::init()
{	
	// Những số cứng 16 bên dưới là khoảng cách các vụ nổ nhỏ trong vụ nổ gồm 4 phát nổ.
	// Chỉ có mục đích thẩm mĩ, không có mục đích nào khác.
	_timer = 0.0f;
	auto pos = _transform->getPosition();
	_explosion1 = new Explosion(2);
	_explosion1->init();
	_explosion1->setScale(SCALE_FACTOR);
	_explosion1->setPosition(pos);

	_explosion2 = new Explosion(2);
	_explosion2->init();
	_explosion2->setScale(SCALE_FACTOR);
	_explosion2->setPosition(GVector2(pos.x + 16, pos.y - 16));

	_explosion3 = new Explosion(2);
	_explosion3->init();
	_explosion3->setScale(SCALE_FACTOR);
	_explosion3->setPosition(GVector2(pos.x, pos.y - 16));

	_explosion4 = new Explosion(2);
	_explosion4->init();
	_explosion4->setScale(SCALE_FACTOR);
	_explosion4->setPosition(GVector2(pos.x - 16, pos.y - 16));
}

void Bridge::QuadExplose::update(float deltatime)
{
	_timer += deltatime;
	if (_timer >= 0)
		_explosion1->update(deltatime);
	if (_timer >= 30)
		_explosion2->update(deltatime);
	if (_timer >= 60)
		_explosion3->update(deltatime);
	if (_timer >= 90)
	{
		_explosion4->update(deltatime);
		if (_explosion4->getStatus() == eStatus::DESTROY)
			this->setStatus(eStatus::DESTROY);
	}
}
void Bridge::QuadExplose::draw(LPD3DXSPRITE spritehandle, Viewport* viewport)
{
	if (this->getStatus() != eStatus::NORMAL)
		return;
	if (_timer >= 0)
		_explosion1->draw(spritehandle, viewport);
	if (_timer >= 30)
		_explosion2->draw(spritehandle, viewport);
	if (_timer >= 60)
		_explosion3->draw(spritehandle, viewport);
	if (_timer >= 90)
		_explosion4->draw(spritehandle, viewport);
}
void Bridge::QuadExplose::setPosition(GVector2 position)
{
	this->_transform->setPosition(position);
	_explosion1->setPosition(position);
	_explosion2->setPosition(GVector2(position.x + 16, position.y - 16));
	_explosion3->setPosition(GVector2(position.x, position.y - 16));
	_explosion4->setPosition(GVector2(position.x - 16, position.y - 16));
}
GVector2 Bridge::QuadExplose::getPosition()
{
	return	this->_transform->getPosition();
}
void Bridge::QuadExplose::reset()
{
	_explosion1->setStatus(eStatus::NORMAL);
	_explosion2->setStatus(eStatus::NORMAL);
	_explosion3->setStatus(eStatus::NORMAL);
	_explosion4->setStatus(eStatus::NORMAL);
	this->setStatus(eStatus::NORMAL);
	_timer = 0.0f;
}
void Bridge::QuadExplose::release()
{

}