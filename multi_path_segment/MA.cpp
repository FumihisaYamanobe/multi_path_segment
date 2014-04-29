#include "class.h"
#include "mobileAgent.h"


void outTest(void){
	static int i=1;
	cout << "test" << endl;
	i++;
}
void outTest(int i){
	cout << "test " << i << endl;

}
//
extern unsigned long genrand();																	//�����Z���k�c�C�X�^���������֐��̊O���錾
//extern double rand_d();																				//[0,1)�̎�����������
//
////���z�M�I�u�W�F�N�g�̃R���X�g���N�^
//DISSEMINATION::DISSEMINATION(double tval, int sval, MA* ptr,LIST<EVENT>* list):EVENT(tval, EVENT::Dissemination){
//	maPtr = ptr;
//	size = sval;
//	list->insert(this);
//}
//
//bool DISSEMINATION::process(LIST<EVENT>* list, _NODE node){
//	int datasize;																						//�z�M�p�P�b�g�T�C�Y
//	for(int i = 0; i < (size + 1024) / 1024; i++){											//�f�[�^�T�C�Y���̃p�P�b�g����������
//		datasize = i != (size + 1024) / 1024 - 1 ? 1024 : size % 1024;					//�p�P�b�g�T�C�Y�̌���
//		int id = maPtr->getNode();
//		PACKET* ptr = new PACKET(getTime(), PACKET::Dissemination, datasize, id, id, -1, -1, 0);
//		ptr->setMa(maPtr);
//		ptr->setLast(id);
//		if(ptr->queue(list, node[id], false, true, getTime()) == false)							//�o�b�t�@�֑}��
//			delete ptr;																					//�}���Ɏ��s���������
//	}
//	setTime(getTime() + DISSEMINATION_INTERVAL);															//����̏���������ݒ�
//	list->insert(this);																					//�C�x���g���X�g�֓o�^
//	return true;																							//�I�u�W�F�N�g�͏������Ȃ�
//}

//���o�C���G�[�W�F���g�̃R���X�g���N�^
MA::MA(SIMU* ptr, SIMUTIME t, LOCATION c, double r, mtype tid, short maId, short node):EVENT(ptr, t, EVENT::Ma, node){
	SIMU* sPtr = getSimu();
	mode = tid;																						//MA��ID
	center = c;																						//���݃G���A�̒��S
	radius = r;																						//���݃G���A�̔��a
	migrationFlag = false;																		//�ړ��������t���O�i�����l�͔�ړ��j
	migrationNumber = 0;																			//�ړ���
//	outTime = 0;
	if(node == -1){																				//MA�m�[�h�̎w�肪�Ȃ����
		double distance;
		double min = 1000000000.0;
		for(int i = 0; i < (short)ptr->node.size(); i++){
			distance = dist(ptr->node[i]->getPos(), center);
			if(distance < min){
				min = distance;
				setNid(i);																				//���݃G���A�̒��S�ɍł��߂��m�[�h��MA�m�[�h
			}
		}
		setTime(t + MACHECKINTERVAL);
	}
	id = maId;
	meshRouteOrder = 0;
	toPtr = NULL;
	for(short i = 0; i < (short)ptr->node.size(); i++)
		nodePos.push_back(LOCATION(0, 0));
	ptr->node[getNid()]->ma.push_back(this);												//�V�~�����[�^�I�u�W�F�N�g��MA��o�^
	ptr->ma.push_back(this);
	ptr->list.insert(this);
	for(int i = 0; i < (int)sPtr->node.size(); i++){
		lastPos.push_back(LOCATION(-1,-1,-1));
		nodePos.push_back(LOCATION(-1,-1,-1));
		estimatePos.push_back(LOCATION(-1,-1,-1));
	}
}

//�R�s�[�쐬�p�̃R���X�g���N�^
MA::MA(SIMU* ptr):EVENT(ptr, 0, EVENT::Ma, -1){
}

//���o�C���G�[�W�F���g�̏����iprocess�֐��j
bool MA::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();	
//	cout << now.dtime() << " MA check " << getNid() << endl;
	short id = getNid();
	NODE* nPtr = sPtr->node[id];
	double distance = dist(nPtr->getDerivePos(0), center);							//���݃G���A�̒��S����̋���
