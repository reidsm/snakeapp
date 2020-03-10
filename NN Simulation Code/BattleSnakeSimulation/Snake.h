#pragma once

#include <iostream>
#include <vector>
#include <ctime>
#include <Windows.h>
#include <algorithm>
//#include <string>


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
const int2d boardSize = {11,11};
vector<vector<Snake*>*> snakes;
vector<Food*> foodVector;
const int maxPlayers = 8;
const int startingPlayerCount = 8;
int2d spawnPositions[maxPlayers];
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
const int maxHunger = 100;


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
	int2d lastMoveDirection;
	bool firstMove;
	int size;
	bool aboutToDie;
	int hunger;

public:
	vector<double> lastOutputValues;
	int lastMove;
	int2d nextMoveDirection;
	vector<double> rewards;




public:
	Snake(int2d spawn) {
		body.push_back(spawn);
		insideFood = 2;
		size = 3;
		lastMoveDirection = { 1,0 };
		firstMove = true;
		aboutToDie = false;
		lastMove = -100;
		nextMoveDirection = { 1,0 };
		hunger = maxHunger;
	}


	void addReward(double rewardAmount)
	{
		rewards.push_back(rewardAmount);
	}


	double getRewards()
	{
		double rewardTotal = 1.0;


		for (int i = 0;i<rewards.size();i++)
		{
			rewards.at(i) = 1.0 - rewards.at(i);
		}

		for (int i = 0; i < rewards.size(); i++)
		{
			rewardTotal *= rewards.at(i);
		}

		rewardTotal = 1.0 - rewardTotal;
		rewards.clear();

		return rewardTotal;
	}


	int getHunger()
	{
		return hunger;
	}


	int2d rotationMoveToDirection(int rotation)
	{
		//last move we were going right
		if ((lastMoveDirection.x == 1) && (lastMoveDirection.y == 0))
		{
			//rotation = -1		turn left
			if (rotation == -1)
				return { 0,-1 };
			//rotation =  0		go straight
			if (rotation == 0)
				return { 1,0 };
			//rotation =  1		turn right
			if (rotation == 1)
				return { 0,1 };
		}
		//last move we were going left
		if ((lastMoveDirection.x == -1) && (lastMoveDirection.y == 0))
		{
			//rotation = -1		turn left
			if (rotation == -1)
				return { 0,1 };
			//rotation =  0		go straight
			if (rotation == 0)
				return { -1,0 };
			//rotation =  1		turn right
			if (rotation == 1)
				return { 0,-1 };
		}
		//last move we were going up
		if ((lastMoveDirection.x == 0) && (lastMoveDirection.y == -1))
		{
			//rotation = -1		turn left
			if (rotation == -1)
				return { -1,0 };
			//rotation =  0		go straight
			if (rotation == 0)
				return { 0,-1 };
			//rotation =  1		turn right
			if (rotation == 1)
				return { 1,0 };
		}
		//last move we were going down
		if ((lastMoveDirection.x == 0) && (lastMoveDirection.y == 1))
		{
			//rotation = -1		turn left
			if (rotation == -1)
				return { 1,0 };
			//rotation =  0		go straight
			if (rotation == 0)
				return { 0,1 };
			//rotation =  1		turn right
			if (rotation == 1)
				return { -1,0 };
		}

		throw ERROR;
	}


	bool isAboutToDie()
	{
		return aboutToDie;
	}


	bool killUpdate()
	{
		//clean up all of the vectors and delete the snake object
		if (aboutToDie)
		{
			for (int i = 0; i < snakes.size(); i++)
			{
				if (snakes.at(i)->at(0) == this)
				{
					//we found this snake's main vector
					snakes.at(i)->clear();
					delete snakes.at(i);
					snakes.erase(snakes.begin() + i);
					i--;
				}
				else
				{
					//this is some other snake's main vector
					for (int f = 1; f < snakes.at(i)->size(); f++)
					{
						if (snakes.at(i)->at(f) == this)
						{
							snakes.at(i)->erase(snakes.at(i)->begin() + f);
							break;
						}
					}
				}
			}

			delete this;
			return true;
		}

		return false;
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


	void move()
	{
		//check if the snake is trying to move backwards
		if ((nextMoveDirection.x == -lastMoveDirection.x) && (nextMoveDirection.y == -lastMoveDirection.y) && (!firstMove))
		{
			nextMoveDirection = lastMoveDirection;
		}
		firstMove = false;

		//keep track of this move
		lastMoveDirection = nextMoveDirection;

		//move the snake forward with the tail following
		int2d previousLocation;
		previousLocation.x = body.at(0).x + nextMoveDirection.x;
		previousLocation.y = body.at(0).y + nextMoveDirection.y;
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

		//hunger check
		hunger--;
		if (hunger <= 0)
			setToDie();
	}


	int getInsideFood()
	{
		return insideFood;
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
				hunger = maxHunger;

				//REWARD
				addReward(0.75);
			}
		}
	}


	void setToDie()
	{
		aboutToDie = true;

		//REWARD
		//every other snake that is alive
		for (int i = 0; i < snakes.size(); i++)
		{
			//no rewards for this dead af snake
			if (this != snakes.at(i)->at(0))
			{
				double rewardAmount = 1;

				if (snakes.size() == 2)
					rewardAmount = 1.00;
				if (snakes.size() == 3)
					rewardAmount = 0.95;
				if (snakes.size() == 4)
					rewardAmount = 0.90;
				if (snakes.size() == 5)
					rewardAmount = 0.80;
				if (snakes.size() == 6)
					rewardAmount = 0.70;
				if (snakes.size() == 7)
					rewardAmount = 0.60;

				//addReward(rewardAmount);
			}
		}
		
	}


	int findSnakeVector()
	{
		for (int i = 0; i < snakes.size(); i++)
		{
			if (snakes.at(i)->at(0) == this)
			{
				return i;
			}
		}
		throw ERROR;
	}


	void collisionUpdate()
	{
		int2d headLocation = body.at(0);
		//check for wall collision
		if ((headLocation.x < 0) || (headLocation.x > boardSize.x - 1) || (headLocation.y < 0) || (headLocation.y > boardSize.y - 1))
		{
			setToDie();
			return;
		}

		int playerNumber = findSnakeVector();

		for (int g = 0; g < snakes.at(playerNumber)->size(); g++)
		{
			Snake* collisionSnake = snakes.at(playerNumber)->at(g);

			int collisionType = collisionSnake->bodyPartAtLocation(headLocation);
			//head on head collision
			if ((collisionType == 2) && (this != collisionSnake))
			{
				int collisionSnakeLength = collisionSnake->getBodyLength();
				if (getBodyLength() < collisionSnakeLength)
				{
					setToDie();
					return;
				}
				else
				{
					//collision snake lost
					collisionSnake->setToDie();

					//if they are both the same size they both die
					if (getBodyLength() == collisionSnakeLength)
					{
						setToDie();
					}
					return;
				}
			}
			//head to body collision
			else if (collisionType == 1)
			{
				setToDie();
				return;
			}
			//self collision
			else if (collisionType == 2)
			{
				for (unsigned int i = 1; i < body.size(); i++)
				{
					if ((headLocation.x == body.at(i).x) && (headLocation.y == body.at(i).y))
					{
						setToDie();
						return;
					}
				}
			}
		}
	}


	bool isEndOfTail(int2d location)
	{
		int2d endOfTail = body.at(body.size() - 1);
		return ((endOfTail.x == location.x) && (endOfTail.y == location.y));
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

};



