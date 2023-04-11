#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <chrono>
#include <bitset>
using namespace std;
using namespace std::chrono;

#define ROW_SIZE 1654
#define COL_SIZE 2000
#define TOT_SIZE 413500
#define IT_LENGTH 1000

#define PRINT_CAP 1000000

bool ***automata;
bool ***visited;

short trow = ROW_SIZE-1, tcol = COL_SIZE-1;
unsigned int iterations;

//For best distance
unsigned short max_row = 0, max_col = 0, max_length = 0;

stack<vector<unsigned short>> moves;

void fill_start()
{
    std::ifstream input("challenges/input1.txt");
    if(!input.is_open())
    {
        cout << "Error opening file" << endl;
        exit(1);
    }
    int v;
    for(int i=0;i<ROW_SIZE;i++){
        for(int j=0;j<COL_SIZE;j++){
            input >> v;
            automata[0][i][j] = v>0;
        }
    }
    automata[0][0][0] = automata[0][ROW_SIZE-1][COL_SIZE-1] = false;
    input.close();
}

void write_state(string path, bool **data)
{
    ofstream wf(path, ios::out || ios::binary);
    if(!wf){
        cout << "Error opening file\t Aborting" << endl;
        exit(1);
    }
    int i, j, count = 0;
    int size = ROW_SIZE*COL_SIZE;
    bitset<TOT_SIZE> buffer;
    for(i=0;i<ROW_SIZE;i++) for(j=0;j<COL_SIZE;j++)
    {
        buffer[count++] = data[i][j];
    }
    wf.write((char *) &buffer, TOT_SIZE/8);
    wf.close();
}

short calc_rank(int i, int j, int k)
{
    short rank = 0;
    if(j > 0){
        if(automata[i][j-1][k]) rank++;
        if(k>0 && automata[i][j-1][k-1]) rank++;
        if(k<COL_SIZE-1 && automata[i][j-1][k+1]) rank++;
    }
    if(j < ROW_SIZE-1){
        if(automata[i][j+1][k]) rank++;
        if(k>0 && automata[i][j+1][k-1]) rank++;
        if(k<COL_SIZE-1 && automata[i][j+1][k+1]) rank++;
    }
    if(k>0 && automata[i][j][k-1]) rank++;
    if(k<COL_SIZE-1 && automata[i][j][k+1]) rank++;
    return rank;
}

void calculate_iterations()
{
    int iprev;
    for(int i=1;i<IT_LENGTH;i++)
    {
        iprev = i-1;
        for(int j=0;j<ROW_SIZE;j++)
        {
            for(int k=0;k<COL_SIZE;k++)
            {
                visited[i][j][k] = false;
                short rank = calc_rank(iprev, j, k);
                if(!automata[iprev][j][k])
                    automata[i][j][k] = (rank > 1 && rank < 5);
                else
                    automata[i][j][k] = (rank > 3 && rank < 6);
            }
        }
        automata[i][0][0] = automata[i][ROW_SIZE-1][COL_SIZE-1] = false;
    }
}


bool solve(unsigned length, unsigned short row, unsigned short col)
{
    if((++iterations)%PRINT_CAP == 0){
        //printf("Heey\n");
        cout << iterations << '\t' << length << '\t' << row << '\t' << col << "\tMax: " << max_row << ',' << max_col << endl;
    }
    //cout << length << '\t' << row << '\t' << col << endl;
    if(length > IT_LENGTH-1)
    {
        //cout << "CAP EXCEEDED" << endl;
        //return true //for interrupting as soon as reach max cap, return false to explore
        return false;
    }
    if(visited[length][row][col]) return false;
    if(automata[length][row][col]) return false;
    if(row+col > max_row+max_col)
    {
        max_row = row; max_col = col; max_length = length;
    }
    if(row == trow && col == tcol){
        cout << "FOUND IT" << endl;
        moves.push({ row, col });
        return true;
    }
    visited[length++][row][col] = true;
    if((row<ROW_SIZE-1 && solve(length, row+1, col)) ||
        (col<COL_SIZE-1 && solve(length, row, col+1)) ||
        (row>0 && solve(length, row-1, col)) ||
        (col>0 && solve(length, row, col-1))){
        moves.push({ row, col });
        return true;
    }
    return false;
}

void show_automata(int length)
{
    for(int i=0;i<ROW_SIZE-1;i++){
        for(int j=0;j<COL_SIZE-1;j++)
        {
            cout << automata[length][i][j] ? 1 : 0;
        }
        cout << endl;
    }
}

void stackPositions()
{
    while(!moves.empty())
    {
        vector<unsigned short> pos = moves.top();
        moves.pop();
        cout << pos[0] << '\t' << pos[1] << endl;
    }
}

void stackMovements()
{
    if(moves.empty())
    {
        cout << "No solution found" << endl;
        return;
    }
    vector<unsigned short> lastPos = moves.top();
    moves.pop();
    while(!moves.empty())
    {
        vector<unsigned short> pos = moves.top();
        moves.pop();
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
    int i,j;
    automata = new bool**[IT_LENGTH];
    visited = new bool**[IT_LENGTH];
    for(i=0;i<IT_LENGTH;i++)
    {
        automata[i] = new bool*[ROW_SIZE];
        visited[i] = new bool*[ROW_SIZE];
        for(j=0;j<ROW_SIZE;j++)
        {
            automata[i][j] = new bool[COL_SIZE];
            visited[i][j] = new bool[COL_SIZE];
        }
    }
}

void free_mem()
{
    int i,j;
    for(i=0;i<IT_LENGTH;i++)
    {
        for(j=0;j<ROW_SIZE;j++)
        {
            delete[] automata[i][j];
            delete[] visited[i][j];
        }
        delete[] automata[i];
        delete[] visited[i];
    }
    delete[] automata;
    delete[] visited;
}

int main() {
    cout << "Memory allocation...";
    init_mem();
    cout << " Complete" << endl;
    cout << "................................................................." << endl;
    int i,j,k;
    visited[0][0][0] = false;
    cout << "States precomputing...";
    fill_start();
    cout << " Complete" << endl;
    //show_automata

    cout << "Calculating states..." << endl;
    auto start = steady_clock::now();
    calculate_iterations();
    auto end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    
    //show_automata(0); cout << endl;
    //show_automata(1); cout << endl;
    //show_automata(2); cout << endl;
    //show_automata(3); cout << endl;
    //show_automata(4); cout << endl;
    //show_automata(65); cout << endl;

    cout << "Solving..." << endl;
    start = steady_clock::now();
    iterations = 0;
    solve(0, 0, 0);
    end = steady_clock::now();
    cout << "Elapsed time in seconds: " << duration_cast<chrono::seconds>(end - start).count() << " sec" << endl;
    cout << "Total iterations until stop: " << iterations << endl;
    cout << "Max dist: " << max_row << "," << max_col << "\t steps: " << max_length << endl;

    stackMovements();
    free_mem();
}