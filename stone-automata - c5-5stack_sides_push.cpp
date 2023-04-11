// Generates 5_3 solution - stack version

#include <unordered_set>
#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <chrono>
#include <bitset>
using namespace std;
using namespace std::chrono;

#define ROW_SIZE 300        // Number of rows in grid
#define COL_SIZE 300        // Number of columns in grid
#define TOT_SIZE 90000      // Total number of cells in grid (ROW_SIZE*COL_SIZE)
#define IT_LENGTH 100000    // Quantity of iterations to memoize and run

#define PRINT_CAP 10000000 // Interval of iterations required to print progress to console

#define MEMO_CHUNKS 101
#define MEMO_CHUNK_SIZE 1000

#define USE_HD_MEMO 1
#define USE_STACK 1

bitset<TOT_SIZE> *automata; // States of automata
bitset<TOT_SIZE> *visited;  // DP information of having visited a given cell at a given time

short trow = ROW_SIZE-1, tcol = COL_SIZE-1;
unsigned long iterations;

//For best distance
unsigned short max_row = 0, max_col = 0, max_length = 0;

stack<vector<unsigned short>> *moves;
unsigned int currMoveLength = 0;

unsigned int lengthLimit = IT_LENGTH;

// Structure for the iterative (non-recursive) BFS implementation
typedef struct bfsNode{
    struct bfsNode *parent;
    unsigned length;
    unsigned short row, col;
    char index;
} BfsNode;

void fill_start()
{
    std::ifstream input("challenges/input5.txt");
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
        sprintf(filename, "data/q5_%d.bat", k);
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
        sprintf(filename, "data/q5_%d.bat", index+(i++));
        load_states(filename, loaded, MEMO_CHUNK_SIZE);
        //cout << loaded << endl;
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
                //visited[i].set(offset)
                //visited[i][offset+k] = false;
                char rank = calc_rank(iprev, offset, k);
                if(!automata[iprev][offset+k]){
                    if(rank > 1 && rank < 5) automata[i].set(offset+k);
                    //automata[i][offset+k] = (rank > 1 && rank < 5);
                }
                else{
                    if(rank > 3 && rank < 6) automata[i].set(offset+k);
                    //automata[i][offset+k] = (rank > 3 && rank < 6);
                }
            }
            offset += COL_SIZE;
        }
        automata[i][0] = automata[i][offset-1] = false;
        quant++;
    }
    cout << endl;
}

void destroy_stack(BfsNode *node)
{
    BfsNode *t1 = node->parent;
    int index = node->index;
    delete node;
    while(t1!=nullptr && index == 0 && t1->index != -1)
    {
        node = t1;
        index = node->index;
        t1 = node->parent;
        delete node;
    }
}

void apply_best_solution(BfsNode *node, BfsNode *oldBest)
{
    BfsNode *t1, *t2;
    for(t1=node;t1!=nullptr && t1->index != -1;t1=t1->parent) t1->index = -1;
    if(oldBest != nullptr){
        for(t2=oldBest->parent;t2!=t1 && t2->index == -1;t2=t2->parent){
            delete oldBest;
            oldBest = t2;
        }
        delete oldBest;
    }
}

