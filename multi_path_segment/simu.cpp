#include <stdlib.h>
#include <math.h>
#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//�����Z���k�c�C�X�^�����O���錾�i�m�[�h�ړ��ȊO�p�j
unsigned long genrand2();																			//�����Z���k�c�C�X�^�����O���錾�i�m�[�h�ړ��p�j

//[0,1)�̗�������
//�����i�Ȃ��j
//�߂�l�F�i���������j
double rand_d(void){																						
	return genrand2() / 4294967296.0;
}

//�|�A�\�����z�ɏ]�����ԊԊu�����̔���
//�����i1�b������̔������[�g�j
//�߂�l�F(�������� �ʕb)
inline int poison(double rate){
	return (int)(-(log(1 - rand_d()) / rate * 1000000));
}

//�R���X�g���N�^
SIMU::SIMU(area id, mac mid){	
	if(AREA % GRIDNUM != 0)																			//�G���A�T�C�Y���i�q���Ŋ���؂�Ȃ��ꍇ
		cout << "grid size error" << endl, exit(1);												//�i�q�T�C�Y�G���[�G���[
	last.setTime(-1, 0);																				//���O�C�x���g�����̏�����
	areaId = id;																						//�G���A�^�C�v�̏�����
	macId = mid;
	areaSize = AREA;																					//�G���A�T�C�Y�̏�����	
	gridLine = GRIDNUM + 1;																			//�i�q���̏�����
	for(char i = 0; i < gridLine; i++){															//�O���b�h�|�C���g�̐ݒ�
		for(char j = 0; j < gridLine; j++){
			LOCATION gridPos(i * areaSize / GRIDNUM, j * areaSize / GRIDNUM);
			gridPoint[i][j] = gridPos;
		}
	}
	staReqRate = 0;																					//STA�v���������[�g
	staReqNum = 0;																						//STA�v�����i���b�V���l�b�g���[�N�Ŏg�p�j
	staReqFail = 0;																					//�v�����s��
	for(char i = 0 ; i < 50; i++)
		packetSize[i] = 0;
	transDelay = 0;
	transDelayCnt = 0;
	delay = 0;
	totalSendUDP = 0;
	totalReceiveUDP = 0;
	totalTcpData = 0;
	totalTcpTime = 0;
	MAlocation = false;																				//MA���ʒu���Ǘ������邩�i�f�t�H���g�͂��Ȃ��j

	rate = 0;

	counter1 = 0;
	counter2 = 0; 
}

//�f�X�g���N�^
SIMU::~SIMU(){
	for(short i = 0; i < (short)node.size(); i++)
		delete node[i];
	for(int i = 0; i < (int)ma.size(); i++)
		delete ma[i];
}

//�V�~�����[�V�����C�x���g����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void SIMU::processEvent(){
	LINK<EVENT>* first = list.getFirst();														//���X�g�̐擪
	EVENT* eventPtr = first->getObject();														//�擪�Ɋi�[����Ă���I�u�W�F�N�g
	delete list.remove(first);																		//�i�[���Ă�������폜
	if(eventPtr->getType() == EVENT::Abort){													//�V�~�����[�V�����I���C�x���g�̏ꍇ
		now.setSec(TIMELIMIT);																		//���ݎ������V�~�����[�V�����I�������ɐݒ�
		return;
	}
	now = eventPtr->getEventTime();																//���ݎ����������C�x���g�̔��������ɐݒ�
////�f�o�b�O�p
//	eventPtr->show(9.0);																				//�����C�x���g�\��
	//if(now.dtime() > 753.8){
	//	//cout << node[57]->path[37].size() << endl;
	//	if(node[57]->path[37].size() == 0)
	//		cout << " out " << endl, exit(1);
	//}
	//node[263]->getMAC()->show(0.88);
	//if(node[657]->getMAC()->getOrPtr())
	//	cout << node[657]->getMAC()->getOrPtr() << "\t" << node[657]->getMAC()->getOrPtr()->getSize() << endl;
	//if(eventPtr->getType() == EVENT::Packet)
	//	cout << eventPtr << endl;
	//if(now.dtime() > 1.025)
	//	cout << node[679]->getMAC()->getListInsert() << endl;
	if(!now.isPositive())																			//���ݎ������}�C�i�X�̏ꍇ�ɂ̓G���[
		cout << "negative time " << eventPtr->getType() << endl, now.show(), exit(1);	
	if(timeCompare(last, now)){																	//���ݎ��������O������菬�����ꍇ�ɂ��G���[
		cout << eventPtr->getType() << "  back time\nnow\t";
		now.show();
		cout << "previous\t";
		last.show();
		exit(1);
	}
	if(eventPtr->getEventTime().getLessMuSec() != eventPtr->getNid()){				//�����̃}�C�N�������ƃm�[�hID����v���Ă��Ȃ���΃G���[				
		cout << "time setting error " << eventPtr->getEventTime().getLessMuSec() 
			<< "\t" << eventPtr->getNid() << endl, exit(1);
	}
	if(!eventPtr->process())																		//�����G���[�łȂ��ꍇ�Y���C�x���g�����֐����Ăяo��
		delete eventPtr;																							//�C�x���g�����̃t���O���Ԃ��Ă��������
	last = now;																									//���̃C�x���g�̂��߂Ɍ��ݎ����𒼑O�����ֈړ�
//	eventPtr->show(628.4879);																				//�����C�x���g�\��
	//if(timeCompare(now, SIMUTIME(120,800000)))
	//	list.orderShow();
		//exit(1);
}