//	cout << now.dtime() << "," << distance << "," << migrationFlag << endl;
	if(distance > NOMIGRATION && !migrationFlag){										//�ړ������𖞂������ꍇ
//		cout << "MA" << getId() << "'s migration process starts at " << now.dtime() << " in " << id << this << endl;
		candidate = -1;																			//�ړ�����̏�����
		candDist = distance;																		//���̒��S����̋���
		myDist = distance;																		//��r���̒��S����̋���
//		if(mode == Predict || mode == PredictDistance){
//			double estimateX = nPtr->getDerivePos(0).getX();
//			double estimateY = nPtr->getDerivePos(0).getY();
//			double estimateSpeedX = (nPtr->getDerivePos(0).getX() - nPtr->getDerivePos(1).getX()) / 1000000.0;
//			double estimateSpeedY = (nPtr->getDerivePos(0).getY() - nPtr->getDerivePos(1).getY()) / 1000000.0;
//			double A = pow(estimateSpeedX, 2.0) + pow(estimateSpeedY, 2.0);
//			double B = 2 * ((estimateX - center.getX()) * estimateSpeedX + (estimateY - center.getY()) * estimateSpeedY);
//			double C = pow(estimateX - center.getX(), 2.0) + pow(estimateY - center.getY(), 2.0) - MARANGE * MARANGE;
//			candSurvive = (sqrt(pow(B, 2.0) - 4 * A * C ) - B) / (2 * A);
//		}
		TIMEOUT* timeout = new TIMEOUT(sPtr, now + MAREQTTL, TIMEOUT::Mareq, id);//MA�ړ��v���^�C���A�E�g�̐ݒ�		
		toPtr = timeout;																			//�^�C���A�E�g�I�u�W�F�N�g��MA�ւ̓o�^
		timeout->setMa(this);																	//�Ή�MA�̃^�C���A�E�g�I�u�W�F�N�g�ւ̓o�^
		sPtr->list.insert(timeout);															//�C�x���g���X�g�֓o�^
		PACKET* pPtr 
			= new PACKET(sPtr, now, PACKET::MigReq, MIGREQ, id, id, -1, 1);		//MA�ړ��v���p�P�b�g�̍쐬
		pPtr->setMa(this);
		if(!pPtr->queue(sPtr, false))															//�p�P�b�g���o�b�t�@�֑}��
			delete pPtr;																				//�}���Ɏ��s������폜
	}
	setTime(now + MACHECKINTERVAL);															//����̏���������ݒ�
	sPtr->list.insert(this);																					//�C�x���g���X�g�֓o�^
	return true;																							//�I�u�W�F�N�g�͏������Ȃ�
}

//GAB�̌���
//�����i�Ȃ��j
//�߂�l�Ȃ�
void MA::exchangeGab(void){
	SIMU* sPtr = getSimu();																				//�V�~�����[�^�I�u�W�F�N�g
	NODE* nPtr = sPtr->node[getNid()];																//�m�[�h�I�u�W�F�N�g
	for(short i = 0; i < sPtr->getSTA(); i++){													//�S�Ă�STA���ɑ΂���
		if(timeCompare(gab[i].getTime(), nPtr->gab[i].getTime()))								//�G�[�W�F���g�̏�񂪐V�������
			nPtr->gab[i] = gab[i];																				//�m�[�h�̏����X�V
		else																										//�m�[�h�̏�񂪐V�������
			gab[i] = nPtr->gab[i];																				//�G�[�W�F���g�̏����X�V
	}
}

//MA�̈ړ�
//����
//�߂�l
void MA::migration(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
//	cout << "MA" << id << "'s migration starts at " << now.dtime()  << " from " << getNid() << " to " << candidate << endl; 
	migrationFlag = true;
	TCP* tcp = new TCP(sPtr, getNid(), now, MASIZE * 1000);
	TCPSINK* tcpsink = new TCPSINK(sPtr, candidate);
	tcp->connectSink(tcpsink);
	tcp->setMA(this);
	tPtr = tcp;
	sPtr->list.insert(tcp);
	TIMEOUT* timeout = new TIMEOUT(sPtr, now + SIMUTIME(2,0), TIMEOUT::Ma, getNid());
	toPtr = timeout;
	timeout->setMa(this);
	sPtr->list.insert(timeout);
}

