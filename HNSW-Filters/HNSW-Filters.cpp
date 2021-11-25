#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Hnsw.h"
#include <chrono>

#define FILE_NAME "Files\\space_points_1000.txt"
#define QFILE_NAME "Files\\query_points_1000.txt"
#define AFILE_NAME "Files\\answer_points_j_1000.txt"
#define UFILE_NAME "Files\\answer_points_u_1000.txt"
#define GFILE_NAME "Files\\graph_j_1000_n.txt"
#define GUFILE_NAME "Files\\graph_u_1000.txt"
#define QUERY_POINT "16 8943 561 84 651"
#define QUERY_POINT_DEFAULT "16 8943 561 84 651"

using namespace std::chrono;
using namespace std;

vector<Node*> LoadNodesFromFile(string fileName)
{
    ifstream file(fileName);
    string line = "";

    vector<Node*> nodes;

    while (getline(file, line))
    {
        vector<long> values;

        size_t pos = 0;
        string token;

        while ((pos = line.find(' ')) != string::npos) {
            token = line.substr(0, pos);

            values.push_back(stoi(token));

            line.erase(0, pos + 1);
        }

        values.push_back(stoi(line));

        Node* n = new Node();
        n->values = values;

        nodes.push_back(n);
    }

    return nodes;
}

Node GetQueryNode()
{
    Node queryNode = Node();

    string values = QUERY_POINT;

    size_t pos = 0;
    string token;

    while ((pos = values.find(' ')) != string::npos) {
        token = values.substr(0, pos);

        queryNode.InsertValue(stoi(token));

        values.erase(0, pos + 1);
    }

    queryNode.InsertValue(stoi(values));

    return queryNode;
}

void GeneratePoints(int numOfPoints, int numOfVectors, int minV, int maxV)
{

    srand(time(nullptr));

    ofstream MyFile(QFILE_NAME);

    for (int i = 0; i < numOfPoints; i++)
    {
        string point = "";

        for (int j = 0; j < numOfVectors; j++)
        {
            int vector = minV + rand() % (maxV - minV + 1);
            point += to_string(vector);

            if (j < numOfVectors - 1)
                point += " ";
        }

        point += "\n";

        MyFile << point;
    }

    MyFile.close();

}

void HNSW()
{
    vector<Node*> nodes = LoadNodesFromFile(FILE_NAME);

    Hnsw hG = Hnsw(16, 16, 16);  //efc 16 - 9.7s, efc 200 - 150.6s


    auto start = high_resolution_clock::now();

    for (auto& n : nodes)
    {
        hG.Insert(n);
    }

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    cout << "\nCas vkladani prvku: " << duration.count() / 1000000.0 << " [s]" << endl;


    Node queryNode = GetQueryNode();
    int K = 3;

    start = high_resolution_clock::now();
    vector<Node*> closestNodes = hG.KNNSearch(queryNode, K);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    cout << "Cas vyhledavani K prvku: " << duration.count() / 1000000.0 << " [s]" << endl;

    cout << K << " nearest point:\n";

    for (auto& n : closestNodes)
    {
        cout << "\t";
        for (auto& v : n->values)
            cout << v << " ";

        n->SetDistance(queryNode);

        cout << "\tdist: " << n->distance << endl;
    }

    cout << endl << endl;

    K = 10;
    start = high_resolution_clock::now();
    closestNodes = hG.KNNSearch(queryNode, K);
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    cout << "Cas vyhledavani K prvku: " << duration.count() / 1000000.0 << " [s]" << endl;

    cout << K << " nearest point:\n";

    for (auto& n : closestNodes)
    {
        cout << "\t";
        for (auto& v : n->values)
            cout << v << " ";

        n->SetDistance(queryNode);

        cout << "\tdist: " << n->distance << endl;
    }

    cout << endl << endl;
}

void HNSWQueryTest()
{
    vector<Node*> nodes = LoadNodesFromFile(FILE_NAME);
    vector<Node*> qNodes = LoadNodesFromFile(QFILE_NAME);    
    Hnsw hG = Hnsw(16,16,16);  //efc 16 - 9.7s, efc 200 - 150.6s

    auto start = high_resolution_clock::now();

    int i = 0;

    for (auto& n : nodes)
    {
        //if (i == 16)
          //  cout << "x" << endl;//getchar();

        hG.Insert(n);

        //hG.PrintInfoSorted(++i);
        //getchar();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "\nCas vkladani prvku: " << duration.count() / 1000000.0 << " [s]" << endl;

    int K = 10;
    int nOQP = 1000; //počet query bodů

    ofstream MyFile(AFILE_NAME);

    for (int i = 0; i < nOQP; i++)
    {
        //start = high_resolution_clock::now();
        vector<int> closestNodes = hG.KNNSearchIndex(*qNodes[i], K);
        //stop = high_resolution_clock::now();
        //duration = duration_cast<microseconds>(stop - start);
        //cout << "Cas vyhledavani K prvku: " << duration.count() / 1000000.0 << " [s]" << endl;

        cout << K << " nearest point: ";

        string line = "";

        for (auto& p : closestNodes)
        {
            line += to_string(p) + " ";
        }
        line += "\n";

        MyFile << line;
        cout << line;
    }    

    MyFile.close();
}

void DistinctNodes(string f1)
{

    ifstream file1(f1);
    string line1 = "";

    int i = 0;

    do
    {
        ifstream file2(f1);
        string line2 = "";

        getline(file1, line1);
        int c = 0;

        do
        {
            getline(file2, line2);

            if (line1 == line2)
                c++;


        } while (line2 != "");

        if (c > 1)
            cout << "Shoda: " << c << endl;

        if (i % 1000 == 0)
            cout << i << endl;

        i++;

    } while (line1 != "");

}

void CompareFiles(string f1, string f2)
{
    int rL = 0;
    int wL = 0;

    ifstream file1(f1);
    ifstream file2(f2);

    string line1 = "";
    string line2 = "";

    int nOL = 1000;

    for (int i = 0; i < nOL; i++)
    {
        getline(file1, line1);
        getline(file2, line2);

        if (line1 == line2)
        {
            rL++;
        }
        else
        {
            wL++;

            cout << "l " << i + 1 << ":\n";
            cout << line1 << endl;
            cout << line2 << endl << endl;
        }
    }

    cout << endl;

    cout << "shoda: " << rL << ", " << " neshoda: " << wL << endl;
}

void HNSWPrint()
{
    vector<Node*> nodes = LoadNodesFromFile(FILE_NAME);
    Hnsw hG = Hnsw(16, 16, 16);

    int c = 0;

    for (auto& n : nodes)
    {
        if (c == 279)
            cout << "wrong" << endl;

        hG.Insert(n);

        if(c == 279)
            hG.PrintInfoSorted(c + 1);

        //hG.PrintInfoSorted(c);
        //getchar();
        c++;

        cout << c << endl;
    }

    hG.PrintInfoSorted(1000);
 }

void HNSWSavePrint()
{
    vector<Node*> nodes = LoadNodesFromFile(FILE_NAME);
    Hnsw hG = Hnsw(16, 16, 16);

    for (auto& n : nodes)
    {
        hG.Insert(n);
    }

    hG.SavePrint(1000,GFILE_NAME);
}

int main()
{
    //GeneratePoints(1000, 5, 0, 1000); 
    //HNSW();
    //HNSWQueryTest();
    HNSWPrint();
    //CompareFiles(AFILE_NAME, UFILE_NAME);
    //HNSWSavePrint();
    //CompareFiles(GFILE_NAME, GUFILE_NAME);
    //DistinctNodes(FILE_NAME);

    return 0;
}
