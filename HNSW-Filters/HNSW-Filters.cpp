﻿#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Hnsw.h"
#include <chrono>
#include "DimFilter.h"

#define FILE_NAME "Files\\space_points_100kp_5vd.txt"
#define QFILE_NAME "Files\\query_points_100kp_5vd.txt"
#define AFILE_NAME "Files\\answer_points_j_100kp_5vd_200efc.txt"
#define UFILE_NAME "Files\\answer_points_u_100kp_5vd_200efc.txt"
#define GFILE_NAME "Files\\graph_j_100kp_5vd_200efc.txt"
#define GUFILE_NAME "Files\\graph_u_100kp_5vd_200efc.txt"

#define NUMBER_OF_GRAPH_NODES 100000
#define NUMBER_OF_QUERY_NODES 100000
#define EF_CONSTRUCTIONS 200
#define VECTOR_SIZE 5

using namespace std::chrono;
using namespace std;

vector<Node> LoadNodesFromFile(string fileName)
{
    ifstream file(fileName);
    string line = "";

    vector<Node> nodes;

    while (getline(file, line))
    {
        vector<float> values;
        //float values[VECTOR_SIZE];

        size_t pos = 0;
        string token;

        int index = 0;

        Node n = Node();

        while ((pos = line.find(' ')) != string::npos) {
            token = line.substr(0, pos);

            //n.InsertValue((float)stoi(token), index);
            //values[index] = (float)stoi(token);
            //index++;
            values.push_back(stoi(token));

            line.erase(0, pos + 1);
        }

        //n.InsertValue((float)stoi(token), index);
        //values[index] = (float)stoi(token);
        values.push_back(stoi(line));

        n.values = values;

        nodes.push_back(n);
    }

    return nodes;
}

void GeneratePoints(int numOfPoints, int numOfVectors, int minV, int maxV)
{

    srand(time(nullptr));

    ofstream MyFile("Files\\space_points_" + to_string(numOfPoints / 1000) + "kp_" + to_string(numOfVectors) + "vd.txt");

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

    int nOL = NUMBER_OF_GRAPH_NODES;

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
    vector<Node> nodes = LoadNodesFromFile(FILE_NAME);
    Hnsw hG = Hnsw(16, 16, EF_CONSTRUCTIONS);

    int c = 0;

    for (auto& n : nodes)
    {
        //if (c == 279)
        if (c == 445)
            cout << "wrong" << endl;

        hG.Insert(&n);

        //if(c == 279)
        //    hG.PrintInfoSorted(c + 1);

        //hG.PrintInfoSorted(c);
        //getchar();

        c++;

        //cout << c << endl;
    }

    hG.PrintInfoSorted(NUMBER_OF_GRAPH_NODES);
}

void HNSWSavePrint()
{
    vector<Node> nodes = LoadNodesFromFile(FILE_NAME);
    Hnsw hG = Hnsw(16, 16, EF_CONSTRUCTIONS);

    std::cout << "Start inserting\n";
    auto start = std::chrono::system_clock::now();

    for (auto& n : nodes)
    {
        hG.Insert(&n);
    }

    auto end = std::chrono::system_clock::now();
    double dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Insert time " << dur / 1000 << " [s] \n";

    hG.SavePrint(NUMBER_OF_GRAPH_NODES, GFILE_NAME);

}

void HNSWGraphAndQuerySavePrint()
{
    vector<Node> nodes = LoadNodesFromFile(FILE_NAME);
    Hnsw hG = Hnsw(16, 16, EF_CONSTRUCTIONS);

    cout << "Start inserting\n";
    auto start = std::chrono::system_clock::now();

    //int counter = 0;

    for (int i = 0; i < NUMBER_OF_GRAPH_NODES; i++)
    {
        //if (i == 2918)
        //    i += 0;

        //cout << nodes[i].values[0] << " " << nodes[i].values[69] << " " << nodes[i].values[127] << " " << endl;

        hG.Insert(&nodes[i]);
    }

    auto end = std::chrono::system_clock::now();
    double dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    cout << "Insert time " << dur / 1000.0 << " [s]" << endl << endl;

    //hG.PrintInfoSorted(NUMBER_OF_GRAPH_NODES);
    hG.SavePrint(NUMBER_OF_GRAPH_NODES, GFILE_NAME);

    int K = 10;
    vector<Node> queryNodes = LoadNodesFromFile(QFILE_NAME);
    ofstream MyFile(AFILE_NAME);
    cout << "Start querying\n";

    start = std::chrono::system_clock::now();
    for (int i = 0; i < NUMBER_OF_QUERY_NODES; i++)
    {
        vector<unsigned int> closestNodes = hG.KNNSearchIndex(&queryNodes[i], K, EF_CONSTRUCTIONS);

        //cout << K << " nearest point: ";

        string line = "";

        for (auto& p : closestNodes)
        {
            line += to_string(p) + " ";
        }
        line += "\n";

        MyFile << line;

        //cout << line << endl;

    }
    end = std::chrono::system_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    cout << "Total query time: " << dur / 1000.0 << " [s]" << endl;
    MyFile.close();
}

