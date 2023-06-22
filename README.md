NAME OF THE GAME: MAZE

LANGUAGE: C++

CREATE EXE: g++ -std=c++2a main.cpp -o MAZE -lncurses

MODES OF GAME:
	PLAY ALONE
	PLAY VS PC
	AUTO PLAY

CREATING MAZE ALGORITHM: PRIM'S ALGORITHM

I have chosen this algorithm because it seemed easy to implement, 
I liked the animation ( how it creates a maze that it looks like a tree),
And as soon as I saw this algorithm I had an idea how to combine it with my program.

How it works:
It chooses cell and marks it as a path.
Adds nearby walls to the list of walls.
In while loop (while there are walls in the list):
	Chooses random wall, checks if this wall can become a path.
	if yes: adds this wall to path list, and adds nearby walls to the list.
	Removes this wall from the list.


SOLVING MAZE ALGORITHM: MODIFIED RECURSIVE ALGORITHM

I have chosen this algorithm because it was the only solving maze algorithm,
Which didn't use "open start and end" maze. 

How it works:
At the start of the function it checks if it is at the end of the maze, 
If yes, it returns true and the algorithm ends.
Then it checks if algorithm reached this cell, or not.
Then it checks if algorithm can go left/right/up/down and evokes itself. (Recursion)


AUTHOR: JAKUB MIKULA