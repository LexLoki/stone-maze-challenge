#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <fstream>
#include <chrono>
#include <bitset>
using namespace std;
using namespace std::chrono;

#define TAM 10

string info[10] = {
"1111111111",
"1000001001",
"1011xxx101",
"1110x1x100",
"1011x0x110",
"1001xx0100",
"1101x11110",
"1000000010",
"1010110110",
"1111100000"
};

/*
string info[10] = {
    "xxxxxx1xxx",
    "x0xxx01xx1",
    "xxx1xxxx0x",
    "x11xx1x1xx",
    "1xxxx0xx1x",
    "xxx1xx010x",
    "x1xxxx11xx",
    "1xxxxx001x",
    "xx1xx1x11x",
    "x1xx1xxxxx"
};*/

char mat[TAM][TAM];
char labels[TAM][TAM];

char getRank(char i, char j)
{
    char r = 0;
    if(i>0 && mat[i-1][j]>=0) r += mat[i-1][j] ? 1 : -1;
    if(i<TAM-1 && mat[i+1][j]>=0) r += mat[i+1][j] ? 1 : -1;
    if(j>0 && mat[i][j-1]>=0) r += mat[i][j-1] ? 1 : -1;
    if(j<TAM-1 && mat[i][j+1]>=0) r += mat[i][j+1] ? 1 : -1;
    return r;
}

bool checkBox(char i, char j, char v)
{
    return !(mat[i][j] == v && mat[i+1][j] == v && mat[i][j+1] == v && mat[i+1][j+1] == v);
}

bool checkBoxRule(char i, char j)
{
    char v = mat[i][j];
    char other = (v+1)%2;
    bool hasNeighbour = false;
    if(v<0) return true;
    if(i>0)
    {
        hasNeighbour = hasNeighbour || (mat[i-1][j] != other);
        if(j>0 && !checkBox(i-1, j-1, v)) return false;
        if(j<TAM-1 && !checkBox(i-1, j, v)) return false;
    }
    if(i<TAM-1)
    {
        hasNeighbour = hasNeighbour || (mat[i+1][j] != other);
        if(j>0 && !checkBox(i, j-1, v)) return false;
        if(j<TAM-1 && !checkBox(i, j, v)) return false;
    }
    if(j>0) hasNeighbour = hasNeighbour || (mat[i][j-1] != other);
    if(j<TAM-1) hasNeighbour = hasNeighbour || (mat[i][j+1] != other);
    return hasNeighbour;
}

bool checkAllBoxRule()
{
    int i,j;
    for(i=0;i<TAM-1;i++)
    {
        for(j=0;j<TAM-1;j++)
        {
            char v = mat[i][j];
            if(v >= 0 && v == mat[i+1][j] && v == mat[i][j+1] && v == mat[i+1][j+1]) return false;
        }
    }
    return true;
}

void labelBFS(char i, char j, char label)
{
    labels[i][j] = label;
    if(i<TAM-1 && mat[i+1][j] == label && labels[i+1][j]<0) labelBFS(i+1, j, label);
    if(i>0 && mat[i-1][j] == label && labels[i-1][j]<0) labelBFS(i-1, j, label);
    if(j<TAM-1 && mat[i][j+1] == label && labels[i][j+1]<0) labelBFS(i, j+1, label);
    if(j>0 && mat[i][j-1] == label && labels[i][j-1]<0) labelBFS(i, j-1, label);
}

bool checkConnectedComponent()
{
    int i,j;
    for(i=0;i<TAM;i++) for(j=0;j<TAM;j++) labels[i][j] = -1;
    bool did[2] = {false, false};
    for(i=0;i<TAM;i++) for(j=0;j<TAM;j++)
    {
        if(labels[i][j]>=0) continue;
        char color = mat[i][j];
        if(did[color]) return false;
        labelBFS(i, j, color);
        did[color] = true;
    }
    return true;
}

bool solve(char i, char j){
    //cout << (int)i << '\t' << (int)j << endl;
    if(i==TAM) return checkConnectedComponent();
    char ni = i+(j+1)/TAM, nj = (j+1)%TAM;
    if(mat[i][j]>=0) return solve(ni, nj);
    //cout << "\tTry boxes" << endl;
    char tryValue = getRank(i,j)>0 ? 1 : 0;
    mat[i][j] = tryValue;
    if(checkBoxRule(i, j) && solve(ni, nj)) return true;
    //cout << "\tTry second" << endl;
    mat[i][j] = (tryValue+1)%2;
    if(checkBoxRule(i, j) && solve(ni, nj)) return true;
    mat[i][j] = -1;
    return false;
}

void showMatrix()
{
    int i,j;
    for(i=0;i<TAM;i++)
    {
        for(j=0;j<TAM;j++) cout << (int)mat[i][j] << '\t';
        cout << '\n';
    }
    cout << endl;
}

int main(void)
{
    int i,j;
    for(i=0;i<TAM;i++)
    {
        for(j=0;j<TAM;j++)
        {
            char c = info[i][j];
            mat[i][j] = c == 'x' ? -1 : (c == '0' ? 0 : 1);
            //mat[i][j] = c == 'x';
        }
    }
    showMatrix();
    cout << solve(0,0) << endl;
    showMatrix();
    return 0;
}