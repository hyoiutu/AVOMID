#include <Siv3D.hpp>
#include <HamFramework.hpp>
#pragma once

using Scene = SceneManager<String>;

struct Note
{
	uint32 ch;
	uint32 noteNumber;
	int32 startMillisec;
	int32 lengthMillisec;

	double alpha;
	bool barPassed;
	bool onBar;
};

struct Item
{
	uint32 ID;
	uint32 height;
	bool isTaken;
};

class Game : public Scene::Scene
{
private:
	Array<Note> noteRects;
	double blockHeight;
	double scale;
	uint32 minPitch, maxPitch;
	bool playerCollisionDetection;
	uint32 playerHP;
	Array<Item> items;
	uint32 numOfGetItem;
	uint32 MAX_ITEM;
	const Font font;
public:
	Game();
	~Game();

	void init() override;
	void update() override;
	void draw() const override;
};

