#include <iostream>
#include <vector>
#include <sstream>
#include <limits.h>
#include <algorithm>
#include <fstream>
#include <omp.h>
#include"kernel.h"
#include"op.h"

//#define debug

using namespace std;


class KL
{
    float* graph;
    float* Dv;
    int* locked;
    int* group;
    int netNum;
    int nodeNum;

public:
    KL(int netNum, int nodeNum)
    {
        this->netNum = netNum;
        this->nodeNum = nodeNum;
        this->Dv = (float*)malloc(nodeNum*sizeof(float));
        this->graph = (float*)malloc(nodeNum*nodeNum*sizeof(float));
        this->locked = (int*)malloc(nodeNum*sizeof(int));
        this->group = (int*)malloc(nodeNum*sizeof(int));

        for (int i = 0; i <= nodeNum / 2; i++)
        {
            this->group[i] = 1;
        }
    }
    void computeDv()
    {
        for (int i = 0; i < nodeNum; i++)
        {
            Dv[i] = 0;
        }
        for (int i = 0; i < nodeNum; i++)
        {
            for (int j = i + 1; j < nodeNum; j++)
            {
                if (group[i] ^ group[j])
                {
                    Dv[i] += graph[i*nodeNum+j];
                    Dv[j] += graph[i*nodeNum+j];
                }
                else
                {
                    Dv[i] -= graph[i*nodeNum+j];
                    Dv[j] -= graph[i*nodeNum+j];
                }
            }
        }
    }
    void run()
    {

        while (1)
        {


            cout << "new while iteration===============================" << endl;
            computeDv();
            printDv();
            vector<struct op> opList;
            //compute gi
            for (int i = 0; i < nodeNum / 2; i++)
            {
#ifdef debug
            cout << i << " new inner for iter=======" << endl;
#endif
            cout << i << " new inner for iter=======" << endl;
                auto tmp = compute_g();
                opList.push_back(*tmp);
                cout <<"result: " <<"nodeI "<< tmp->nodeI << "nodeJ" << tmp->nodeJ << "g:" <<tmp->g<< endl;
#ifdef debug
                cout <<"result: " <<"nodeI "<< tmp->nodeI << "nodeJ" << tmp->nodeJ << "g:" <<tmp->g<< endl;
                printLocked();
                printDv();
#endif
            }

            float tmpGk = 0;
            float maxGk = -1 * numeric_limits<float>::max();
            int maxGkIndex = -1;
            //find k such that Gk is max
            for (int i = 0; i < opList.size(); i++)
            {
                tmpGk += opList[i].g;
                if (tmpGk > maxGk)
                {
                    maxGk = tmpGk;
                    maxGkIndex = i;
                }
            }
            cout << "---------------maxGk:" << maxGk << " maxGkIndex:" << maxGkIndex << endl;
            if (maxGk <= 0)
                return;
            // change the group
            else
            {
                for (int i = 0; i <= maxGkIndex; i++)
                {
                    group[opList[i].nodeI] = !group[opList[i].nodeI];
                    group[opList[i].nodeJ] = !group[opList[i].nodeJ];
                }
            }
            printGroup();
            // unlock
            for (int i = 0; i < nodeNum; i++)
            {
                locked[i] = 0;
            }
        }
    }
    // compteMaxG , lock ,recompute  Dv
    struct op* compute_g()
    {
        // nodeI ,nodeJ ,maxG
        vector<int> op;
        float maxG = -1 * numeric_limits<float>::max();
        // nodeI :group 0
        // nodeJ:group 1
        int nodeI = -1, nodeJ = -1;
        // find a pair makes the largest decrease
        struct op* result;
        result = hostFE(nodeNum,graph,Dv,locked,group);
        #ifdef debug
        cout <<"compute_g------" << endl;
        cout << "hostFE result" << "g: " << result->g << "nodeI: " << result->nodeI << "nodeJ: " << result->nodeJ << endl;
        #endif
        // postprocessing
        locked[result->nodeI] = 1;
        locked[result->nodeJ] = 1;
        recomputeDv(result->nodeI);
        recomputeDv(result->nodeJ);
        return result;
    }
    void recomputeDv(int node)
    {
        for (int i = 0; i < nodeNum; i++)
        {
            if (!locked[i])
            {
                Dv[i] += group[i] ^ group[node] ? -2 * graph[i*nodeNum+node] : 2 * graph[i*nodeNum+node];
            }
        }
    }
    void insertWeight(int nodeI, int nodeJ, float netConnectNum)
    {
        graph[nodeI*nodeNum+nodeJ] = 2 / netConnectNum;
    }
    friend ostream &operator<<(ostream &, const KL &kl_obj);
    void printResult()
    {
        fstream result_file("output.txt", ios::out);
        for (int i = 0; i < nodeNum; i++)
        {
            result_file << group[i] << endl;
        }
        result_file.close();
    }
    void printGraph()
    {
#ifdef debug
        cout << "graph--------" << endl;
        cout << "locked------------" << endl;
        for (int i=0;i<nodeNum;i++)
        {
            for (int j=0;j<nodeNum;j++) cout << graph[i*nodeNum+j] << " ";
            cout << endl;
        }
        cout << endl;
#endif
    }
    void printDv()
    {
#ifdef debug
        cout << "Dv------------" << endl;
        for (int i=0;i<nodeNum;i++)
        {
            cout << Dv[i] << " ";
        }
        cout << endl;
#endif
    }
    void printGroup()
    {
#ifdef debug
        cout << "group------------" << endl;
        for (int i=0;i<nodeNum;i++)
        {
            cout << group[i] << " ";
        }
        cout << endl;
#endif
    }
    void printLocked()
    {
#ifdef debug
        cout << "locked------------" << endl;
        for (int i=0;i<nodeNum;i++)
        {
            cout << locked[i] << " ";
        }
        cout << endl;
#endif
    }
    void printInfo()
    {
#ifdef debug
        cout << "net" << netNum << "nodeNum : " << nodeNum << endl;
#endif
    }
};

int main(int argc, char *argv[])
{
    freopen(argv[1], "r", stdin);
    int netNum, nodeNum;
    // handle input
    cin >> netNum >> nodeNum;
    cin.ignore();
    KL KL_obj(netNum, nodeNum);
    while (netNum--)
    {
        string line;
        getline(cin, line);
        stringstream ss(line);
        // get nodes which edge connect to
        string tmp;
        vector<int> nodeList;
        while (ss >> tmp)
        {
            nodeList.push_back(stoi(tmp));
        }
        for (auto item1 : nodeList)
        {
            for (auto item2 : nodeList)
            {
                if (item1 != item2)
                    KL_obj.insertWeight(item1 - 1, item2 - 1, nodeList.size());
            }
        }
    }
    cout << "before run" << endl;
    KL_obj.printGraph();
    KL_obj.printGroup();
    KL_obj.printLocked();
    KL_obj.printDv();
    KL_obj.run();
    cout << "after run" << endl;
    KL_obj.printResult();
    KL_obj.printGroup();
    KL_obj.printLocked();
    KL_obj.printDv();
}