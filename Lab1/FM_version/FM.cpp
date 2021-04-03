#include <iostream>
#include <vector>
#include<math.h>
#include <sstream>
#include <limits.h>
#include <algorithm>
#include <fstream>
#include <omp.h>
#include<map>
#include <list>
#include<unordered_map>
#include<unordered_set>
//#define debug
#define constraintRatio 0.5
//#define testPDF

//#define debug
enum Group{
    groupA,groupB
};
using namespace std;

struct op
{
    int cell;
    int g;
    float balanceRatio;
    friend ostream &operator<<(ostream &os, struct op &op_obj)
    {
        cout <<"op: cell " << op_obj.cell << " g: " << op_obj.g<< endl;
        return os;
    }
};

class CellsStat{
    public:
    vector<Group> group;
    vector<bool> locked;
    static int groupACount;
    static int groupBCount;
    CellsStat() = delete;
    #ifdef testPDF
    CellsStat(int cellNum){
        group = vector<Group> (cellNum,Group::groupB);
        locked = vector<bool> (cellNum,false);
        CellsStat::groupACount = 4;
        CellsStat::groupBCount = cellNum - CellsStat::groupACount;
        group[0] = Group::groupA;
        group[2] = Group::groupA;
        group[3] = Group::groupA;
        group[6] = Group::groupA;
    }
    #else 
    CellsStat(int cellNum){
        group = vector<Group> (cellNum,Group::groupB);
        locked = vector<bool> (cellNum,false);
        CellsStat::groupACount = cellNum/2;
        CellsStat::groupBCount = cellNum - CellsStat::groupACount;
        for (int i = 0; i < cellNum/ 2; i++)
        {
            group[i] = Group::groupA;
        }
    }
    #endif
    friend ostream& operator<<(ostream& os ,CellsStat&obj){
        #ifdef debug
        cout << "groupACount :" << obj.groupACount << endl;
        cout << "groupBCount :" << obj.groupBCount << endl;
        obj.printGroup();
        obj.printLocked();
        #endif
        return os;
    }
    void printGroup(){
        #ifdef debug
        cout << "===========group===============" << endl;
        for(int i=0;i<group.size();i++){
            cout << i << " " ;
        }
        cout << endl;
        for(int i=0;i<group.size();i++){
            cout <<  group[i] << " ";
        }
        cout << endl;
        cout << "===========group end===============" << endl;
        #endif
    }
    void printLocked(){
        #ifdef debug
        cout << "===========locked===============" << endl;
        for(int i=0;i<group.size();i++){
            cout << i << " " ;
        }
        cout << endl;
        for(int i=0;i<locked.size();i++){
            cout << locked[i] << " ";
        }
        cout << endl;
        cout << "===========locked end===============" << endl;
        #endif
    }
};
int CellsStat::groupACount =0;
int CellsStat::groupBCount =0;

struct singleCellStat{
    int id;
    int gain;
    singleCellStat()=delete;
    singleCellStat(int id ,int gain){
        this->id= id;
        this->gain = gain;
    }

};

