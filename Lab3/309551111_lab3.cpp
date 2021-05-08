#include<iostream>
#include<stdio.h>
#include<sstream>
#include<unordered_map>
#include<queue>
#include<vector>
#include<algorithm>
#include<fstream>
#include<iomanip>

#include<unordered_set>
using namespace std;




struct vertex{
    string levelToken;
    int id;
    int TokenOrderIndex;
    static int curMaxId;
    struct vertex* leftChild;
    struct vertex* rightChild;
    int getTokenOrderIndex(){
        if(levelToken == "1" || levelToken == "0"){
            return -1;
        }
        string tmp = levelToken.substr(1);
        return stoi(tmp);
    }
    vertex() = delete;
    vertex(string levelToken,struct vertex* leftChild,struct vertex* rightChild):levelToken(levelToken),leftChild(leftChild),rightChild(rightChild){
        this->TokenOrderIndex = getTokenOrderIndex();
        this->id = vertex::curMaxId++;
    }
    bool equal(string levelToken,struct vertex* left,struct vertex* right){
        if(leftChild == left && rightChild == right && this->levelToken== levelToken){
            return true;
        }
        return false;
    }
    friend ostream& operator<<(ostream& os ,struct vertex& vertexObj){
        cout << "vertex:";
        cout << "address:" << &vertexObj ;
        cout << " levelToken:" <<vertexObj.levelToken << " id:" <<vertexObj.id ;
        cout << " TokenOrderIndex:" << vertexObj.TokenOrderIndex ;
        cout << " leftChild:" << vertexObj.leftChild << " rightChild:" << vertexObj.rightChild << endl;
        return os;
    }
};

int vertex::curMaxId = 0;

class expression{
    public:
    vector<unordered_set<char>> productTermList; 
    string expressionString;
    int isOneOrTwo=-1;
    expression()= delete;
    expression(string expressionString):expressionString(expressionString){
        preprocessExpression();
    }
    friend ostream& operator<<(ostream& os,expression& expressionObj){
        cout << "--------expressionObj-------------" <<endl;
        cout << "productTermList---" <<endl;
        for(auto item:expressionObj.productTermList){
            cout << "newSet:";
            for(auto charInSet:item){
                cout<<charInSet;
            }
            cout << " ";
        }
        cout << endl;
        cout << "--------expressionObj end-------------" <<endl;
    }

    void preprocessExpression(){
        stringstream ss(this->expressionString);
        string productTerm;
        while(getline(ss,productTerm,'+')){
            //this->productTermList.push_back(productTerm);
            unordered_set<char> tmpSet;
            for(auto token:productTerm){
                tmpSet.insert(token);
            }
            productTermList.push_back(tmpSet);
        }
    }
    char inverse(char token){
        if(token >= 'a' && token <='z'){
            return token-'a' + 'A';
        }
        else{
            return token-'A' + 'a';
        }
    }
    bool isLeaf(){
        return isOneOrTwo == 1 || isOneOrTwo == 0;
    }
    void inputVariableValue(char token){
        // cout << "---inputVariableValue---" << endl;
        // cout << "token:" << token << endl;
        for(auto iter = productTermList.begin();iter != productTermList.end();){
            // find inverse token
            /*
            cout << "minTerm:" ;
            for(auto chr:*iter){
                cout <<  chr ;
            }
            cout << endl;
            */
            unordered_set<char>::iterator innerIter;
            if(iter->find(inverse(token)) != iter->end()){
                // cout << "find inverse token" << endl;
                if(productTermList.size()==1){
                    isOneOrTwo = 0;
                    // cout << "leaf 0" << endl;
                    return;
                }
                // cout << "erase minTerm" << endl;
                productTermList.erase(iter);
                continue;
            }
            // find token
            else if(iter->find(token) != iter->end()){
                //f = 1
                // cout << "find token" << endl;
                if(iter->size() == 1){
                    isOneOrTwo = 1;
                    // cout << "leaf 1" << endl;
                    return;
                }
                else{
                    // cout << "erase char"<< endl;
                    iter->erase(token);
                    iter++;
                }
            }
            else{
                iter++;
            }
        }
    }
};
struct tokenInfo{
    char token;
    float probability;
    int index;
    tokenInfo() = delete;
    tokenInfo(char token ,float probability,int index){
        if(token <= 'Z' && token >= 'A'){
            this->probability = 1-probability;
            this->token = token - 'A' +'a';
        }
        else{
            this->token = token;
            this->probability = probability;
        }
        this->index = index;
    }
    friend ostream& operator<<(ostream& os ,struct tokenInfo& tokenInfoObj){
        cout << "------------tokenInfo-------------" << endl;
        cout << "token :" << tokenInfoObj.token<< " probability:" <<tokenInfoObj.probability << " index:"<<tokenInfoObj.index<< endl;
        cout << "------------tokenInfo end-------------" << endl;
        return os;
    }
};
class Info{
    public:
    string expressionString;
    vector<struct tokenInfo> tokenProbability;
    
    Info() = delete; 
    Info(char** argv){
        readInput(argv);
        // cout << *this;
    }
    
