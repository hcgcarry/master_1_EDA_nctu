#include <iostream>
#include <vector>
#include <sstream>
#include <limits.h>
#include <algorithm>
#include <fstream>
#include <omp.h>
//#define debug

using namespace std;

struct op
{
    int nodeI;
    int nodeJ;
    float g;
    friend ostream &operator<<(ostream &os, struct op &op_obj)
    {
        cout << "nodeI:" << op_obj.nodeI << "nodeJ:" << op_obj.nodeJ << "g:" << op_obj.g << endl;
    }
};

class KL
{
    vector<vector<float>> graph;
    vector<float> Dv;
    vector<bool> locked;
    vector<int> group;
    int netNum;
    int nodeNum;

public:
    KL(int netNum, int nodeNum)
    {
        this->netNum = netNum;
        this->nodeNum = nodeNum;
        this->Dv = vector<float>(nodeNum);
        this->graph = vector<vector<float>>(nodeNum, vector<float>(nodeNum));
        this->locked = vector<bool>(nodeNum, false);
        this->group = vector<int>(nodeNum, 0);
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
                    Dv[i] += graph[i][j];
                    Dv[j] += graph[i][j];
                }
                else
                {
                    Dv[i] -= graph[i][j];
                    Dv[j] -= graph[i][j];
                }
            }
        }
    }
    void run()
    {

        while (1)
        {

            cout << "new while iteration===============================" << endl;

#ifdef debug
            cout << "new while iteration===============================" << endl;
#endif
            computeDv();
            printDv();
            vector<struct op> opList;
            //compute gi
            for (int i = 0; i < nodeNum / 2; i++)
            {
                auto tmp = compute_g();
                opList.push_back(tmp);
//cout <<i <<" knew inner for iter======="<< endl;
#ifdef debug
                cout << i << " knew inner for iter=======" << endl;
                cout << tmp;
                printLocked();
                printDv();
#endif
            }

            float tmpGk = 0;
            float maxGk = -1 * numeric_limits<float>::max();
            int maxGkIndex = -1;
            cout << "fjdksfjsfdsjfsdkfskfjsdkf" << endl;
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
#ifdef debug
            cout << "---maxGk:" << maxGk << "maxGkIndex:" << maxGkIndex << endl;
#endif
            cout << "---maxGk:" << maxGk << "maxGkIndex:" << maxGkIndex << endl;
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
    struct op compute_g()
    {
        // nodeI ,nodeJ ,maxG
        vector<int> op;
        float maxG = -1 * numeric_limits<float>::max();
        // nodeI :group 0
        // nodeJ:group 1
        int nodeI = -1, nodeJ = -1;
        // find a pair makes the largest decrease
        #pragma omp parallel
        {
            int nthred =omp_get_num_threads();
            int id = omp_get_thread_num();
            float LmaxG = -1 * numeric_limits<float>::max();
            int LnodeI = -1, LnodeJ = -1;
            #pragma omp for
            for (int i = 0; i < nodeNum; i++)
            {
                if (group[i] == 0 && locked[i] == 0)
                {
                    for (int j = 0; j < nodeNum; j++)
                    {
                        if (group[j] == 1 && locked[j] == 0)
                        {

                            float localG = Dv[i] + Dv[j] - 2 * graph[i][j];
#ifdef debug
                            cout << "i:" << i << "J:" << j << "localG" << localG << "LmaxG" << LmaxG << endl;

#endif
                            if (localG >= LmaxG)
                            {

                                /*
                #ifdef debug
                cout << "!!!!change i:" << i << "J:" << j << "localG" << localG <<"LmaxG"<< LmaxG<< endl;
                #endif
                */
                                LmaxG = localG;
                                LnodeI = i;
                                LnodeJ = j;
                            }
                        }
                    }
                }
            }
            #pragma omp critical
            {
                    if (LmaxG >= maxG)
                    {

                        maxG = LmaxG;
                        nodeI = LnodeI ;
                        nodeJ = LnodeJ  ;
                    }
            }
        }
        locked[nodeI] = 1;
        locked[nodeJ] = 1;
        recomputeDv(nodeI);
        recomputeDv(nodeJ);
        struct op result;
        result.nodeI = nodeI;
        result.nodeJ = nodeJ;
        result.g = maxG;
        return result;
    }
    void recomputeDv(int node)
    {
        for (int i = 0; i < nodeNum; i++)
        {
            if (!locked[i])
            {
                Dv[i] += group[i] ^ group[node] ? -2 * graph[i][node] : 2 * graph[i][node];
            }
        }
    }
    void insertWeight(int nodeI, int nodeJ, float netConnectNum)
    {
        graph[nodeI][nodeJ] = 2 / netConnectNum;
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
        for (auto list : graph)
        {
            for (auto item : list)
            {
                cout << item << " ";
            }
            cout << endl;
        }
        cout << endl;
#endif
    }
    void printDv()
    {
#ifdef debug
        cout << "Dv------------" << endl;
        for (auto item : Dv)
        {
            cout << item << " ";
        }
        cout << endl;
#endif
    }
    void printGroup()
    {
#ifdef debug
        cout << "group--------" << endl;
        for (auto item : group)
        {
            cout << item << " ";
        }
        cout << endl;
#endif
    }
    void printLocked()
    {
#ifdef debug
        cout << "locked-----------" << endl;
        for (auto item : locked)
        {
            cout << item << " ";
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