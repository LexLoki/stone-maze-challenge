#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <chrono>
#include <bitset>
using namespace std;
using namespace std::chrono;

#define ROW_SIZE 2500       // Number of rows in grid
#define COL_SIZE 2500       // Number of columns in grid
#define TOT_SIZE 6250000    // Total number of cells in grid (ROW_SIZE*COL_SIZE)
#define IT_LENGTH 10000     // Quantity of iterations to memoize and run. When memoizing to HD, this will be the chunk size

// For the memoization on HD approach
#define MEMO_SIZE 10000
#define MEMO_CHUNKS 51      // Quantity of chunks to memoize to HD, when running hd memoization
#define MEMO_CHUNK_SIZE 1000

#define USE_HD_MEMO 1   //Read states from "./data" folder instead of calculating then

#define PUZZLE_START 2300   // ROW,COL of puzzle origin
#define PUZZLE_SIZE 10      // Dimension of puzzle

#define PRINT_CAP 100000000 // Interval of iterations required to print progress to console

bitset<TOT_SIZE> *automata; // States of automata
bitset<TOT_SIZE> *visited;  // DP information of having visited a given cell at a given time

//For dalloc
int alloc_offset = 0;

//short trow = ROW_SIZE-1, tcol = COL_SIZE-1;
short trow = 69, tcol = 2230; // after verifying this is the longest we can reach, we set this up
unsigned long iterations;

//For best distance
unsigned short max_row = 0, max_col = 0, max_length = 0;

stack<vector<unsigned short>> *moves;
unsigned int currMoveLength = IT_LENGTH;

void fill_puzzle()
{
    std::ifstream input("challenges/sol_puzzle.txt");
    if(!input.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    int v;
    int offset = PUZZLE_START*COL_SIZE + PUZZLE_START;
    for(int i=0;i<PUZZLE_SIZE;i++){
        for(int j=0;j<PUZZLE_SIZE;j++){
            input >> v;
            automata[0][offset+j] = v>0;
        }
        offset += COL_SIZE;
    }
    input.close();
}

void fill_start()
{
    std::ifstream input("challenges/input4.txt");
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
    fill_puzzle();
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

void load_states(string path, int start, int len)
{
    ifstream wf(path, ios::in | ios::binary);
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    for(int i=0;i<len;i++) wf.read((char *) &automata[start+i], TOT_SIZE/8);
    wf.close();
}

void write_curr_states(string path, int len)
{
    ofstream wf(path, ios::out | ios::binary);
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    for(int i=0;i<len;i++) wf.write((char *) &(automata[i]), TOT_SIZE/8);
    wf.close();
}

void write_state(string path, bitset<TOT_SIZE> &buffer)
{
    ofstream wf(path, ios::out | ios::binary);
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    wf.write((char *) &buffer, TOT_SIZE/8);
    wf.close();
}

void memoize_iterations()
{
    int iprev, offset;
    int quant = 0;
    char filename[30];
    cout << "Memoizing.";
    for(int k=0;k<MEMO_CHUNKS;k++){
        cout << "\n\tChunk " << k*IT_LENGTH << " - " << (k+1)*IT_LENGTH << " _ ";
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
                        automata[i].set(offset+k, (rank > 1 && rank < 5));
                    }
                    else{
                        automata[i].set(offset+k, (rank > 3 && rank < 6));
                    }
                }
                offset += COL_SIZE;
            }
            automata[i][0] = automata[i][offset-1] = false;
        }
        cout << "_ computed _ ";
        sprintf(filename, "data/q4_%d.bat", k);
        write_curr_states(filename, IT_LENGTH);
        cout << "saved";
        iprev = IT_LENGTH-1;
        offset = 0;
        for(int j=0;j<ROW_SIZE;j++)
        {
            for(int k=0;k<COL_SIZE;k++)
            {
                char rank = calc_rank(iprev, offset, k);
                if(!automata[iprev][offset+k]){
                    automata[0].set(offset+k, (rank > 1 && rank < 5));
                }
                else{
                    automata[0].set(offset+k, (rank > 3 && rank < 6));
                }
            }
            offset += COL_SIZE;
        }
        automata[0][0] = automata[0][offset-1] = false;
    }
    cout << endl;
}

