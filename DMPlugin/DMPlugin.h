// Unstoppable's Deathmatch Plugin
// Copyright 2020 Unstoppable
// 
// This plugin is free software: you can redistribute it and /or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.


#include "da_event.h"
#include "da_gamefeature.h"

class DADeathmatchPlugin : public DAEventClass, public DAGameFeatureClass {
public:
	DADeathmatchPlugin();
	~DADeathmatchPlugin();

	virtual void Init();
	virtual void Settings_Loaded_Event();
	virtual void Game_Over_Event();
	virtual void Level_Loaded_Event();
	virtual void Player_Join_Event(cPlayer* Player);
	virtual int Character_Purchase_Request_Event(BaseControllerClass* Base, cPlayer* Player, float& Cost, const SoldierGameObjDef* Item) { return 4; }
	virtual int Vehicle_Purchase_Request_Event(BaseControllerClass* Base, cPlayer* Player, float& Cost, const VehicleGameObjDef* Item) { return 4; }
	virtual void Object_Created_Event(GameObject* obj);
private:
	DynamicVectorClass<SpawnerClass*> *Get_Soldier_Spawners();
	DynamicVectorClass<Matrix3D> *Get_Soldier_Spawnpoints();
	Vector3 Select_Random_Spawner();

protected:
	StringClass PlayerModel;
	int ForceTeamID;
	int GameOverForceTeamID;
	DynamicVectorClass<SpawnerClass*> *SoldierSpawners;
	DynamicVectorClass<Matrix3D> *SoldierSpawnPoints;
};