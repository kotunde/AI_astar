#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <set>

typedef struct nodes
{
	int** mtrx;
	struct nodes *parent; 
	int g;
	int h;
	int f;
}nodes;

struct nodesCompare
{
    bool operator() (const nodes& lhs, const nodes& rhs) const
	{
        if(lhs.f == rhs.f)
		{
			return lhs.g > rhs.g;
		}
		else
		{
			return lhs.f < rhs.f;
		}
    }
};

int** generateRandInitMtrx(int, int);
void printMtrx(int **, int);
bool isGoalState(int**, int);
bool traverser(int, int, int, int, int);
int h(int**, int , int);
int countTilesInWrongPlace(int **, int);
int calcManhattanDist(int **, int);
int** copyMtrx(int**,int);
nodes* findInOPEN(nodes,int);
nodes* findInCLOSED(nodes,int);
int compareMtrx(int**,int**,int);
int** allocateMtrx(int);

int x[] = {1, 0, -1, 0};
int y[] = {0, -1, 0, 1};

std::set<nodes,nodesCompare> OPEN;
std::set<nodes,nodesCompare> CLOSED;

int main (int argc, char *argv[])
{

	char * file = "";
	int solseq = 0; //solution sequence
	int pcost = 0;	//cost of path
	int nvisited = 0; //number of visited nodes
	int heuristic = 0; //type of heuristic
	int i = 1;
	int initParams = 0;//for n and m
	int n=-1;
	int m =-1;
	int** puzzle;
	// check options
	while (i< argc)
	{
		if( (strcmp(argv[i],"-input")==0) && (strcmp(file,"") == 0))
		{
			++i;
			file = argv[i];
		}
		else if ((strcmp(argv[i],"-solseq")==0) && !solseq)
		{
			solseq = !solseq;
		}
		else if ((strcmp(argv[i],"-pcost")==0) && !pcost)
		{
			pcost = !pcost;
		}
		else if ((strcmp(argv[i],"-nvisited")==0) && !nvisited)
		{
			nvisited = !nvisited;
		}
		else if ((strcmp(argv[i],"-h")==0) && !heuristic)
		{
			++i;
			heuristic = atoi(argv[i]);
		}
		else if ((strcmp(argv[i],"-rand")==0) && !initParams)
		{
			initParams = !initParams;
			++i;
			n = atoi(argv[i]);
			++i;
			m = atoi(argv[i]);
		}
		++i;
	}


	if (initParams)
	{
		printf("Kezdeti random allapot generalasa a parameterek alapjan...\n");
		puzzle = generateRandInitMtrx(n,m);
	}
	else
	// if no -rand parameters nor input file given, read init state from stdin
	if (strcmp(file,"") == 0 )//&& !initParams)
	{
		printf("Kezdeti allapot beolvasasa a standard bemenetrol...\n");
		printf("A tabla merete, n: ");
		scanf("%i",&n);
		puzzle = allocateMtrx(n);
		printf("A tabla (ures csepe = -1):\n");
		for (int i=0; i<n; ++i)
		{
			for (int j=0; j<n; ++j)
			{
				scanf("%i",&puzzle[i][j]);
			}
		}
	}
	else if (strcmp(file,"") != 0)
	{
		printf("Kezdeti allapot beolvasasa %s filebol...\n",file);
		FILE *fin = fopen( file, "r" );
		fscanf(fin,"%i",&n);
		puzzle = allocateMtrx(n);
		for (int i=0; i<n; ++i)
		{
			for (int j=0; j<n; ++j)
			{
				fscanf(fin,"%i",&puzzle[i][j]);
			}
		}
	}
	else
	{
		//error
	}

	if (!heuristic)
	{
		printf("Heurisztika tipusa nincs meghatarozva!\n");
		return -1;
	}

	printf("Kezdeti allapot: \n");
	printMtrx(puzzle,n);
	//printf("Is goal state: %i\n",isGoalState(puzzle,n));
	//printf("Number of tiles in wrong place: %i\n",countTilesInWrongPlace(puzzle,n));
	//printf("Manhattan distance: %i\n",calcManhattanDist(puzzle,n));
	
	// create start node and add to OPEN
	nodes node;
	node.mtrx = puzzle;
	node.parent = NULL;
	node.g = 0;
	node.h = h(node.mtrx, n, heuristic);
	node.f = node.g + node.h;
	OPEN.insert(node);
	//call THE Function
	printf("Megoldas...\n");
	bool solution = traverser(n, solseq, pcost, nvisited, heuristic);
	return 0;
}



