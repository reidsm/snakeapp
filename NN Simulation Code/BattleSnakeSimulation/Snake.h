#pragma once



#include <iostream>
#include <vector>
#include <ctime>
#include <Windows.h>
//#include "NN.h"

using namespace std;


////////////////////
//structs
////////////////////
struct int2d {
	int x;
	int y;
};

////////////////////
//forward declarations
////////////////////
class Snake;
class Food;
void drawBoard();
bool isSpotTaken(int2d location);


////////////////////
//globals
////////////////////
int2d boardSize;
vector<Snake*> snakes;
vector<Snake*> enemySnakes;
Snake* playerSnake;
vector<Food*> foodVector;
const int maxPlayers = 8;
int players = 8;
int2d spawnPositions[maxPlayers];
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);



////////////////////
//classes
////////////////////

class Food {
private:
	int2d position;
	bool justEaten;

public:
	Food(int2d startPosition)
	{
		position = startPosition;
		justEaten = false;
	}

	void eatFood()
	{
		justEaten = true;
	}

	bool isEaten()
	{
		return justEaten;
	}

	bool isAtPosition(int2d ask2d)
	{
		return ((ask2d.x == position.x) && (ask2d.y == position.y));
	}

};

class Snake {
private:
	vector<int2d> body;
	int insideFood;
	int2d previousMove;
	bool isAI;
	bool firstMove;
	int size;

public:
	Snake(int2d spawn, bool aiControlled) {
		//cout << "Snake Created" << endl;
		body.push_back(spawn);
		insideFood = 2;
		size = 3;
		previousMove = { 1,0 };
		isAI = aiControlled;
		firstMove = true;
	}

	~Snake()
	{
		//cout << "Snake Destroyed" << endl;
	}

	int2d bodyDirection(int2d bodyPart)
	{
		for (unsigned int i = 0; i < body.size(); i++)
		{
			if((body.at(i).x== bodyPart.x)&& (body.at(i).y == bodyPart.y))
				return { body.at(i - 1).x - body.at(i).x,body.at(i - 1).y - body.at(i).y };
		}

		//ERROR
		throw ERROR;
		return { 0,0 };
	}

	void move(int2d offset)
	{
		//check if the snake is trying to move backwards
		if ((offset.x == -previousMove.x) && (offset.y == -previousMove.y) && (!firstMove))
		{
			offset = previousMove;
		}
		firstMove = false;

		//keep track of this move
		previousMove = offset;

		//move the snake forward with the tail following
		int2d previousLocation;
		previousLocation.x = body.at(0).x + offset.x;
		previousLocation.y = body.at(0).y + offset.y;
		for (unsigned int i = 0; i < body.size(); i++)
		{
			int2d currentLocation = body.at(i);
			body.at(i) = previousLocation;
			previousLocation = currentLocation;
		}
		//Snake is not fully grown... tail must get longer
		if (insideFood > 0)
		{
			body.push_back(previousLocation);
			insideFood--;
		}
	}

	void eatFood()
	{
		int2d newHeadPosition = body.at(0);
		for (unsigned int i = 0; i < foodVector.size(); i++)
		{
			if (foodVector.at(i)->isAtPosition(newHeadPosition))
			{
				foodVector.at(i)->eatFood();
				size++;
				insideFood++;
			}
		}
	}

	void collisionPlayer()
	{
		for (unsigned int i = 0; i < snakes.size(); i++)
		{
			if (collisionCheck(snakes.at(i)))
			{
				//we lost the collision with an enemy
				for (unsigned int f = 0; f < snakes.size(); f++)
					if (snakes.at(f) == this)
					{
						snakes.erase(snakes.begin() + f);
						break;
					}

				playerSnake = NULL;

				delete this;
				break;
			}
		}
	}

	void movePlayer(int2d offset)
	{
		move(offset);
	}

	void moveAI()
	{
		//don't hit walls
		if ((previousMove.x == 1) && (body.at(0).x >= boardSize.x - 2))
			move({ 0,1 });
		else if ((previousMove.y == 1) && (body.at(0).y >= boardSize.y - 2))
			move({ -1,0 });
		else if ((previousMove.x == -1) && (body.at(0).x <= 1))
			move({ 0,-1 });
		else if ((previousMove.y == -1) && (body.at(0).y <= 1))
			move({ 1,0 });
		else
			move(previousMove);
	}