//MA�ړ���m�[�h����
//�����i�m�[�hID, MIGREP�I�u�W�F�N�g�j
//�߂�l
void MA::decideMigratingNode(short id, MAREP* repPtr){
	double d = dist(repPtr->getPos(), center);										//���S����̋���
	switch(mode){																				//�ړ��A���S���Y���ɂ��ꍇ����
		case Distance:																				//�����ˑ������̏ꍇ
			if(d < candDist){																			//�V�K���̋������Z�����
				candDist = d;																				//���̍X�V
				candidate = id;																			//���S����̋����̍X�V
			}
			break;
//		case Predict:{
//			if(d > MARANGE){
//				if(d < candDist){
//					candDist = d;
//					candidate = id;
////					cout << candDist << "\t" << candidate << endl;
//				}
//			}
//			else{
//				double survive;												//�l�`�̐��萶�����ԁi�����l�F���g�̋����j
//				int estimateX = repPtr->getPos().getX();
//				int estimateY = repPtr->getPos().getY();
//				double estimateSpeedX = repPtr->getDifX() / repPtr->getInterval();
//				double estimateSpeedY = repPtr->getDifY() / repPtr->getInterval();
////				cout << estimateX << "\t" << estimateY << "\t" << repPtr->getDifX() << "\t" << repPtr->getDifY() << "\t" << repPtr->getInterval() << endl;
//				double A = pow(estimateSpeedX, 2.0) + pow(estimateSpeedY, 2.0);
//				double B = 2 * ((estimateX - center.getX()) * estimateSpeedX + (estimateY - center.getY()) * estimateSpeedY);
//				double C = pow(estimateX - center.getX(), 2.0) + pow(estimateY - center.getY(), 2.0) - MARANGE * MARANGE;
//				survive = (sqrt(pow(B, 2.0) - 4 * A * C ) - B) / (2 * A);
////				cout << id << "--" << survive << "\t(" << estimateX << "," << estimateY << ")  " 
////					<< "(" << estimateSpeedX << "," << estimateSpeedY << ")" 
////					<< sqrt(pow(estimateSpeedX, 2.0) + pow(estimateSpeedY, 2.0)) * 1000000.0 << "\t" << candSurvive << "--" << candidate<< endl;
//				if(survive > candSurvive){
//					candSurvive = survive;
//					candidate = id;
//				}
//			}
//					 }
//			break;
//		case PredictDistance:{
//			if(d > MARANGE){
//				if(d < candDist){
//					candDist = d;
//					candidate = id;
//				}
//			}
//			else{
//				double survive;												//�l�`�̐��萶�����ԁi�����l�F���g�̋����j
//				int estimateX = repPtr->getPos().getX();
//				int estimateY = repPtr->getPos().getY();
//				double estimateSpeedX = repPtr->getDifX() / repPtr->getInterval();
//				double estimateSpeedY = repPtr->getDifY() / repPtr->getInterval();
//				double A = pow(estimateSpeedX, 2.0) + pow(estimateSpeedY, 2.0);
//				double B = 2 * ((estimateX - center.getX()) * estimateSpeedX + (estimateY - center.getY()) * estimateSpeedY);
//				double C = pow(estimateX - center.getX(), 2.0) + pow(estimateY - center.getY(), 2.0) - MARANGE * MARANGE;
//				survive = (sqrt(pow(B, 2.0) - 4 * A * C ) - B) / (2 * A);
////				cout << id << "--" << survive << "\t(" << estimateX << "," << estimateY << ")  " 
////					<< "(" << estimateSpeedX << "," << estimateSpeedY << ")" 
////					<< sqrt(pow(estimateSpeedX, 2.0) + pow(estimateSpeedY, 2.0)) * 1000000.0 << "\t" << candSurvive << " -- " << candidate<< "\t" << d << "\t" << myDist << endl;
//				if(survive > candSurvive && d < myDist){
//					candDist = d;
//					candSurvive = survive;
//					candidate = id;
//				}
//			}
//					 }
//			break;
	}
}

