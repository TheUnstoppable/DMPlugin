// Unstoppable's Deathmatch Plugin
// Copyright 2020 Unstoppable
// 
// This plugin is free software: you can redistribute it and /or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.


#include "general.h"
#include "engine.h"
#include "engine_da.h"
#include "da.h"
#include "da_log.h"
#include "da_event.h"
#include "da_settings.h"
#include "DMPlugin.h"
#include <SpawnerClass.h>
#include <SpawnerDefClass.h>

#define IteratePlayers(Out) SLNode<cPlayer> *Out = Get_Player_List()->Head(); Out; Out = Out->Next()
REF_DEF2(DynamicVectorClass<SpawnerClass*>, SpawnerList, 0, 0x008564A8);

void DADeathmatchPlugin::Init()
{
	Register_Event(DAEvent::SETTINGSLOADED, 1);
	Register_Event(DAEvent::GAMEOVER, INT_MAX);
	Register_Event(DAEvent::LEVELLOADED);
	Register_Event(DAEvent::PLAYERJOIN);
	Register_Event(DAEvent::CHARACTERPURCHASEREQUEST, INT_MAX);
	Register_Event(DAEvent::VEHICLEPURCHASEREQUEST, INT_MAX);
	Register_Object_Event(DAObjectEvent::CREATED, DAObjectEvent::SOLDIER);
}

void DADeathmatchPlugin::Settings_Loaded_Event()
{
	PlayerModel = DASettingsManager::Get_String(PlayerModel, "DMPlugin", "PlayerModel", "c_ag_gdi_mp");
	ForceTeamID = DASettingsManager::Get_Int("DMPlugin", "ForceTeam", -1);
	GameOverForceTeamID = DASettingsManager::Get_Int("DMPlugin", "GameOverTeam", 1);
}

void DADeathmatchPlugin::Game_Over_Event()
{
	for (IteratePlayers(x))
	{
		cPlayer* Player = x->Data();
		if (Player && Player->Is_Active())
		{
			Player->Set_Player_Type(GameOverForceTeamID);
		}
	}
}

void DADeathmatchPlugin::Level_Loaded_Event()
{
	for (IteratePlayers(x))
	{
		cPlayer* Player = x->Data();
		if (Player && Player->Is_Active())
		{
			Player->Set_Player_Type(ForceTeamID);
		}
	}

	SoldierSpawners = new DynamicVectorClass<SpawnerClass*>;
	SoldierSpawnPoints = new DynamicVectorClass<Matrix3D>;

	Get_Soldier_Spawners();
	Get_Soldier_Spawnpoints();

	Console_Output("[Unstoppable's Deathmatch] Initializing Unstoppable's Deathmatch 1.1...\n", SoldierSpawners->Count());
	Console_Output("[Unstoppable's Deathmatch] Loaded %d spawners.\n", SoldierSpawners->Count());
	Console_Output("[Unstoppable's Deathmatch] Loaded %d spawn points.\n", SoldierSpawnPoints->Count());

	if (!SoldierSpawnPoints->Count())
	{
		Console_Output("[Unstoppable's Deathmatch] This game feature depends on at least 1 soldier spawner with at least 1 spawn point.\n");
		Console_Output("[Unstoppable's Deathmatch] ERROR: Can't continue initialization. Disabling game feature...\n");

		Unregister_Event(DAEvent::SETTINGSLOADED);
		Unregister_Event(DAEvent::GAMEOVER);
		Unregister_Event(DAEvent::LEVELLOADED);
		Unregister_Event(DAEvent::PLAYERJOIN);
		Unregister_Event(DAEvent::CHARACTERPURCHASEREQUEST);
		Unregister_Event(DAEvent::VEHICLEPURCHASEREQUEST);
		Unregister_Object_Event(DAObjectEvent::CREATED);
	}
}

void DADeathmatchPlugin::Player_Join_Event(cPlayer* Player)
{
	Player->Set_Player_Type(!The_Game()->Is_Game_Over() ? ForceTeamID : GameOverForceTeamID);
}

void DADeathmatchPlugin::Object_Created_Event(GameObject* obj)
{
	if (Commands->Get_Player_Type(obj) != ForceTeamID)
	{
		if(!The_Game()->Is_Game_Over())
			Commands->Set_Player_Type(obj, ForceTeamID);
		else
			Commands->Set_Player_Type(obj, GameOverForceTeamID);
	}
	else if(obj)
	{
		Set_Death_Points(obj, -Get_Death_Points(obj));
		Set_Damage_Points(obj, -Get_Damage_Points(obj));

		auto Location = Select_Random_Spawner();
		Commands->Set_Model(obj, PlayerModel);
		Commands->Set_Position(obj, Location);
		Fix_Stuck_Object(obj->As_SoldierGameObj(), 3.0f);
	}
	else
	{
		Console_Output("[Unstoppable's Deathmatch] Failed to handle one of the Object Created event. Please contact with developer for fix!\n");
	}
}

DynamicVectorClass<SpawnerClass*> *DADeathmatchPlugin::Get_Soldier_Spawners()
{
	SoldierSpawners->Delete_All();

	for (int i = 0; i < SpawnerList.Count(); ++i)
	{
		SpawnerClass* Spawner = SpawnerList[i];
		const SpawnerDefClass* Definition = Spawner->definition;
		if (Definition->IsSoldierStartup)
		{
			SoldierSpawners->Add(Spawner);
		}
	}

	return SoldierSpawners;
}

DynamicVectorClass<Matrix3D> *DADeathmatchPlugin::Get_Soldier_Spawnpoints()
{
	SoldierSpawnPoints->Delete_All();

	for (int i = 0; i < SoldierSpawners->Count(); ++i)
	{
		auto Spawner = (*SoldierSpawners)[i];
		SoldierSpawnPoints->Add(Spawner->transform);
		for (int a = 0; a < Spawner->Get_Spawn_Point_List().Count(); ++a)
		{
			SoldierSpawnPoints->Add(Spawner->Get_Spawn_Point_List()[a]);
		}
	}

	return SoldierSpawnPoints;
}

Vector3 DADeathmatchPlugin::Select_Random_Spawner()
{
	int Random = Get_Random_Int(0, SoldierSpawnPoints->Count());
	auto Point = (*SoldierSpawnPoints)[Random];
	return Point.Get_Translation();
}

Register_Game_Feature(DADeathmatchPlugin, "Unstoppable's Deathmatch", "EnableExtendedDeathmatch", 0);