void SiftTest()
{
    size_t node_count = 1000000;
    size_t qsize = 10000;
    size_t vecdim = 128;
    size_t answer_size = 100;
    size_t efconstr = 200;
    uint32_t k = 10;

    float* mass = new float[node_count * vecdim];
    std::ifstream input("Data\\sift1M.bin", std::ios::binary);
    if (!input.good()) throw std::runtime_error("Input data file not opened!");
    input.read((char*)mass, node_count * vecdim * sizeof(float));
    input.close();

    vector<Node*> graphNodes;
    for (int i = 0; i < node_count; i++)
    {
        Node* graphNode = new Node();
        vector<float> position;
        //float position[VECTOR_SIZE];

        for (int p = 0; p < vecdim; p++)
        {
            position.push_back(mass[(i * vecdim) + p]);
            //position[p] = mass[(i * vecdim) + p];
        }

        graphNode->values = position;

        graphNodes.push_back(graphNode);
    }
    delete[] mass;
    cout << "Graph nodes done" << endl;

    vector<Node*> queryNodes;
    float* massQ = new float[qsize * vecdim];
    std::ifstream inputQ("Data\\siftQ1M.bin", std::ios::binary);
    if (!inputQ.good()) throw std::runtime_error("Input query file not opened!");
    inputQ.read((char*)massQ, qsize * vecdim * sizeof(float));
    inputQ.close();
    for (int i = 0; i < qsize; i++)
    {
        Node* queryNode = new Node();
        vector<float> position;

        for (int p = 0; p < vecdim; p++)
        {
            position.push_back(massQ[(i * vecdim) + p]);
        }

        queryNode->values = position;

        queryNodes.push_back(queryNode);
    }
    delete[] massQ;
    cout << "Query nodes done" << endl;

    unsigned int* massQA = new unsigned int[qsize * answer_size];
    std::ifstream inputQA("Data\\knnQA1M.bin", std::ios::binary);
    if (!inputQA.is_open()) throw std::runtime_error("Input result file not opened!");
    inputQA.read((char*)massQA, qsize * answer_size * sizeof(int));
    inputQA.close();
    cout << "Answer nodes done" << endl;

    Hnsw hnsw = Hnsw(16, 16, efconstr);

    /////////////////////////////////////////////////////// INSERT PART
    std::cout << "Start inserting\n";
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < node_count; i++)
    {
        hnsw.Insert(graphNodes[i]);
    }
    auto end = std::chrono::system_clock::now();
    double dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Insert time " << dur / 1000 << " [s] \n";

    //hnsw.PrintInfoSorted(node_count);
    //hnsw.SavePrint(node_count, "Files\\Sift\\SiftGraphJ.txt");

    /////////////////////////////////////////////////////// QUERY PART
    std::cout << "Start querying\n";
    std::vector<std::pair<float, float>> precision_time;
    for (int ef = 20; ef <= 200; ef += 10)
    {
        if (ef > 100) ef += 10;

        float positive = 0;
        for (int i = 0; i < qsize; i++)
        {
            vector<unsigned int> result = hnsw.KNNSearchIndex(queryNodes[i], k, ef);

            //int c1 = result.size();

            /*for (auto item : hnsw.W_)
            {
                result.push_back(item.node_order);

                if (c1++ >= k) break;
            }*/

            int c2 = 0;
            while (c2 < k)
            {
                if (std::find(result.begin(), result.end(), massQA[i * answer_size + c2]) != result.end())
                {
                    positive++;
                    //cout << "positive++" << endl;
                }
                c2++;
            }
        }
        std::cout << "Precision: " << positive / (qsize * k) << ", ";

        int sum = 0;
        int min_time;
        std::cout << "ef: " << ef << ", ";
        for (int i = 0; i < 3; i++)
        {
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < qsize; i++)
            {
                hnsw.KNNSearchIndex(queryNodes[i], k, ef);
            }
            auto end = std::chrono::steady_clock::now();
            int time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            sum += time;
            min_time = i == 0 ? time : std::min(min_time, time);

            //std::cout << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / qsize << " [us]; ";
        }
        std::cout << "avg: " << (float)sum / (qsize * 3) << " [us]; " << "min: " << min_time / qsize << " [us]; \n";
        precision_time.emplace_back((float)positive / (qsize * k), (float)min_time / qsize);

    }
    std::cout << "\nPrecision Time [us]\n";
    for (auto item : precision_time)
    {
        std::cout << item.first << " " << item.second << "\n";
    }

    delete[] massQA;
}

