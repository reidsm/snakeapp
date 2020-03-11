/**
	TODO:


**/

#include "NN.h"
#include "Snake.h"
#include <cstdlib>
#include <functional>
#include <random>


//our snakes NN
vector<unsigned> snakeQNetTopology;
Net* snakeQNet;
vector<double> inputVals;
double future = 0.95;
double risk = 1.00;
double riskDecay = 0.9999;


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
	//9 special bits for each player
	inputNodeCount += 9 * 8;

	//for testing we will try out 1 hidden layer with the same amount of nodes as the input
	int hiddenLayer1NodeCount = inputNodeCount;

	//there are 3 possiable moves
	int outputLayerNodeCount = 3;

	snakeQNetTopology.push_back(inputNodeCount);
	snakeQNetTopology.push_back(hiddenLayer1NodeCount);
	snakeQNetTopology.push_back(outputLayerNodeCount);
	snakeQNet = new Net(snakeQNetTopology);
}


void loadBoardStateIntoInputNodes(int player)
{
	inputVals.clear();

	//loop through all of the board locations
	int2d cBL;
	for (cBL.y = 0; cBL.y < boardSize.y; cBL.y++)
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
				for (unsigned int i = 0; i < snakes.at(player)->size(); i++)
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
					int snakePart = snakes.at(player)->at(i)->bodyPartAtLocation(cBL);

					if (snakePart == 1)
					{
						int2d bodyDirection = snakes.at(player)->at(i)->bodyDirection(cBL);
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
						if (snakes.at(player)->at(i)->isEndOfTail(cBL))
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

	////////
	//Special Info
	////////
	//1bit for each player		if their snake will grow next move
	unsigned int f;
	for (f = 0; f < snakes.at(player)->size(); f++)
	{
		int currentFood = snakes.at(player)->at(f)->getInsideFood();
		if (currentFood > 0)
			inputVals.push_back(1);
		else
			inputVals.push_back(0);
	}
	for (; f < maxPlayers; f++)
		inputVals.push_back(0);

	//6bits for each player		the size of each player's snake
	for (f = 0; f < snakes.at(player)->size(); f++)
	{
		int currentSize = snakes.at(player)->at(f)->getBodyLength();
		double sizeB32 = (currentSize & 32) / 32;
		double sizeB16 = (currentSize & 16) / 16;
		double sizeB08 = (currentSize & 8) / 8;
		double sizeB04 = (currentSize & 4) / 4;
		double sizeB02 = (currentSize & 2) / 2;
		double sizeB01 = (currentSize & 1) / 1;

		inputVals.push_back(sizeB32);
		inputVals.push_back(sizeB16);
		inputVals.push_back(sizeB08);
		inputVals.push_back(sizeB04);
		inputVals.push_back(sizeB02);
		inputVals.push_back(sizeB01);
	}
	for (; f < maxPlayers; f++)
	{
		inputVals.push_back(0);
		inputVals.push_back(0);
		inputVals.push_back(0);
		inputVals.push_back(0);
		inputVals.push_back(0);
		inputVals.push_back(0);
	}

	//1 bit for each player for their hunger level
	for (f = 0; f < snakes.at(player)->size(); f++)
	{
		double curHungerPercent = ((double)snakes.at(player)->at(0)->getHunger()) / ((double)maxHunger);
		inputVals.push_back(curHungerPercent);
	}
	for (; f < maxPlayers; f++)
		inputVals.push_back(0);

	//1 bit for each player		if their snake exists
	for (f = 0; f < snakes.at(player)->size(); f++)
		inputVals.push_back(1);
	for (; f < maxPlayers; f++)
		inputVals.push_back(0);
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
				current[i] = inputVals.at((y * 11 * 7 + x * 7 + i));
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
	for (int i = 0; i < 10; i++)
		cout << endl;
	inputVals.clear();
}


/*class Rand_double
{
public:
	Rand_double(double low, double high)
		:r(std::bind(std::uniform_real_distribution<>(low, high), std::default_random_engine())) {}

	double operator()() { return r(); }

private:
	std::function<double()> r;
};*/

double rd()
{
	int temp = rand() % 1000;
	return ((double)temp) / 1000.0;

}


////////////////////
//main
////////////////////
int main()
{
	srand((unsigned)time(NULL));
	//Rand_double rd{ 0.0,1.0 };


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
	createRandomSnakes(startingPlayerCount);

	for (int i = 0; i < startingPlayerCount; i++)
		randomlyCreateFood(100);

	//setup the NN
	setUpSnakeNN();


	while (1)
	{
		vector<double> outputVals;

		//only draw the board if the right key is down
		//if (GetKeyState(VK_RIGHT) & 0x8000)
		drawBoard();


		//check if the match is over
		if ((snakes.size() <= 1))
		{
			clearTheBoard();
			createRandomSnakes(startingPlayerCount);

			for (int i = 0; i < startingPlayerCount; i++)
				randomlyCreateFood(100);
		}

		cout << "Risk: " << risk << endl;
		//start of input
		for (int i = 0; i < snakes.size(); i++)
		{
			//load the board state into the QLearning NN
			loadBoardStateIntoInputNodes(i);
			//run the QLearning NN to get the rating of the 3 possible moves
			snakeQNet->feedForward(inputVals);
			//the rating of the 3 possible moves
			snakeQNet->getResults(outputVals);

			//outputVals								this frames q values
			//snakes.at(i)->at(0)->lastOutputValues		last frames q values
			//snakes.at(i)->at(0)->getRewards()
			//blend2Numbers

			int bestNewState;
			if ((outputVals.at(0) > outputVals.at(1)) && (outputVals.at(0) > outputVals.at(2)))
				//move left is best
				bestNewState = 0;
			else
			{
				if (outputVals.at(1) > outputVals.at(2))
					//move straight is best
					bestNewState = 1;
				else
					//move right is best
					bestNewState = 2;
			}

			double oldStateQValue = future * blend2Numbers(outputVals.at(bestNewState), snakes.at(i)->at(0)->getRewards());
			vector <double> qMapReward;
			//set the output nodes to get ready for back propagation
			for (int f = 0; f < 3; f++)
			{
				if (snakes.at(i)->at(0)->lastMove + 1 == f)
				{
					snakeQNet->m_layers.back().at(f).setOutputVal(snakes.at(i)->at(0)->lastOutputValues.at(f));
					qMapReward.push_back(oldStateQValue);
				}
				else
				{
					snakeQNet->m_layers.back().at(f).setOutputVal(0.0);
					qMapReward.push_back(0.0);
				}
			}

			//Update the NN QList by back propagating 
			snakeQNet->backProp(qMapReward);








			//save the 3 QLearning NN output values to be used in back propagation later
			snakes.at(i)->at(0)->lastOutputValues.clear();
			for (int f = 0; f < outputVals.size(); f++)
				snakes.at(i)->at(0)->lastOutputValues.push_back(outputVals.at(f));



			int2d moveDirection;
			int moveSelected;


			cout << "left\t\t" << outputVals.at(0) << endl;
			cout << "straight\t" << outputVals.at(1) << endl;
			cout << "right\t\t" << outputVals.at(2) << endl;

			//risk is used to explore different paths
			//it can lead to a smarter NN
			//it is best to take risks at the start and stop taking them later
			//that is were riskDecay comes in
			if (rd() <= risk)
			{
				//add up all positive outputs
				double totalOutput = 0.0;
				totalOutput += outputVals.at(0);
				totalOutput += outputVals.at(1);
				totalOutput += outputVals.at(2);
				totalOutput += 3.0;
				double randomPick = rd() * totalOutput;

				//randomize our moves
				if (randomPick < outputVals.at(0) + 1.0)
					//move left selected
					moveSelected = -1;
				else if (randomPick < outputVals.at(0) + outputVals.at(1) + 2.0)
					//move straight selected
					moveSelected = 0;
				else
					//move right selected
					moveSelected = 1;
			}
			else
			{
				//safer move that is stronger
				if ((outputVals.at(0) > outputVals.at(1)) && (outputVals.at(0) > outputVals.at(2)))
					//0 is bigger
					//move left selected
					moveSelected = -1;
				else
				{
					if (outputVals.at(1) > outputVals.at(2))
						//1 is bigger
						//move straight selected
						moveSelected = 0;
					else
						//2 is bigger
						//move right selected
						moveSelected = 1;
				}
			}

			//convert [-1:left],[0:straight],[1:right] into right,up,left,down
			moveDirection = snakes.at(i)->at(0)->rotationMoveToDirection(moveSelected);

			//riskier move that can lead to more learning
			risk *= riskDecay;

			//set the move the direction given by the QLearning NN
			snakes.at(i)->at(0)->nextMoveDirection = moveDirection;
			//keep track of the QLearning NN Output Move we just did
			snakes.at(i)->at(0)->lastMove = moveSelected;
		}
		//end of input

		//move all of the snakes
		for (int i = 0; i < snakes.size(); i++)
		{
			snakes.at(i)->at(0)->move();
		}

		//convertTest();

		//showVectorVals("Outputs:", outputVals);

		//system("PAUSE");




		eatFoodUpdate();
		removeAllEatenFood();
		updateCollisions();

		//snake's QLearning NN can not be punished after they die
		//so a special case check has to happen here before they die
		for (int i = 0; i < snakes.size(); i++)
		{
			vector<double> qMapPunishment;


			if (snakes.at(i)->at(0)->isAboutToDie())
			{
				//this snake is about to die we must punish its QLearning NN for making this move

				//set the output nodes to get ready for back propagation
				for (int f = 0; f < 3; f++)
				{
					if (snakes.at(i)->at(0)->lastMove + 1 == f)
					{
						snakeQNet->m_layers.back().at(f).setOutputVal(snakes.at(i)->at(0)->lastOutputValues.at(f));
						qMapPunishment.push_back(-1.0);
					}
					else
					{
						snakeQNet->m_layers.back().at(f).setOutputVal(0.0);
						qMapPunishment.push_back(0.0);
					}
				}

				//Update the NN QList by back propagating 
				snakeQNet->backProp(qMapPunishment);
			}
		}

		deathUpdate();
		randomlyCreateFood(15);
	}

	system("PAUSE");
}
