//
//  maze.cpp
//  maze
//
//  Created by Jakub Mikuła on 16/03/2021.
//

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <ncurses.h>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <list>
#include <algorithm>

#include <thread>
#include <chrono>

using namespace std;

int randGenerator(int, int);//random number generator
void setSizeOfTerminal(int, int);//if needed change size of terminal
int menu(vector<string>);

class cell{
    int x;
    int y;
    
public:
    cell();
    cell(int, int);
    
    bool operator==(const cell& a){
        if(x==a.x && y==a.y){
            return true;
        }
        return false;
    }
    bool operator!=(const cell& a){
        if(x!=a.x ||y!=a.y){
            return true;
        }
        return false;
    }
    friend class maze;
    friend class player;
    friend class computer;
};

cell::cell(){
    x=0;
    y=0;
}

cell::cell(int xGiven, int yGiven){
    x=xGiven;
    y=yGiven;
}

class player{
protected:
    char playersChar;
    cell here;
    
public:
    player();
    player(char);
    
    int getX(){return here.x;}
    int getY(){return here.y;}
    
    void printPlayer(WINDOW *);
    virtual void setStart(WINDOW *, int);
    virtual int move(WINDOW *, int, int);
    int checkInput(WINDOW *);
};

player::player(){
    playersChar='@';
    here.x=0;
    here.y=0;
}

player::player(char c){
    here.x=0;
    here.y=0;
    playersChar=c;
}

void player::printPlayer(WINDOW * mazeWin){
    mvwaddch(mazeWin, here.y, here.x, playersChar);
    wrefresh(mazeWin);
    refresh();
}

void player::setStart(WINDOW * mazeWin, int rows){
    int x, y=1;
    x=2;
    while(mvwinch(mazeWin, y, x)==ACS_CKBOARD){
        y=randGenerator(1, rows-1);
    }
    
    here.x=x;
    here.y=y;
    
    mvwaddch(mazeWin, here.y, here.x, playersChar);
    wrefresh(mazeWin);
    refresh();
}

int player::move(WINDOW * mazeWin, int whereX, int whereY){

    if(mvwinch(mazeWin, whereY, whereX)=='$'){
        return 0;
    }
    
    else if(mvwinch(mazeWin, whereY, whereX)!=ACS_CKBOARD){
        mvwaddch(mazeWin, here.y, here.x, ' ');
        here.x=whereX;
        here.y=whereY;
        
        printPlayer(mazeWin);
        
        wrefresh(mazeWin);
        refresh();
    }
    return 1;
}

int player::checkInput(WINDOW * mazeWin){
    int isWinningMove=1;
    switch (wgetch(mazeWin)) {
        case KEY_UP:
            isWinningMove=move(mazeWin, getX(), getY()-1);
            break;
            
        case KEY_DOWN:
            isWinningMove=move(mazeWin, getX(), getY()+1);
            break;
            
        case KEY_LEFT:
            isWinningMove=move(mazeWin, getX()-1, getY());
            break;
            
        case KEY_RIGHT:
            isWinningMove=move(mazeWin, getX()+1, getY());
            break;
            
        case 'q':
            clear();
            return 0;
    }
    return isWinningMove;
}

class computer:public player{
    list<cell> correctPath;
public:
    computer(char c= 'C'): player(c){};
    
    virtual void setStart(WINDOW *, int);
    bool findCorrectPath(WINDOW *, list<cell>, int, int);
    virtual int move(WINDOW *, char, bool, int);
};

void computer::setStart(WINDOW * mazeWin, int rows){
    int x, y=1;
    x=2;
    while(mvwinch(mazeWin, y, x)==ACS_CKBOARD || mvwinch(mazeWin, y, x)=='@'){
        y=randGenerator(1, rows-1);
    }
    
    here.x=x;
    here.y=y;
    
    mvwaddch(mazeWin, here.y, here.x, playersChar);
    wrefresh(mazeWin);
    refresh();
}

//recursive algorithm for solving maze
bool computer::findCorrectPath(WINDOW *mazeWin, list<cell> wH, int x, int y){
    list<cell>::iterator it;
    cell C(x,y);
    
    //reached the end
    if(mvwinch(mazeWin, y, x) == '$'){
        correctPath.push_front(C);
        return true;
    }
    //check if algorithm already was here
    it=find(wH.begin(), wH.end(), C);
    if(it!=wH.end()){
        return false;
    }
    //input into was here list
    wH.push_back(C);
    //checks individual cell
    if(mvwinch(mazeWin, y, x) != ACS_CKBOARD){
        //checks up
        if(findCorrectPath(mazeWin, wH, x, y-1)){
            correctPath.push_front(C);
            return true;
        }
        //checks down
        if(findCorrectPath(mazeWin, wH, x, y+1)){
            correctPath.push_front(C);
            return true;
        }
        //checks on the left
        if(findCorrectPath(mazeWin, wH, x-1, y)){
            correctPath.push_front(C);
            return true;
        }
        //check on the right
        if(findCorrectPath(mazeWin, wH, x+1, y)){
            correctPath.push_front(C);
            return true;
        }
    }
    return false;
}

