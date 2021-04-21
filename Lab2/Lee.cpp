#include <iostream>
#include <vector>
#include<algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <queue>
#include <fstream>

using namespace std;
struct location
{
    int x;
    int y;
    location() = delete;
    location(int x, int y) : x(x), y(y) {}
    friend ostream &operator<<(ostream &os, struct location &locationObj)
    {
        cout << "x:" << locationObj.x << " y:" << locationObj.y;
        return os;
    }
};

struct netInfo
{
    int id;
    int source_x, source_y, target_x, target_y;
    vector<struct location> netPath;
    netInfo() = delete;
    netInfo(int id, int source_x, int source_y, int target_x, int target_y) : id(id), source_x(source_x), source_y(source_y), target_x(target_x), target_y(target_y)
    {
    }
    bool isTarget(int x, int y)
    {
        return (target_x == x && target_y == y);
    }
    bool isSource(int x, int y)
    {
        return (source_x == x && source_y == y);
    }
    friend ostream &operator<<(ostream &os, struct netInfo &netInfoObj)
    {
        cout << "-------------net Info------------" << endl;
        cout << "Id: " << netInfoObj.id << " source_x: " << netInfoObj.source_x << " source_y: " << netInfoObj.source_y;
        cout << " target_x: " << netInfoObj.target_x << " target_y: " << netInfoObj.target_y << endl;
        cout << "net Path" << endl;
        for (auto item : netInfoObj.netPath)
        {
            cout << item << " ,";
        }
        cout << endl;
        cout << "-------------net Info end------------" << endl;
        return os;
    }
    void printResult(ofstream &fs)
    {
        fs << "Net" << id << endl;
        fs << "begin" << endl;
        fs << netPath.size()-2<< endl;
        removeDuplicateAndPrintPath(fs);
        fs << "end" << endl;
    }
    void removeDuplicateAndPrintPath(ofstream &fs)
    {
        //struct location previousLocation(source_x,source_y);
        int i = 0;
        for (int i = 0; i < netPath.size(); i++)
        {
            if (i == 0)
                fs << netPath[i].x << " " << netPath[i].y << " ";
            else if (i + 1 == netPath.size())
            {
                fs << netPath[i].x << " " << netPath[i].y << endl;
            }
            else if (i + 1 < netPath.size() && ((netPath[i].x == netPath[i - 1].x && netPath[i].y == netPath[i + 1].y) ||
                                                (netPath[i].y == netPath[i - 1].y && netPath[i].x == netPath[i + 1].x)))
            {
                fs << netPath[i].x << " " << netPath[i].y << endl;
                fs << netPath[i].x << " " << netPath[i].y << " ";
            }
        }
    }
};
class Graph
{
public:
    vector<vector<int>> blockages_graph;
    vector<vector<int>> netGraph;
    vector<struct netInfo> netInfoList;
    int row;
    int col;
    Graph()
    {
        buildGraph();
        #ifdef debug
        printGraph();
        printNetInfoList();
        #endif
    }
    void printNetInfoList()
    {
        for (auto &net : netInfoList)
        {
            cout << net;
        }
    }
    void ripeUp(int netId)
    {
        for (auto location : netInfoList[netId - 1].netPath)
        {
            netGraph[location.y][location.x] = 0;
        }
        netInfoList[netId - 1].netPath.clear();
    }
    void printGraph()
    {
        cout << "----------graph---------" << endl;
        cout << setw(4) << " ";
        for (int i = 0; i < col; i++)
        {
            cout << setw(4) << i;
        }
        cout << endl;
        for (int i = row - 1; i >= 0; i--)
        {
            cout << setw(4) << i;
            for (int j = 0; j < col; j++)
            {
                string tmp;
                if (blockages_graph[i][j] == 1)
                    tmp += "b";
                if (netGraph[i][j] != 0)
                    tmp += to_string(netGraph[i][j]);
                if (tmp == "")
                    tmp = "0";
                cout << setw(4) << tmp;
            }
            cout << endl;
        }
        cout << "----------graph end---------" << endl;
    }
    int GridIsBlock(struct location &curlocation)
    {
        return blockages_graph[curlocation.y][curlocation.x] || netGraph[curlocation.y][curlocation.x];
    }
    void buildGraph()
    {
        string tmp;
        cin >> tmp >> this->row;
        cin >> tmp >> this->col;
        blockages_graph = vector<vector<int>>(this->row, vector<int>(this->col));
        netGraph = vector<vector<int>>(this->row, vector<int>(this->col));
        int blockNum;
        cin >> tmp >> blockNum;
        while (blockNum--)
        {
            int left_down_x, left_down_y, right_up_x, right_up_y;
            cin >> left_down_x >> left_down_y >> right_up_x >> right_up_y;
            for (int i = left_down_x; i <= right_up_x; i++)
            {
                for (int j = left_down_y; j <= right_up_y; j++)
                {
                    blockages_graph[j][i] = 1;
                }
            }
        }
        int source_x, source_y, target_x, target_y;
        int netNum;
        cin >> tmp >> netNum;
        for (int i = 1; i <= netNum; i++)
        {
            cin >> tmp >> source_x >> source_y >> target_x >> target_y;
            blockages_graph[source_y][source_x] = 1;
            blockages_graph[target_y][target_x] = 1;
            struct netInfo netInfoTmp(i, source_x, source_y, target_x, target_y);
            netInfoList.push_back(netInfoTmp);
        }
    }
    void printResult(ofstream &fs)
    {
        for (auto &net : netInfoList)
        {
            net.printResult(fs);
        }
    }
};
class LeeAlgo
{
public:
    Graph GraphObj;
    LeeAlgo()
    {
    }
    void run()
    {
        cout << "================== initial end============" << endl;
        findPath();
    };
    void findPath()
    {
        while(1){
            bool finish = true;
            for (auto &net : GraphObj.netInfoList)
            {
                if(net.netPath.empty()){
                    finish = false;
                    singleNetfindPath(net);
                }
            }
            if(finish == true){
                break;
            }
        }
    }
    void printMatirx(vector<vector<int>> &matrix)
    {
        cout << "------------mark number matrix-----" << endl;
        cout << setw(4) << " ";
        int col = matrix[0].size();
        int row = matrix.size();
        for (int i = 0; i < col; i++)
        {
            cout << setw(4) << i;
        }
        cout << endl;
        for (int i = row - 1; i >= 0; i--)
        {
            cout << setw(4) << i;
            for (int j = 0; j < col; j++)
            {
                string tmp;
                if (GraphObj.blockages_graph[i][j] == 1)
                    tmp += "b";
                if (GraphObj.netGraph[i][j] != 0)
                    tmp += to_string(GraphObj.netGraph[i][j]);
                if (matrix[i][j])
                    tmp += "w" + to_string(matrix[i][j]);
                cout << setw(4) << tmp;
            }
            cout << endl;
        }
        cout << "------------mark number matrix end-----" << endl;
    }
    void singleNetfindPath(struct netInfo &curNetInfo)
    {
        cout << "-------------- singleNetfindPath-------------" << endl;
        vector<vector<int>> dict{{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
        while (1)
        {
            queue<struct location> bfsQue;
            bfsQue.push(location(curNetInfo.source_x, curNetInfo.source_y));
            vector<vector<int>> markNumberGraph(GraphObj.row, vector<int>(GraphObj.col));

            vector<int> adjNetList;
            int level = 0;
            bool findPath = false;
            while (!bfsQue.empty() && !findPath)
            {
                int levelSize = bfsQue.size();
                level++;
                while (levelSize-- && !findPath)
                {
                    struct location curLocation = bfsQue.front();
                    bfsQue.pop();
                    for (int i = 0; i < dict.size(); i++)
                    {
                        struct location adjLocation(curLocation.x + dict[i][0], curLocation.y + dict[i][1]);
                        if (adjLocation.x < 0 || adjLocation.x >= GraphObj.col || adjLocation.y < 0 || adjLocation.y >= GraphObj.row)
                            continue;
                        if (curNetInfo.isTarget(adjLocation.x, adjLocation.y))
                        {
                            cout << "!!!!!!!!!!!!!!!!!target find\n"
                                 << adjLocation << endl;
                            findPath = true;
                            break;
                        }
                        else if (GraphObj.GridIsBlock(adjLocation) || (markNumberGraph[adjLocation.y][adjLocation.x] != 0))
                        {
                            if (GraphObj.netGraph[adjLocation.y][adjLocation.x])
                            {
                                adjNetList.push_back(GraphObj.netGraph[adjLocation.y][adjLocation.x]);
                            }
                            continue;
                        }
                        else
                        {
                            markNumberGraph[adjLocation.y][adjLocation.x] = level;
                        }
                        bfsQue.push(adjLocation);
                    }
                }
                #ifdef debug
                printMatirx(markNumberGraph);
                #endif
            }

            // reverse find path
            if (findPath)
            {
                curNetInfo.netPath.push_back({curNetInfo.source_x, curNetInfo.source_y});
                struct location curLocation(curNetInfo.target_x, curNetInfo.target_y);
                bool sourceFind = false;
                while (!sourceFind)
                {
                    for (int i = 0; i < dict.size(); i++)
                    {
                        struct location adjLocation(curLocation.x + dict[i][0], curLocation.y + dict[i][1]);
                        if (adjLocation.x < 0 || adjLocation.x >= GraphObj.col || adjLocation.y < 0 || adjLocation.y >= GraphObj.row)
                            continue;
                        if (curNetInfo.isSource(adjLocation.x, adjLocation.y))
                        {
                            printf("!!!!!!!!!!!!source find\n");
                            cout << adjLocation << endl;
                            sourceFind = true;
                            break;
                        }
                        else if (markNumberGraph[adjLocation.y][adjLocation.x] != 0 && markNumberGraph[adjLocation.y][adjLocation.x] < level)
                        {
                            level--;
                            curNetInfo.netPath.push_back(adjLocation);
                            curLocation = adjLocation;
                        }
                    }
                }
                curNetInfo.netPath.push_back({curNetInfo.target_x, curNetInfo.target_y});
                reverse(curNetInfo.netPath.begin()+1,curNetInfo.netPath.end()-1);
                break;
            }
            // 沒找到path
            else
            {
                int randIndex = rand() % adjNetList.size();
                cout << "ripeUp:" << randIndex << endl;
                GraphObj.ripeUp(adjNetList[randIndex]);
            }
        }
        // path is find ,finish
        addNetToGraph(curNetInfo);
        #ifdef debug
        GraphObj.printGraph();
        cout << curNetInfo;
        #endif
    }
    void addNetToGraph(struct netInfo &curNetInfo)
    {
        for (auto &curLocation : curNetInfo.netPath)
        {
            GraphObj.netGraph[curLocation.y][curLocation.x] = curNetInfo.id;
        }
    }
    void printResult(string outputFileName)
    {
        ofstream fs(outputFileName);
        GraphObj.printResult(fs);
    }
};

int main(int argc, char *argv[])
{
    freopen(argv[1], "r", stdin);
    LeeAlgo LeeAlgoObj;
    LeeAlgoObj.run();
    LeeAlgoObj.printResult(string(argv[2]));
}