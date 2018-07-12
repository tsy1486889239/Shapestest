#include <fstream>
#include "Level.h"
#include "dist\json\json.h"

Level::Level()
{
	this->loadMap("../Assets/Songs/level0.json");
}

Level::Level(std::string mapname)
{
	this->loadMap("../Assets/Songs/" + mapname);
}

void Level::loadMap(std::string songInfoFilePath)
{
	std::ifstream ifs;
	ifs.open(songInfoFilePath);
	if (!ifs.is_open()) {
		printf("Error to open file");
		return;
	}
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(ifs, root, false))
	{
		printf("parse failed \n");
		return;
	}

	//把json里面的数据一一对应到这个类的各个成员
	this->songName = root["songName"].asString();
	this->songTempo = root["songTempo"].asFloat();
	//尝试新东西
}