//�m�[�h�̍쐬
//�����i�쐬�����C���Ŏ����C�ʒu���C�ړ��^�C�v�C���[�e�B���O�^�C�v�j
//�߂�l�i�Ȃ��j
void SIMU::newNode(SIMUTIME active, SIMUTIME off, LOCATION pos, NODE::move mid, NODE::route rid)
{
	NODE* nPtr = new NODE(this, (short)node.size(), active, off, pos, mid, rid);	//�m�[�h�I�u�W�F�N�g�̍쐬
	if(timeCompareMu(nPtr->getActiveTime(), now))											//���ݎ������������J�n��������Ȃ�
		nPtr->setIsActive(false);																		//������ԃt���O������
	else																									//���ݎ������������J�n�������O�Ȃ�
		nPtr->setIsActive(true);																		//������ԃt���O�𗧂Ă�
	if(nPtr->getPos().getT().getSec() == -1)													//�ʒu��񂪐ݒ肳��Ă��Ȃ��Ȃ�
		nPtr->posInit();																					//�m�[�h�z�u����	
	if(mid != NODE::NO){																				//�Î~�m�[�h�łȂ����
		nPtr->speedSet();																					//���x�̐ݒ�
		nPtr->destInit();																					//�ړI�n�̏�����
	}
	if(rid == NODE::PRO || areaId == MESH 
		|| ((nPtr->getRoute() == NODE::GEDIR || nPtr->getRoute() == NODE::MAMR) && !nPtr->getNeighborLocEnable())){		//�v���A�N�e�B�u���[�e�B���O�����b�V���l�b�g���[�N���r�[�R���ɂ��ʒu���擾�����̏ꍇ
		SIMUTIME beaconTime = active + genrand() % BEACONINT;									//�r�[�R���^�C�~���O�̌���
		BEACON* beaconPtr = new BEACON(this, (short)node.size(), beaconTime);			//�r�[�R���I�u�W�F�N�g�̍쐬		
		list.insert(beaconPtr);																			//�쐬�I�u�W�F�N�g�̃C�x���g�o�^
	}
	short i;
	for(i = 0; i < (short)node.size(); i++){													//�����m�[�h�̃��[�e�B���O�e�[�u���ǉ�
		ROUTING_DATA* rPtr = new ROUTING_DATA(-1, -1);											//���[�e�B���O�f�[�^�I�u�W�F�N�g
		node[i]->routing.push_back(rPtr);															//�m�[�h�Ƀ��[�e�B���O�I�u�W�F�N�g��ǉ�
		node[i]->nodePos.push_back(LOCATION(-1,-1, -1));												//�ʒu���e�[�u�����쐬
		short last = (short)node[i]->path.size();													//���݂̌o�H�e�[�u���T�C�Y���i�ǉ��e�[�u���̗v�f�ԍ��Ɠ����j
		short last1 = (short)node[i]->path1.size();													//�o�H�e�[�u���T�C�Y���i�}���`�p�X�P�p�j
		short last2 = (short)node[i]->path2.size();													//�o�H�e�[�u���T�C�Y���i�}���`�p�X�Q�p�j
		node[i]->path.push_back(vector<short>(1));													//�o�H�e�[�u����ǉ�
		node[i]->path1.push_back(vector<short>(1));													//�o�H�e�[�u����ǉ��i�}���`�p�X�P�j
		node[i]->path2.push_back(vector<short>(1));													//�o�H�e�[�u����ǉ��i�}���`�p�X�Q�j
		node[i]->path[last][0] = i;																	//�ǉ��e�[�u�����̏�����
		node[i]->path1[last][0] = i;																//�ǉ��e�[�u�����̏�����
		node[i]->path2[last][0] = i;																//�ǉ��e�[�u�����̏�����
		node[i]->requestTime.push_back(SIMUTIME(-10,0));										//���[�g���N�G�X�g�^�C���������̓o�^
		node[i]->floodSeq.push_back(-1);																//�t���b�f�B���O�^�C���������̓o�^
	}
	node.push_back(nPtr);																			//�m�[�h���X�g�ւ̍쐬�m�[�h�̒ǉ�
	ROUTING_DATA* rPtr;																				//���[�e�B���O�f�[�^�I�u�W�F�N�g
	for(short j = 0; j < (short)node.size(); j++){											//�쐬�m�[�h�̃��[�e�B���O�e�[�u���쐬															
		if(j != i )																							//���[�e�B���O�̑Ώۂ������ȊO�Ȃ�
			rPtr = new ROUTING_DATA(-1, -1);																//�I�u�W�F�N�g�̒��g�͕s��		
		else{																									//�Ώۂ��������g�Ȃ�
			rPtr = new ROUTING_DATA(j, 0);																//���g������Ƃ����I�u�W�F�N�g���쐬
			rPtr->setSeq(0);																					//���g�̃V�[�P���X�ԍ������l��0
		}
		node[i]->routing.push_back(rPtr);															//���[�e�B���O�I�u�W�F�N�g��ǉ�
		node[i]->nodePos.push_back(LOCATION(-1,-1, -1));												//�ʒu���e�[�u�����쐬
		node[i]->path.push_back(vector<short>(1));												//�o�H�e�[�u����ǉ�
		node[i]->path1.push_back(vector<short>(1));												//�o�H�e�[�u����ǉ�
		node[i]->path2.push_back(vector<short>(1));												//�o�H�e�[�u����ǉ�
		node[i]->path[j][0] = i;																		//�o�H�e�[�u���̏�����
		node[i]->path1[j][0] = i;																		//�o�H�e�[�u���̏�����
		node[i]->path2[j][0] = i;																		//�o�H�e�[�u���̏�����
		node[i]->requestTime.push_back(SIMUTIME(-10,0));										//���[�g���N�G�X�g�^�C���������̓o�^
		node[i]->floodSeq.push_back(-1);																//�t���b�f�B���O�^�C���������̓o�^
	}
	if(node.size() == 1)																				//�ŏ��̃m�[�h�I�u�W�F�N�g�̂�
		list.insert(nPtr);																				//�ړ��p�C�x���g�̓o�^
}

