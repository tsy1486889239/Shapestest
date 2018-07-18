#include "LevelManager.h"
#include "fmod_inc\fmod.h"
#include "MathHandle.h"

//构造函数：初始化
LevelManager::LevelManager()
{
	
	this->stats = GAME_STATS_INITIAL;
	
	LevelManager::soundSystem = new SoundSystem();

	this->m_Player = new Player();

	this->m_NotePrefab = new CSprite("note_prefab");
	
	this->init();

	this->bindEffects();

	this->playingLevel = NULL;
	//test song
	
}

LevelManager::~LevelManager()
{
	//this->soundEngine->drop();
	//this->soundEngine = 0;
}

void LevelManager::init()
{


	//将json放在这里声明
	// EXAMPLE: 
	// this->playlist.push_back(new Level("examplesong.json",GAME_LEVEL_TYPE_PURESONG));
	this->playlist.push_back(new Level());//defaule song

}

//绑定特效
void LevelManager::bindEffects()
{
	//bind
	effects.insert(std::pair<char*, Effect*>("process_bar",new Effect_ProcessBar()));
	effects.insert(std::pair<char*, Effect*>("noteSlide_UP", new Effect_NoteSlide_Up()));
	effects.insert(std::pair<char*, Effect*>("noteSlide_RIGHT", new Effect_NoteSlide_Right()));
	effects.insert(std::pair<char*, Effect*>("noteSlide_DOWN", new Effect_NoteSlide_Down()));
	effects.insert(std::pair<char*, Effect*>("noteSlide_LEFT", new Effect_NoteSlide_Left()));
	effects.insert(std::pair<char*, Effect*>("handleFlash_UP", new Effect_HandleFlash_UP()));
	/*effects.insert(std::pair<char*, Effect*>("handleFlash_DOWN", new Effect_HandleFlash_DOWN()));
	effects.insert(std::pair<char*, Effect*>("handleFlash_RIGHT", new Effect_HandleFlash_RIGHT()));
	effects.insert(std::pair<char*, Effect*>("handleFlash_LEFT", new Effect_HandleFlash_LEFT()));*/
	
	
	//once
	effectsOnce.insert(std::pair<char*, Effect_Once*>("fade_in", new Effect_FadeIn()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("fade_out", new Effect_FadeOut()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("fade_128", new Effect_Fade128()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("move_x", new Effect_Pos_MovetoX()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("move_y", new Effect_Pos_MovetoY()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("size_h", new Effect_Size_Height()));
	effectsOnce.insert(std::pair<char*, Effect_Once*>("size_w", new Effect_Size_Width()));
	//how to use:
	// effectsOnce["move_y"]->addObject(m_NotePrefab,2);

}


void LevelManager::update(float deltaTime)
{
	soundSystem->update();
	m_Player->UpdateRender();
	
	if (stats == GAME_STATS_PLAYING) {
		
		/////////
		//effects
		std::map<char*, Effect*>::iterator _it;
		_it = effects.begin();
		while (_it != effects.end())
		{
			_it->second->loop(deltaTime);
			_it++;
		}

		/////////
		//effects once
		std::map<char*, Effect_Once*>::iterator _it2;
		_it2 = effectsOnce.begin();
		while (_it2 != effectsOnce.end())
		{
			_it2->second->update(deltaTime);
			_it2++;
		}

		//effect list
		((Effect_ProcessBar*)effects["process_bar"])->setSongPos(soundSystem->getPositionInMs()*1.0f / soundSystem->getSongLengthInMs());
		
		/////////
		//in play
		if (playingLevel->getLevelType() == GAME_LEVEL_TYPE_PURESONG) {
			//彦恺，你来调参数
			m_Player->currentSize = m_Player->DefaultSize + 1500.0f * soundSystem->getSpectrumCurruentTime(7);
		}
		else if (playingLevel->getLevelType() == GAME_LEVEL_TYPE_LEVEL) {//dspTIME refresh
			
			//path effects
			effects["noteSlide_UP"]->currents =
			effects["noteSlide_DOWN"]->currents =
			effects["noteSlide_LEFT"]->currents =
			effects["noteSlide_RIGHT"]->currents =
				MathHandle::ClampFloat(50, 128, (50 + 1000 * soundSystem->getSpectrumCurruentTime(2)));


			songPosition = soundSystem->getPositionInMs();

			songPosInBeats = (songPosition - playingLevel->songOffset) / (60000.0f / playingLevel->songTempo);

			//load NextClick
			if (nextHitObjectCur < playingLevel->beatmap.size() &&
				this->playingLevel->beatmap[nextHitObjectCur].getPosInBeat() <= beatsShownInAdvance+ songPosInBeats) {

				//Instantiate( /* Music Note Prefab */);
				char tempName[128];
				sprintf(tempName, "note_%d", nextHitObjectCur);
				playingLevel->beatmap[nextHitObjectCur].bindSprite = new CSprite(tempName);
				if (playingLevel->beatmap[nextHitObjectCur].bindSprite->CloneSprite("note_prefab")) {

					//pathBuffer[0].push_back(this->playingLevel->beatmap[nextHitObjectCur]);
					int _I = 0;//"_I"用来确定方向
					switch (playingLevel->beatmap[nextHitObjectCur].getType()) {
					case HIT_UP:
						_I = 0; break;
					case HIT_RIGHT:
						_I = 1; break;
					case HIT_DOWN:
						_I = 2; break;
					case HIT_LEFT:
						_I = 3; break;
					default:
						_I = -1;
					}
					if (_I != -1)
						pathBuffer[_I].push_back(this->playingLevel->beatmap[nextHitObjectCur]);
					nextHitObjectCur++;
				}
			}

			//update notes positions and check
			for (int _I = 0; _I < 4; _I++) {
				for (int _J = 0; _J < pathBuffer[_I].size(); _J++) {
					
					pathBuffer[_I][_J].bindSprite->SetSpritePosition(
						MathHandle::LerpFloat(
							beatsSpawn,
							(m_Player->HitSize + m_Player->DefaultSize) / 4.0f,
							(beatsShownInAdvance - (pathBuffer[_I][_J].getPosInBeat() - songPosInBeats)) / beatsShownInAdvance
						)*(_I == 1 ? 1 : _I == 3 ? -1 : 0),
						MathHandle::LerpFloat(
							beatsSpawn,
							(m_Player->HitSize + m_Player->DefaultSize) / 4.0f,
							(beatsShownInAdvance - (pathBuffer[_I][_J].getPosInBeat() - songPosInBeats)) / beatsShownInAdvance
						)*(_I == 0 ? -1 : _I == 2 ? 1 : 0)
					);
					
				}
				////////////////////////////////////
				//判定！！在这里！！
				//对每一边的第一个元素进行判定
				if (pathBuffer[_I].size() > 0) {
					//到达底线
					if (pathBuffer[_I][0].getPosInMs() - songPosition < - HIT_CHECKTIME_MISS) {
						
						//Miss Event Here
						m_Player->Hitted(0);
						//if (m_Player->combo >= 20) soundSystem->playFX(SOUND_FX_MISS);
						
						//destroy
						pathBuffer[_I][0].bindSprite->DeleteSprite();
						pathBuffer[_I][0].bindSprite = NULL;
						std::vector<HitObject>::iterator it = pathBuffer[_I].begin();
						pathBuffer[_I].erase(it);
					}
				}
			}
		}
		else if (playingLevel->getLevelType() == GAME_LEVEL_TYPE_CHAT) {
			//依晨你来写
		}
	}
}
//按键按下
void LevelManager::keyDown(const int iKey)
{
	HitObjectType type = HIT_UNDEFINE;
	int _I = -1;
	bool isHit = true;
	switch (iKey) {
	case KEY_UP:
		type = HIT_UP;
		_I = 0;
		break;
	case KEY_DOWN:
		type = HIT_DOWN;
		_I = 2;
		break;
	case KEY_LEFT:
		type = HIT_LEFT;
		_I = 3;
		break;
	case KEY_RIGHT:
		type = HIT_RIGHT;
		_I = 1;
		break;
	default:
		isHit = false;
	}

	if (stats == GAME_STATS_PLAYING) {//正在玩？
		if (playingLevel->getLevelType() == GAME_LEVEL_TYPE_LEVEL) {//按键只在游戏、对话里面响应方向。
			//Level Mode
			m_Player->OnKeyPressed(type);//传入数据
			if (isHit) { 
				//音效
				soundSystem->playFX(SOUND_FX_CLICK); 
				//判定
				if (pathBuffer[_I].size() > 0) {
					if (MathHandle::AbsFloat(pathBuffer[_I][0].getPosInMs() - songPosition) < HIT_CHECKTIME_MISS) {
						//Hit Event Here
						
						m_Player->Hitted(1);

						//flash effects
						effects["handleFlash_UP"]->trigger();

						//destroy
						pathBuffer[_I][0].bindSprite->DeleteSprite();
						pathBuffer[_I][0].bindSprite = NULL;
						std::vector<HitObject>::iterator it = pathBuffer[_I].begin();
						pathBuffer[_I].erase(it);
					}
				}
			}
			
			
		}
		else if (playingLevel->getLevelType() == GAME_LEVEL_TYPE_CHAT) {
			//Chat Mode

			//依晨你来写
		}
		else {//PureSong Mode
			_nextdelay--;//防误触
			//再按一次跳过
			if (_nextdelay < 1) { this->nextLevel(); _nextdelay = 1; }
		}
	}

	if (iKey == KEY_R) {//test
		this->playIndex = 0;
		this->playLevel(playlist[playIndex]);
	}
}

void LevelManager::playLevel(Level* level)
{
	this->stats = GAME_STATS_PASSING;

	nextHitObjectCur = 0;
	for (int __I = 0; __I < 4; __I++) { 
		for (int __J = 0; __J < pathBuffer[__I].size(); __J++) {
			pathBuffer[__I][__J].bindSprite->DeleteSprite();
			pathBuffer[__I][__J].bindSprite = NULL;
		}
		this->pathBuffer[__I].clear(); 
	}

	soundSystem->killPlaying();

	// instantiate
	if (level->loadLevel()) {
		this->playingLevel = level;
		if (level->getLevelType() == GAME_LEVEL_TYPE_LEVEL) {
			effects["handleFlash_UP"]->start();

			beatsShownInAdvance = level->songTempo / 70.0f;
		}
		if(soundSystem->playMusic(level->getSongPath()))
			this->stats = GAME_STATS_PLAYING;
		else {
			printf("playLevel Failed! retry..");
			playLevel(level);//tryAnain

		}
	}
	else printf("failed to play Level.");
		
}

void LevelManager::nextLevel() {
	if (playIndex < playlist.size() - 1) {
		playLevel(playlist[playIndex++]);
	}
}