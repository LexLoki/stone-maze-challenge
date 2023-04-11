#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <chrono>
#include <bitset>
using namespace std;
using namespace std::chrono;

#define ROW_SIZE 2000       // Number of rows in grid
#define COL_SIZE 2000       // Number of columns in grid
#define TOT_SIZE 4000000    // Total number of cells in grid (ROW_SIZE*COL_SIZE)
#define IT_LENGTH 6200      // Quantity of iterations to memoize and run

#define MAX_LIFES 6
#define VIS_TOT 12000000    // Quantity of bits in state matrix (3*TOT_SIZE) (since max_life<8)

#define PRINT_CAP 100000000 // Interval of iterations required to print progress to console

bitset<TOT_SIZE> *automata; // States of automata
bitset<VIS_TOT> *visited;  // DP information of having visited a given cell at a given time

short trow = ROW_SIZE-1, tcol = COL_SIZE-1; // Objective/target coordinates
unsigned long iterations;                   // Number of iterations (will overflow though...)

// To show max coordinates found while iterating
unsigned short max_row = 0, max_col = 0, max_length = 0;

stack<vector<unsigned short>> *moves;       // Positions of best path
unsigned int currMoveLength = IT_LENGTH;    // Maximum possible length to interrupt unfeasible / worse than better solutions

