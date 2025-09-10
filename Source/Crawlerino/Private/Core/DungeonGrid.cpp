// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/DungeonGrid.h"

#include <queue>
#include <unordered_set>

using namespace Crawlerino;

void DungeonGrid::GenerateTerrain(DungeonGrid& Grid, const std::vector<RoomDescriptor>&  RoomsToPlace)// int RoomToPlace, DungeonPos MinRoomSize, DungeonPos MaxRoomSize)
{
	std::vector<DungeonPos> DoorPositions(RoomsToPlace.size());

	int Width = Grid.Width();
	int Height = Grid.Height();

	// Place rooms at random location
	for (int i = 0; i < RoomsToPlace.size(); i++)
	{
		RoomDescriptor Room = RoomsToPlace[i];
		DungeonPos RoomSize = DungeonPos{Room.Width, Room.Height};

		// Find suitable position for room
		DungeonPos TopLeftCorner;
		do
		{
			TopLeftCorner = {FMath::RandRange(0, Width - 1), FMath::RandRange(0, Height - 1)};
		} while (!DoesRoomFit(Grid, TopLeftCorner, RoomSize));

		if (i == 0)
		{
			Grid.SetStartPos(TopLeftCorner);
		}

		DungeonPos BottomRightCorner = TopLeftCorner + RoomSize;

		Direction DoorDirection = GetRandomDoorDirection(Grid, TopLeftCorner);
		DoorPositions[i] = GetRandomDoorPos(TopLeftCorner, BottomRightCorner, DoorDirection);;

		// Fill room
		for (int Y = TopLeftCorner.Y; Y < BottomRightCorner.Y; Y++)
		{
			for (int X = TopLeftCorner.X; X < BottomRightCorner.X; X++)
			{
				if (!(X >= 0 && X < Grid.Width() && Y >= 0 && Y < Grid.Height()))
				{
					UE_LOG(LogTemp, Warning, TEXT("Could not create room at (%d, %d) of size (%d, %d)"), TopLeftCorner.X, TopLeftCorner.Y, RoomSize.X, RoomSize.Y);
				} else
				{
					Grid.SetValue(X, Y, i+2);
				}
			}
		}
	}

	// Connect room with corridors
	std::vector<bool> Connected(RoomsToPlace.size(), false);

	int Index = 0;

	while (true)
	{
		UE_LOG(LogTemp, Log, TEXT("Processing door of index %d"), Index);
		int Nearest = GetNearestDoor(Index, DoorPositions, Connected);

		if (Nearest == -1) break;

		DungeonPos From = DoorPositions[Index];
		DungeonPos To = DoorPositions[Nearest];

		while (From != To)
		{
			if (From.X != To.X)
			{
				From.X += To.X - From.X > 0 ? 1 : -1;
			} else
			{
				From.Y += To.Y - From.Y > 0 ? 1 : -1;
			}

			Grid.SetValue(From.X, From.Y, 1);
		}

		Connected[Index] = true;
		Index = Nearest;
	}
}

bool DungeonGrid::DoesRoomFit(const DungeonGrid& Grid, DungeonPos TopLeft, DungeonPos Size)
{
	DungeonPos BottomRight = TopLeft + Size;
	
	for (int Y = TopLeft.Y; Y < BottomRight.Y; Y++)
	{
		// Room exceeds on north side
		if (Y >= Grid.Height()) return false;
		
		for (int X = TopLeft.X; X < BottomRight.X; X++)
		{
			// Room exceeds on east side
			if (X >= Grid.Width()) return false;

			// Room overlap with already existing door
			if (Grid.GetValue(X, Y) != 0) return false;
		}
	}

	// Room fits if no test above success
	return true;
}