void FilterTest()
{
    Node::vectorSize = VECTOR_SIZE;

    //Filter
    //0: <50, 250> || 0
    //1: -
    //2: <10, 20> || <30, 100>
    //3: -
    //4: <0, 100> || <150, 200> 

    vector<DimFilter> filters;
    
    DimFilter f0 = DimFilter(0);
    f0.AddInterval(50, 250);
    f0.AddEqNumber(0);

    DimFilter f2 = DimFilter(2);
    f2.AddInterval(10, 20);
    f2.AddInterval(30, 100);

    DimFilter f4 = DimFilter(4);
    f4.AddInterval(0, 100);
    f4.AddInterval(150, 200);

    filters.push_back(f0);
    filters.push_back(f2);
    filters.push_back(f4);

    Node queryNode = Node();
    queryNode.values = vector<float>({0,10,20,30,40});

    cout << "Inserting:" << endl;
    Hnsw hnsw = Hnsw(16, 16, 200);  //M MMax Efc
    vector<Node> graphNodes = LoadNodesFromFile(FILE_NAME);
    //for (int i = 0; i < graphNodes.size(); i++)
    for (int i = 0; i < 30000; i++)
    {
        hnsw.Insert(&graphNodes[i]);
    }

    cout << "Serch:" << endl;
    vector<unsigned int> result = hnsw.KNNFilter(&queryNode, filters, 10, 200);

    for (auto i : result)
    {
        cout << i << ":\t";

        for (auto v : graphNodes[i].values)
        {
            cout << v << " ";
        }
        cout << endl;
    }

}

void FilterFullTest()
{
    Node::vectorSize = VECTOR_SIZE;

    cout << "Inserting:" << endl;
    Hnsw hnsw = Hnsw(16, 16, 200);  //M MMax Efc
    vector<Node> graphNodes = LoadNodesFromFile(FILE_NAME);
    //for (int i = 0; i < graphNodes.size(); i++)
    for (int i = 0; i < 30000; i++)
    {
        hnsw.Insert(&graphNodes[i]);
    }

    cout << "Serch:" << endl;

    Node queryNode = Node();
    queryNode.values = vector<float>({ 0,10,20,30,40 });

    for (int i = 0; i < 10; i++)
    {
        cout << "Test " << i << endl;

        vector<DimFilter> filters = DimFilterHelper::GenerateFilter(Node::vectorSize, 0.5, 0, 250);

        vector<unsigned int> result = hnsw.KNNFilter(&queryNode, filters, 10, 200);

        for (auto i : result)
        {
            cout << i << ":\t";

            for (auto v : graphNodes[i].values)
            {
                cout << v << " ";
            }
            cout << endl;
        }

        cout << endl;
    }
}

int main()
{
    //GeneratePoints(100000, 5, 0, 255);   //numberOfNodes, vecdim, minV, maxV
    //
    //HNSWPrint();
    //HNSWSavePrint();
    //
    //DistinctNodes(FILE_NAME);
    //SiftTest();
    //CompareFiles("Files\\Sift\\SiftGraphJ.txt", "Files\\Sift\\SiftGraphU.txt");

    //HNSWGraphAndQuerySavePrint();
    //CompareFiles(GFILE_NAME, GUFILE_NAME);
    //CompareFiles(AFILE_NAME, UFILE_NAME);

    //FilterTest();
    FilterFullTest();

    return 0;
}