	void collisionAI()
	{
		for (unsigned int i = 0; i < snakes.size(); i++)
		{
			if (collisionCheck(snakes.at(i)))
			{
				//we lost the collision with a snake
				for (unsigned int f = 0; f < snakes.size(); f++)
					if (snakes.at(f) == this)
					{
						snakes.erase(snakes.begin() + f);
						break;
					}
				for (unsigned int f = 0; f < enemySnakes.size(); f++)
					if (enemySnakes.at(f) == this)
					{
						enemySnakes.erase(enemySnakes.begin() + f);
						break;
					}

				delete this;
				break;
			}
		}
	}

	bool isEndOfTail(int2d location)
	{
		int2d endOfTail =  .at(body.size() - 1);
		return ((endOfTail.x == location.x) && (endOfTail.y == location.y));
	}

	//return true if the main snake should die
	bool collisionCheck(Snake* collisionSnake)
	{
		int2d headLocation = body.at(0);
		//check for wall collision
		if ((headLocation.x < 0) || (headLocation.x > boardSize.x-1) || (headLocation.y < 0) || (headLocation.y > boardSize.y-1))
			return 1;

		int collisionType = collisionSnake->bodyPartAtLocation(headLocation);
		//head on head collision
		if ((collisionType == 2) && (this != collisionSnake))
		{
			int collisionSnakeLength = collisionSnake->getBodyLength();
			if (getBodyLength() < collisionSnakeLength)
				return true;
			else
			{
				//remove it the collision snake from the vector lists
				//find out if its the player
				if (playerSnake == collisionSnake)
					playerSnake = NULL;
				else
					for (unsigned int i = 0; i < enemySnakes.size(); i++)
						if (enemySnakes.at(i) == collisionSnake)
						{
							enemySnakes.erase(enemySnakes.begin() + i);
							break;
						}
				for (unsigned int i = 0; i < snakes.size(); i++)
					if (snakes.at(i) == collisionSnake)
					{
						snakes.erase(snakes.begin() + i);
						break;
					}

				delete collisionSnake;

				//if they are both the same size they both die
				if (getBodyLength() == collisionSnakeLength)
					return true;
				else
					return false;
			}
		}
		//head to body collision
		else if (collisionType == 1)
			return true;
		//self collision
		else if (collisionType == 2)
		{
			for (unsigned int i = 1; i < body.size(); i++)
			{
				if ((headLocation.x == body.at(i).x) && (headLocation.y == body.at(i).y))
					return true;
			}
		}

		return false;
	}

	//return 0		nothing here
	//return 1		body here
	//return 2		head here
	int bodyPartAtLocation(int2d ask2d)
	{
		for (unsigned int i = 0; i < body.size(); i++)
		{
			int2d currentLocation = body.at(i);

			if ((ask2d.x == currentLocation.x) && (ask2d.y == currentLocation.y))
			{
				if (i == 0)
					return 2;
				else
					return 1;
			}
		}

		return 0;
	}

	int getBodyLength()
	{
		return size;
	}

	bool getAIStatus()
	{
		return isAI;
	}
};



////////////////////
//Functions
////////////////////
void drawBoard()
{
	system("CLS");
	for (int y = -1; y < boardSize.y + 1; y++)
	{
		for (int x = -1; x < boardSize.x + 1; x++)
		{
			if ((y == -1) || (y == boardSize.y) || (x == -1) || (x == boardSize.x))
			{
				SetConsoleTextAttribute(hConsole, 7);
				cout << "#";
			}
			else
			{
				bool spotTaken = false;
				for (unsigned int i = 0; i < snakes.size(); i++)
				{
					if (snakes.at(i)->getAIStatus())
						SetConsoleTextAttribute(hConsole, 4);
					else
						SetConsoleTextAttribute(hConsole, 2);

					int snakeBodyPart = snakes.at(i)->bodyPartAtLocation({ x,y });
					if (snakeBodyPart == 2)
					{
						spotTaken = true;
						cout << "@";
						break;
					}
					else if (snakeBodyPart == 1)
					{
						spotTaken = true;
						cout << "O";
						break;
					}
				}
				for (unsigned int i = 0; i < foodVector.size(); i++)
				{
					if (foodVector.at(i)->isAtPosition({ x,y }))
					{
						spotTaken = true;
						SetConsoleTextAttribute(hConsole, 1);
						cout << "x";
						break;
					}
				}

				if (!spotTaken)
					cout << " ";
			}
		}
		cout << endl;
	}
	cout << endl;
	SetConsoleTextAttribute(hConsole, 8);
}


