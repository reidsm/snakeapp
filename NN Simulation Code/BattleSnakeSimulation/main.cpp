/**
	TODO:


**/

//#include <iostream>
//#include <vector>
//#include <ctime>
//#include <Windows.h>
#include "NN.h"
#include "Snake.h"

//using namespace std;



//our snakes NN
vector<unsigned> snakeQNetTopology;
Net* snakeQNet;//(snakeQNetTopology);


////////////////////
//NODES:
////////////////////
//What's on the board
//0000000	Blank
//xxxx%%%	Player Number
//xx00xxx	Body Part Moving Down
//xx01xxx	Body Part Moving Up
//xx10xxx	Body Part Moving Left
//xx11xxx	Body Part Moving Right
//11xxxxx	Body
//0100xxx	Head
//10xxxxx	Tail
//0011000	Food
//
//Special Info
//1bit for each player		if their snake will grow next move
//6bits for each player		the size of each player's snake
//1 bit for each player		if their snake exists
void setUpSnakeNN()
{
	cout << "Creating the Neural Network" << endl;

	int inputNodeCount = 0;
	//7 bits for each tile * 11 * 11 for the whole board
	inputNodeCount += 7 * 11 * 11;
	//8 special bits for each player
	inputNodeCount += 8 * 8;

	//for testing we will try out 1 hidden layer with the same amount of nodes as the input
	int hiddenLayer1NodeCount = inputNodeCount;

	//there are 3 possiable moves
	int outputLayerNodeCount = 3;

	snakeQNetTopology.push_back(inputNodeCount);
	snakeQNetTopology.push_back(hiddenLayer1NodeCount);
	snakeQNetTopology.push_back(outputLayerNodeCount);
	snakeQNet = new Net(snakeQNetTopology);
}
vector<double> inputVals;
void loadBoardStateIntoInputNodes(int player)
{
	//vector<double> inputVals;
	
	//loop through all of the board locations
	int2d cBL;
	for (cBL.y=0; cBL.y < boardSize.y; cBL.y++)
	{
		for (cBL.x = 0; cBL.x < boardSize.x; cBL.x++)
		{
			bool inputPushed = false;
			//check for food
			for (unsigned int i = 0; i < foodVector.size(); i++)
			{
				if (foodVector.at(i)->isAtPosition(cBL))
				{
					//0011000	Food
					inputVals.push_back(0);
					inputVals.push_back(0);
					inputVals.push_back(1);
					inputVals.push_back(1);
					inputVals.push_back(0);
					inputVals.push_back(0);
					inputVals.push_back(0);
					inputPushed = true;
					break;
				}
			}

			if (!inputPushed)
			{
				//check for snakes
				for (unsigned int i = 0; i < snakes.size(); i++)
				{
					//get the player number of the snake
					//xxxx%%%	Player Number
					double player4bit = i / 4;
					double player2bit = (i & 2) / 2;
					double player1bit = i % 2;

					//0100xxx	Head
					//10xxxxx	Tail
					//00		Food
					//11		Body Part / Empty
					double special2b;
					double special1b;

					//snake body OR tail
					double bodyMoving2b;
					double bodyMoving1b;

					//get the body part of the snake at this board location
					int snakePart = snakes.at(i)->bodyPartAtLocation(cBL);

					if (snakePart == 1)
					{
						int2d bodyDirection = snakes.at(i)->bodyDirection(cBL);
						//xx00xxx	Body Moving Down
						//xx01xxx	Body Moving Up
						//xx10xxx	Body Moving Left
						//xx11xxx	Body Moving Right
						if (bodyDirection.x == 1)
						{
							//xx11xxx	Body Moving Right
							bodyMoving2b = 1;
							bodyMoving1b = 1;
						}
						else if (bodyDirection.x == -1)
						{
							//xx10xxx	Body Moving Left
							bodyMoving2b = 1;
							bodyMoving1b = 0;
						}
						else if (bodyDirection.y == 1)
						{
							//xx00xxx	Body Moving Down
							bodyMoving2b = 0;
							bodyMoving1b = 0;
						}
						else
						{
							//xx01xxx	Body Moving Up
							bodyMoving2b = 0;
							bodyMoving1b = 1;
						}

						//determine if this tile holds a tail or a normal body part
						if (snakes.at(i)->isEndOfTail(cBL))
						{
							//10xxxxx	Tail
							special2b = 1;
							special1b = 0;
						}
						else
						{
							//00		Body Part
							special2b = 1;
							special1b = 1;
						}
					}
					else if (snakePart == 2)
					{
						//0100xxx	Head
						special2b = 0;
						special1b = 1;
						bodyMoving2b = 0;
						bodyMoving1b = 0;
					}

					//a snake was found
					if (snakePart > 0)
					{
						inputVals.push_back(special2b);
						inputVals.push_back(special1b);
						inputVals.push_back(bodyMoving2b);
						inputVals.push_back(bodyMoving1b);
						inputVals.push_back(player4bit);
						inputVals.push_back(player2bit);
						inputVals.push_back(player1bit);
						inputPushed = true;
					}
				}
			}

			if (!inputPushed)
			{
				//nothing is found so the tile must be blank
				//0000000	Blank
				inputVals.push_back(0);
				inputVals.push_back(0);
				inputVals.push_back(0);
				inputVals.push_back(0);
				inputVals.push_back(0);
				inputVals.push_back(0);
				inputVals.push_back(0);
			}
		}
	}
}