void load_page(int index)
{
    char filename[30];
    int i=0;
    alloc_offset = index;
    index /= MEMO_CHUNK_SIZE;
    for(int loaded = 0; loaded<MEMO_SIZE; loaded += MEMO_CHUNK_SIZE)
    {
        sprintf(filename, "data/q4_%d.bat", index+(i++));
        load_states(filename, loaded, MEMO_CHUNK_SIZE);
    }
}

void load_iterations()
{
    load_page(0);
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

//inline void callSubSolve(unsigned length, unsigned short row, unsigned short col)

stack<vector<unsigned short>> *solve(unsigned length, unsigned short row, unsigned short col)
{
    if((++iterations)%PRINT_CAP == 0){
        cout << iterations << "\tMax: " << max_row << ',' << max_col << "\tLength: " << max_length << endl;
    }
    if(length + abs(trow-row) + abs(tcol-col) >= currMoveLength)
    {
        return nullptr;
    }
    unsigned int pos = ((unsigned int)row)*COL_SIZE + col;
    if(visited[length][pos]) return nullptr;
    if(length-alloc_offset >= MEMO_SIZE)
    {
        int page = length/MEMO_CHUNK_SIZE - (length%MEMO_CHUNK_SIZE < (MEMO_CHUNK_SIZE/2));
        //cout << length << '\t' << page << endl;
        load_page(page*MEMO_CHUNK_SIZE);
    }
    if(automata[length-alloc_offset][pos]) return nullptr;
    if(row+col >= max_row+max_col)
    {
        max_row = row; max_col = col;
        if(row+col > max_row+max_col || length < max_length) max_length = length;
    }
    if(row == trow && col == tcol){
        //cout << "FOUND IT" << endl;
        auto st = new stack<vector<unsigned short>>();
        currMoveLength = max_length = length;
        st->push({ row, col });
        return st;
    }
    visited[length++][pos] = true;
    stack<vector<unsigned short>> *mv = nullptr;
    stack<vector<unsigned short>> *t;
    if(row<ROW_SIZE-1) mv = solve(length, row+1, col);
    if(col<COL_SIZE-1 && (t=solve(length, row, col+1)))
        if(mv == nullptr || t->size() < mv->size()) mv = t;
    if(row>0 && (t=solve(length, row-1, col)))
        if(mv == nullptr || t->size() < mv->size()) mv = t;
    if(col>0 && (t=solve(length, row, col-1)))
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

void testSol()
{
    std::ifstream input("challenges/sol4 - 2299 (optimal).txt");
    if(!input.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    char move;
    int row = 0, col = 0;
    unsigned int pos;
    for(int i=1;i<=2299;i++)
    {
        input >> move;
        if(move == 'L') row--;
        else if(move == 'R') row++;
        else if(move == 'U') col--;
        else if(move == 'D') col++;
        pos = ((unsigned int)row)*COL_SIZE + col;
        if(automata[i][pos])
        {
            cout << "invalid pos" << endl;
            input.close();
            return;
        }
    }
    cout << "All valid" << endl;
    input.close();
    cout << automata[2300][pos+1] << endl;
    cout << automata[2300][pos+COL_SIZE] << endl;
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
    automata = new bitset<TOT_SIZE>[MEMO_SIZE];
    visited = new bitset<TOT_SIZE>[IT_LENGTH];
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
    cout << " Complete" << endl;
    //show_automata

    cout << "Calculating states..." << endl;
    auto start = steady_clock::now();
    //memoize_iterations();     //To calculate chunks and store on HD
    if(USE_HD_MEMO)
        load_iterations(); //To load ALL from HD
    else
        calculate_iterations(); //To calculate all on RAM
    auto end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;

    //return 0; //return here when we only want to HD memo

    //show_automata(0); cout << endl;
    //show_automata(1); cout << endl;
    //show_automata(2); cout << endl;
    //show_automata(3); cout << endl;
    //show_automata(4); cout << endl;
    //show_automata(65); cout << endl;

    //test solution
    //testSol();
    //return 0;


    cout << "Solving..." << endl;
    start = steady_clock::now();
    iterations = 0;
    moves = solve(0, 0, 0);
    end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    cout << "Total iterations until stop: " << iterations << endl;
    cout << "Max dist: " << max_row << "," << max_col << "\t steps: " << max_length << endl;

    stackMovements();
    if(moves) delete moves;
    free_mem();
}