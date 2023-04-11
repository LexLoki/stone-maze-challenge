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

#define POWER_CAP 0
#define DIST_TARGET 60

#define USE_PRE_PROP 0
#define INVESTIGATE 0
//#define INVESTIGATE_PATH "challenges/sol3 - nopw - 6200.txt"
#define INVESTIGATE_PATH "challenges/sol3 - 1 - 6258.txt"

#define INVESTIGATE_VALUE 7
#define INFORM_VALUE 9

#define PRINT_CAP 100000000 // Interval of iterations required to print progress to console

#define MEMO_CHUNKS 63
#define MEMO_CHUNK_SIZE 100

#define USE_HD_MEMO 1

bitset<TOT_SIZE> *automata; // States of automata
bitset<TOT_SIZE> *visited;  // DP information of having visited a given cell at a given time
int higherLoaded = 0;
//bool ***automata;
//bool ***visited;

short trow = ROW_SIZE-1, tcol = COL_SIZE-1;
unsigned long iterations;

//For best distance
unsigned short max_row = 0, max_col = 0, max_length = 0;

stack<vector<unsigned short>> *moves;
unsigned int currMoveLength = 6128;

void fill_start()
{
    std::ifstream input("challenges/input3.txt");
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

vector<int> *neigh(int i, int offset, int k)
{
    vector<int> *v = new vector<int>();
    if(offset > 0){
        if(automata[i][offset-COL_SIZE+k]) v->push_back(offset+k-COL_SIZE);
        if(k>0 && automata[i][offset-COL_SIZE+k-1]) v->push_back(offset+k-1-COL_SIZE);
        if(k<COL_SIZE-1 && automata[i][offset-COL_SIZE+k+1]) v->push_back(offset+k+1-COL_SIZE);
    }
    if(offset < TOT_SIZE-COL_SIZE){
        if(automata[i][offset+COL_SIZE+k]) v->push_back(offset+COL_SIZE+k);
        if(k>0 && automata[i][offset+COL_SIZE+k-1]) v->push_back(offset+COL_SIZE+k+1);
        if(k<COL_SIZE-1 && automata[i][offset+COL_SIZE+k+1]) v->push_back(offset+COL_SIZE+k-1);
    }
    if(k>0 && automata[i][offset+k-1]) v->push_back(offset+k-1);
    if(k<COL_SIZE-1 && automata[i][offset+k+1]) v->push_back(offset+k+1);
    return v;
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
    //cout << path << endl;
    ifstream wf(path, ios::in | ios::binary);
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    //cout << "bye" << endl;
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
        sprintf(filename, "data/q3/q3_%d.bat", k);
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
    //alloc_offset = index;
    index /= MEMO_CHUNK_SIZE;
    for(int loaded = 0; loaded<IT_LENGTH; loaded += MEMO_CHUNK_SIZE)
    {
        sprintf(filename, "data/q3/q3_%d.bat", index+(i++));
        load_states(filename, loaded, MEMO_CHUNK_SIZE);
        //cout << loaded << endl;
    }
}

void load_iterations()
{
    load_page(0);
}

void random_changes()
{
    srand(20);
    //rand() % myvector.size()
    vector<int> positions;
    int offset = 0;
    for(int i=0;i<ROW_SIZE;i++)
    {
        for(int j=0;j<COL_SIZE;j++)
        {
            if((i==0 && j==0) || (i==ROW_SIZE-1 && j==COL_SIZE-1)) continue;
           char rank = calc_rank(0, offset, j);
           if(rank > 2) positions.push_back(offset+j);
        }
        offset += COL_SIZE;
    }
    int n = positions.size();
    cout << "Got " << n << " valid values to switch with power" << endl;
    for(int i=0;i<30 && n;i++)
    {
        int index = rand()%n;
        int temp = positions[index];
        positions[index] = positions[n-1];
        positions[n-1] = temp;
        automata[0][index] = !automata[0][index];
        n--;
    }
}

void load_next(int minRow, int minCol)
{
    int iprev = higherLoaded-1;
    int offset;
    //cout << minRow << ' ' << minCol << endl;
    minRow = max(minRow, 0);
    minCol = max(minCol, 0);
    offset = minRow * COL_SIZE;
    for(int j=minRow;j<ROW_SIZE;j++)
    {
        for(int k=minCol;k<COL_SIZE;k++)
        {
            char rank = calc_rank(iprev, offset, k);
            if(!automata[iprev][offset+k]){
                automata[higherLoaded][offset+k] = (rank > 1 && rank < 5);
            }
            else{
                automata[higherLoaded][offset+k] = (rank > 3 && rank < 6);
            }
        }
        offset += COL_SIZE;
    }
    visited[higherLoaded].reset();
    automata[higherLoaded][0] = automata[higherLoaded][offset-1] = false;
    higherLoaded++;
}

void drawAround(unsigned length, unsigned row, unsigned col)
{
    unsigned pos = row*COL_SIZE + col;
    unsigned size = 4;
    cout << "Showing around: " << row << ',' << col << " (player at 1,1)"<< endl;
    row--, col--;
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            pos = (row+i)*COL_SIZE + col+j;
            cout << automata[length][pos] << ' ';
        }
        cout << '\t';
        for(int j=0;j<size;j++)
        {
            pos = (row+i)*COL_SIZE + col+j;
            cout << automata[length+1][pos] << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

void propagate()
{
    vector<vector<unsigned>> props = {{85, 22, 26, 21, 26}};
    int index = 0;
    bool active = false;
    cout << "Start propagation check" << endl;
    for(int i=0;i<props.size();i++)
    {
        cout << "\tCalc until: " << props[i][0] << endl;
        while(index<props[i][0])
        {
            index++;
            if(active) {
                higherLoaded = index; load_next(0,0);
            }//propagate change
        }
        unsigned row = props[i][1], col = props[i][2];
        if(row<col) row++;
        else col++;
        //check rank 4
        cout << "\tIndv on: " << index << ',' << props[i][1] << ',' << props[i][2] << endl;
        unsigned pos = props[i][1]*COL_SIZE + props[i][2];
        cout << "\tBefore:" << endl;
        drawAround(props[i][0], props[i][3], props[i][4]);
        automata[index][pos] = !automata[index][pos];
        active = true;
        index++; higherLoaded = index; load_next(0,0);
        cout << "\tAfter:" << endl;
        drawAround(props[i][0], props[i][3], props[i][4]);
    }
    cout << "\tCalculating rest" << endl;
    while(index<IT_LENGTH-1)
    {
        index++; higherLoaded = index; load_next(0,0);
    }
}

int checkLoops()
{
    std::ifstream input(INVESTIGATE_PATH);
    if(!input.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    int l = 6200;
    int coords[6201][2];
    int lengths[6201];
    coords[0][0] = 0; coords[0][1] = lengths[0] = 0;
    int row = 0, col = 0;
    char dir;
    for(int i=0;i<l;i++)
    {
        input >> dir;
        if(dir == 'D') row++;
        else if(dir == 'U') row--;
        else if(dir == 'R') col++;
        else col--;
        coords[i+1][0] = row;
        coords[i+1][1] = col;
        lengths[i+1] = row+col;
    }
    input.close();
    int higherLoopIndex, higherLoopValue = 0;
    int count = 0;
    for(int i=0;i<l;){
        int j = i+1;
        while(j<l+1 && lengths[j]<=lengths[i]) j++;
        if(j>=l+1) break;
        if(j-i>INVESTIGATE_VALUE){
            //cout << lengths[i] << " " << j-i << " . ";
            cout << j-i << "\t";
            //cout << coords[i][0] << ',' << coords[i][1] << ':' << j-i << ". ";
            if((++count)%30 == 0) cout << endl; //20 for all
        }
        if(INFORM_VALUE > 0 && j-i == INFORM_VALUE)
        {
            cout << "\nGet: " << coords[i][0] << ',' << coords[i][1] << " at " << i << endl;
            drawAround(i, coords[i][0], coords[i][1]);
            break;
        }
        if(j-i > higherLoopValue)
        {
            higherLoopIndex = i;
            higherLoopValue = j-i;
        }
        i = j;
    }
    cout << endl;
    cout << "Higher:" << endl;
    cout << higherLoopIndex << '\t' << higherLoopValue << endl;
    cout << coords[higherLoopIndex][0] << ' ' << coords[higherLoopIndex][1] << endl;
    cout << endl;
    //exit(1);
    return higherLoopIndex;
}

void calculate_iterations()
{
    higherLoaded = 1;
    load_next(0,0);
}

//inline void callSubSolve(unsigned length, unsigned short row, unsigned short col)

stack<vector<unsigned short>> *solve(unsigned length, char power, unsigned short row, unsigned short col, bool withPw)
{
    if((++iterations)%PRINT_CAP == 0){
        cout << iterations << "\tMax: " << max_row << ',' << max_col << "\tLength: " << max_length << endl;
    }
    if(length + (trow-row + tcol-col) >= currMoveLength)
    {
        return nullptr;
    }
    //(currMoveLength - (length + (trow-row + tcol-col))) /2 * (3) (4 conservative)
    while(length >= higherLoaded){
        int movesLeft = currMoveLength - length - (trow-row + tcol-col);
        //cout << row << ',' << col << '.' << length << '\t';
        load_next(row-movesLeft*2, col-movesLeft*2);
    }
    unsigned int pos = ((unsigned int)row)*COL_SIZE + col;
    if(visited[length][pos]) return nullptr;
    if(automata[length][pos]) return nullptr;
    if(row+col >= max_row+max_col)
    {
        max_row = row; max_col = col;
        if(row+col > max_row+max_col || length < max_length) max_length = length;
    }
    if(trow-row + tcol-col == 0){
    //if(row == trow && col == tcol){
        auto st = new stack<vector<unsigned short>>();
        currMoveLength = max_length = length;
        if(withPw){
            cout << "ERROR, CANNOT POWERUP ON LAST MOVE" << endl;
            exit(1);
        }
        st->push({ row, col });
        return st;
    }
    unsigned thisLength = length;
    visited[length++][pos] = true;
    stack<vector<unsigned short>> *mv = nullptr;
    stack<vector<unsigned short>> *t;
    bool side = row <= col;
    if(side){
        if(row<ROW_SIZE-1) mv = solve(length, power, row+1, col, false);
        if(col<COL_SIZE-1 && (t=solve(length, power, row, col+1, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
    }
    else{
        if(col<COL_SIZE-1) mv = solve(length, power, row, col+1, false);
        if(row<ROW_SIZE-1 && (t=solve(length, power, row+1, col, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
    }
    bool canPower = power>0 && (trow-row + tcol-col < DIST_TARGET);
    if(canPower) //when cannot move we try individuality, if next iteration is the last
    {
        char rank;
        unsigned int offset = ((unsigned int)row)*COL_SIZE;
        if(row<ROW_SIZE-1 && automata[length][offset + COL_SIZE + col]) //try to make down movement valid
        {
            rank = calc_rank(thisLength, offset + COL_SIZE, col);
            //cout << (int)rank << endl;
            if(rank!=4) //when change a cell that would be on would will make it die
            {
                //cout << "got it" << endl;
                automata[thisLength][offset+ COL_SIZE + col] = !automata[thisLength][offset+ COL_SIZE + col];
                higherLoaded = length;
                //cout << automata[length][offset+ COL_SIZE+col] << ' ';
                if(t=solve(length, power-1, row+1, col, true)){
                    if(mv == nullptr || t->size() < mv->size()) mv = t;
                }
                //cout << automata[length][offset+ COL_SIZE+col] << endl;
                automata[thisLength][offset+ COL_SIZE + col] = !automata[thisLength][offset+ COL_SIZE + col];
                higherLoaded = length;
            }
        }
        if(col<COL_SIZE-1 && automata[length][offset + 1 + col]) //try to make right movement valid
        {
            rank = calc_rank(thisLength, offset, col+1);
            //cout << (int)rank << endl;
            if(rank!=4) //when change a cell that would be on would will make it die
            {
                //cout << "got it" << endl;
                automata[thisLength][offset+1+col] = !automata[thisLength][offset+1+col];
                higherLoaded = length;
                if(t=solve(length, power-1, row, col+1, true)){
                    if(mv == nullptr || t->size() < mv->size()) mv = t;
                }
                automata[thisLength][offset+1+col] = !automata[thisLength][offset+1+col];
                higherLoaded = length;
            }
        }
        //try reload
    }
    if(side){
        if(col>0 && (t=solve(length, power, row, col-1, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
        if(row>0 && (t=solve(length, power, row-1, col, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
    }
    else{
        if(row>0 && (t=solve(length, power, row-1, col, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
        if(col>0 && (t=solve(length, power, row, col-1, false)))
            if(mv == nullptr || t->size() < mv->size()) mv = t;
    }
    if(mv != nullptr){
        if(withPw) mv->push({row, col, row, col});
        else mv->push({row, col});
    }
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
        if(pos.size()>2){
            cout << "A " << pos[2] << ' ' << pos[3] << ' ';
        }
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
    cout << "States precomputing..." << endl;
    fill_start();
    //random_changes();
    cout << " Complete" << endl;
    //show_automata

    cout << "Calculating states..." << endl;
    auto start = steady_clock::now();

    if(USE_HD_MEMO) load_iterations();
    else calculate_iterations();

    if(INVESTIGATE){
        checkLoops();
        return 0;
    }
    //memoize_iterations();
    if(USE_PRE_PROP) propagate();

    auto end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;

    //show_automata(0); cout << endl;
    //show_automata(1); cout << endl;
    //show_automata(2); cout << endl;
    //show_automata(3); cout << endl;
    //show_automata(4); cout << endl;
    //show_automata(65); cout << endl;
    //return 0;

    //drawAround(85, 21, 26);

    cout << "Solving..." << endl;
    start = steady_clock::now();
    iterations = 0;
    higherLoaded = IT_LENGTH;
    moves = solve(0, POWER_CAP, 0, 0, false);
    end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    cout << "Total iterations until stop: " << iterations << endl;
    cout << "Max dist: " << max_row << "," << max_col << "\t steps: " << max_length << endl;

    stackMovements();
    if(moves) delete moves;
    free_mem();
}