// Non-recursive DFS solver
// we stack a node for each coordinate-time to visit, and map them to their
// parent node (calleer). Each child node has a index, the higher index is visited
// first, and when a node with index 0 fails path finding, it is finally destroyed
// and recurse/fallback to parent
stack<vector<unsigned short>> *solveStack(unsigned l, bool side)
{
    stack<BfsNode*> data;
    BfsNode *node = nullptr;
    data.push(new BfsNode{nullptr, l, 0, 0, 0});
    unsigned int length, pos;
    unsigned short row, col;
    BfsNode *bestNodeSeq = nullptr;
    while(!data.empty())
    {
        ++iterations;
        node = data.top();
        data.pop();
        if(node == nullptr) continue;
        length = node->length, row = node->row, col = node->col;
        if(length + trow-row + tcol-col >= lengthLimit)
        {
            destroy_stack(node); continue;
        }
        unsigned int pos = ((unsigned int)row)*COL_SIZE + col;
        if(visited[length][pos] || automata[length][pos])
        {
            destroy_stack(node); continue;
        }
        if((row == trow && col == tcol) || iterations >= PRINT_CAP){
            bool shouldExit = false;
            if(length < currMoveLength)
            {
                destroy_stack(node);
                continue;
            }
            bestNodeSeq = node; // Solution found
            break;
        }
        visited[length++][pos] = true;
        char cIndex = 0;
        if(side){ // this makes particles tend to follow a path above the diagonal of the grid
            if (col>0) data.push(new BfsNode{node, length, row, (unsigned short)(col-1), cIndex++});
            if (row>0) data.push(new BfsNode{node, length, (unsigned short)(row-1), col, cIndex++});
            if (row<ROW_SIZE-1) data.push(new BfsNode{node, length, (unsigned short)(row+1), col, cIndex++});
            if (col<COL_SIZE-1) data.push(new BfsNode{node, length, row, (unsigned short)(col+1), cIndex++});
        }
        else{ // this makes particles tend to follow a path below the diagonal of the grid
            if (row>0) data.push(new BfsNode{node, length, (unsigned short)(row-1), col, cIndex++});
            if (col>0) data.push(new BfsNode{node, length, row, (unsigned short)(col-1), cIndex++});
            if (col<COL_SIZE-1) data.push(new BfsNode{node, length, row, (unsigned short)(col+1), cIndex++});
            if (row<ROW_SIZE-1) data.push(new BfsNode{node, length, (unsigned short)(row+1), col, cIndex++});
        }
    }
    if(bestNodeSeq == nullptr) return nullptr;
    stack<vector<unsigned short>> *v = new stack<vector<unsigned short>>();
    while(bestNodeSeq!=nullptr)
    {
        BfsNode *t1 = bestNodeSeq;
        bestNodeSeq = bestNodeSeq->parent;
        v->push({t1->row, t1->col});
        delete t1;
    }
    // Important: halts path finding and takes too much iterations
    if(iterations >= PRINT_CAP){ delete v; return nullptr; }
    return v;
}