//**********************************************************************************************************//
// �_�C�N�X�g���@�ɂ��o�H���l�����^�}���`�p�X���[�e�B���O												//
// ����(SIMU�C�ԐM�p�P�b�g�̈���(�o�H�ŏ�)�C�o�H�Ō�C���ݎ����C�o�H�v���p�P�b�g�C���݃m�[�h�j				//
// Fumihisa Yamanobe					//
//**********************************************************************************************************//
PACKET* MA::makeMultiRoute(SIMU* sPtr, short sid, short did, SIMUTIME now, PACKET* repPtr, short hid){

	PACKET* newPtr = new PACKET(sPtr, now, PACKET::MrRep, RREQDSR, hid, hid , sid, -1);
	NODE* nPtr = sPtr->node[hid];														//���݃m�[�h�̃|�C���^
	nPtr->path[sid].clear();
	nPtr->path1[sid].clear();
	nPtr->path2[sid].clear();
	nPtr->path[sid].push_back(hid);
	nPtr->path1[sid].push_back(hid);
	nPtr->path2[sid].push_back(hid);
	newPtr->path.push_back(hid);

	for(short i = 0; i < (short)sPtr->node.size(); i++){
		double xdiff = nodePos[i].getX() - lastPos[i].getX();
		double ydiff = nodePos[i].getY() - lastPos[i].getY();
		double tdiff = nodePos[i].getT().dtime() - lastPos[i].getT().dtime();
		if(tdiff == 0 || now.dtime() == nodePos[i].getT().dtime())
			estimatePos[i] = nodePos[i];
		else{
			estimatePos[i].setX((int)(nodePos[i].getX() + xdiff / tdiff * (now.dtime() - nodePos[i].getT().dtime())));
			estimatePos[i].setY(nodePos[i].getY() + ydiff / tdiff * (now.dtime() - nodePos[i].getT().dtime()));
			estimatePos[i].setT(now);
		}
	}

	for(short i = (short)repPtr->mamrPath.size() - 1; i >= 0; i--){
		nPtr->path[sid].push_back(repPtr->mamrPath[i]);											//�m�[�h�̌o�H���̏�������	
		newPtr->path.push_back(repPtr->mamrPath[i]);											//���v���C�p�P�b�g�̌o�H���̏�������
	}
	newPtr->increSize(repPtr->mamrPath.size() * 4);												//�o�H��񕪂����p�P�b�g�T�C�Y�𑝉�
	newPtr->setSeq(nPtr->getSeq());															//�V�[�P���X�ԍ��̕t�^
	nPtr->increSeq();																		//�V�[�P���X�ԍ��̃C���N�������g

	short dest = repPtr->getReqDest();
	int nid = sid;
	int packetSize = 0;
	

	grouping(sPtr, sid, dest);					//�S�m�[�h���O���[�s���O

	vector<bool> checkFlag;
	vector<double> cost;
	vector<vector<int>> cand;
	int nextCheck;
	for(int i = 0; i < (int)sPtr->node.size(); i++){						//vector�^��������
		checkFlag.push_back(false);
		cost.push_back(100000);
		cand.push_back(vector<int>(1));
	}
	checkFlag[nid] = true;
	cost[nid] = 0;	
	

	//�V���O���p�X����
	for(;;){
		//cout << "!!!�P" << endl;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			//cout << "!!!!!!�P" << endl;
			if(i != nid){
				if(dist(sPtr->ma[0]->estimatePos[i], sPtr->ma[0]->estimatePos[nid]) <= RANGE * 0.9){
//				if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[nid]) <= RANGE * 0.9){
					double linkCost = 1;
					if(cost[i] > cost[nid] + linkCost){
						cand[i].clear();
						cand[i].push_back(nid);
						cost[i] = cost[nid] + linkCost;
						//cout << "cost 1 " << cost[i] << endl;
					}
					else if(cost[i] == cost[nid] + linkCost){
						//cout << "cost 2 " << cost[i] << endl;
						cand[i].push_back(nid);
					}
				}
			}
		}
		nextCheck = nid;
		double minCost = 10000;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			//cout << "cost " << i << "\t" << cost[i] << "\t" << minCost << endl;
			if(checkFlag[i] == false && cost[i] < minCost){
				//cout << "test 3.1" << endl;
				nextCheck = i;
				minCost = cost[i];
			}
		}
		//cout << "test next1 " << nextCheck << "\t" << dest << "\t" << nid << endl;
		if(nextCheck == dest || nextCheck == nid){
			break;
		}
		nid = nextCheck;
		checkFlag[nid] = true;
	}

	packetSize = 0;
	if(repPtr != NULL)
		packetSize = (int)repPtr->reqPath.size();
	route.clear();
	//cout << "test 6" << endl;
	if(nextCheck == dest){
		//cout << "test 7" << endl;
		int hid = dest;
		route.push_back(dest);
		//cout << "test 8" << endl;
		while(hid != sid){
			//cout << "test 9" << endl;
			bool flag = false;
			for(int i = 0; i < (int)cand[hid].size(); i++){
				if(cand[hid][i] == sid){
					 flag = true;
					 break;
				}
			}
			//cout << "test 10" << endl;
			if(flag == true)
				hid = sid;
			else{
				hid = cand[hid][genrand() % (int)cand[hid].size()];
				route.push_back(hid);
			}
			//cout << "test 11" << endl;
		}
		route.push_back(hid);
		newPtr->setSize(ROUTEREPMASIZE + 4 * (packetSize + (int)route.size()));
	   //cout << sid  << "  make Dyk route !!!! " << endl;
		//cout << " test route " <<  route.size() << endl;
		for(int i = 0; i < (int)route.size(); i++){											//�Z�o���ꂽ�o�H�������p�P�b�g�ɃR�s�[
			newPtr->reqPath.push_back(route[i]);	
			//cout << route[i] << "--->";
		}
		//cout << endl;	
	}
	else{
		//cout << "no Dyk route " << endl;
		//cout << "test 12" << endl;
		newPtr->reqPath.push_back(dest);
		newPtr->setSize(ROUTEREPMASIZE + 4 * packetSize);
	}
	//�V���O���p�X����---end---


	//�}���`�p�X�P����
	int nid1 = sid;
	int packetSize1 = 0;
	vector<bool> checkFlag1;
	vector<double> cost1;
	vector<vector<int>> cand1;
	int nextCheck1;
	for(int i = 0; i < (int)sPtr->node.size(); i++){						//vector�^��������
		checkFlag1.push_back(false);
		cost1.push_back(100000);
		cand1.push_back(vector<int>(1));
	}
	checkFlag1[nid1] = true;
	cost1[nid1] = 0;		
	for(int i = 0; i < (int)sPtr->node.size(); i++)
		if(sPtr->node[i]->getGroupe() != NODE::M_PATH1)
			checkFlag1[i] = true;
	checkFlag1[dest] = false;
	sPtr->node[dest]->setGroupe(NODE::M_PATH1);
	for(;;){
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(i != nid1){
				if(dist(sPtr->ma[0]->estimatePos[i], sPtr->ma[0]->estimatePos[nid1]) <= RANGE * 0.9 && (sPtr->node[i]->getGroupe() == NODE::M_PATH1 || i == dest)){
//				if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[nid1]) <= RANGE * 0.9 && (sPtr->node[i]->getGroupe() == NODE::M_PATH1 || i == dest)){
					double linkCost = 1;
					if(cost1[i] > cost1[nid1] + linkCost){
						cand1[i].clear();
						cand1[i].push_back(nid1);
						cost1[i] = cost1[nid1] + linkCost;
					}
					else if(cost1[i] == cost1[nid1] + linkCost){
						cand1[i].push_back(nid1);
					}
				}
			}
		}
		nextCheck1 = nid1;
		double minCost = 10000;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(checkFlag1[i] == false && cost1[i] < minCost){
				nextCheck1 = i;
				minCost = cost1[i];
			}
		}
		if(nextCheck1 == dest || nextCheck1 == nid1){
			break;
		}
		nid1 = nextCheck1;
		checkFlag1[nid1] = true;
	}
	packetSize = 0;
	if(repPtr != NULL)
		packetSize = (int)repPtr->reqPath1.size();
	route1.clear();
	if(nextCheck1 == dest){
		int hid = dest;
		route1.push_back(dest);
		while(hid != sid){
			bool flag = false;
			for(int i = 0; i < (int)cand1[hid].size(); i++){
				if(cand1[hid][i] == sid){
					 flag = true;
					 break;
				}
			}
			if(flag == true)
				hid = sid;
			else{
				hid = cand1[hid][genrand() % (int)cand1[hid].size()];
				route1.push_back(hid);
			}
		}
		route1.push_back(hid);
		newPtr->setSize(ROUTEREPMASIZE + 4 * (packetSize1 + (int)route1.size()));
		for(int i = 0; i < (int)route1.size(); i++){											//�Z�o���ꂽ�o�H�������p�P�b�g�ɃR�s�[
			newPtr->reqPath1.push_back(route1[i]);	
		}
	}
	else{
		newPtr->reqPath1.push_back(dest);
		newPtr->setSize(ROUTEREPMASIZE + 4 * packetSize1);
	}
	//�}���`�p�X�P����---end---


	//�}���`�p�X2����
	int nid2 = sid;
	int packetSize2 = 0;
	vector<bool> checkFlag2;
	vector<double> cost2;
	vector<vector<int>> cand2;
	int nextCheck2;
	for(int i = 0; i < (int)sPtr->node.size(); i++){						//vector�^��������
		checkFlag2.push_back(false);
		cost2.push_back(100000);
		cand2.push_back(vector<int>(1));
	}
	checkFlag2[nid2] = true;
	cost2[nid2] = 0;		
	for(int i = 0; i < (int)sPtr->node.size(); i++)
		if(sPtr->node[i]->getGroupe() != NODE::M_PATH2)
			checkFlag2[i] = true;
	checkFlag2[dest] = false;
	sPtr->node[dest]->setGroupe(NODE::M_PATH2);
	for(;;){
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(i != nid2){
			if(dist(sPtr->ma[0]->estimatePos[i], sPtr->ma[0]->estimatePos[nid2]) <= RANGE * 0.9 && (sPtr->node[i]->getGroupe() == NODE::M_PATH1 || i == dest)){
//			if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[nid2]) <= RANGE * 0.9 && (sPtr->node[i]->getGroupe() == NODE::M_PATH2 || i == dest)){
					double linkCost = 1;
					if(cost2[i] > cost2[nid2] + linkCost){
						cand2[i].clear();
						cand2[i].push_back(nid2);
						cost2[i] = cost2[nid2] + linkCost;
					}
					else if(cost2[i] == cost2[nid2] + linkCost){
						cand2[i].push_back(nid2);
					}
				}
			}
		}
		nextCheck2 = nid2;
		double minCost = 10000;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(checkFlag2[i] == false && cost2[i] < minCost){
				nextCheck2 = i;
				minCost = cost2[i];
			}
		}
		if(nextCheck2 == dest || nextCheck2 == nid2){
			break;
		}
		nid2 = nextCheck2;
		checkFlag2[nid2] = true;
	}
	packetSize = 0;
	if(repPtr != NULL)
		packetSize = (int)repPtr->reqPath2.size();
	route2.clear();
	if(nextCheck2 == dest){
		int hid = dest;
		route2.push_back(dest);
		while(hid != sid){
			bool flag = false;
			for(int i = 0; i < (int)cand2[hid].size(); i++){
				if(cand2[hid][i] == sid){
					 flag = true;
					 break;
				}
			}
			if(flag == true)
				hid = sid;
			else{
				hid = cand2[hid][genrand() % (int)cand2[hid].size()];
				route2.push_back(hid);
			}
		}
		route2.push_back(hid);
		newPtr->setSize(ROUTEREPMASIZE + 4 * (packetSize2 + (int)route2.size()));
		for(int i = 0; i < (int)route2.size(); i++){											//�Z�o���ꂽ�o�H�������p�P�b�g�ɃR�s�[
			newPtr->reqPath2.push_back(route2[i]);	
		}
	}
	else{
		newPtr->reqPath2.push_back(dest);
		newPtr->setSize(ROUTEREPMASIZE + 4 * packetSize2);
	}
	//�}���`�p�X2����---end---

	////�m�[�h�̈ʒu����\��(�}���`�p�X�������o�����ꍇ�̂ݏo��)
	//static int count = 0;
	//if(newPtr->reqPath1.size() > 2  && newPtr->reqPath2.size() > 2){
	//	cout << "\n\n\n\nsingle" << count << endl;
	//	for(short i = 0; i < newPtr->reqPath.size(); i++)
	//		cout << sPtr->ma[0]->nodePos[newPtr->reqPath[i]].getX() << "," <<  sPtr->ma[0]->nodePos[newPtr->reqPath[i]].getY() << endl; 
	//	cout << "multi1" << endl;
	//	for(short i = 0; i < newPtr->reqPath1.size(); i++)
	//		cout << sPtr->ma[0]->nodePos[newPtr->reqPath1[i]].getX() << "," <<  sPtr->ma[0]->nodePos[newPtr->reqPath1[i]].getY() << endl; 
	//	cout << "multi2" << endl;
	//	for(short i = 0; i < newPtr->reqPath2.size(); i++)
	//		cout << sPtr->ma[0]->nodePos[newPtr->reqPath2[i]].getX() << "," <<  sPtr->ma[0]->nodePos[newPtr->reqPath2[i]].getY() << endl; 
	//	cout << endl;

	//	cout << "all" << endl;
	//	for(short i = 0; i < sPtr->node.size(); i++)
	//		cout << sPtr->ma[0]->nodePos[i].getX() << "," << sPtr->ma[0]->nodePos[i].getY() << endl;
	//	exit(1);
	//}



	//cout << "newPtr1---" << newPtr->reqPath1.size() << endl;
	//cout << "newPtr2---" << newPtr->reqPath2.size() << endl;


	//cout << "\n\n\n\n group test !! \nspos: ";
	//sPtr->ma[0]->nodePos[sid].show();
	//cout << "dpos: ";
	//sPtr->ma[0]->nodePos[dest].show();
	//cout << "node2pos: ";
	//sPtr->ma[0]->nodePos[2].show();
	//cout << "group:";
	//sPtr->node[2]->showGroupe();
	//cout << "end...\n\n\n\n\n";

	return newPtr;
}

