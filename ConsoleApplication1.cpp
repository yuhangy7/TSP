// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <iostream>
//
//int main()
//{
//    std::cout << "Hello World!\n";
//}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
//////////////////////////////////////////////////////////////////////////
// Traveling Salesman Problem.
// Note: this is a C++ program: compile it with g++
//
// Starting city is assumed to be city 0.
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <chrono>


const int MAXCITIES = 20;


int** Dist;			// Dist[i][j] =  distance from  i to j

class Path
{
public:
    int numVisited;	// Number of cities in the partial path
    int length;			// Current length of partial path
    int city[MAXCITIES];
    int numCities;

    // Array city[] is a permutation of all cities.
    // city[0]..city[numVisited-1] is the current partial path;
    // city[numVisited]..city[numCities-1] are the cities not yet in the path 

    Path(int n) : length(0), numVisited(1), numCities(n)			// Initialize a Path with city 0 as visited
    {
        for (int i = 0; i < numCities; i++)
            city[i] = i;
    }
    Path(const Path& other) : numVisited(other.numVisited), length(other.length), numCities(other.numCities)
    {
        for (int i = 0; i < numCities; i++)
            city[i] = other.city[i];
    }

    void AddCity(int i)		// Extends path by adding the ith city not yet visited. 
    {
        assert(numVisited <= i && i < numCities);
        // In order to keep the path as a permutation of all cities
        // we "add" a city on the path by swapping 
        std::swap(city[i], city[numVisited]);

        // visit city[numVisited]
        length += Dist[city[numVisited - 1]][city[numVisited]];
        numVisited++;
    }

    void Print()
    {
        for (int i = 0; i < numVisited; i++)
            std::cout << ' ' << city[i];
        std::cout << "; length = " << length << std::endl;
    }
};


class Queue
{
    static const int MAXQSIZE = 100000;
    int size;
    Path* path[MAXQSIZE]; // keeps pointers to objects of type Path

public:
    Queue() : size(0) {};
    void Put(Path* P);
    Path* Get();
    int isEmpty() { return size == 0; };
};

void Queue::Put(Path* P)
{
    assert(size < MAXQSIZE);
    path[size++] = P;
}

Path* Queue::Get()
{
    if (isEmpty())
        return NULL;

    // to decrease the size of the queue, move the last element
    Path* p = path[0];
    path[0] = path[--size];

    return p;
}

///////////////////////////////////////////////////////////////////////////


void Fill_Dist(int numCities)
{
    std::cout << "Distance matrix:\n";
    Dist = new int* [numCities];
    for (int i = 0; i < numCities; i++) {
        Dist[i] = new int[numCities];
        for (int j = 0; j < numCities; j++) {
            // Not necessary, but let's make Dist simmetric:
            if (i == j)
                Dist[i][j] = 0;
            else if (j < i)
                Dist[i][j] = Dist[j][i];
            else
                Dist[i][j] = rand() % 1000;
            std::cout << Dist[i][j] << '\t';
        }
        std::cout << std::endl << std::endl;
    }
}



struct Params
{
    int numCities;
    Queue* Q;
    Path* shortestPath;
};

void* tsp(void* arg)
{
    Params* params = (Params*)arg;
    int numCities = params->numCities;
    Queue* Q = params->Q;
    Path* shortestPath = params->shortestPath;

    // For every partial path in the queue, extend the partial path with all
    // of the unvisited cities (one by one) and add these newly formed partial
    // paths back to the queue
    while (!Q->isEmpty())
    {
        Path* p = Q->Get();

        // For each city not yet visited, extend the path:
        for (int i = p->numVisited; i < numCities; i++)
        {
            Path* p1 = new Path(*p);	// initially p1 is a clone of p
            p1->AddCity(i);

            // decide what to do with new path
            if (p1->numVisited == numCities) // If we visited them all
            {
                // Add last hop to return to origin:
                p1->length += Dist[p1->city[numCities - 1]][0];

                // update shortestPath, if p1 is better
                if (p1->length < shortestPath->length)
                    *shortestPath = *p1;

                delete p1;
            }
            else
            {
                if (p1->length > shortestPath->length)
                {
                    // This path is bad; just discard it
                    delete p1;
                }
                else
                {
                    Q->Put(p1);
                }
            }
        } // end for
        delete p;			// p is not needed any more
    }
    return NULL;
}

void* tsp_parallel(void* arg)
{


    return NULL;

}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " num_cities\n";
        exit(-1);
    }
    int NumCities = atoi(argv[1]);
    assert(NumCities <= MAXCITIES);

    Fill_Dist(NumCities);			// initialize Distance matrix

    
    // Config sequential TSP funtion.
    Path* P = new Path(NumCities);
    Queue Q;
    Q.Put(P);			// initialize Q with one zero-length path

    Path Shortest(NumCities);
    Shortest.length = INT_MAX;    // The initial Shortest path must be bad

    Params params = { NumCities, &Q, &Shortest };

    // Run TSP sequentially.
    auto startTime = std::chrono::steady_clock::now();
    tsp(&params);
    auto endTime = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Shortest path:";
    Shortest.Print();


    std::cout << "TSP solution took " << ms.count() << " ms\n";

    // Config sequential TSP funtion.
    Path* P_parallel = new Path(NumCities);
    // After the previous sequential manipulation, the Q is always empty, so we can reused. Creating a new queue will cause stack overflow due to the size of the queue is very big.
    Q.Put(P_parallel);			// initialize Q with one zero-length path

    Path Shortest_parallel(NumCities);
    Shortest_parallel.length = INT_MAX;    // The initial Shortest path must be bad

    Params params_parallel = { NumCities, &Q, &Shortest_parallel };


    // Run TSP parallelly.
    auto startTimeParallel = std::chrono::steady_clock::now();
    tsp_parallel(&params_parallel);
    auto endTimeParallel = std::chrono::steady_clock::now();
    auto msParallel = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeParallel - startTimeParallel);

    std::cout << "Shortest path (parallel):";
    Shortest_parallel.Print();


    std::cout << "TSP solution (parallel) took " << msParallel.count() << " ms\n";
    return 0;
}