Direction DungeonGrid::GetRandomDoorDirection(const DungeonGrid& Grid, DungeonPos TopLeft)
{
	Direction Dir = Direction::North;
	int Dice = FMath::RandRange(0, 1);
	if (TopLeft.X < Grid.Width() / 2.0)
	{
		if (TopLeft.Y < Grid.Height() / 2.0)
		{
			Dir = Dice ? East : South;
		}
		else
		{
			Dir = Dice ? North : East;
		}
	}
	else
	{
		if (TopLeft.X < Grid.Height() / 2.0)
		{
			Dir = Dice ? West : South;
		}
		else
		{
			Dir = Dice ? North : West;
		}
	}

	return Dir;
}

DungeonPos DungeonGrid::GetRandomDoorPos(DungeonPos TopLeft, DungeonPos BottomRight, Direction Facing)
{
	switch (Facing)
	{
		case North:
			return DungeonPos{FMath::RandRange(TopLeft.X, BottomRight.X - 1), TopLeft.Y};
		case East:
			return DungeonPos{BottomRight.X - 1, FMath::RandRange(TopLeft.Y, BottomRight.Y - 1)};
		case South:
			return DungeonPos{FMath::RandRange(TopLeft.X, BottomRight.X - 1), BottomRight.Y - 1};
		case West:
			return DungeonPos{TopLeft.X, FMath::RandRange(TopLeft.Y, BottomRight.Y - 1)};
	}
	return {-1, -1};
}

int DungeonGrid::GetNearestDoor(int From, const std::vector<DungeonPos>& Doors, std::vector<bool> Connected)
{
	DungeonPos FromPos = Doors[From];

	int Result = -1;
	int MinDistance = INT_MAX;
	
	for (int i = 0 ; i < Doors.size(); i++)
	{
		// Don't connect to itself
		if (i == From)
		{
			UE_LOG(LogTemp, Log, TEXT("i == From (%d == %d)"), i, From);
			continue;
		}

		// Door already connected
		if (Connected[i])
		{
			UE_LOG(LogTemp, Log, TEXT("i == Connected[%d]"), i);
			continue;
		}

		DungeonPos ToPos = Doors[i];
		int Dist = abs(FromPos.X - ToPos.X) + abs(FromPos.Y - ToPos.Y);

		if (Dist < MinDistance)
		{
			UE_LOG(LogTemp, Log, TEXT("Nearest found %d at distance %d (Previous Min %d)"), i, Dist, MinDistance);
			MinDistance = Dist;
			Result = i;
		} else
		{
			UE_LOG(LogTemp, Log, TEXT("Further found %d at distance %d (Min %d)"), i, Dist, MinDistance);
		}
	}

	return Result;
}

DungeonGrid::DungeonGrid(int Height, int Width)
{
	check(Height > 0 && Width > 0);

	_Height = Height;
	_Width = Width;
	
	_Data = std::vector<DungeonTile>(0);
	for (int i = 0; i < _Height * _Width; i++)
	{
		_Data.push_back(DungeonTile{0, DungeonPos{i % _Width, i / _Width}});
	}
	_StartPos = DungeonPos{0, 0};
}

DungeonGrid::~DungeonGrid()
{
	_Data.clear();
}

int DungeonGrid::GetValue(int X, int Y) const
{
	check(X >= 0 && X < _Width && Y >= 0 && Y < _Height);

	return _Data[Y * _Width + X].Value;
}

void DungeonGrid::SetValue(int X, int Y, int Value)
{
	check(X >= 0 && X < _Width && Y >= 0 && Y < _Height);

	_Data[Y * _Width + X].Value = Value;
}

void DungeonGrid::SetStartPos(DungeonPos StartPos)
{
	check(StartPos.X >= 0 && StartPos.X < _Width && StartPos.Y >= 0 && StartPos.Y < _Height);

	_StartPos = StartPos;
}

bool DungeonGrid::CanMoveTo(int X, int Y) const
{
	if (X >= 0 && X < _Width && Y >= 0 && Y < _Height)
	{
		return _Data[Y * _Width + X].Value > 0;
	}

	return false;
}