void MA::grouping(SIMU* sPtr, short sid, short dest)
{
	//�}���`�p�X�e�X�g�p
	double x1,y1;	//���M���p
	double x2,y2;	//����p
	double a,b;
	x1 = (double)sPtr->ma[0]->nodePos[sid].getX();
	y1 = (double)sPtr->ma[0]->nodePos[sid].getY();
	x2 = (double)sPtr->ma[0]->nodePos[dest].getX();
	y2 = (double)sPtr->ma[0]->nodePos[dest].getY();
	bool fb = false, fs = false, fd = false;

	//�X�����b���`�F�b�N
	if(x1 == x2)					//�X�����b�̏ꍇ
		fb = true;					//�t���O�𗧂Ă�
	else{							//�����łȂ��ꍇ
		a = (y2 - y1) / (x2 - x1);									//s��d�̌X��
		b = ((x1 * y2) - (x2 * y1)) / (x1 - x2);	//y�Ƃ̌�_
	}

	//�O���[�s���O
	if(fb == true){																//�X�����b�̏ꍇ
		//cout << "�X���b" << endl;
		for(int i = 0; i < (int)sPtr->node.size(); i++){									//�m�[�h�̐��������[�v����
			if(sPtr->ma[0]->nodePos[i].getX() < x1 - RANGE/2)							//x1��x2�������͂��Ȃ̂łƂ肠����x1
				sPtr->node[i]->setGroupe(NODE::M_PATH1);													//�}���`�p�X�O���[�v�P
			else if(sPtr->ma[0]->nodePos[i].getX() > x1 + RANGE/2)
				sPtr->node[i]->setGroupe(NODE::M_PATH2);														//�}���`�p�X�O���[�v�Q
			else
				sPtr->node[i]->setGroupe(NODE::S_PATH);														//�V���O���p�X�O���[�v
		}
	}
	else if(a == 0){															//�X�����[�̏ꍇ
		//cout << "�X���\" << endl;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(sPtr->ma[0]->nodePos[i].getY() > y1 + RANGE/2)												//y1��y2�������͂��Ȃ̂łƂ肠����y1
				sPtr->node[i]->setGroupe(NODE::M_PATH1);														//�}���`�p�X�O���[�v�P
			else if(sPtr->ma[0]->nodePos[i].getY() < y1 - RANGE/2)
				sPtr->node[i]->setGroupe(NODE::M_PATH2);														//�}���`�p�X�O���[�v�Q
			else
				sPtr->node[i]->setGroupe(NODE::S_PATH);														//�V���O���p�X�O���[�v
		}
	}
	else if(a != 0){															//�X��������ꍇ
		//cout << "�X������" << endl;
		for(int i = 0; i < (int)sPtr->node.size(); i++){
			if(sPtr->ma[0]->nodePos[i].getY() > (a * sPtr->ma[0]->nodePos[i].getX()) + b + (RANGE/2 * sqrt( a * a + 1 )))		//y = ax+b+r��a^2+1
				sPtr->node[i]->setGroupe(NODE::M_PATH1);														//�}���`�p�X�O���[�v�Ps
			else if(sPtr->ma[0]->nodePos[i].getY() < (a * sPtr->ma[0]->nodePos[i].getX()) + b - (RANGE/2 * sqrt( a * a + 1)))
				sPtr->node[i]->setGroupe(NODE::M_PATH2);														//�}���`�p�X�O���[�v�Q
			else
				sPtr->node[i]->setGroupe(NODE::S_PATH);														//�V���O���p�X�O���[�v
		}
	}
	else{					//�ǂ��̏����ɂ�����Ȃ��ꍇ
		cout << "??? new dijkstra at ma.cpp" << endl, exit(1);					//��O����
	}

	//cout << "\n\n\ngrouping test !!" << endl;
	//sPtr->ma[0]->nodePos[sid].show();
	//sPtr->ma[0]->nodePos[dest].show();
	//cout << a << endl;
	//cout << b << endl;
	//for(int i = 0; i < (int)sPtr->node.size(); i++){
	//	cout << "path test " << i << " pos:";
	//	sPtr->ma[0]->nodePos[i].show();
	//	sPtr->node[i]->showGroupe();
	//	cout << endl;
	//}

}