class bucketListClass{
    public:
    unordered_map<int,list<struct singleCellStat>::iterator> cellIterList;
    vector<list<struct singleCellStat>> bucketList;
    int curMaxGain = INT_MIN;
    int maxPin;
    int curNodeCount=0;
    bucketListClass() {};
    void init(int maxPin){
        this->maxPin = maxPin;
        bucketList = vector<list<struct singleCellStat>> (maxPin*2+1);
    }
    friend ostream& operator<<(ostream& os ,bucketListClass& obj){
        cout << "===========bucketListClass===========" << endl;
        cout << "curMaxGain:" << obj.curMaxGain << endl;
        cout << "curNodeCount:" << obj.curNodeCount << endl;
        for(int i=obj.bucketList.size()-1;i>=0;i--){
            if(obj.bucketList[i].empty()) continue;
            cout << "gain:" << i-obj.maxPin << " " ;
            for(auto& cell:obj.bucketList[i]){
                cout << "gain:" << cell.gain << " id:" << cell.id<<" ";
            }
            cout << endl;
        }
        cout << "===========bucketListClassEnd===========" << endl;
        return os;
    }
    bool empty(){
        return curNodeCount == 0;
    }
    /*
    bool returnMaxGainCell(int& gain,list<struct singleCellStat>::iterator &iter){
        if(bucketList.empty()) return false;
        iter = bucketList[curMaxGain].begin();
        gain = curMaxGain;
        return true;
    }
    */
    bool returnMaxGainCell(int& gain,list<struct singleCellStat>::iterator &iter){
        if(empty()) return false;
        #ifdef testPDF
        int minId =INT_MAX;
        for(auto tmpIter= bucketList[curMaxGain+maxPin].begin();tmpIter!= bucketList[curMaxGain+maxPin].end();tmpIter++){
            if(tmpIter->id < minId){
                minId = tmpIter->id;
                iter = tmpIter;
            }
        }
        #else
        iter = bucketList[curMaxGain+maxPin].begin();
        #endif
        gain = curMaxGain;
        return true;
    }
    void updateCurMaxGain(){
        for(int i=curMaxGain+maxPin;i>=0;i--){
            if(!bucketList[i].empty()){
                curMaxGain = i-maxPin;
                return ;
            }
        }
        curMaxGain = INT_MIN;
    }
    void eraseCell(int gain,list<struct singleCellStat>::iterator iter){
        curNodeCount--;
        bucketList[gain+maxPin].erase(iter);
        cellIterList.erase(iter->id);
        if(gain == curMaxGain && bucketList[gain+maxPin].empty()){
            updateCurMaxGain();
        }
        #ifdef debug
        cout << "------------ eraseCell-------------"<< endl;
        cout << "gain:" << gain << " id" << iter->id << endl;
        cout << "curMaxGain" << curMaxGain << endl;
        cout << *this;
        cout << "------------ eraseCellEnd-------------"<< endl;
        #endif
    }
    void insert(int gain,int cell){
        curNodeCount++;
        struct singleCellStat tmpCell(cell,gain) ;
        bucketList[gain+maxPin].push_front(tmpCell);
        cellIterList[cell] = bucketList[gain+maxPin].begin();
        curMaxGain = max(gain,curMaxGain);

        #ifdef debug
        cout << "------------ insertCell-------------"<< endl;
        cout << "gain:" << gain << " id" << cell << endl;
        cout << "curMaxGain" << curMaxGain << endl;
        cout << *this;
        cout << "------------ insertCellEnd-------------"<< endl;
        #endif
    }
    void updateGain(int cell,int decOrInc){
        auto iter = cellIterList[cell];
        int gain = iter->gain;
        eraseCell(gain,iter);
        gain+=decOrInc==1?1:-1;
        curMaxGain = max(gain,curMaxGain);
        insert(gain,cell);
        #ifdef debug
        cout << "--------------------- inner updateGain----------------------" << endl;
        cout << *this;
        cout << "----------------------- inner updateGainEnd------------------------" << endl;
        #endif
    }
};