// Recursive DFS solver (halts in around 12-16k iterations)
stack<vector<unsigned short>> *solve(unsigned length, unsigned short row, unsigned short col)
{
    if((++iterations)%PRINT_CAP == 0){
        //cout << iterations << "\tMax: " << max_row << ',' << max_col << "\tLength: " << max_length << endl;
    }
    if(length + trow-row + tcol-col >= lengthLimit)//currMoveLength)
    {
        //we can improve by summing remaining moves
        //cout << "CAP EXCEEDED" << endl;
        //return true //for interrupting as soon as reach max cap, return false to explore
        return nullptr;
    }
    unsigned int pos = ((unsigned int)row)*COL_SIZE + col;
    if(visited[length][pos]) return nullptr;
    if(automata[length][pos]) return nullptr;
    if(row+col >= max_row+max_col)
    {
        max_row = row; max_col = col;
        if(row+col > max_row+max_col || length < max_length) max_length = length;
    }
    if(row == trow && col == tcol && length > currMoveLength){
        //cout << "FOUND IT" << endl;
        //cout << "FOUND: " << length << endl;
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
        if(mv == nullptr || t->size() > mv->size()) mv = t;
    if(row>0 && (t=solve(length, row-1, col)))
        if(mv == nullptr || t->size() > mv->size()) mv = t;
    if(col>0 && (t=solve(length, row, col-1)))
        if(mv == nullptr || t->size() > mv->size()) mv = t;
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
    visited = new bitset<TOT_SIZE>[IT_LENGTH];
}

void free_mem()
{
    delete[] automata;
    delete[] visited;
}

void count_free_quant()
{
    for(int i=0;i<5000;i++)
    {
        unsigned int quant = 0;
        for(int j=0;j<TOT_SIZE;j++)
        {
            if(!automata[i][j]) quant++;;
        }
        if(i%40 == 0) cout << endl << i << ":\t";
        cout << quant << ' ';
    }
}

void test_reachability()
{
    for(int i=0;i<50000;i++)
    {
        //moves = solve(i, 0, 0);
        cout << i << '\t';
        if(moves == nullptr) cout << -1;
        else{
            cout << (moves->size()-1);
            delete moves;
        }
        cout << endl;

        currMoveLength = IT_LENGTH;
        iterations = max_row = max_col = max_length = 0;
        for(int j=i;j<IT_LENGTH;j++)
        {
            visited[j].reset();
        }
        currMoveLength = max_col = max_row = max_length = 0;
    }
}

void solve_multi_particle()
{
    int bornLimit = IT_LENGTH; //Particles cannot be "born" after this time
    stack<vector<unsigned short>> *mvs;// = solve(0, 0, 0);
    vector<unsigned short> pos, lastPos;
    int moveLength;
    int v = 1;
    string path = "challenges/sol5_5.txt";
    ofstream wf(path);
    currMoveLength = 49997; //The longest feasible path is 49998, we want it for the first iteration
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    bool side = true;
    for(int l = 0; l < bornLimit; l++)
    {
        lengthLimit = l + 50000; //used to limit depth in solve (particles cannot have more than 50k moves)
        max_col = max_row = max_length = 0;
        iterations = 0;
        for(int i=l; i<=lengthLimit; i++) visited[i].reset();
        mvs = solveStack(l, side);
        side = !side;
        cout << "Start: " << l << "\tPart: " << v << "\t=> ";
        if(mvs == nullptr)
        {
            for(int i=l; i<=lengthLimit; i++) visited[i].reset();
            mvs = solveStack(l, side);
            side = !side;
        }
        if(mvs == nullptr)
        {
            cout << "path not found";
        }
        else{
            lastPos = mvs->top();
            mvs->pop();
            moveLength = mvs->size();
            if(bornLimit == IT_LENGTH) bornLimit = l+moveLength; //Just for first iteration
            if(l+moveLength < bornLimit) //We only accept paths ending after bornLimit (it should never go here in this approach)
            {
                cout << "too short, rejected";
            }
            else{
                cout << "Reach at: " << l+moveLength << ", Length: " << moveLength << "\tLimit: " << bornLimit;
                moveLength = l+1;
                wf << l << ' ';
                while(!mvs->empty()) // Output path to file
                {
                    pos = mvs->top();
                    mvs->pop();
                    if(!mvs->empty()){
                        automata[moveLength++][((unsigned int)pos[0])*COL_SIZE + pos[1]] = true;
                    }
                    //print to output file
                    char move = 'U';
                    if(pos[0] > lastPos[0]) move = 'D';
                    else if(pos[1] > lastPos[1]) move = 'R';
                    else if(pos[1] < lastPos[1]) move = 'L';
                    wf << move << ' ';
                    lastPos = pos;
                }
                wf << '\n';
                v++;
            }
            delete mvs;
        }
        cout << endl;
    }
    wf.close();
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
    //show_automata

    cout << "Calculating states..." << endl;
    auto start = steady_clock::now();
    //memoize_iterations();
    //return 0;
    //calculate_iterations();
    if(USE_HD_MEMO)
        load_iterations(); //To load ALL from HD
    else
        calculate_iterations(); //To calculate all on RAM
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
    //test_reachability();
    //count_free_quant();
    //moves = USE_STACK ? solveStack(0) : solve(0, 0, 0);
    solve_multi_particle();
    end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;

    return 0;
    cout << "Total iterations until stop: " << iterations << endl;
    cout << "Max dist: " << max_row << "," << max_col << "\t steps: " << max_length << endl;

    stackMovements();
    if(moves) delete moves;
    free_mem();
}