    void readInput(char ** argv){
        freopen(argv[1],"r",stdin);
        string line;
        getline(cin,line);
        line.pop_back();
        this->expressionString = line;
        char token;
        float probablity;
        int index=0;
        while( cin >> token >> probablity){
            tokenProbability.push_back(tokenInfo(token,probablity,index++));
        }
    }
    friend ostream& operator<<(ostream& os ,Info& InfoObj){
        cout << "------------expressionString-------------" << endl;
        cout << "expressionString: " << InfoObj.expressionString << endl;
        cout << "--tokenProbability--" << endl;
        for(auto item : InfoObj.tokenProbability){
            cout << item;
        }
        cout << "------------expressionString end-------------" << endl;
        return os;
    }
};

class BDD{
    public:
    struct vertex* root;
    unordered_set<vertex*> vertexSet;
    BDD(){
        vertexSet.insert(new vertex("0",NULL,NULL));
        vertexSet.insert(new vertex("1",NULL,NULL));
    }
    void printVertexSet(){
        cout << "--------- vertexSet--------------------------------" << endl;
        for(auto item:vertexSet){
            cout << *item ;
        }
        cout << "--------- vertexSet end---------------------------" << endl;
    }
    void  printVertexTree(struct vertex* root){
        cout << "--------printVertexTree---------" << endl;
        if(root == NULL) return ;
        queue<struct vertex*> que;
        que.push(root);
        while(!que.empty()){
            cout << "---------new level--------" << endl;
            int n = que.size();
            while(n--){
                struct vertex* curRoot = que.front();
                que.pop();
                cout << *curRoot;
                if(curRoot->rightChild){
                    que.push(curRoot->rightChild);
                }
                if(curRoot->leftChild){
                    que.push(curRoot->leftChild);
                }
            }
        }
        cout << "--------printVertexTree end---------" << endl;

    }
    struct vertex* old_or_new(string levelToken,struct vertex* left,struct vertex* right){
        // cout << "----old_or_new-----" << endl;
        // printVertexSet();
        auto tmp= find_if(vertexSet.begin(),vertexSet.end(),[levelToken,left,right](struct vertex* vertexObj){
            return vertexObj->equal(levelToken,left,right);
        });
        if(tmp == vertexSet.end()){
            vertex* newNode = new vertex(levelToken,left,right);
            // cout << "return new:" << *newNode ;
            vertexSet.insert(newNode);
            return newNode;
        }
        else{
            // cout << "return old:" << **tmp ;
            return *tmp;
        }
        // cout << "----old_or_new end-----" << endl;
        
    }
    void build(const Info& InfoObj){
        expression expressionObj(InfoObj.expressionString);
        this->root = robdd_build(expressionObj,0,InfoObj);
    }
    struct vertex* robdd_build(expression expressionObj,int i,const Info& InfoObj){
        /*
        cout << "-----------------------------robdd_build------" << endl;
        cout << "i:" << i <<  expressionObj ;
        cout << "---------------------------robdd_build end------" << endl;
        */
        struct vertex* right,*left;
        if(expressionObj.isLeaf()){
            // cout << "is leaf" << endl;
            if(expressionObj.isOneOrTwo == 1){
                return old_or_new("1",NULL,NULL);
            }
            else{
                return old_or_new("0",NULL,NULL);
            }
        }
        else{
            string levelToken = "x"+to_string(i) ;
            auto expressionObjLeft = expressionObj;
            expressionObj.inputVariableValue(InfoObj.tokenProbability[i].token);
            right = robdd_build(expressionObj,i+1,InfoObj);
            expressionObjLeft.inputVariableValue(toupper(InfoObj.tokenProbability[i].token));
            left = robdd_build(expressionObjLeft,i+1,InfoObj);
            if(right == left){
                return right;
            }
            else{
                return old_or_new(levelToken,left,right);
            }
        }
    }

};
class caculateProbability{
    public:
    float probability;
    void computeResult(const BDD& BDDObj,const Info& InfoObj){

        this->probability = computeRecur(BDDObj.root,InfoObj);
    }

    float computeRecur(struct vertex* root,const Info& InfoObj){
        if(root->levelToken == "0"){
            return 0;
        }
        else if(root->levelToken == "1"){
            return 1;
        }
        float right= computeRecur(root->rightChild,InfoObj);
        float left = computeRecur(root->leftChild,InfoObj);
        float TokenProbability = InfoObj.tokenProbability[root->getTokenOrderIndex()].probability ;
        float result = right * TokenProbability + left * (1-TokenProbability);
        //cout << "token:" << (char)(root->getTokenOrderIndex()+'a') << " right Probability:" << right << " left Probabaility:" << left << " tokenProbability:" << TokenProbability;

        //cout << "result:" << result << endl;
        return result;
    }
    void output(char** argv){
        cout << "probability:"  << std::fixed << std::setprecision(3)<< probability << endl;
        string outputFileName(argv[2]);
        ofstream outputFile(outputFileName);
        outputFile <<   std::fixed << std::setprecision(3) << probability;
    }
};
int main(int argc,char*  argv[]){
    Info InfoObj(argv);
    BDD BDDObj;
    BDDObj.build(InfoObj);
    // BDDObj.printVertexTree(BDDObj.root);

    caculateProbability caculateProbabilityObj;
    caculateProbabilityObj.computeResult(BDDObj,InfoObj);
    caculateProbabilityObj.output(argv);
}