class twoBucketList{
    public:
    bucketListClass groupA;
    bucketListClass groupB;
    int nodeNum;
    twoBucketList() = delete;
    twoBucketList(int nodeNum):groupA(),groupB(){
        this->nodeNum = nodeNum;
    }
    void init(int maxPin){
        groupA.init(maxPin);
        groupB.init(maxPin);
    }
    bool checkIFAllLocked(){
        if(groupA.bucketList.empty() && groupB.bucketList.empty()) return true;
    }
    bool passConstraint(Group group){
        if(group == Group::groupA){
            if((float)(CellsStat::groupACount-1)/nodeNum >= 0.45)
                return true;
        }
        else if(group == Group::groupB){
            if((float)(CellsStat::groupBCount-1)/nodeNum >= 0.45)
                return true;
        }
        return false;
    }
    bool findMaxGainCell(struct op& maxGainOp){
        int gainA=INT_MIN;
        int gainB=INT_MIN;
        list<struct singleCellStat>::iterator iterA;
        list<struct singleCellStat>::iterator iterB;
        groupA.returnMaxGainCell(gainA,iterA);
        groupB.returnMaxGainCell(gainB,iterB);
        if(groupA.empty() && groupB.empty()) {
            return false;
        }
        if(!passConstraint(Group::groupA)){
            if(groupB.empty()){
            #ifdef debug
                cout << "groupA not pass constraint,groupB empty" << endl;
                #endif
                return false;
            }
        } 
        if(!passConstraint(Group::groupB)){
            if(groupA.empty()){
        #ifdef debug
                cout << "groupA empty ,groupB not pass constraint" << endl;
                #endif
                return false;
            }
        } 

        if(!groupA.empty()&& passConstraint(Group::groupA) &&(groupB.empty() || !passConstraint(Group::groupB) ||  gainA > gainB  || (gainA == gainB && iterA->id < iterB->id))){
            maxGainOp.cell = iterA->id;
            maxGainOp.g= iterA->gain;
            maxGainOp.balanceRatio =(float) (CellsStat::groupACount-1)/nodeNum;
            groupA.eraseCell(gainA,iterA);
        }
        else{
            maxGainOp.cell = iterB->id;
            maxGainOp.g= iterB->gain;
            maxGainOp.balanceRatio =(float) (CellsStat::groupBCount-1)/nodeNum;
            groupB.eraseCell(gainB,iterB);
        }
        return true;

    }
    void insert(int gain,Group group,int cell){
        if(group == Group::groupA){
            groupA.insert(gain,cell);
        }
        else{
            groupB.insert(gain,cell);
        }
    }
    void updateGain(int cell,Group group,int incORDec){
        if(group == Group::groupA){
            groupA.updateGain(cell,incORDec);
        }
        else{
            groupB.updateGain(cell,incORDec);
        }

    }
    void print(){
        #ifdef debug
        cout << "=============gain===========" << endl;
        cout << "groupA" << endl;
        cout <<groupA << endl;
        cout << "groupB" << endl;
        cout <<groupB << endl;
        cout << "=============gain End===========" << endl;
        #endif
    }

};
struct NetStat{
    int id;
    int countOfGroupA=0;
    int countOfGroupB=0;
    friend ostream& operator<<(ostream&os ,struct NetStat& obj){
        #ifdef debug
            cout << "id: " << obj.id << "countOfGroupA: " << obj.countOfGroupA << "countOFGroupB: " << obj.countOfGroupB << endl;
        #endif
            return os;
    }
};

class Graph{
    public:
    vector<vector<int>> netConnectCells;
    vector<vector<int>> cellAdjNets;
    int netNum;
    Graph() = delete;
    Graph(int netNum,int nodeNum):netConnectCells(vector<vector<int>>(netNum)),cellAdjNets(vector<vector<int>>(nodeNum)){
        this->netNum = netNum;
        buildGraph();
        printNetConnectCells();
        printCellAdjNets();
    }
    void printNetConnectCells(){
        #ifdef debug
        cout << "==============netconnectcells===========" << endl;
        for(int i=0;i<netConnectCells.size();i++){
            cout << "net:" << i;
            for(auto cell:netConnectCells[i]){
                cout << " cell:" << cell;
            }
            cout << endl;
        }
        cout << "==============netconnectcellsEnd===========" << endl;
        #endif
    }
    void printCellAdjNets(){
        #ifdef debug
        cout << "==============CellAdjNets===========" << endl;
        for(int i=0;i<cellAdjNets.size();i++){
            cout << "cell:" << i;
            for(auto cell:cellAdjNets[i]){
                cout << " net:" << cell;
            }
            cout << endl;
        }
        cout << "==============CellAdjNets===========" << endl;
        #endif
    }
        
    void buildGraph(){
        // handle input
        cin.ignore();
        int curNetIndex=0;
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
                int node = stoi(tmp)-1;
                nodeList.push_back(node);
                cellAdjNets[node].push_back(curNetIndex);
            }
            netConnectCells[curNetIndex] = nodeList;
            curNetIndex++;
        }
    }
    int getMaxPin(){
        int maxPin = INT_MIN;
        for(auto item:cellAdjNets){
            maxPin = max(maxPin,(int)item.size());
        }
        return maxPin;
    }
};