int computer::move(WINDOW *mazeWin, char toPut, bool waitToSee, int wait){
    list<cell>::iterator it;
    it=correctPath.begin();
    
    usleep(wait);
    
    if(mvwinch(mazeWin, it->y, it->x)=='$'){
        correctPath.pop_front();
        if(waitToSee){
            int y, x;
            getmaxyx(stdscr, y, x);
            mvwprintw(stdscr, y-2, 2, "Any key to continue...");
            wrefresh(mazeWin);
            getch();
        }
        clear();
        return 0;
    }
    
    mvwaddch(mazeWin, here.y, here.x, toPut);
    here.x=it->x;
    here.y=it->y;
    correctPath.pop_front();
    
    printPlayer(mazeWin);
    
    wrefresh(mazeWin);
    refresh();
    return 1;
}

class maze{
    int difficulty;
    int collumns;
    int rows;
    list<cell> path;
    
public:
    maze();
    maze(int);
    
    int getCollumns(){return collumns;}
    int getRows(){return rows;}
    
    void createCanvas(WINDOW *);
    void addWalls(cell, list<cell> &);
    
    void pickFirstCell(list<cell> &);
    bool twoNeighborsVisited(cell);
    int isLLRU(cell);//check is it left/right/down/up
    
    void generateMaze();// done????
    void pickEndPoint(WINDOW *);
    void printPath(WINDOW *);
};

maze::maze(){
    difficulty=-1;
    collumns=0;
    rows=0;
}

maze::maze(int diff){
    difficulty=diff;
    
    switch (difficulty) {
        case 0:
            rows=21;
            collumns=41;
            break;
        case 1:
            rows=21;
            collumns=61;
            break;
        case 2:
            rows=31;//31
            collumns=71;//71
            break;
            
        default:
            break;
    }
}
//creates box of #
void maze::createCanvas(WINDOW * mazeWin){
    //draw # in chosen size
    for( int i=1 ;i<collumns+1 ;i++ ){
        for( int j=1; j<rows+1; j++){
            mvwaddch(mazeWin, j, i, ACS_CKBOARD);
        }
    }
    
    refresh();
    wrefresh(mazeWin);
}

void maze::addWalls(cell c, list<cell> &allWalls){
    
    for (int i=-1; i<2; i+=2){
        cell wallX(c.x-i, c.y);
        if(wallX.x>0 && wallX.y>0 && wallX.x<collumns-1 && wallX.y<rows-1){
            allWalls.push_back(wallX);
        }
        
        cell wallY(c.x, c.y-i);
        if(wallY.x>0 && wallY.y>0 && wallY.x<collumns-1 && wallY.y<rows-1){
            allWalls.push_back(wallY);
        }
    }
}

void maze::pickFirstCell(list<cell> &allWalls){
    cell c;
    
    c.x=1;
    while(c.y%2!=1){
        c.y=randGenerator(1, rows-1);
    }
    path.push_back(c);
    addWalls(c, allWalls);
}
//checks if cell has two neighbors
bool maze::twoNeighborsVisited(cell c){
    std::list<cell> ::iterator it;
    int neighbors=0;
    
    for (int i=-1; i<2; i+=2){
        cell pathNeighbor(c.x-i, c.y);
        it= find(path.begin(), path.end(), pathNeighbor);
        
        if(it != path.end()){
            neighbors++;
        }
    }
    for (int j=-1; j<2; j+=2){
        cell pathNeighbor(c.x, c.y-j);
        it= find(path.begin(), path.end(), pathNeighbor);
        
        if(it != path.end()){
            neighbors++;
        }
    }
    
    if(neighbors > 1){
        return true;
    }
    return false;
}
//finds the "start" cell (cell that we want to convert to path)
int maze::isLLRU(cell c){
    list<cell> ::iterator it;
    
    for(int i=-1; i<2; i+=2){
        cell toFind(c.x, c.y-i);
        it=find(path.begin(), path.end(), toFind);
        
        if(it != path.end()){
            if(i==-1){
                return 1;
            }
            return 2;
        }
    }
    
    for(int i=-1; i<2; i+=2){
        cell toFind(c.x-i, c.y);
        it=find(path.begin(), path.end(), toFind);
        
        if(it != path.end()){
            if(i==-1){
                return 3;
            }
            return 4;
        }
    }
    //if something gone bad
    exit(0);
}
//prim's algorithm
void maze::generateMaze(){
    list<cell> allWalls;
    pickFirstCell(allWalls);
    
    while(!allWalls.empty()){
        list<cell>::iterator it= allWalls.begin();
        advance(it, randGenerator(0, (int) allWalls.size()));
        
        if(!twoNeighborsVisited(*it)){
            cell toAdd(it->x, it->y);
            path.push_back(toAdd);
            
            switch (isLLRU(*it)) {
                case 1:
                    toAdd.y--;
                    break;
                case 2:
                    toAdd.y++;
                    break;
                case 3:
                    toAdd.x--;
                    break;
                case 4:
                    toAdd.x++;
                    break;
                default:
                    break;
            }
            addWalls(toAdd,allWalls);
            path.push_back(toAdd);
        }
        it= allWalls.erase(it);
    }
}