bool traverser(int n, int solseq, int pcost,int nvisited, int heuristic)
{
	bool solution = false;
	int numberOfVisitedNodes = 0;
	while(OPEN.size())
	{
		// get first element of set (element with the least f cost (if more than one, with the greatest g cost))
		std::set<nodes>::iterator it=OPEN.begin();
		//printf("element's f value: %i; g value: %i\n",(*it).f,(*it).g );
		nodes node = *it;
		if (solseq)
		{
			printMtrx(node.mtrx,n);
			printf("h : %i; g : %i\n", node.h, node.g );
		}
		// add node to CLOSED
		CLOSED.insert(node);
		//number of nodes taken out of OPEN to open and process
		++numberOfVisitedNodes;
		OPEN.erase(node);
		// check if node's state is solution
		if (isGoalState(node.mtrx,n))
		{
			printf("Megoldas megtalalva!\n");
			if (pcost)
			{
				printf("Megoldas koltsege: %i\n", node.g);
			}
			if (nvisited)
			{ 
				printf("Meglatogatott csomopontok szama: %i\n",numberOfVisitedNodes);
			}
			return true;
		}
		else
		{
			//find postion of empty cell
			int emptyRow, emptyCol;
			bool emptyPosFound = false;
			for (int i=0; i<n && !emptyPosFound; ++i)
			{
				for (int j=0; j<n && !emptyPosFound; ++j)
				{
					if (node.mtrx[i][j] == -1)
					{
						emptyRow = i;
						emptyCol = j;
						emptyPosFound = true;
					}
				}
			}
			//printf("Position of empty cell: %i %i\n",emptyRow,emptyCol);
			// open children of current node
			for (int i=0; i<4; ++i)
			{
				//calculate new possible position of the empty tile
				int newPosRow = emptyRow+x[i];
				int newPosCol = emptyCol+y[i];
				//if the position is aacceptable (inside of the table)
				if (newPosRow>=0 && newPosRow<n && newPosCol>=0 && newPosCol<n)
				{
					//create child node
					nodes childNode;
					//allocation and copy of the parent mtrx
					childNode.mtrx = copyMtrx(node.mtrx,n);
					//move empty tile in child's state
					childNode.mtrx[emptyRow][emptyCol] = childNode.mtrx[newPosRow][newPosCol];
					childNode.mtrx[newPosRow][newPosCol] = -1;
					//set fields of child
					childNode.parent = &node;
					childNode.g = (*childNode.parent).g + 1;
					childNode.h = h(childNode.mtrx, n, heuristic);
					childNode.f = childNode.g + childNode.h;
					//check if child's state already in OPEN
					nodes *X;
					if (( X = findInOPEN(childNode,n)) && (X != nullptr))
					{
						nodes xNode = *X;
						if (xNode.f <= childNode.f )
						{
							continue;
						}
						else
						{
							OPEN.erase(xNode);
						}
					}

					//check if child's state already in CLOSED
					if (( X = findInCLOSED(childNode,n)) != nullptr)
					{
						nodes xNode = *X;
						if (xNode.f <= childNode.f )
						{
							continue;
						}
						else
						{
							OPEN.erase(xNode);
						}
					}
					OPEN.insert(childNode);			
				}
			}			
		}
	}
	return true;
}


nodes* findInOPEN(nodes node,int n)
{
	// Creating a iterator pointing to start of set
	std::set<nodes>::iterator it = OPEN.begin();
	 
	// Iterate till the end of set
	while (it != OPEN.end())
	{
		//compare the state of the two node
		if(compareMtrx(node.mtrx, (*it).mtrx, n) == 0)
		{
			//return (nodes*)&it;
			return  (nodes*)&(*it);
		}
		//Increment the iterator
		it++;
	}
	return nullptr;
}
nodes* findInCLOSED(nodes node,int n)
{
	// Creating a iterator pointing to start of set
	std::set<nodes>::iterator it = CLOSED.begin();
	 
	// Iterate till the end of set
	while (it != CLOSED.end())
	{
		//compare the state of the two node
		if(compareMtrx(node.mtrx, (*it).mtrx, n) == 0)
		{
			//return (nodes*)&it;
			return  (nodes*)&(*it);
		}
		//Increment the iterator
		it++;
	}
	return nullptr;
}

int h(int ** mtrx, int n, int heuristic)
{
	if (heuristic == 1)
	{
		return countTilesInWrongPlace(mtrx,n);
	}
	else if (heuristic == 2)
	{
		return calcManhattanDist(mtrx,n);
	}
	else
	{
		//TODO handle error
		return 0;
	}
}