//STA�v���̍쐬
//�����i�����Ԋu�C�v�����j
//�߂�l�i�Ȃ��j
void SIMU::makeStaReq(short tval, double rate){
	interval = tval;
	staReqRate = rate;
	staReqNum = 0;
	staReqFail = 0;
}

//���b�V���l�b�g���[�N�̍쐬
//�����iMAP���CSTA���j
//�߂�l�i�Ȃ��j
void SIMU::makeMesh(mesh id, short mnum, short snum, double rate, SIMUTIME stay){
	meshId = id;
	mapNum = mnum;
	staNum = snum;
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(0, 0), NODE::NO, NODE::PRO);	//�������ɔz�u
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(0, AREA), NODE::NO, NODE::PRO);//�E�����ɔz�u
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(AREA, 0), NODE::NO, NODE::PRO);//����ɔz�u
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(AREA, AREA), NODE::NO, NODE::PRO);//�E��ɔz�u
	LOCATION pos;
	for(short i = 4; i < mapNum; i++)															//���̑���MAP�̓����_���ɔz�u
		newNode(0, SIMUTIME(TIMELIMIT + 1, 0), pos, NODE::NO, NODE::PRO);
	NODE* nPtr;																							//�����`�F�b�N���̃m�[�h�I�u�W�F�N�g
	char* connectFlag;																				//������Ԃ������t���O(0�͖������C1�͌��������̐�𖢃`�F�b�N�C2�͌��������̐���`�F�b�N�ς݁j
	connectFlag = (char*)malloc(sizeof(char)*mapNum);										//�t���O�̃������̈���m��
	bool meshFlag = false;																			//���b�V���l�b�g���[�N�Ƃ��Đ������Ă邩�������t���O
	short trialCnt = 0;
	while(!meshFlag){																					//���b�V���l�b�g���[�N�Ƃ��Đ������ĂȂ�����ȉ������s
		if(++trialCnt > mapNum)
			cout << "mesh network cannot be generated " << endl, exit(1);
		connectFlag[0] = 1;																				//�ŏ���MAP�͌������Ă���Ɖ���
		for(short i = 1; i < mapNum; i++)															//����MAP�͌����_�Ŗ�����
			connectFlag[i] = 0;
		bool checkFlag = true;																			//�V�K����MAP�����������������t���O
		while(checkFlag){																					//�V�K����MAP���������ȉ������s
			checkFlag = false;																				//�ŏ��͐V�K�������Ȃ��Ƃ���
			for(short i = 0; i < mapNum; i++){
				if(connectFlag[i] == 1){																		//����MAP�ł��̐悪���`�F�b�N�Ȃ�
					connectFlag[i] = 2;																				//�`�F�b�N�ς݂ɕύX
					nPtr = node[i];																					//�`�F�b�N���m�[�h�I�u�W�F�N�g�̐ݒ�
					for(short j = 0; j < mapNum; j++){
						if(connectFlag[j] == 0 && dist(nPtr->getPos(), node[j]->getPos()) < RANGE){		//�������m�[�h�Ń`�F�b�N���̑��M�͈͂ɂ���Ȃ�
							connectFlag[j] = 1;																				//������Ԃɂ���
							checkFlag = true;																					//�V�K����MAP���݂̃t���O�𗧂Ă�
						}
					}
				}
			}
		}
		meshFlag = true;																					//���b�V���l�b�g���[�N�����Ɖ���
		for(short i = 0; i < mapNum; i++){
			if(connectFlag[i] == 0){
				meshFlag = false;
				break;
			}
		}
		if(!meshFlag){																						//���b�V���l�b�g���[�N�����t���O������Ă�ꍇ
			short disconnectId;																				//�񌋍�MAP��ID
			short replaceId;																					//�ʒu�ύXMAP��ID
			char maxConnect = 0;																				//�ő匋�������N��
			char minConnect = 100;																			//�ŏ����������N��
			for(short i = 0; i < mapNum; i++){															//�S�Ă�MAP�ɑ΂���
				char cnt = 0;
				for(short j = 0; j < mapNum; j++)															//���������N���𒲂ׂ�
					if(i != j && dist(node[i]->getPos(), node[j]->getPos()) < RANGE)
						cnt++;
				if(cnt < minConnect){																			//���������N�����ŏ��Ȃ�
					minConnect = cnt;
					disconnectId = i;																					//�ŏ�����MAP�̍X�V
				}
				if(cnt > maxConnect){																			//���������N�����ő�Ȃ�
					maxConnect = cnt;
					replaceId = i;																						//�ő匋��MAP�̍X�V
				}
			}
			node[replaceId]->replace(disconnectId);													//�ő匋��MAP���ŏ�����MAP�̋߂��ɍĔz�u
		}
	}
	free(connectFlag);
	if(meshId == CENTRAL){																			//�W���Ǘ������̏ꍇ
		double minDist = AREA;
		for(short i = 0; i < mapNum; i++){
			if(dist(node[i]->getPos(), LOCATION(AREA / 2, AREA /2)) < minDist){
				minDist = dist(node[i]->getPos(), LOCATION(AREA / 2, AREA /2));
				centerId = i;
			}
		}
	}
	for(short i = 0; i < staNum; i++){															//STA�̓����_���ɔz�u
		newNode(0, SIMUTIME(TIMELIMIT + 1, 0), pos, NODE::RWP, NODE::PRO);
		double minDist = dist(node[0]->getPos(), node[mapNum + i]->getPos());			//�ڑ�MAP�Ƃ̋����i�����l��MAP0�Ƃ̋����j
		short connectMap = 0;																			//�ڑ�MAPID�i�����l0�j
		for(short j = 1; j < mapNum; j++){															//�ł��߂�MAP�����߂�
			if(dist(node[j]->getPos(), node[mapNum + i]->getPos()) < minDist){
				minDist = dist(node[j]->getPos(), node[mapNum + i]->getPos());
				connectMap = j;
			}
		}
		if(minDist >= RANGE)																				//�ł��߂�MAP�Ƃ̋������ʐM�͈͈ȏ�Ȃ�
			connectMap = -1;																					//�ڑ�MAP�͖���
		gab[mapNum + i] = LAB(connectMap);															//���zGAB�ւ̓o�^
		for(short j = 0; j < (short)ma.size(); j++)												//�G�[�W�F���g�ւ�GAB�o�^
			ma[j]->gab[i] = LAB(connectMap);
		for(short j = 0; j < mapNum; j++)															//�eMAP��GAB�ւ̓o�^�i�ŏ������͗��z�I�Ȓl�j
			node[j]->gab.push_back(LAB(connectMap));
		node[mapNum + i]->setMAP(connectMap);														//STA�ɐڑ�MAP��o�^
		node[mapNum + i]->routing[connectMap]->setHop(1);										//���[�e�B���O���̏�����
		node[mapNum + i]->routing[connectMap]->setNext(connectMap);
		node[connectMap]->routing[mapNum + i]->setHop(1);										
		node[connectMap]->routing[mapNum + i]->setNext(mapNum + i);
	}
	short source = genrand2() % mapNum;
	if(rate > 0){
		TIMEOUT* toPtr = new TIMEOUT(this, SIMUTIME(poison(rate)), TIMEOUT::MakeUdp, source);
		toPtr->setRate(rate);
		list.insert(toPtr);
	}
	if(meshId == AGENT){
		//����G�[�W�F���g�̍쐬�ƌo�H�̐ݒ�
		LOCATION pos;																						//�_�~�[�ʒu���
		short maId = 0;																					//MA��ID�i������MA�����ꍇ�ɂ͂����𒲐��j
		short nodeId = 0;																					//�ŏ��ɑ؍݂���m�[�h�ʒu
		MA* maPtr = new MA(this, 0, pos, 0, MA::Distance, maId, nodeId);					//�G�[�W�F���g�I�u�W�F�N�g�̍쐬
		maPtr->setSize(30 * 1000 + 12 * staNum);													//�T�C�Y�̐ݒ�iGAB��1�m�[�h������12�o�C�g�Ɗ��Z�j
		maPtr->setStayTime(stay);																		//�e�m�[�h�ł̑؍ݎ��Ԃ̐ݒ�
		for(short i = 0; i < mapNum; i++){												
			maPtr->meshRoute.push_back(i);															//���̗�ł͌o�H�͒P����ID��
			maPtr->gab.push_back(LAB());																//GAB���̏�����
		}
		maPtr->meshRoute.push_back(0);																//�o�H�̍Ō�ɍŏ��̃m�[�h������
	}
}