//0000000	Blank
//xxxx%%%	Player Number
//xx00xxx	Body Part Moving Down
//xx01xxx	Body Part Moving Up
//xx10xxx	Body Part Moving Left
//xx11xxx	Body Part Moving Right
//11xxxxx	Body
//0100xxx	Head
//10xxxxx	Tail
//0011000	Food
void convertTest()
{
	system("CLS");
	string convert[11][11];
	double current[7];

	for (int y = 0; y < 11; y++)
	{
		for (int x = 0; x < 11; x++)
		{
			for (int i = 0; i < 7; i++)
			{
				current[i] = inputVals.at(y*11*7 + x * 7 + i);
			}

			if ((current[0] == 0) && (current[1] == 0) && (current[2] == 1) && (current[3] == 1))
				cout << "fod";
			else if (current[0] + current[1] + current[2] + current[3] + current[4] + current[5] + current[6] == 0)
				cout << " . ";

			else
			{
				if ((current[0] == 1) && (current[1] == 1))
					cout << "b";

				if ((current[0] == 1) && (current[1] == 0))
					cout << "t";

				if ((current[0] == 0) && (current[1] == 1))
					cout << "hh";
				else {
					if ((current[2] == 0) && (current[3] == 0))
						cout << "v";
					if ((current[2] == 0) && (current[3] == 1))
						cout << "^";
					if ((current[2] == 1) && (current[3] == 0))
						cout << "<";
					if ((current[2] == 1) && (current[3] == 1))
						cout << ">";
				}

				cout << current[4] * 4 + current[5] * 2 + current[6] * 1;
			};


		}
		cout << endl;
	}
	for(int i=0;i<10;i++)
		cout << endl;
	inputVals.clear();
}









/*vector<double> inputVals;
vector<double> resultVals;
vector<double> targetVals;
snakeQNet.feedForward(inputVals);
snakeQNet.getResults(resultVals);
snakeQNet.backProp(targetVals);*/


////////////////////
//main
////////////////////
int main()
{
	srand((unsigned)time(NULL));
	boardSize.x = 11;
	boardSize.y = 11;

	/*
	vector<unsigned> snakeQNetTopology;
	snakeQNetTopology.push_back(1);
	snakeQNetTopology.push_back(1);
	snakeQNetTopology.push_back(1);
	snakeQNetTopology.push_back(2);
	Net snakeQNet(snakeQNetTopology);
	vector<double> inputVals;
	vector<double> resultVals;
	vector<double> targetVals;
	snakeQNet.feedForward(inputVals);
	snakeQNet.getResults(resultVals);
	snakeQNet.backProp(targetVals);
	*/

	
	SetSpawn();
	createRandomSnakes();

	for (int i = 0; i < players; i++)
		randomlyCreateFood(100);

	drawBoard();
	setUpSnakeNN();
	

	while (1)
	{
		//drawBoard();
		loadBoardStateIntoInputNodes(0);
		convertTest();
		system("PAUSE");
		updatePlayer();
		updateAI();
		eatFoodUpdate();
		removeAllEatenFood();
		updateCollisions();
		randomlyCreateFood(15);
	}

	system("PAUSE");
}