void maze::pickEndPoint(WINDOW * mazeWin){
    int x, y=1;
    x=collumns-1;
    while(mvwinch(mazeWin, y, x)==ACS_CKBOARD){
        y=randGenerator(1, rows-1);
    }
    
    mvwaddch(mazeWin, y, x, '$');
    wrefresh(mazeWin);
}

void maze::printPath(WINDOW * mazeWin){
    createCanvas(mazeWin);
    
    list<cell>::iterator it;
    for(it= path.begin(); it!= path.end(); ++it){
        mvwaddch(mazeWin, it->y+1, it->x+1, ' ');
        usleep(3000);
        wrefresh(mazeWin);
    }
    for(it= path.begin(); it!= path.end(); ++it){
        path.pop_front();
    }
    
    pickEndPoint(mazeWin);
}

int randGenerator(int min, int max){
    return rand() % (max-min) +min;
}

//creates a command and resizes terminal to work with game
void setSizeOfTerminal(int y, int x){
    int tempY, tempX;
    
    getmaxyx(stdscr, tempY, tempX);
    
    //if terminal is full screen, or just big do not resize.
    if(tempY<y+2 || tempX<x+2){
        //creates command. example:"printf '\e[8;40;90t'"
        string command = "printf \'\\e[8;";
        command+=to_string(y)+";"+to_string(x)+"t\'";
        
        system(command.c_str());//resize terminal window
        resizeterm(y, x);
    }
}

//print informations about game
void info(){
    fstream file;
    file.open("ABOUT.txt", ios::in);
    
    setSizeOfTerminal(60, 120);
    int y, x, i=0;
    getmaxyx(stdscr, y, x);
    
    WINDOW * infoWin = newwin((y-10), (x-10), (y-(y-10))/2 , (x-(x-10))/2);
    
    getmaxyx(infoWin, y, x);
    box(infoWin, 0, 0);
    keypad(infoWin, true);
    
    string read;
    while(getline(file, read)){
        mvwprintw(infoWin, 1+i++, x/2-((int)read.length()/2), read.c_str());
    }
    mvwprintw(infoWin, y-2, 1, "ENTER to leave");
    wattron(infoWin, A_REVERSE);
    mvwprintw(infoWin, y-2, x/2-2, "LEAVE");
    wattroff(infoWin, A_REVERSE);
    
    refresh();
    wrefresh(infoWin);
    
    while(getch()!='\n');
    file.close();
    delwin(infoWin);
}

//menu algorithm... for main menu and game menu
int menu(vector<string> options){
    setSizeOfTerminal(40,100);
    
    int y, x;
    getmaxyx(stdscr, y, x);
    
    WINDOW * menuWin = newwin((y-10), (x-10), (y-(y-10))/2 , (x-(x-10))/2); //height, width, from top, form left
    getmaxyx(menuWin, y, x);
    box(menuWin, 0, 0);
    keypad(menuWin, true);
    
    mvwprintw(menuWin, y-3, 1, "Use ARROWS to choose (UP, DOWN)");
    mvwprintw(menuWin, y-2, 1, "Use ENTER to select");
    refresh();
    
    int highlight=0;
    
    while(1){
        int pressedKey;
        for(int i=0; i< options.size(); i++){
            if(i==highlight){
                wattron(menuWin, A_REVERSE);
                mvwprintw(menuWin, (y-10)/2+i, x/2-(int)options[i].length()/2, options[i].c_str());
                wattroff(menuWin, A_REVERSE);
            }
            else{
                mvwprintw(menuWin, (y-10)/2+i, x/2-(int)options[i].length()/2, options[i].c_str());
            }
        }
        refresh();
        pressedKey= wgetch(menuWin);
        
        if(pressedKey==KEY_UP && highlight!=0){
            highlight--;
        }
        else if(pressedKey==KEY_DOWN && highlight!=options.size()-1){
            highlight++;
        }
        else if(pressedKey=='\n'){
            keypad(menuWin, false);
            delwin(menuWin);
            clear();
            return highlight;
        }
    }
}