////////////////////
//Functions
////////////////////
double blend2Numbers(double n1, double n2)
{
	return 1 - (1 - n1) * (1 - n2);
}

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
				if (snakes.size() > 0)
				{
					for (unsigned int i = 0; i < snakes.at(0)->size(); i++)
					{
						SetConsoleTextAttribute(hConsole, 2);
						int snakeBodyPart = snakes.at(0)->at(i)->bodyPartAtLocation({ x,y });
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

void createRandomSnakes(int count)
{
	Snake* newSnakes[maxPlayers];
	//random_shuffle(std::begin(test), std::end(test));
	int newSpawn;
	int2d newSpawnPosition;
	bool spotTaken;

	//switch up the spawn positions
	random_shuffle(std::begin(spawnPositions), std::end(spawnPositions));
	for (int i = 0; i < count; i++)
	{
		//create snakes in each spawn position on the map
		newSnakes[i] = new Snake(spawnPositions[i]);
		//make a new vector for each snake
		snakes.push_back(new vector<Snake*>);
		//load in the master for each snake vector
		snakes.at(i)->push_back(newSnakes[i]);
	}

	//add the rest of the snakes to each snake vector
	for (int i = 0; i < count; i++)
	{
		random_shuffle(std::begin(newSnakes), std::begin(newSnakes)+count);
		for (int f = 0; f < count; f++)
		{
			if (snakes.at(i)->at(0) != newSnakes[f])
			{
				snakes.at(i)->push_back(newSnakes[f]);
			}
		}
	}
}

void clearTheBoard()
{
	//clear all of the food
	for (int i = 0; i < foodVector.size(); i++)
	{
		delete foodVector.at(i);
	} 
	foodVector.clear();

	//clear all of the snakes
	for (int i = 0; i < snakes.size(); i++)
	{
		delete snakes.at(i)->at(0);
		snakes.at(i)->clear();

		delete snakes.at(i);
	}
	snakes.clear();
}


bool isSpotTaken(int2d location)
{
	if(snakes.size()>0)
		for (unsigned int i = 0; i < snakes.size(); i++)
		{
			if (snakes.at(0)->at(i)->bodyPartAtLocation(location))
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

/*
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
*/

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


void deathUpdate()
{
	if (snakes.size() > 0)
		for (unsigned int i = 0; i < snakes.size(); i++)
		{
			if (snakes.at(i)->at(0)->killUpdate())
				i--;
		}
}


void eatFoodUpdate()
{
	if(snakes.size()>0)
	for (unsigned int i = 0; i < snakes.at(0)->size(); i++)
	{
		snakes.at(0)->at(i)->eatFood();
	}
}


void updateCollisions()
{
	if (snakes.size() > 0)
		for (unsigned int i = 0; i < snakes.at(0)->size(); i++)
		{
			snakes.at(0)->at(i)->collisionUpdate();
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