int countTilesInWrongPlace(int ** mtrx, int n)
{
	int numberOfTilesInWrongPlace = 0;
	int value = 1;
	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			if ((mtrx[i][j] != value) && !((i == n-1) && (j == n-1)))
			{
				numberOfTilesInWrongPlace++;
			}
			++value;
		}
	}
	return numberOfTilesInWrongPlace;
}



int calcManhattanDist(int ** mtrx, int n)
{
	int distance = 0;
	int value = 0;

	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			//if there is a  tile on this position
			if(mtrx[i][j] != -1)
			{
				//the value at the current position
				value = mtrx[i][j];

				//the desired position of the tile
				int row_d = ((value-1) / n);
				int col_d = ((value-1) % n);

				//the actual position of the tile
				int row_a = i;
				int col_a = j;

				//manhattan distance
				int man_dist = abs(row_d - row_a) + abs(col_d - col_a);

				//printf("i:%i j:%i     row_d:%i col_d:%i     row_a:%i col_a: %i     man_dist:%i   val:%i\n", i,j,row_d,col_d,row_a,col_a,man_dist,mtrx[i][j]);

				//summing up the overall distance for this state
				distance += man_dist;
			}
		}
	}
	return distance;
}



int** generateRandInitMtrx(int n,int m)
{
	//allocation
	int **mtrx = (int**)malloc(n * sizeof(int*));
	if (mtrx == NULL)
	{
		printf("Allocation failed");
		return 0;
	}
	for (int i=0; i<n; ++i)	
	{
		mtrx[i] = (int*)malloc(n * sizeof(int));
		if (mtrx[i] == NULL)
		{
			printf("Allocation failed");
			return 0;
		}
	}

	//generate initial state
	int value = 1,i,j;
	for (i=0; i<n; ++i)
	{
		for (j=0; j<n; ++j)
		{
			mtrx[i][j] = value++;
		}
	}
	//set last element as empty (-1)
	mtrx[i-1][j-1] = -1;

	//save empty cell's position
	int emptyRow = i-1;
	int emptyCol = j-1;
	srand(time(NULL));

	//make m random shift
	for(i=0; i<m; ++i)
	{
		bool emptyCellMoved = false;
		while(!emptyCellMoved)
		{
			int index = rand()%4;
			int newPosRow = emptyRow+x[index];
			int newPosCol = emptyCol+y[index];
			if (newPosRow>=0 && newPosRow<n && newPosCol>=0 && newPosCol<n)
			{
				mtrx[emptyRow][emptyCol] = mtrx[newPosRow][newPosCol];
				mtrx[newPosRow][newPosCol] = -1;
				emptyRow = newPosRow;
				emptyCol = newPosCol;
				emptyCellMoved = true;
			}
		}
	}
	return mtrx;
}

int compareMtrx(int** mtrx1, int** mtrx2,int n)
{
	bool isEqual = 0;
	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			if (mtrx1[i][j] != mtrx2[i][j])
			{
				isEqual = -1;
				return isEqual;
			}
		}
	}
	return isEqual;
}

//copy content of mtrxFrom to the newly allocated mtrxTo
int** copyMtrx(int** mtrxFrom, int n)
{
	//allocation
	int **mtrxTo = (int**)malloc(n * sizeof(int*));
	if (mtrxTo == NULL)
	{
		printf("Allocation failed");
		return 0;
	}
	for (int i=0; i<n; ++i)	
	{
		mtrxTo[i] = (int*)malloc(n * sizeof(int));
		if (mtrxTo[i] == NULL)
		{
			printf("Allocation failed");
			return 0;
		}
	}
	//assignment
	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			mtrxTo[i][j] = mtrxFrom[i][j]; 
		}
	}
	return mtrxTo;
}

void printMtrx(int ** mtrx,int n)
{
	printf("- - - - - - - - - \n\n");
	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			printf("%2d ",mtrx[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int** allocateMtrx(int n)
{
	int **mtrx = (int**)malloc(n * sizeof(int*));
	if (mtrx == NULL)
	{
		printf("Allocation failed");
		return 0;
	}
	for (int i=0; i<n; ++i)	
	{
		mtrx[i] = (int*)malloc(n * sizeof(int));
		if (mtrx[i] == NULL)
		{
			printf("Allocation failed");
			return 0;
		}
	}
	return mtrx;
}

bool isGoalState(int ** mtrx, int n)
{
	bool goalState = true;
	int value = 1;
	for (int i=0; i<n; ++i)
	{
		for (int j=0; j<n; ++j)
		{
			//if mtrx element not equal value and it is not the last element
			
			if ((mtrx[i][j] != value) && !((i == n-1) && (j == n-1)))
			{
				goalState = false;
				return goalState;
			}
			++value;		
		}
	}
	return goalState;
}