void printInfoForGame(){
    int y, x;
    getmaxyx(stdscr, y, x);
    
    mvprintw(y-3, 1, "Use ARROWS to move");
    mvprintw(y-2, 1, "Use q to leave");
    refresh();
}
//Just a player
void gameModeOne(WINDOW *mazeWin, player P){
    int loop=1;
    printInfoForGame();
    while (loop){
        usleep(10000);
        
        loop=P.checkInput(mazeWin);
    }
}
//just pc
void gameModeThree(WINDOW *mazeWin, computer C, char toWrite, bool canWin){
    int loop=1;
    while (loop){
        loop=C.move(mazeWin, toWrite, canWin, 100000);
    }
}
//thread for game mode one
void threadGMO(std::atomic_bool& done1, WINDOW *mazeWin, player P, unsigned int ms){
    const auto sleepGMO = std::chrono::milliseconds(ms);
    
    while(!done1){
        int loop=1;
        printInfoForGame();//Maybe no need for that
        while (loop && !done1){
            usleep(10000);
            
            loop=P.checkInput(mazeWin);
            std::this_thread::sleep_for(sleepGMO);
        }
        std::this_thread::sleep_for(sleepGMO);
        done1 = true;
    }
}
//thread for game mode one
void threadGMT(std::atomic_bool& done2, WINDOW *mazeWin, computer C, char toWrite, bool canWin, unsigned int ms){
    const auto sleepGMT = std::chrono::milliseconds(ms);
    
    while(!done2){
        int loop=1;
        while (loop && !done2){
            loop=C.move(mazeWin, toWrite, canWin, randGenerator(10000, 300000));
            std::this_thread::sleep_for(sleepGMT);
        }
        std::this_thread::sleep_for(sleepGMT);
        done2 = true;
    }
}
//player vs pc
void gameModeTwo(WINDOW *mazeWin, player P, computer C){
    printInfoForGame();
    
    std::atomic_bool done1(false);
    std::atomic_bool done2(false);
    
    thread (threadGMO, ref(done1), mazeWin, P, 100).detach();
    thread (threadGMT, ref(done2), mazeWin, C, ' ', false, 200).detach();
    
    while(!done1 && !done2){}
    if(!done1 || !done2){
        if(done1){
            done2=true;
        }
        else{
            done1=true;
            int y, x;
            getmaxyx(stdscr, y, x);
            mvwprintw(stdscr, y-2, 2, "Double click Any key to continue...");
            refresh();
            getch();
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}

int main(int argc, const char * argv[]) {
    srand((unsigned)time(NULL));

    // Initialize ncurses, no line buffering and no echo for getch
    initscr();
    curs_set(0);
    noecho(); // do not print input on the screen
    
    vector<std::string> mainMenuOpt {"PLAY SOLO", "PLAY VS COMPUTER", "AUTO PLAY", "ABOUT", "EXIT"};

    while(1){
        int chosen = menu(mainMenuOpt);
        clear();
        if(mainMenuOpt[chosen].find("PLAY") != string::npos){
            vector<string> difficultyMenu {"EASY", "MEDIUM", "HARD", "EXIT"};
            int difficulty=menu(difficultyMenu);

            if(difficulty!=(int)difficultyMenu.size()-1){
                maze M(difficulty);
                
                setSizeOfTerminal(M.getRows()+4, M.getCollumns()+4);
                int y, x;
                getmaxyx(stdscr, y, x);

                WINDOW * mazeWin = newwin((M.getRows()+2), (M.getCollumns()+2), (y-M.getRows())/2 , (x-M.getCollumns())/2);
                
                getmaxyx(mazeWin, y, x);
                box(mazeWin, 0, 0);
                keypad(mazeWin, true);
                
                M.generateMaze();
                M.printPath(mazeWin);
                //choose gamemode (chosen)
                if(chosen==0){
                    player P;
                    P.setStart(mazeWin, M.getRows());
                    gameModeOne(mazeWin, P);
                }
                else{
                    computer C;
                    C.setStart(mazeWin, M.getRows());
                    list<cell>wasHere;
                    C.findCorrectPath(mazeWin, wasHere, C.getX(), C.getY());
                    
                    if(chosen==1){
                        player P;
                        P.setStart(mazeWin, M.getRows());
                        gameModeTwo(mazeWin, P, C);
                    }
                    else{
                        gameModeThree(mazeWin, C, '+', true);
                    }
                }
                
                delwin(mazeWin);
                clear();
            }
        }
        else if(mainMenuOpt[chosen].find("ABOUT") != string::npos){
            info();
        }
        else{
            break;
        }
    }
    endwin();
    return 0;
}
