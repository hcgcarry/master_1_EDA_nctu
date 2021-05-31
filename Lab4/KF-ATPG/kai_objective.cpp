
/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 2002-2006, University of California at Santa Barbara (UCSB).  
 All rights reserved. 

 --- This software and any associated documentation is provided "as is" 

 UCSB and the authors MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS 
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A 
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR 
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, 
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.  

 UCSB and the authors shall not be liable under any circumstances for 
 any direct, indirect, special, incidental, or consequential damages 
 with respect to any claim by USER or any third party on account of 
 or arising from the use, or inability to use, this software or its 
 associated documentation, even if UCSB and/or the authors has been advised
 of the possibility of those damages.
*********************************************************************/
// Author: Kai Yang (kyang@ece.ucsb.edu)

using namespace std;

#include <string.h>
#include <cstdlib>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "GetLongOpt.h"
#include "kai_gate.h"
#include "kai_path.h"
#include "kai_objective.h"
#include "kai_typeemu.h"
#include <map>

extern GetLongOpt option;
extern map<string, KaiGATE*> CUTNameTable;

int getNonControllingValue(GATEFUNC gatefunc_obj){
	int result = -1;
	switch(gatefunc_obj){
		case G_AND:
			result = 1;
			break;
		case G_NAND:
			result = 1;
			break;
		case G_OR:
			result = 0;
			break;
		case G_NOR:
			result = 0;
			break;
	}
	return result;
}
int getControllingValue(GATEFUNC gatefunc_obj){
	return 1 ^ getNonControllingValue(gatefunc_obj);
}


string ToCUTName(KaiGATE *gptr, int TF)
{
	char tmp[100];
	sprintf(tmp, "%d", TF);
	string dupname;
	if(gptr->GetFunction()!=G_DFF)
		dupname=gptr->GetName()+"_dup_t"+tmp;
	else
		dupname=gptr->GetName()+"_ppi_t"+tmp;
	KaiGATE *tg=CUTNameTable[dupname];
	if(!tg) { cerr<<"Miss ToCUTName()"<<endl; exit(-1); }
	return dupname;
}

void AtpgObj::BuildFromPath_NR(PATH *pptr)
{
	cleanup();
	KaiGATE *CurG, *PreG;
	TRANSITION CurT, PreT;
	assert(pptr->NoGate()==pptr->NoTrans());
	pptr->Dump();
	this->Dump();
	
}
enum transitionCV { cv2ncv=0 , ncv2cv};
transitionCV convertTransition2TransitionCv(TRANSITION curTransition,KaiGATE* curGate){
	transitionCV result;
	if(curTransition == R){
		if(getControllingValue(curGate->GetFunction()) == 0){
			result = cv2ncv;
		}
		else{
			result = ncv2cv;
		}
	}
	if(curTransition == F){
		if(getControllingValue(curGate->GetFunction()) == 0){
			result  = ncv2cv;
		}
		else{
			result = cv2ncv;
		}
	}
	return result;
}
enum robustOrNonRobust{
	robust,nonRobust
};

void addOffInputCluaseToATPG(AtpgObj* curAtpgObj,TRANSITION prevOnInputTransition,KaiGATE* prevOnInputGate,KaiGATE* curGate,robustOrNonRobust curRobustOrNonRobust){
	KaiGATE* prevOffInputGate=NULL;
	if(curGate->NoFanin() == 1) return;

	transitionCV prevOnInputTransitionCV= convertTransition2TransitionCv(prevOnInputTransition,curGate);
	cout << "prevOnInputTransition:" << prevOnInputTransition ;
	cout << " prevOnInputTransitionCV:" << prevOnInputTransitionCV ;
	cout << " prevOnInputGate:" << prevOnInputGate->GetName() <<  endl;
	//cout << "prevOffInputGate:" << prevOffInputGate->GetName() <<  endl;
	cout << "curGate:" << curGate->GetName() << " func" << curGate->GetFunction() << " controlValu:" << getControllingValue(curGate->GetFunction()) << endl;

	for(int i=0;i<curGate->NoFanin();i++){
		if(curGate->Fanin(i)->GetName() != prevOnInputGate->GetName()){
			prevOffInputGate = curGate->Fanin(i);
			if(prevOnInputTransitionCV == cv2ncv ){
				if(curRobustOrNonRobust == robust){
					curAtpgObj->AddObj(ToCUTName(prevOffInputGate, 1),getNonControllingValue(curGate->GetFunction()));
				}
			}
			else{
				if(curRobustOrNonRobust == robust){
					curAtpgObj->AddObj(ToCUTName(prevOffInputGate, 0),getNonControllingValue(curGate->GetFunction()));
					curAtpgObj->AddObj(ToCUTName(prevOffInputGate, 1),getNonControllingValue(curGate->GetFunction()));
				}
				else{
					curAtpgObj->AddObj(ToCUTName(prevOffInputGate, 1),getNonControllingValue(curGate->GetFunction()));
				}
			}
		}
	}

}

void AtpgObj::BuildFromPath_R(PATH *pptr)
{
	cout << "---------------------new build from path_R--------------" << endl;
	cleanup();
	KaiGATE *CurG, *PreG;
	TRANSITION CurT, PreT;
	assert(pptr->NoGate()==pptr->NoTrans());
	cout << "------------path dump " << endl;
	pptr->Dump();
	/*Do Fault Activation & Fault Propagation under Robust test setting*/

	PreG= pptr->GetGate(0);
	PreT= pptr->GetTrans(0);  /* input transition on sensitive path*/
	
	
	cout << "preT" << PreT << " preG->getFunction " << PreG->GetFunction() << endl;
	// Fault Activation at 1st TimeFrame
	if(PreT==R) AddObj(ToCUTName(PreG, 0),0);
	else if(PreT==F) AddObj(ToCUTName(PreG, 0), 1);
	else { cerr<<"R/F Error !"<<endl; exit(-1); }

	// Fault Activation at 2nd TimeFrame 
	if(PreT==R) AddObj(ToCUTName(PreG, 1), 1);
	else if(PreT==F) AddObj(ToCUTName(PreG, 1), 0);
	else { cerr<<"R/F Error !"<<endl; exit(-1); }
	
	/*Fault Propagation = off-input setting on sensitive path */
	for(int i=1;i<pptr->NoGate();i++){
		addOffInputCluaseToATPG(this,pptr->GetTrans(i-1),pptr->GetGate(i-1),pptr->GetGate(i),robust);
		cout << "---atpg dump:" ;
		this->Dump();
	}
	cout << "---------final ---atpg dump" << endl;
	this->Dump();
	for(int i=0;i<pptr->NoGate();i++){
		cout << "==== new gate on path===" << endl;
		KaiGATE* gateptr = pptr->GetGate(i) ;
		cout << "R or F:" << pptr->GetTrans(i);
		cout << " func:" << gateptr->GetFunction() ;
		cout << " name:" << gateptr->GetName()<< endl;
		//addOffInputCluaseToATPG(this,pptr->GetTrans(i),pptr->GetGate(i))
		cout <<"_fanin:" ;
		for(int i=0;i<gateptr->NoFanin();i++){
			cout << gateptr->Fanin(i)->GetName() << " ";
		}
		cout << endl;
		cout <<"_fanout:" ;
		for(int i=0;i<gateptr->NoFanout();i++){
			cout << gateptr->Fanout(i)->GetName() << endl;
		}
	}

}


void AtpgObj::Dump()
{
	assert(_name.size()==_value.size());
	for(unsigned int i=0; i<_name.size(); ++i) {
		cout<<_name[i]<<"("<<_value[i]<<") ";
	}
	cout<<endl;
}