int DungeonGrid::GetRoomIndex(DungeonPos Pos) const
{
	check(Pos.X >= 0 && Pos.X < _Width && Pos.Y >= 0 && Pos.Y < _Height);
	int Value = GetValue(Pos.X, Pos.Y);
	return Value > 1 ? Value : -1;
}

bool DungeonGrid::GrabRoomInfo(DungeonPos Pos, RoomInfo& RoomInfo, std::vector<DungeonPos>& TilePositions, int Radius) const
{
	DungeonPos MinTile{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
	DungeonPos MaxTile{std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
	int TileAmount{0};

	TilePositions.clear();
	for (DungeonTile Data : _Data)
	{
		DungeonPos TilePos = Data.Pos;
		int Dist = abs(TilePos.X - Pos.X) + abs(TilePos.Y - Pos.Y);

		if (Dist <= Radius)
		{
			TilePositions.push_back(Data.Pos);

			if (TilePos.X < MinTile.X) MinTile.X = TilePos.X;
			if (TilePos.X > MaxTile.X) MaxTile.X = TilePos.X;
			if (TilePos.Y < MinTile.Y) MinTile.Y = TilePos.Y;
			if (TilePos.Y > MaxTile.Y) MaxTile.Y = TilePos.Y;

			TileAmount++;
		}
		/*if (Data.Value == RoomIndex)
		{
			DungeonPos TilePos = Data.Pos;
			if (TilePos.X < MinTile.X) MinTile.X = TilePos.X;
			if (TilePos.X > MaxTile.X) MaxTile.X = TilePos.X;
			if (TilePos.Y < MinTile.Y) MinTile.Y = TilePos.Y;
			if (TilePos.Y > MaxTile.Y) MaxTile.Y = TilePos.Y;

			TilePositions.push_back(TilePos);

			TileAmount++;
		}*/
	}

	if (TileAmount == 0) return false;

	// RoomInfo.RoomIndex = RoomIndex;
	RoomInfo.MinRoomPos = MinTile;
	RoomInfo.MaxRoomPos = MaxTile;

	return true;
}

bool Crawlerino::GetShortestPath(DungeonGrid& Grid, DungeonPos Start, DungeonPos Target, std::vector<DungeonPos>& Path)
{
	/*struct SearchNode
	{
		SearchNode* From;
		DungeonPos Pos;
		int Weight;
	};

	class Comp
	{
	public:
		bool operator()(const SearchNode& A, const SearchNode& B) const
		{
			return A.Weight >= B.Weight;
		}
	};

	auto Dist = [](DungeonPos Origin, DungeonPos Target)
	{
		return abs(Target.X - Origin.X) + abs(Target.Y - Origin.Y);
	};

	auto GetNeighbors = [](const DungeonGrid& Grid , DungeonPos Pos, std::vector<DungeonPos>& Neighbors)
	{
		Neighbors.clear();

		DungeonPos Up{Pos.X, Pos.Y + 1};
		DungeonPos Down{Pos.X, Pos.Y - 1};
		DungeonPos Left{Pos.X + 1, Pos.Y};
		DungeonPos Right{Pos.X - 1, Pos.Y};

		if (Grid.IsValidPosition(Up)) { Neighbors.push_back(Up); }
		if (Grid.IsValidPosition(Down)) { Neighbors.push_back(Down); }
		if (Grid.IsValidPosition(Left)) { Neighbors.push_back(Left); }
		if (Grid.IsValidPosition(Right)) { Neighbors.push_back(Right); }
	};

	std::unordered_set<DungeonPos, std::hash<DungeonPos>> Closed{};

	std::vector<SearchNode> OpenNodes{SearchNode{nullptr, Start, 0}};
	std::priority_queue<SearchNode, std::vector<SearchNode>, Comp> Open{OpenNodes.begin(), OpenNodes.end()};

	while (!Open.empty())
	{
		SearchNode Node = Open.top();

		if (Node.Pos == Target)
		{
			// TODO Reconstruct path
			return true;
		}

		Open.pop();
	}*/
	
	
	return false;
}