class FM
{
    twoBucketList twoBucketListObj;
    Graph GraphObj;
    CellsStat CellsStatObj;
    vector<struct NetStat> NetStatList;
    int netNum;
    int nodeNum;
public:
    FM()  = delete;
    FM(int netNum,int nodeNum):twoBucketListObj(nodeNum),GraphObj(netNum,nodeNum),CellsStatObj(nodeNum),NetStatList(netNum)
    {
        this->netNum = netNum;
        this->nodeNum = nodeNum;
        int maxPin = GraphObj.getMaxPin();
        twoBucketListObj.init(maxPin);
    }
    void printNetStatList(){
        #ifdef debug
        cout << "===============NetStatList========="<< endl;
        for(auto item:NetStatList){
            cout << item;
        }
        cout << "===============NetStatList End========="<< endl;
        #endif
    }
    void init(){
        initNetStatList();
        #ifdef debug
        cout << "====init@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<< endl;
        printAllStatus();
        cout << "====initEnd@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<< endl;
        #endif
    }
    void printAllStatus(){
        #ifdef debug
        printNetStatList();
        twoBucketListObj.print();
        cout <<CellsStatObj;
        #endif

    }
    void initNetStatList(){
        for(int i=0;i<NetStatList.size();i++){
            NetStatList[i].id = i;
            for(auto &cell:GraphObj.netConnectCells[i]){
                if(CellsStatObj.group[cell] == Group::groupA){
                    NetStatList[i].countOfGroupA++;
                }
                else 
                    NetStatList[i].countOfGroupB++;
            }
        }
    }
    // compute and insert to bucketlist
    void initComputeGain(){
        
        for(int i=0;i<nodeNum;i++){
            int gain=0;
            Group group = CellsStatObj.group[i];
            for(auto net:GraphObj.cellAdjNets[i]){
                gain += FS(i,NetStatList[net]);
                gain += TE(i,NetStatList[net]);
            }
            twoBucketListObj.insert(gain,group,i);
        }
    }
    void updateCellGain(int cell){
        #ifdef debug
        cout << "*************update gain***********"<< endl;
        printAllStatus();
        cout << "**************update gain End************"<< endl;
        #endif
        auto fromGroup = CellsStatObj.group[cell];
        CellsStatObj.locked[cell] = true;
        for(auto& net:GraphObj.cellAdjNets[cell]){
            auto curNetStat = &NetStatList[net];
            int Tvalue = T(cell,curNetStat);
            if(Tvalue == 0){
                for(auto tmpCell:GraphObj.netConnectCells[net]){
                    if(CellsStatObj.locked[tmpCell] == false)
                     twoBucketListObj.updateGain(tmpCell,CellsStatObj.group[tmpCell],1);
                }
            }
            if(Tvalue == 1){
                for(auto tmpCell:GraphObj.netConnectCells[net]){
                    if(CellsStatObj.locked[tmpCell] == false && CellsStatObj.group[tmpCell]!=fromGroup)
                     twoBucketListObj.updateGain(tmpCell,CellsStatObj.group[tmpCell],-1);
                }
            }
            curNetStat->countOfGroupA += fromGroup == Group::groupA?-1:1;
            curNetStat->countOfGroupB += fromGroup == Group::groupB?-1:1;

        #ifdef debug
            cout << "----------update FValue---------" << endl;
            #endif
            int Fvalue = F(cell,curNetStat);
            if(Fvalue == 0){
                for(auto tmpCell:GraphObj.netConnectCells[net]){
                    if(CellsStatObj.locked[tmpCell] == false)
                     twoBucketListObj.updateGain(tmpCell,CellsStatObj.group[tmpCell],-1);
                }
            }
            if(Fvalue == 1){
                for(auto tmpCell:GraphObj.netConnectCells[net]){
                    if(CellsStatObj.locked[tmpCell] == false && CellsStatObj.group[tmpCell]==fromGroup)
                     twoBucketListObj.updateGain(tmpCell,CellsStatObj.group[tmpCell],1);
                }
            }
        #ifdef debug
            cout << "----curCell:" <<cell <<endl;
            cout << "net:" << net << " Tvalue:" << Tvalue << " Fvalue:" << Fvalue <<endl;
            #endif
            
            
        }
        changGroup(cell);
    }
    void changGroup(int cell){
        CellsStat::groupACount+= CellsStatObj.group[cell]== Group::groupA?-1:1;
        CellsStat::groupBCount+= CellsStatObj.group[cell]== Group::groupB?-1:1;
        CellsStatObj.group[cell] = CellsStatObj.group[cell]== Group::groupA?Group::groupB:Group::groupA;
    }
    int T(int cell,struct NetStat* net){
        if(CellsStatObj.group[cell] ==Group::groupA){
            return net->countOfGroupB;
        }
        else {
            return net->countOfGroupA;
        }
    }
    int F(int cell,struct NetStat* net){
        if(CellsStatObj.group[cell] ==Group::groupA){
            return net->countOfGroupA;
        }
        else 
        return net->countOfGroupB;
    }
    int FS(int cell,struct NetStat& net){
        if(CellsStatObj.group[cell] ==Group::groupA && net.countOfGroupA==1){
            return 1;
        }
        else if(CellsStatObj.group[cell]==Group::groupB && net.countOfGroupB==1){
            return 1;
        }
        return 0;
    }
    int TE(int cell,struct NetStat& net){
        if(CellsStatObj.group[cell] == Group::groupA && net.countOfGroupB==0){
            return -1;
        }
        else if(CellsStatObj.group[cell]==Group::groupB && net.countOfGroupA==0){
            return -1;
        }
        return 0;
    }
    void run()
    {
        

        while (1)
        {

            cout << "!!!!!!!!!!!!!!!!!!new while iteration!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;

        #ifdef debug
            cout << "----before change the group---" << endl;
            cout << CellsStatObj ;
            #endif
            vector<Group> groupBackup = CellsStatObj.group;
            int groupACountBackup = CellsStat::groupACount;
            int groupBCountBackup = CellsStat::groupBCount;
            vector<struct NetStat> NetStatListBackup;
            NetStatListBackup =  NetStatList;

            vector<struct op> opList;
            //compute gi
            struct op maxGainOp;
            initComputeGain();
            while(twoBucketListObj.findMaxGainCell(maxGainOp)){
                updateCellGain(maxGainOp.cell);
                opList.push_back(maxGainOp);
                #ifdef debug
                printAllStatus();
                cout << "result~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ " << opList.back();
                #endif
            }
            cout << endl;
            int tmpGk = 0;
            int maxGk = INT_MIN;
            int maxGkIndex = -1;
            int balanceRatio = 0.5;
            //find k such that Gk is max
            #ifdef debug
            cout << "---------------op list--------" << endl;
            #endif
            for (int i = 0; i < opList.size(); i++)
            {
                #ifdef debug
                cout << opList[i] << " ";
                #endif
                tmpGk += opList[i].g;
                if (tmpGk > maxGk || (tmpGk == maxGk && fabs(opList[i].balanceRatio - 0.5) <fabs(balanceRatio-0.5) ))
                {
                    balanceRatio = opList[i].balanceRatio;
                    maxGk = tmpGk;
                    maxGkIndex = i;
                }
            }
            cout << "---maxGk:" << maxGk << " maxGkIndex:" << maxGkIndex << endl;
            if (maxGk <= 0)
                return;
            // change the group
            else
            {
                for (int i = 0; i <= maxGkIndex; i++)
                {
                    int cell = opList[i].cell;
                    if(groupBackup[opList[i].cell] == Group::groupA){
                        groupBackup[opList[i].cell] = Group::groupB;
                        groupACountBackup --;
                        groupBCountBackup ++;
                        for(auto net: GraphObj.cellAdjNets[cell]){
                            NetStatListBackup[net].countOfGroupA--;
                            NetStatListBackup[net].countOfGroupB++;
                        }
                    }
                    else{
                        groupBackup[opList[i].cell] = Group::groupA;
                        groupBCountBackup --;
                        groupACountBackup ++;
                        for(auto net: GraphObj.cellAdjNets[cell]){
                            NetStatListBackup[net].countOfGroupB--;
                            NetStatListBackup[net].countOfGroupA++;
                        }
                    }
                }
                NetStatList.swap(NetStatListBackup);
                CellsStatObj.group.swap(groupBackup);
                CellsStat::groupACount = groupACountBackup;
                CellsStat::groupBCount = groupBCountBackup;
            }
            #ifdef debug
            cout << "----after change the group---" << endl;
            cout << CellsStatObj;
            printNetStatList();
            #endif
            // unlock
            for (int i = 0; i < nodeNum; i++)
            {
                CellsStatObj.locked[i] = false;
            }
        }
    }
    void printResult(){
        ofstream outputFile("output.txt");
        for(auto item:CellsStatObj.group){
            if(item == Group::groupA)
            outputFile << 0 << endl;
            else
            outputFile << 1 << endl;
        }
    }
};

int main(int argc, char *argv[])
{
        string inputFile = string(argv[1]);
        int netNum,nodeNum;
        freopen(inputFile.c_str(), "r", stdin);
        cin >> netNum >> nodeNum;
        FM FMObj(netNum,nodeNum);
        FMObj.init();
        FMObj.run();
        FMObj.printResult();
}