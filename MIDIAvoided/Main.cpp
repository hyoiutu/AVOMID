# include <Siv3D.hpp>

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



void Main()
{

	const uint32 PLAYER_MAX_HP = 200;
	uint32 playerHP = PLAYER_MAX_HP;
	double scale = 0.5;
	Window::Resize(1280, 720);
	bool playerCollisionDetection = false;
	const Font font(30);


	Midi::Open(Dialog::GetOpenMidi().value());
	const auto score = Midi::GetScore();
	Array<Note> noteRects;
	Array<Item> items;
	uint32 minPitch = 127, maxPitch = 0;
	const uint32 PRUNED_MIN_PITCH = 0, PRUNED_MAX_PITCH = 127;

	Cursor::SetStyle(CursorStyle::None);

	for (auto ch : step(static_cast<uint32>(score.size())))
	{
		for (const auto& note : score[ch])
		{
			if (PRUNED_MIN_PITCH <= note.noteNumber && PRUNED_MAX_PITCH >= note.noteNumber) {
				noteRects.push_back({ ch, note.noteNumber, note.startMillisec, note.lengthMillisec, 1.0, false, false });
				minPitch = Min(minPitch, note.noteNumber);
				maxPitch = Max(maxPitch, note.noteNumber);
			}
		}
	}
	const double blockHeight = static_cast<double>(Window::Height()) / (maxPitch - minPitch + 1);
	Array<uint32> indexes;
	const uint32 MAX_ITEM = 100;
	uint32 numOfGetItem = 0;
	for (int i = 0; i < noteRects.size(); ++i)
	{
		indexes.push_back(i);
	}
	Shuffle(indexes);
	for (int i = 0; i < MAX_ITEM; ++i)
	{
		uint32 itemHeight = 0;
		do {
			itemHeight = Random<uint32>(0, 720);
		} while (itemHeight >= (maxPitch - noteRects[indexes[i]].noteNumber) * blockHeight && itemHeight <= (maxPitch - noteRects[indexes[i]].noteNumber) * blockHeight + blockHeight);
		items.push_back({ indexes[i], Random<uint32>(0,720), false });
	}
	

	while (System::Update())
	{
		const int32 offset = 160;
		const int32 offsetMillisec = static_cast<int32>(offset / scale);
		const RectF line(offset - 1, 0, 6, Window::Height());
		const double PLAYER_RADIUS = 0.4*blockHeight;
		const double PLAYER_COLLISION_RADIUS = 0.6*PLAYER_RADIUS;
		const double ITEM_HEIGHT = 0.9*blockHeight;
		const double ITEM_WIDTH = ITEM_HEIGHT;

		// 現在のカーソルの位置
		const Point mousePosition = Point(line.x,Mouse::Pos().y);
		const Circle player(mousePosition, PLAYER_RADIUS);
		const Circle playerCollision(player.center, PLAYER_COLLISION_RADIUS);
		Color playerColor;
		bool  collisionSwitch = false;
		if (Mouse::Pos().y > Window::Height())
		{
			Cursor::SetPos(Point(Mouse::Pos().x, 0));
		}
		else if (Mouse::Pos().y < 0)
		{
			Cursor::SetPos(Point(Mouse::Pos().x, Window::Height()));
		}
		if (!Midi::IsPlaying())
		{
			for (auto& note : noteRects)
			{
				note.alpha = 1.0;
				note.barPassed = false;
				note.onBar = false;
			}
		}

		if (Input::MouseL.clicked)
		{
			Midi::Play();
		}

		const double pos = (Midi::GetPosSec() * 1000 - 100)* scale;
		const int32 bar = static_cast<int32>(Midi::GetPosSec() * 1000 - 100);
		const int32 left = bar - offsetMillisec;
		const int32 right = static_cast<int32>(left + Window::Width() / scale);

		Array<size_t> visibleNoteIndices;
		size_t index = 0;

		for (auto& note : noteRects)
		{
			if (right < note.startMillisec || (note.startMillisec + note.lengthMillisec) < left)
			{
				++index;
				continue;
			}

			note.onBar = note.startMillisec <= bar && bar <= (note.startMillisec + note.lengthMillisec);
			note.barPassed = note.startMillisec <= bar;

			if (note.barPassed)
			{
				note.alpha *= note.onBar ? 0.98 : 0.85;
			}

			visibleNoteIndices.push_back(index++);
		}

		Graphics2D::SetBlendState(BlendState::Default);

		for (auto& i : visibleNoteIndices)
		{
			const auto& note = noteRects[i];

			if (!note.barPassed)
			{
				const RectF r(note.startMillisec * scale + offset - pos, (maxPitch - note.noteNumber) * blockHeight, note.lengthMillisec * scale, blockHeight);
				RoundRect(r, 4).draw(ColorF(0.2, 0.25, 0.3));
				
				playerCollisionDetection = playerCollision.intersects(r);
				if (playerCollisionDetection) {
					collisionSwitch = true;
				}
			}
		}

		Graphics2D::SetBlendState(BlendState::Additive);

		for (auto& i : visibleNoteIndices)
		{
			const auto& note = noteRects[i];

			if (note.barPassed)
			{
				const RectF r(note.startMillisec * scale + offset - pos, (maxPitch - note.noteNumber) * blockHeight, note.lengthMillisec * scale, blockHeight);
				RoundRect(r, 5).drawShadow({ 0, 0 }, 12 + note.alpha * 8, 2 + note.alpha * 8, HSV(30 + note.ch * 100, 0.5, 1).toColorF(note.alpha*0.4));
			}
		}

		for (auto& i : visibleNoteIndices)
		{
			const auto& note = noteRects[i];

			if (note.barPassed)
			{
				const RectF r(note.startMillisec * scale + offset - pos, (maxPitch - note.noteNumber) * blockHeight, note.lengthMillisec * scale, blockHeight);
				RoundRect(r, 4).draw(HSV(30 + note.ch * 100, 1, 1).toColorF(note.alpha));
				playerCollisionDetection = playerCollision.intersects(r);
				if (playerCollisionDetection) {
					collisionSwitch = true;
				}
			}
		}

		playerColor = collisionSwitch ? Palette::Red : Palette::Yellow;
		if (collisionSwitch)
		{
			--playerHP;
		}

		line.draw(Alpha(20));

		for (auto& i : items)
		{
			if (AnyOf(visibleNoteIndices, [=](const uint32 note) { return note == i.ID; })) 
			{
				Rect visibleItem(noteRects[i.ID].startMillisec * scale + offset - pos, i.height, ITEM_WIDTH, ITEM_HEIGHT);
				if (visibleItem.intersects(player) && !i.isTaken)
				{
					i.isTaken = true;
					++numOfGetItem;
				}
				if (!i.isTaken)
				{
					visibleItem.draw(Palette::Blue);
				}
			}
		}

		player.draw(playerColor);
		font(L"HP:" + Format(playerHP) + L"\n" +
			L"アイテム:" + Format(numOfGetItem) + L"/" + Format(MAX_ITEM)).draw();
	}
}