void fill_start()
{
    std::ifstream input("challenges/input2.txt");
    if(!input.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    int v;
    int offset = 0;
    for(int i=0;i<ROW_SIZE;i++){
        for(int j=0;j<COL_SIZE;j++){
            input >> v;
            automata[0][offset++] = v>0;
        }
    }
    automata[0][0] = automata[0][offset-1] = false;
    input.close();
}

char calc_rank(int i, int offset, int k)
{
    char rank = 0;
    if(offset > 0){
        if(automata[i][offset-COL_SIZE+k]) rank++;
        if(k>0 && automata[i][offset-COL_SIZE+k-1]) rank++;
        if(k<COL_SIZE-1 && automata[i][offset-COL_SIZE+k+1]) rank++;
    }
    if(offset < TOT_SIZE-COL_SIZE){
        if(automata[i][offset+COL_SIZE+k]) rank++;
        if(k>0 && automata[i][offset+COL_SIZE+k-1]) rank++;
        if(k<COL_SIZE-1 && automata[i][offset+COL_SIZE+k+1]) rank++;
    }
    if(k>0 && automata[i][offset+k-1]) rank++;
    if(k<COL_SIZE-1 && automata[i][offset+k+1]) rank++;
    return rank;
}

void calculate_iterations()
{
    int iprev, offset;
    int quant = 0;
    cout << "\t.";
    for(int i=1;i<IT_LENGTH;i++)
    {
        if(i%100 == 0) cout << '.' << flush;
        iprev = i-1;
        offset = 0;
        for(int j=0;j<ROW_SIZE;j++)
        {
            for(int k=0;k<COL_SIZE;k++)
            {
                char rank = calc_rank(iprev, offset, k);
                if(!automata[iprev][offset+k]){
                    if(rank > 1 && rank < 5) automata[i].set(offset+k);
                }
                else{
                    if(rank > 3 && rank < 6) automata[i].set(offset+k);
                }
            }
            offset += COL_SIZE;
        }
        automata[i][0] = automata[i][offset-1] = false;
        quant++;
    }
    cout << endl;
}

stack<vector<unsigned short>> *solve(unsigned length, unsigned char lifes, unsigned short row, unsigned short col)
{
    if((++iterations)%PRINT_CAP == 0){
        cout << iterations << "\tMax: " << max_row << ',' << max_col << "\tLength: " << max_length << endl;
    }
    if(length + trow-row + tcol-col >= currMoveLength) //Interrupt path that cannot reach in time
    {
        return nullptr;
    }
    unsigned int pos = ((unsigned int)row)*COL_SIZE + col;
    unsigned int vpos = pos*3;
    if((visited[length][vpos] | (visited[length][vpos+1] << 1) | (visited[length][vpos+2] << 2)) >= lifes )
        return nullptr;
    if(automata[length][pos])
    {
        if(--lifes == 0) return nullptr;
    }
    if(row+col >= max_row+max_col)
    {
        max_row = row; max_col = col;
        if(row+col > max_row+max_col || length < max_length) max_length = length;
    }
    if(row == trow && col == tcol){
        auto st = new stack<vector<unsigned short>>();
        currMoveLength = max_length = length;
        st->push({ row, col });
        return st;
    }
    visited[length][vpos++] = lifes&1;
    visited[length][vpos++] = lifes&2;
    visited[length][vpos] = lifes&4;
    length++;
    stack<vector<unsigned short>> *mv = nullptr;
    stack<vector<unsigned short>> *t;
    if(row<ROW_SIZE-1) mv = solve(length, lifes, row+1, col);
    if(col<COL_SIZE-1 && (t=solve(length, lifes, row, col+1)))
        if(mv == nullptr || t->size() < mv->size()) mv = t;
    if(row>0 && (t=solve(length, lifes, row-1, col)))
        if(mv == nullptr || t->size() < mv->size()) mv = t;
    if(col>0 && (t=solve(length, lifes, row, col-1)))
        if(mv == nullptr || t->size() < mv->size()) mv = t;
    if(mv != nullptr) mv->push({row, col});
    return mv;
}

void show_automata(int length)
{
    int offset = 0;
    for(int i=0;i<ROW_SIZE-1;i++){
        for(int j=0;j<COL_SIZE-1;j++)
        {
            cout << automata[length][offset++] ? 1 : 0;
        }
        cout << endl;
    }
}

void stackPositions()
{
    if(moves == nullptr) return;
    while(!moves->empty())
    {
        vector<unsigned short> pos = moves->top();
        moves->pop();
        cout << pos[0] << '\t' << pos[1] << endl;
    }
}

void stackMovements()
{
    if(moves==nullptr)
    {
        cout << "No solution found" << endl;
        return;
    }
    cout << "Solution with size " << moves->size() << ", steps " << currMoveLength << endl;
    vector<unsigned short> lastPos = moves->top();
    moves->pop();
    while(!moves->empty())
    {
        vector<unsigned short> pos = moves->top();
        moves->pop();
        char move = 'U';
        if(pos[0] > lastPos[0]) move = 'D';
        else if(pos[1] > lastPos[1]) move = 'R';
        else if(pos[1] < lastPos[1]) move = 'L';
        cout << move << ' ';
        lastPos = pos;
    }
}

void init_mem()
{
    automata = new bitset<TOT_SIZE>[IT_LENGTH];
    visited = new bitset<VIS_TOT>[IT_LENGTH];
}

void free_mem()
{
    delete[] automata;
    delete[] visited;
}

int main() {
    cout << "Memory allocation...";
    init_mem();
    cout << " Complete" << endl;
    cout << "................................................................." << endl;
    int i,j,k;
    visited[0][0] = false;
    cout << "States precomputing...";
    fill_start();

    cout << "Calculating states..." << endl;
    auto start = steady_clock::now();
    calculate_iterations();
    auto end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    cout << " Complete" << endl;

    //show_automata(0); cout << endl;
    //show_automata(1); cout << endl;
    //show_automata(2); cout << endl;
    //show_automata(3); cout << endl;
    //show_automata(4); cout << endl;
    //show_automata(65); cout << endl;

    cout << "................................................................." << endl;
    cout << "Solving..." << endl;
    start = steady_clock::now();
    iterations = 0;
    moves = solve(0, MAX_LIFES, 0, 0);
    end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    cout << "Total iterations until stop: " << iterations << endl;
    cout << "Max dist: " << max_row << "," << max_col << "\t steps: " << max_length << endl;

    stackMovements();
    if(moves) delete moves;
    free_mem();
}