//�ڑ�MAP�̊m�F
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void SIMU::checkConnectMap(void){
	for(short i = mapNum; i < mapNum + staNum; i++){										//�eSTA�ɂ��ă`�F�b�N����
		bool flag = false;																				//�ڑ��m�F�K�v���t���O
		if(node[i]->getMAP() == -1)																	//����܂Ŗ��ڑ��Ȃ�
			flag = true;																						//�m�F�t���O�𗧂Ă�
		else if(dist(node[i]->getPos(), node[node[i]->getMAP()]->getPos()) >= RANGE){	//�ڑ�MAP�Ƃ̋������ʐM�͈͂𒴂����ꍇ
			//if(i == 255)
			//	cout << "before " << node[i]->getMAP() << endl;
			node[node[i]->getMAP()]->gab[i - mapNum] = LAB(-1, now);								//�Y��STA�Ɋւ���LAB���̏�����
			node[i]->routing[node[i]->getMAP()]->setHop(-1);										//���[�e�B���O���̏�����
			node[i]->routing[node[i]->getMAP()]->setNext(-1);
			node[node[i]->getMAP()]->routing[i]->setHop(-1);
			node[node[i]->getMAP()]->routing[i]->setNext(-1);
			flag = true;																						//�m�F�t���O�𗧂Ă�
		}
		if(flag){																							//�m�F�t���O�������Ă�����	
			double minDist = dist(node[i]->getPos(), node[0]->getPos());						//�ڑ�MAP�Ƃ̋����i�����l��MAP0�Ƃ̋����j
			short connectMap = 0;																			//�ڑ�MAPID�i�����l0�j
			for(short j = 1; j < mapNum; j++){															//�ł��߂�MAP�����߂�
				if(dist(node[i]->getPos(), node[j]->getPos()) < minDist){
					minDist = dist(node[i]->getPos(), node[j]->getPos());
					connectMap = j;
				}
			}
			if(minDist >= RANGE)																				//�ł��߂�MAP�Ƃ̋������ʐM�͈͈ȏ�Ȃ�
				connectMap = -1;																					//�ڑ�MAP�͖���
			else																									//�V�ڑ�MAP�����݂���Ȃ�
				node[connectMap]->sendLab(i, node[i]->getMAP());																	//LAB���̑��M
//			if(i == 255)
//				cout << i << " �ڑ��ύX  " << connectMap << endl;
			gab[i] = LAB(connectMap, now);																//���zGAB�ւ̓o�^
			node[i]->setMAP(connectMap);																	//STA�ɐڑ�MAP��o�^
			if(connectMap != -1){
				node[i]->routing[connectMap]->setHop(1);													//���[�e�B���O���̐ݒ�
				node[i]->routing[connectMap]->setNext(connectMap);
				node[connectMap]->routing[i]->setHop(1);
				node[connectMap]->routing[i]->setNext(i);
			}
		}
	}
}