//	
//	//�X���ɂ���Ă͌v�Z���ʂ��c��Ȑ����ƂȂ�\��������H�H�H				BOSS�Ɋm�F
//
//	//�m�[�h�̈ʒu���ǂ̃O���[�v�ɏ������邩���߂�
//
//	////s��d�̂ǂ����x�����������@����������true�ɂ���  �ǂ���������ꍇ�͂ǂ����false
//	//if(x1 < x2)
//	//	fs = true;
//	//else
//	//	fd = true;
//
//	//cout << "a: " << a << "   b: " << b << endl;
//
//	if(fb == true){																//�X�����b��������
//		//cout << "�X���b" << endl;
//		for(int i = 0; i < (int)node.size(); i++){									//�m�[�h�̐��������[�v����
//			if(node[i]->getPos().getX() < x1 - RANGE/2)							//x1��x2�������͂��Ȃ̂łƂ肠����x1�@�@�@n.getX()�͌�X�ύX����D�S�Ẵm�[�h���O���[�s���O����
//				node[i]->setGroupe(1);													//�}���`�p�X�O���[�v�P
//			else if(node[i]->getPos().getX() > x1 + RANGE/2)
//				node[i]->setGroupe(2);														//�}���`�p�X�O���[�v�Q
//			else
//				node[i]->setGroupe(3);														//�V���O���p�X�O���[�v
//		}
//	}
//	else if(a != 0){															//�X��������ꍇ
//		//cout << "�X������" << endl;
//		for(int i = 0; i < (int)node.size(); i++){
//			if(node[i]->getPos().getY() > (a * node[i]->getPos().getX()) + b + (RANGE/2 * sqrt( a * a + 1 )))		//y = ax+b+r��a^2+1
//				node[i]->setGroupe(1);														//�}���`�p�X�O���[�v�Ps
//			else if(node[i]->getPos().getY() < (a * node[i]->getPos().getX()) + b - (RANGE/2 * sqrt( a * a + 1)))
//				node[i]->setGroupe(2);														//�}���`�p�X�O���[�v�Q
//			else
//				node[i]->setGroupe(3);														//�V���O���p�X�O���[�v
//		}
//	}
//	else if(a == 0){															//�X�����\�̏ꍇ
//		//cout << "�X���\" << endl;
//		for(int i = 0; i < (int)node.size(); i++){
//			if(node[i]->getPos().getY() > y1 + RANGE/2)												//y1��y2�������͂��Ȃ̂łƂ肠����y1
//				node[i]->setGroupe(1);														//�}���`�p�X�O���[�v�P
//			else if(node[i]->getPos().getY() < y1 - RANGE/2)
//				node[i]->setGroupe(2);														//�}���`�p�X�O���[�v�Q
//			else
//				node[i]->setGroupe(3);														//�V���O���p�X�O���[�v
//		}
//	}
//	else{
//		cout << "??? new dijkstra at ma.cpp" << endl;
//	}
//
//
//	//�m�[�h�̃O���[�v�������Ƀ_�C�N�X�g���@��3�̌o�H�����
//	//new Dijkstra for Multi-path
//	
//	vector<bool> checkFlag;
//	vector<double> cost;
//	vector<vector<int>> cand;
//	int nid = reqSid;
//	int nextCheck;
//
//	for(int i = 0; i < (int)node.size(); i++){						//������
//		checkFlag.push_back(false);
//		cost.push_back(100000);
//		cand.push_back(vector<int>(1));
//	}
//	checkFlag[nid] = true;
//	cost[nid] = 0;
//
//	//�ȉ��̏����������Ă��邩BOSS�Ɋm�F
//	for(;;){
//		for(int i = 0; i < (int)node.size(); i++){
//			if(i != nid){
//				if(dist(sPtr->ma[0]->estimatePos[i], sPtr->ma[0]->nodePos[i]) <= 4000 
//					&& dist(sPtr->ma[0]->estimatePos[i], sPtr->ma[0]->estimatePos[nid]) <= 9000){
//					//cout << "link " << nid << " --- " << i << endl;
//					double linkCost = 1;
//					double difX_i = sPtr->ma[0]->nodePos[i].getX() - sPtr->ma[0]->lastPos[i].getX();
//					double difY_i = sPtr->ma[0]->nodePos[i].getY() - sPtr->ma[0]->lastPos[i].getY();
//					double difT_i = (sPtr->ma[0]->nodePos[i].getT().dtime() - sPtr->ma[0]->lastPos[i].getT().dtime()) / 1000000.0;
//					double vX_i = difX_i / difT_i;
//					double vY_i = difY_i / difT_i;
//					double difX_nid = sPtr->ma[0]->nodePos[nid].getX() - sPtr->ma[0]->lastPos[nid].getX();
//					double difY_nid = sPtr->ma[0]->nodePos[nid].getY() - sPtr->ma[0]->lastPos[nid].getY();
//					double difT_nid = (sPtr->ma[0]->nodePos[nid].getT().dtime() - sPtr->ma[0]->lastPos[nid].getT().dtime()) / 1000000.0;
//					double vX_nid = difX_nid / difT_nid;
//					double vY_nid = difY_nid / difT_nid;
//					double a = vX_i - vX_nid;
//					double b = sPtr->ma[0]->estimatePos[i].getX() - sPtr->ma[0]->estimatePos[nid].getX();
//					double c = vY_i - vY_nid;
//					double d = sPtr->ma[0]->estimatePos[i].getY() - sPtr->ma[0]->estimatePos[nid].getY();
//					double lifeTime = (-(a * d - b * c) + sqrt((a * a + c * c) * RANGE * RANGE - pow((a * d) - (b * c), 2.0))) / ((a * a) + (c * c));
//					//linkCost = 1.0 / lifeTime;			//�������R�����g�A�E�g�ŏ]���̃z�b�v�����l�������o�H�\�z�ɂȂ�
//					//cout<<"�����N�R�X�g��"<<linkCost<<endl;
//					if(linkCost < 0)
//						linkCost = 100000;
//					if(cost[i] > cost[nid] + linkCost){
//						cand[i].clear();
//						cand[i].push_back(nid);
//						cost[i] = cost[nid] + linkCost;
//					}
//					else if(cost[i] == cost[nid] + linkCost)
//						cand[i].push_back(nid);
//				}
//			}
//		}
//		nextCheck = nid;
//		double minCost = 10000;
//		for(int i = 0; i < (int)node.size(); i++){
//			if(checkFlag[i] == false && cost[i] < minCost){
//				nextCheck = i;
//				minCost = cost[i];
//			}
//		}
//		if(nextCheck == did || nextCheck == nid)
//			break;
//		nid = nextCheck;
//		checkFlag[nid] = true;
//	}
//
//	//�����܂ł����Ă��邩BOSS�Ɋm�F���Ă�����ā@�p�P�b�g�ƃ��[�g�̍���
//
//	//�O���[�v���ƂɃ��[�g�����
//	//for(;;){
//	//	for(int i = 0; i < (int)node.size(); i++){
//	//		if(i != nid){
//	//			if(node[i]->getGroupe() == 3);
//	//				//�V���O���p�X�p���[�e�B���O
//	//			else if(node[i]->getGroupe() == 1);
//	//				//�}���`�p�X�p���[�e�B���O�P
//	//			else if(node[i]->getGroupe() == 2);
//	//				//�}���`�p�X�p���[�e�B���O�Q
//	//			else{
//	//				cout << "node:" << i << " �̓O���[�v�������Ă��܂���D" << endl;
//	//				return;
//	//			}
//	//		}
//	//	}
//	//}
//
//
//
//}