bool createFood(int2d newPosition)
{
	bool spotTaken = isSpotTaken(newPosition);
	if (!spotTaken)
	{
		Food* newFood = new Food(newPosition);
		foodVector.push_back(newFood);
	}

	return spotTaken;
}

void randomlyCreateFood(int chance)
{
	int2d newPosition;
	bool spotTaken = true;
	if (rand() % 100 < chance)
	{
		while (spotTaken)
		{
			newPosition.x = rand() % boardSize.x;
			newPosition.y = rand() % boardSize.y;
			spotTaken = createFood(newPosition);
		}
	}
}

void createRandomSnakes()
{
	int newSpawn;
	int2d newSpawnPosition;
	bool spotTaken;
	Snake* enemySnake;

	newSpawn = rand() % maxPlayers;
	playerSnake = new Snake(spawnPositions[newSpawn], false);
	snakes.push_back(playerSnake);

	for (int i = 0; i < players - 1; i++)
	{
		spotTaken = true;
		while (spotTaken)
		{
			newSpawn = rand() % maxPlayers;
			newSpawnPosition = spawnPositions[newSpawn];
			spotTaken = isSpotTaken(newSpawnPosition);
			if (!spotTaken)
			{
				enemySnake = new Snake(newSpawnPosition, true);
				enemySnakes.push_back(enemySnake);
				snakes.push_back(enemySnake);
			}
		}
	}
}

bool isSpotTaken(int2d location)
{
	for (unsigned int i = 0; i < snakes.size(); i++)
	{
		if (snakes.at(i)->bodyPartAtLocation(location))
		{
			return true;
		}
	}
	for (unsigned int i = 0; i < foodVector.size(); i++)
	{
		if (foodVector.at(i)->isAtPosition(location))
		{
			return true;
		}
	}
	return false;
}

void updateAI()
{
	for (unsigned int i = 0; i < enemySnakes.size(); i++)
		enemySnakes.at(i)->moveAI();
}

void updatePlayer()
{
	if (playerSnake == NULL)
		return;

	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		playerSnake->movePlayer({ 1,0 });
	}
	if (GetKeyState(VK_LEFT) & 0x8000)
	{
		playerSnake->movePlayer({ -1,0 });
	}
	if (GetKeyState(VK_UP) & 0x8000)
	{
		playerSnake->movePlayer({ 0,-1 });
	}
	if (GetKeyState(VK_DOWN) & 0x8000)
	{
		playerSnake->movePlayer({ 0,1 });
	}
}

void removeAllEatenFood()
{
	for (unsigned int i = 0; i < foodVector.size(); i++)
	{
		if (foodVector.at(i)->isEaten())
		{
			Food* currentFood = foodVector.at(i);
			foodVector.erase(foodVector.begin() + i);
			delete currentFood;
			i--;
		}
	}
}

void eatFoodUpdate()
{
	for (unsigned int i = 0; i < snakes.size(); i++)
	{
		snakes.at(i)->eatFood();
	}
}


void updateCollisions()
{
	if (playerSnake != NULL)
		playerSnake->collisionPlayer();
	for (unsigned int i = 0; i < enemySnakes.size(); i++)
	{
		enemySnakes.at(i)->collisionAI();
	}
}

void SetSpawn()
{
	spawnPositions[0] = { 1,1 };
	spawnPositions[1] = { (boardSize.x - 1) / 2,1 };
	spawnPositions[2] = { boardSize.x - 2,1 };
	spawnPositions[3] = { boardSize.x - 2,(boardSize.y - 1) / 2 };
	spawnPositions[4] = { boardSize.x - 2,boardSize.y - 2 };
	spawnPositions[5] = { (boardSize.x - 1) / 2,boardSize.y - 2 };
	spawnPositions[6] = { 1,boardSize.y - 2 };
	spawnPositions[7] = { 1,(boardSize.y - 1) / 2 };
}
