// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "IntVectorTypes.h"

#include "Terrain/FDungeonPos.h"

// using FDungeonPos = UE::Geometry::FVector2i;

// --- CORE UTILS
UENUM(BlueprintType)
enum  FDirection : uint8
{
	North = 0,
	West  = 1,
	South = 2,
	East  = 3
};


/**
 * 
 */
namespace Crawlerino
{
	namespace Utils
	{
		inline float EaseOutCubic(float x)
		{
			return 1 - pow(1 - x, 3);
		}

		inline float EaseOutCirc(float x) {
			return sqrt(1 - pow(x - 1, 2));
		}

		inline float EaseInOutCirc(float x)
		{
			return x < 0.5
				       ? (1 - sqrt(1 - pow(2 * x, 2))) / 2
				       : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
		}

		inline float ClampAngle(float RawAngle)
		{
			float Angle = static_cast<int>(RawAngle) % 360;
			if (Angle < 0) { Angle += 360.0f; }
			return Angle;
		}

		inline FDirection DirFromAngle(float Angle)
		{
			if (Angle > 315.0f || Angle < 45.0f)
			{
				return FDirection::North;
			} else if (Angle > 45.0f && Angle < 135.0f)
			{
				return FDirection::East;
			} else if (Angle > 135.0f && Angle < 225.0f)
			{
				return FDirection::South;
			} else /*(Angle > 225.0f && Angle < 315.0f)*/
			{
				return FDirection::West;
			}
		}
	}
	
	inline float GetYawFromDirection(FDirection direction)
	{
		switch (direction)
		{
			case FDirection::North: return   0.0f;
			case FDirection::West:  return  90.0f;
			case FDirection::South: return 180.0f;
			case FDirection::East:  return 270.0f;
			default: return 0;
		}
	}

	// --- GRID UTILS
	struct CRAWLERINO_API DungeonTile
	{
		int Value{-1};
		FDungeonPos Pos{-1, -1};
	};

	struct CRAWLERINO_API RoomDescriptor
	{
		int Width;
		int Height;
	};

	struct CRAWLERINO_API RoomInfo
	{
		int RoomIndex{-1};
		FDungeonPos MinRoomPos{-1, -1};
		FDungeonPos MaxRoomPos{-1, -1};
	};
	
	class CRAWLERINO_API DungeonGrid
	{
	public:
		/**
		 * Basic random room placement algorithm :
		 *	- Grid based, using height, width, amount of room to place and pattern descriptions of rooms
		 *	- Place each room making sure it fits the grid size and it doesn't overlap with other rooms
		 *	- For each room created one door position is stored
		 *	- Take the first door position and connect it to the nearest other door, until all doors are connected to another
		 */
		static void GenerateTerrain(DungeonGrid& Grid, const std::vector<RoomDescriptor>& RoomsToPlace);

		// Room placing utils methods
		static bool DoesRoomFit(const DungeonGrid& Grid, FDungeonPos TopLeft, FDungeonPos Size);
		static FDirection GetRandomDoorDirection(const DungeonGrid& Grid, FDungeonPos TopLeft);
		static FDungeonPos GetRandomDoorPos(FDungeonPos TopLeft, FDungeonPos BottomRight, FDirection Facing);
		static int GetNearestDoor(int From, const std::vector<FDungeonPos>& Doors, std::vector<bool> Connected);
	public:
		DungeonGrid() = delete;
		DungeonGrid(int Height, int Width);
		~DungeonGrid();
	public:
		int Height() const { return _Height; }
		int Width() const { return _Width; }
		bool IsValidPosition(FDungeonPos Pos) const { return Pos.X >= 0 && Pos.X < _Width && Pos.Y >= 0 && Pos.Y < _Height; }
		FDungeonPos StartPos() const { return _StartPos; }
		int GetValue(int X, int Y) const;
		bool IsWalkable(const FDungeonPos& Pos) const;

		int GetRoomIndex(FDungeonPos Pos) const;
		bool GrabRoomInfo(FDungeonPos Pos, RoomInfo& RoomInfo, std::vector<FDungeonPos>& TilePositions, int Radius) const;
	private:
		void SetValue(int X, int Y, int Value);
		void SetStartPos(FDungeonPos StartPos);
	private:
		int _Height, _Width;
		FDungeonPos _StartPos;
		std::vector<DungeonTile> _Data;
	};
	
	static bool GetShortestPath(DungeonGrid& Grid, FDungeonPos Start, FDungeonPos Target, std::vector<FDungeonPos>& Path);
}
