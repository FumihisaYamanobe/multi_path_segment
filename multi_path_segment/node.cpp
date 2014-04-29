#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//�����Z���k�c�C�X�^���������֐��̊O���錾
unsigned long genrand2();																			//�����Z���k�c�C�X�^���������֐��̊O���錾
int poison(double);
extern int cntP;
extern int cntS;
extern int cntF;


//extern double rand_d();
////[0,1)�̗�������
////�����i�Ȃ��j
////�߂�l�i���������j
double rand2_d(void){																						
	return genrand2() / 4294967296.0;
}

//
////�m�[�h�I�u�W�F�N�g�̃R���X�g���N�^
//NODE::NODE(LIST<EVENT>* list, short idVal, double timeVal, double off, move mid, LOCATION pos, route rid, char sector, 
//			  ANTENNAE::antenna antennaType, double power, bool catMode):EVENT(active, EVENT::Node){	
NODE::NODE(SIMU* ptr, short idval, SIMUTIME tval, SIMUTIME offval, LOCATION pos, move mid, route rid):EVENT(ptr, tval, EVENT::Node, idval){
	bufferPtr = new BUFFER(BUFFERSIZE);	
	mPtr = new MAC(ptr, tval, idval);
	cPtr = new CHANNEL(ptr, mPtr, tval, idval);
	now = ptr->getNow();
	id = idval;																							//ID�ԍ��̐ݒ�
	activeTime = tval;																				//�����J�n�����̐ݒ�
	activeTime.setLessMuSec(id);																	//�����J�n�����i�ʖ����j�̐ݒ�
	offTime = offval;																					//�I�������̐ݒ�
	offTime.setLessMuSec(id);																		//�I�������i�ʖ����j�̐ݒ�
	position = pos;																					//�ʒu���̐ݒ�
	destId = -1;																						//�ړI�nID��-1�ŏ�����
	deriveTime = genrand2() % 10;																	//�ʒu�擾�^�C�~���O
	moveId = mid;																						//�ړ��^�C�v�̐ݒ�
	isRecieveingSignal = false;																	//��M���t���O�i�����l�͔��M�j
	routeId = rid;																						//���[�e�B���O�v���g�R���^�C�v
	seq = 0;																								//�V�[�P���X�ԍ�
	aodvSeq = 0;																						//AODV�p�V�[�P���X�ԍ�
	overflow = 0;																						//�o�b�t�@�I�[�o�t���[�ɂ����p�t���[����
	neighborLocEnable = false;																		//�����ŗאڃm�[�h�̈ʒu������ɓ�����邩
	ptr->gab.push_back(LAB());																		//���zGAB���
	for(short i = 0; i < 200; i++)
		receivedPacketList.push_back(RECEIVED_PACKET(0, -1, -1, -1, -1));
	pmode = WAIT;																						//�d�͏���^�C�v�i�����l�͑ҋ@�j
	usedPower = 0;																						//����d��
	calcPowerTime = 0;																				//����d�͌v�Z�p����
	lastInformPos = pos;																				//���݈ʒu���ŋ߂̍L���ʒu�ɐݒ�


//	remainPower = power;																			//�c���d��
//	powerDecreTime = 0;																			//�d�͏���v�Z����
//	relayRate = 0;																					//���p���i�Z���t�B�b�V���m�[�h�p�j
//	categoryMode = catMode;																		//�J�e�S�����[�h�̐ݒ�
//	catNum = (categoryMode == false) ? 1 : 4;	
//�J�e�S�����[�h�ɂ��J�e�S����������
//	macPtr = new MAC(id);																			//MAC�I�u�W�F�N�g�̍쐬																				//MAC�I�u�W�F�N�g�̓o�^
//	activeCategory = 0;																			//���M���������Ă���J�e�S��
//	block = 0;																						//�ʐM�ؒf�ɂ����p�t���[����
//	retransfail = 0;																				//�đ�����ɂ����p�t���[����
}

NODE::~NODE(){
//	cout << "delete node " << this << endl;
	delete bufferPtr;
	delete mPtr;
	delete cPtr;
}

//�m�[�h�C�x���g�����iprocess�֐��j
bool NODE::process(void){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	now = sPtr->getNow();																			//���ݎ���
	// cout << "node " << now.dtime() << endl;
	// sPtr->list.orderShow();
	// cout << sPtr->node[79]->getAodvSeq() << endl;
	for(short i = 0; i < (short)sPtr->node.size(); i++){									//���݂���m�[�h����������
		NODE* nPtr = sPtr->node[i];																	//�����Ώۃm�[�h�I�u�W�F�N�g
		nPtr->setNow(now);																				//�m�[�h�̃C�x���g�������X�V
		switch(nPtr->getMove()){																		//�ړ��^�C�v�ɂ��ꍇ����
			case NO:																							//�ړ��Ȃ��̏ꍇ
				nPtr->noMove();
				break;																							//�ʒu�ύX�͂��Ȃ�
			case RWP:																						//�����_���E�F�C�|�C���g�̏ꍇ
				if(timeCompare(now, nPtr->getPauseRelease()))										//��~�������������ݒ肩�߂��Ă����ꍇ
					nPtr->randomWayPoint();																		//�����_���E�F�C�|�C���g����				
				break;
			case GRID:																						//�O���b�h�ړ��̏ꍇ
				if(timeCompare(now, nPtr->getPauseRelease()))										//��~�������������ݒ肩�߂��Ă����ꍇ
					nPtr->gridWayPoint();																		//�O���b�h�ړ�����				
				break;
		}
		if(now.getLessSec() / 100000 == nPtr->getDeriveTime())								//�ʒu���擾�^�C�~���O�̏ꍇ
			nPtr->posDerive();																				//�ʒu���̎擾
		nPtr->neighborList.clear();																	//�אڃm�[�h���̏�����
	}
	if(sPtr->getMAloc()){																			//MA�ֈʒu��񑗐M�t���O�������Ă���Ȃ�
		for(short i = 0; i < (short)sPtr->node.size(); i++)
			sPtr->node[i]->sendInfoemLoc();															//�ʒu���p�P�b�g���M
		if(now.dtime() > 30){
			static double maxDif = 0;
			static LOCATION posA, posB;
			static int cnt, cnt2;
			static double totalDif;
			static int totalLinkDif;
			double nowTotalDif = 0;
			for(short i = 0; i < (short)sPtr->node.size(); i++){
				double dif = dist(sPtr->ma[0]->nodePos[i], sPtr->node[i]->getPos());
				totalDif += dif;
				nowTotalDif += dif;
				if(dif > maxDif)
					maxDif = dif;
			}
			for(short i = 0; i < (short)sPtr->node.size() - 1 ; i++){
				for(short j = i + 1; j < (short)sPtr->node.size(); j++){
					if(dist(sPtr->node[i]->getPos(), sPtr->node[j]->getPos()) <= RANGE){
						if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[j]) > RANGE)
							totalLinkDif++;
					}
					else{
						if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[j]) <= RANGE)
							totalLinkDif++;
					}
				}
			}
			cnt2 += sPtr->node.size() * (sPtr->node.size() - 1) / 2;
			cnt += sPtr->node.size();
			//if(now.getSec() % 1 == 0 && now.getLessSec() == 0)
			//	cout << now.dtime() << "," << (double)sPtr->counter2 / sPtr->counter1 << "," << maxDif << "," 
			//	<< totalDif / cnt << "," << (double)totalLinkDif / (cnt2 += sPtr->node.size() * (sPtr->node.size() - 1) / 2 * 100) 
			//	<< "\t" << cntP << "\t" << cntF << "\t" << cntS << endl;
		}
	}
	calcDistance();																					//�m�[�h�ԋ����v�Z����їאڃm�[�h�o�^
	if(sPtr->getArea() == SIMU::MESH)
		sPtr->checkConnectMap();
	for(short i = 0; i < (short)sPtr->node.size(); i++)									//���݂���m�[�h����������
		if(sPtr->node[i]->getMAC()->checkIdling() &&  sPtr->node[i]->getMAC()->getState() == MAC::Idle)
			sPtr->node[i]->checkPacket();
	addTime(100000);																					//���̃C�x���g���������֍X�V
	sPtr->list.insert(this);																		//�C�x���g���X�g�֒ǉ�
	static bool flag = false;																		//STA�v�������t���O�i�~��Ă���Δ����j�̏�����
	if(sPtr->getArea() != SIMU::MESH || flag)													//���b�V���l�b�g���[�N�łȂ����t���O�������Ă����
		return true;																						//�ȍ~�̏����������ɏI���i
	int counter = 0;																					//���[�e�B���O���ݒ萔�̃J�E���^
	for(short i =0 ; i < (short)sPtr->getMAP(); i++){										//�eMAP�̃��[�e�B���O���ݒ�ӏ����J�E���g
		for(short j = 0; j < (short)sPtr->getMAP(); j++){
			if(sPtr->node[i]->routing[j]->getNext() == -1)
				counter++;
		}
	}
	if(!counter){																						//���[�e�B���O���ݒ�ӏ����Ȃ��Ȃ�����
		flag = true;																						//�t���O�𗧂Ă�
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(poison(sPtr->getReqRate())), 
				TIMEOUT::StaReq, genrand2() % sPtr->getSTA() + sPtr->getMAP());			//STA�v���̍쐬
		toPtr->setFlag(false);																			//�t���O�͂��낵�Ă���
		short dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();							//STA�v���̈���
		while(dest == toPtr->getNid())																//���悪���g�Ɠ����ł������
			dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();									//������Đݒ�
		toPtr->setDest(dest);																			//����STA�̓o�^
		sPtr->list.insert(toPtr);																		//�C�x���g���X�g�֓o�^
		for(char i = 0; i < (char)sPtr->ma.size(); i++){										//MA�̐�����
			toPtr = new TIMEOUT(sPtr, now, TIMEOUT::MeshMa, sPtr->ma[i]->getNid());			//�G�[�W�F���g�ړ��^�C���A�E�g�I�u�W�F�N�g�̍쐬
			toPtr->setMa(sPtr->ma[i]);																		//�Ή�MA�̓o�^
			sPtr->list.insert(toPtr);																		//�C�x���g���X�g�֓o�^		
		}
		//short relayCnt[200] = { 0 };
		//for(short i = 0; i < sPtr->getMAP() - 1; i++){
		//	for(short j = i + 1; j < sPtr->getMAP(); j++){
		//		short source = i;
		//		relayCnt[source]++;
		//		while(sPtr->node[source]->routing[j]->getNext() != j){
		//			source = sPtr->node[source]->routing[j]->getNext();
		//			relayCnt[source]++;
		//		}
		//		relayCnt[j]++;
		//	}
		//}
		//for(short i = 0; i < sPtr->getMAP(); i++){
		//	cout << i << "\t" << relayCnt[i] << endl;
		//}
	
	}
	return true;																						//�I�u�W�F�N�g�͔j�����Ȃ�
}

//�m�[�h�ʒu�̏�����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::posInit(void){
	SIMU* sPtr = getSimu();
	int x, y;																							//���W�p�ϐ�
	SIMUTIME t;																							//�����p�ϐ�
	char line = sPtr->getGridLine();																//�i�q����
	LOCATION tmpPos, centerPos;																	//�ʒu���p�ϐ�
	switch(sPtr->getArea()){																		//�G���A���f���ɂ��ꍇ����
	case SIMU::CIRCLE:																				//�~�`�G���A�̏ꍇ
		x = genrand2() % (AREA) - AREA / 2;															//�b��x���W
		y = genrand2() % (AREA) - AREA / 2;															//�b��y���W
		tmpPos.set(x, y, t);																				//�b��ʒu���
		centerPos.set(0, 0, t);																			//�V�~�����[�V�����G���A�̒��S
		while(dist(tmpPos, centerPos) > sPtr->getAreaSize()){									//�b��ʒu���~�̊O�Ȃ��
			x = genrand2() % (AREA) - AREA / 2;															//�b��x���W�̍X�V
			y = genrand2() % (AREA) - AREA / 2;															//�b��y���W�̍X�V
			tmpPos.set(x, y, t);																				//�b��ʒu���̍X�V
		}
		position = tmpPos;																				//�b��l���ʒu���֓o�^
		break;
	case SIMU::SQUARE: case SIMU::MESH:															//�����`�G���A��MESH�G���A�̏ꍇ
		x = genrand2() % (AREA);																		//x���W
		y = genrand2() % (AREA);																		//y���W
		position.set(x, y, t);																			//�ʒu���̓o�^
		break;
	case SIMU::GRID:																					//�i�q�G���A�̏ꍇ
		gridId = genrand2() % (line * line);													//�i�q�ԍ��̌���
		position = sPtr->getGridPoint(gridId % line, gridId / line);					//�ʒu���̓o�^
		switch(genrand2() % 4){																		//�ړI�i�q�_�͊O���̕ӂ̂ǂ���
		case 0:
			destId = genrand2() % line;
			break;
		case 1:
			destId = (genrand2() % line) * line + 10;
			break;
		case 2:
			destId = genrand2() % 10 + 110;
			break;
		case 3:
			destId = (genrand2() % line) * line;
			break;
		}
		while(destId == gridId){																	//�ړI�n�����ݒn�Ɠ����ꍇ�łȂ��ꍇ
			switch(genrand2() % 4){																		//�ړI�n�̍Đݒ�
			case 0:
				destId = genrand2() % line;
				break;
			case 1:
				destId = (genrand2() % line) * line + 10;
				break;
			case 2:
				destId = genrand2() % 10 + 110;
				break;
			case 3:
				destId = (genrand2() % line) * line;
				break;
			}
		}
		break;
	}
	posDerive();
	lastInformPos = position;
}

//�m�[�h�Ĕz�u
//�����i�߂Â������m�[�hID�i�Ȃ����-1)�j
//�߂�l�i�Ȃ��j
void NODE::replace(short id){
	LOCATION pos(genrand2() % (AREA), genrand2() % (AREA));							//�Ĕz�u�ƂȂ���W
	if(id != -1){																					//�߂Â������m�[�h�����݂���Ȃ�
		while(dist(getSimu()->node[id]->getPos(), pos) >= RANGE)										//���̃m�[�h�̒ʐM�͈͂ɂȂ�����
			pos = LOCATION(genrand2() % (AREA), genrand2() % (AREA));						//�ꏊ���Č���
	}
	position.setX(pos.getX());
	position.setY(pos.getY());
}


//���x�̐ݒ�
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::speedSet(void){
	speed = rand2_d() * 9.0 + 1.0;																//�ړ����x�̐ݒ�
}

//�ʒu���̎擾
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::posDerive(void){
	int x = position.getX() + (int)((2 * rand2_d() - 1) * LOCERR);						//�擾�ʒu���W�i�����W�j
	int y = position.getY() + (int)((2 * rand2_d() - 1) * LOCERR);						//�擾�ʒu���W�i�����W
	derivePos[1].set(derivePos[0].getX(), derivePos[0].getY(), derivePos[0].getT());													//���O�擾���W�̐ݒ�i�����l�͍ŐV�Ɠ����j
	derivePos[0].set(x, y, position.getT());													//�ŐV�擾���W�̐ݒ�
}

//�ړI�n�̏�����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::destInit(void){
	SIMU* sPtr = getSimu();
	int x, y;																							//���W�p�ϐ�
	double angle;																						//�~�`�G���A�̕Ίp�p�ϐ�
	char newDest;
	char line = sPtr->getGridLine();																//�i�q����
	switch(moveId){																					//�ړ��^�C�v�ɂ��ꍇ����
	case RWP:																							//�����_���E�F�C�|�C���g�̏ꍇ
		switch(sPtr->getArea()){																		//�G���A�^�C�v�ɂ��ꍇ����
		case SIMU::CIRCLE:																				//�~�`�G���A�̏ꍇ
			angle = rand2_d() * 2 * 3.14159265;															//�Ίp
			x = (int)(AREA * cos(angle));																	//�ɍ��W�n�𗘗p�����ړI�n���W�ix�j
			y = (int)(AREA * sin(angle));																	//�ɍ��W�n�𗘗p�����ړI�n���W�iy�j
			break;
		case SIMU::SQUARE: case SIMU::MESH:															//�����`�G���A��MESH�G���A�̏ꍇ
			newDest = genrand2() % 4;																		//�ړI�n��ID������
			while(newDest == destId)																		//���Ɠ����ӂ��I�΂�����
				newDest = genrand2() % 4;																		//�ړI�n��ID���Č���
			destId = newDest;
			switch(destId){																					//4�ӂ̂ǂ�����ړI�n�Ƃ���
			case 0:																								//���ӂ̏ꍇ
				x = genrand2() % (AREA + 1);
				y = 0;
				break;
			case 1:																								//�E�ӂ̏ꍇ
				x = AREA;
				y = genrand2() % (AREA + 1);
				break;
			case 2:																								//��ӂ̏ꍇ
				x = genrand2() % (AREA + 1);
				y = AREA;
				break;
			case 3:																								//���ӂ̏ꍇ
				x = 0;
				y = genrand2() % (AREA + 1);
				break;
			}
			break;
		default:
			cout << "Contradiction between area model and move model" << endl,  exit(1);
			break;
		}
		moveDestPos.set(x, y);																		//�ړI�n�̐ݒ�
		pause.setTime(PAUSE_SEC, PAUSE_MSEC * 1000);											//�ړI�n���B���̒�~���Ԃ̐ݒ�
		pauseReleaseTime.setTime(-1, 0);															//��~���������̏�����
		break;
	case GRID:																							//�i�q�ړ��̏ꍇ
		if(sPtr->getArea() != SIMU::GRID)															//�i�q�G���A�łȂ���΃G���[
			cout << "Contradiction between area model and move model" << endl,  exit(1);
		if(destId % line > gridId % line){															//�ړI�n�����ݒn��蓌�ɂ���ꍇ
			if(destId / line > gridId / line){															//�ړI�n�����ݒn���k�ɂ���ꍇ
				if(genrand2() % 2)																				//1/2�̊m����
					gridId++;																						//���֐i��
				else
					gridId += line;																				//�k�֐i��
			}
			else if(destId / line == gridId / line)													//�ړI�n���^���ɂ���Ȃ�
				gridId++;																							//���֐i��
			else{																									//�ړI�n�����ݒn����ɂ���ꍇ
				if(genrand2() % 2)																				//1/2�̊m����
					gridId++;																						//���֐i��
				else
					gridId -= line;																				//��֐i��
			}
		}
		else if(destId % line == gridId % line){													//�ړI�n�����ݒn���^�k���^��ɂ���ꍇ
			if(destId / line > gridId / line)															//�ړI�n�����ݒn���^�k�ɂ���ꍇ
				gridId += line;																					//�k�֐i��
			else																									//�ړI�n�����ݒn���^�k��ɂ���ꍇ
				gridId -= line;																					//��֐i��
		}
		else{																									//�ړI�n�����ݒn��萼�ɂ���ꍇ
			if(destId / line > gridId / line){															//�ړI�n�����ݒn���k�ɂ���ꍇ
				if(genrand2() % 2)																				//1/2�̊m����
					gridId--;																						//���֐i��
				else
					gridId += line;																				//�k�֐i��
			}
			else if(destId / line == gridId / line)													//�ړI�n���^���ɂ���Ȃ�
				gridId--;																							//���֐i��
			else{																									//�ړI�n�����ݒn����ɂ���ꍇ
				if(genrand2() % 2)																				//1/2�̊m����
					gridId--;																						//���֐i��
				else
					gridId -= line;																				//��֐i��
			}
		}
		if(gridId < 0 || gridId >= line * line)
			cout << "gridId error" << endl, exit(1);
		moveDestPos = sPtr->getGridPoint(gridId % line, gridId / line);				//�ړI�n�ʒu���̓o�^
		pause.setTime(PAUSE_SEC, PAUSE_MSEC * 1000);											//�ړI�n���B���̒�~���Ԃ̐ݒ�
		pauseReleaseTime.setTime(-1, 0);																//��~���������̏�����
		break;
	}
}

//��ړ�����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::noMove(void){
	position.set(position.getX(), position.getY(), getSimu()->getNow());
}

//�����_���E�F�C�|�C���g�ړ�����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::randomWayPoint(void){
	if(toDestination(moveDestPos)){																//�ړI�n�ɒ�������
		destInit();
		speedSet();																							//���x�̐ݒ�
		pauseReleaseTime = now;																			//��~���������̏�����
		pauseReleaseTime.addTime(pause);																//��~���������̐ݒ�
	}
	return;
}

//�i�q�ړ�����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::gridWayPoint(void){
	SIMU* sPtr = getSimu();
	if(toDestination(moveDestPos)){																//�ړI�n�ɒ�������
		if(gridId == destId){																			//�ŏI�ړI�n�ɒ�������
			char line = sPtr->getGridLine();
			switch(genrand2() % 4){																		//���̍ŏI�ړI�n�͊O���̕ӂ̂ǂ���
			case 0:
				destId = genrand2() % line;
				break;
			case 1:
				destId = (genrand2() % line) * line + 10;
				break;
			case 2:
				destId = genrand2() % 10 + 110;
				break;
			case 3:
				destId = (genrand2() % line) * line;
				break;
			}
			while(destId % line == gridId % line || destId / line == gridId / line){		//���ݒn�ƖړI�n�������ӂɂ���Ȃ�
				switch(genrand2() % 4){																		//���̍ŏI�ړI�n���Č��肷��
				case 0:
					destId = genrand2() % line;
					break;
				case 1:
					destId = (genrand2() % line) * line + 10;
					break;
				case 2:
					destId = genrand2() % 10 + 110;
					break;
				case 3:
					destId = (genrand2() % line) * line;
					break;
				}
			}
		}
		destInit();
		speedSet();																							//���x�̐ݒ�
		pauseReleaseTime = now;																			//��~���������̏�����
		pauseReleaseTime.addTime(pause);																//��~���������̐ݒ�
	}
}	

//�ړI�n�܂łɂ̈ړ�����
//�����i�ړI�n�j
//�߂�l�i�ړI�n�ɓ��B������^�C���B���Ȃ���΋U�j
bool NODE::toDestination(LOCATION dest){
	bool flag;																							//�ړI�n���B�t���O
	int x = position.getX();																		//���݈ʒu�i�����W�j
	int y = position.getY();																		//���݈ʒu�i�����W�j
	int dx = dest.getX();																			//�ړI�n�i�����W�j
	int dy = dest.getY();																			//�ړI�n�i�����W�j
	int difX = dx - x;																				//�ړI�l�Ƃ̍��i�����W�j
	int difY = dy - y;																				//�ړI�l�Ƃ̍��i�����W�j
	int newX, newY;																					//�ړ���̍��W
	double dist = sqrt(pow((double)difX, 2.0) + pow((double)difY, 2.0));				//�ړI�n�܂ł̎c�苗��
	if(dist > speed * 100 * 0.1){																	//�ړ��������c�苗�����Z���ꍇ
		bool isNegative = (difX < 0);																	//�����W�̍������ł��邩���`�F�b�N
		if(isNegative)																						//���̏ꍇ
			difX = -difX;																						//���ɕ����ϊ�
		difX = (int)(difX / dist * speed * 100.0 * 0.1 + 0.5);								//�����W�̈ړ������v�Z�i�[���͎l�̌ܓ��j
		if(isNegative)																						//��قǕ��ł�������
			difX = -difX;																						//�����𕉂ɖ߂�
		isNegative = (difY < 0);																		//�����W�̍������ł��邩���`�F�b�N
		if(isNegative)																						//���̏ꍇ
			difY = -difY;																						//���ɕ����ϊ�
		difY = (int)(difY / dist * speed * 100.0 * 0.1 + 0.5);								//�����W�̈ړ������v�Z�i�[���͎l�̌ܓ��j
		if(isNegative)																						//��قǕ��ł�������
			difY = -difY;																						//�����𕉂ɖ߂�
		newX = x + difX;																					//�ړ���̍��W�i�����W�j�X�V
		newY = y + difY;																					//�ړ���̍��W�i�����W�j�X�V
		flag = false;																						//�ړI�n���B�t���O���U��
	}
	else{																									//�ړ��������c�苗����蒷���ꍇ�i�ړI�n���B�j
		newX = dx;																							//�ړ���̍��W�͖ړI�n
		newY = dy;																							//�ړ���̍��W�͖ړI�n
		flag = true;																						//�ړI�n���B�t���O��^��
	}
	position.set(newX, newY, getSimu()->getNow());											//���݈ʒu�̍X�V
	return flag;																						//�ړI�n���B�t���O��Ԃ�
}

//���m�[�h�Ƃ̋����v�Z
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::calcDistance(void){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	if(routeId == AODV)																				//������AODV���[�e�B���O�Ȃ�
		for(short i = 0; i < (short)sPtr->node.size(); i++)								//�m�[�h�������`�F�b�N
			sPtr->node[i]->increAodvSeq();															//AODV�p�V�[�P���X�̃C���N�������g
	for(short i = 0; i < (short)sPtr->node.size() - 1; i++){								//�m�[�h�������`�F�b�N
		if(!sPtr->node[i]->getIsActive())															//�m�[�h���񊈓���ԂȂ�
			continue;																							//����ȍ~�̏����͂��Ȃ�
		for(short j = i + 1; j < (short)sPtr->node.size(); j++){								//�v�Z�Ώۂ��`�F�b�N
			if(!sPtr->node[j]->getIsActive())															//�Ώۂ��񊈓���ԂȂ�
				continue;																							//����ȍ~�̏����͂��Ȃ�
			double distance = dist(sPtr->node[i]->getPos(), sPtr->node[j]->getPos());		//�Ώۂ܂ł̋���
			if(sPtr->getArea() == SIMU::MESH && (i >= sPtr->getMAP() || j >= sPtr->getMAP()));
			else if(distance < RANGE){																			//�������ʐM�͈͓��Ȃ�
				sPtr->node[i]->routing[j]->setNext(j);														//�Ώۂ������̗̂אڃm�[�h���Đݒ�
				sPtr->node[i]->routing[j]->setHop(1);
				sPtr->node[j]->routing[i]->setNext(i);														//�Ώۂ��猩�Ď������אڃm�[�h
				sPtr->node[j]->routing[i]->setHop(1);
				if(routeId == DSR || routeId == MAMR){																				//DSR���[�e�B���O�̏ꍇ
					sPtr->node[i]->path[j].clear();																//�Ώۂ܂ł̌o�H����������
					sPtr->node[i]->path[j].push_back(i);														//�Ώۂ܂ł̌o�H��ݒ�
					sPtr->node[i]->path[j].push_back(j);
					sPtr->node[j]->path[i].clear();																//�Ώۂ��玩���܂ł̌o�H����������
					sPtr->node[j]->path[i].push_back(j);														//�Ώۂ��玩���܂ł̌o�H��ݒ�
					sPtr->node[j]->path[i].push_back(i);
				}
			}
			else{																									//�������ʐM�͈͊O�Ȃ�
				if(sPtr->node[i]->routing[j]->getNext() == j){											//����܂łɑΏۂ��אڃm�[�h�ɂȂ��Ă�����
					sPtr->node[i]->routing[j]->setNext(-1);													//�Ώۂ�אڃm�[�h���珜�O����
					sPtr->node[i]->routing[j]->setHop(-1);
					sPtr->node[j]->routing[i]->setNext(-1);													//�Ώۂ���݂Ď������אڃm�[�h�ł͂Ȃ��Ȃ�
					sPtr->node[j]->routing[i]->setHop(-1);
					if(routeId == DSR || routeId == MAMR){														//DSR���[�e�B���O�̏ꍇ
						sPtr->node[i]->path[j].clear();															//�Ώۂ܂ł̌o�H����������
						sPtr->node[i]->path[j].push_back(i);													//�Ώۂ܂ł̌o�H��ݒ�
						sPtr->node[j]->path[i].clear();															//�Ώۂ��玩���܂ł̌o�H����������
						sPtr->node[j]->path[i].push_back(j);													//�Ώۂ��玩���܂ł̌o�H��ݒ�
					}
				}
				if(routeId == PRO){																				//�v���A�N�e�B�u���[�e�B���O�̏ꍇ
					for(short k = 0; k < (short)sPtr->node.size(); k++){									//�S�Ẵm�[�h�ɑ΂���
						if(sPtr->node[i]->routing[k]->getNext() == j){											//�Ώۂ�]���m�[�h�ɐݒ肵�Ă���o�H�̏ꍇ
							sPtr->node[i]->routing[k]->setNext(-1);													//���̌o�H��������
							sPtr->node[i]->routing[k]->setHop(-1);
						}
						if(sPtr->node[j]->routing[k]->getNext() == i){											//�Ώۂ���̓]���m�[�h�Ɏ��g���ݒ肳��Ă���ꍇ
							sPtr->node[j]->routing[k]->setNext(-1);													//���̌o�H��������
							sPtr->node[j]->routing[k]->setHop(-1);
						}
					}
				}
			}
			if(distance > RANGE * 2)																		//�������ʐM�͈͂�2�{�i�L�����A�Z���X�\�����j���傫���ꍇ
				continue;																							//����ȍ~�̏����͂��Ȃ�
			if(distance < RANGE){
				if(sPtr->node[i]->getNeighborLocEnable())
					sPtr->node[i]->nodePos[j] = sPtr->node[j]->getDerivePos(0);
				if(sPtr->node[j]->getNeighborLocEnable())
					sPtr->node[j]->nodePos[i] = sPtr->node[i]->getDerivePos(0);
			}
			NEIGHBOR_LIST listI(i, distance);																//��������Ώۂւ̗אڏ��I�u�W�F�N�g�i�ʒu���Ȃ��j
			NEIGHBOR_LIST listJ(j, distance);																//��������Ώۂւ̗אڏ��I�u�W�F�N�g�i�ʒu���Ȃ��j
			short first = 0;																					//����ȍ~�̓��X�g�ւ̑}������
			short last = sPtr->node[i]->neighborList.size();										//�}���A���S���Y���ɂ��Ă͏ȗ�
			short here = (first + last) / 2;
			while(first < last){
				if(sPtr->node[i]->neighborList[here].getDist() > distance)
					last = here;
				else
					first = here + 1;
				here = (first + last) / 2;
			}
			sPtr->node[i]->neighborList.insert(sPtr->node[i]->neighborList.begin() + here, listJ);
			first = 0;
			last = sPtr->node[j]->neighborList.size();
			here = (first + last) / 2;
			while(first < last){
				if(sPtr->node[j]->neighborList[here].getDist() > distance)
					last = here;
				else
					first = here + 1;
				here = (first + last) / 2;
			}
			sPtr->node[j]->neighborList.insert(sPtr->node[j]->neighborList.begin() + here, listI);
		}
	}
}

//���M�o�b�t�@�̃p�P�b�g���݃`�F�b�N
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�|�C���^�j
//�߂�l�F�i�Ȃ��j
void NODE::checkPacket(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = getSimu()->getNow();
	//if(timeCompare(now, SIMUTIME(14,900000)) && getNid() == 248){
	//if(getNid() == 334){	
	//	cout << now.dtime() << "\t" << getNid() << " �`�F�b�N�p�P�b�g " << sPtr->node[334]->path[450].size() << endl;
	//	sPtr->node[334]->queueShow();
	//}
	if(!bufferPtr->queue.size())																	//�o�b�t�@����̏ꍇ
		return;																								//����ȍ~�̏����͂��Ȃ�
	PACKET* pPtr = bufferPtr->queue[0];																//�擪�̃p�P�b�g�I�u�W�F�N�g																					
	if(sPtr->list.find(pPtr))																		//���ɃC�x���g���X�g�ɓo�^����Ă���ꍇ
		return;																								//����ȍ~�̏����͂��Ȃ�
	bool flag = true;																					//�p�P�b�g���������Ɋւ���t���O
	while(timeCompare(now, pPtr->getSendStart() + SIMUTIME(TTLTIME))){				//�p�P�b�g�̐������Ԃ��߂��Ă�p�P�b�g���������
		if(pPtr->getType() == PACKET::Tcp && getNid() == pPtr->getSource()){				//�p�P�b�g��TCP�Ŏ��g�����M���Ȃ�
			TIMEOUT* ptr = new TIMEOUT(sPtr, now + 100000, TIMEOUT::Segment, getNid());	//�^�C���A�E�g�I�u�W�F�N�g�̍쐬	
			pPtr->getSeg()->setTimeout(ptr);																//�Z�O�����g�Ƀ^�C���A�E�g��o�^
			ptr->setSegment(pPtr->getSeg());																//�^�C���A�E�g�ɑΉ��Z�O�����g��o�^
			sPtr->list.insert(ptr);																			//�^�C���A�E�g���C�x���g�ɒǉ�
		}
		bufferPtr->decreLength(pPtr->getSize());													//�s�񒷂��p�P�b�g�T�C�Y�������炷
//		cout << now.dtime() << "   old packet discarded!! " << endl;
		delete pPtr;																						//�p�P�b�g�I�u�W�F�N�g������
		bufferPtr->queue.erase(bufferPtr->queue.begin());										//�p�P�b�g������
		if(!bufferPtr->queue.size()){																	//�o�b�t�@����̏ꍇ
			flag = false;																						//�t���O�����낷
			break;																								//while�����I��
		}
		pPtr = bufferPtr->queue[0];																	//���̐擪�p�P�b�g�ɑ΂��ē��l�̏������s��
	}
	if(flag == false)																					//�o�b�t�@����̏ꍇ
		return;																								//����ȍ~�̏����͂��Ȃ�
	PACKET::ptype type = pPtr->getType();														//�p�P�b�g�^�C�v
	if(type == PACKET::Udp || type == PACKET::Tcp || type == PACKET::Ack
		|| type == PACKET::MigRep || type == PACKET::RerrAodv || type == PACKET::RerrDsr
		|| type == PACKET::StaReq || type == PACKET::StaRep || type == PACKET::MapReq
		|| type == PACKET::MapRep || type == PACKET::LabCenter || type == PACKET::LabNeighbor
		|| type == PACKET::MigRep || type == PACKET::InformLoc || type == PACKET::MrRep || type == PACKET::MrReq){//���[�e�B���O����p�P�b�g�ȊO�̃��j�L���X�g�p�P�b�g�̎�
		switch(routeId){													//���[�e�B���O�^�C�v�ɂ��ꍇ����

		//****************************************//
		// MAMR�̏ꍇ�̏���
		// �}���`�p�X�������Ƃ��́Z�Z
		// �V���O���p�X��������Z�Z
		//****************************************//
		case NODE::MAMR:{
			//if()//�Q�̌o�H������ꍇ
			//else if()//1�̌o�H�����Ȃ��ꍇ
			//else //�ǂ���̌o�H���Ȃ��ꍇ
			//cout << "test checkpacket --- " << now.dtime() << endl;
			if(pPtr->getSource() == id){																	//���g�����M���̏ꍇ
				short dest = pPtr->getDest();																//���M����
				if(dest >= (short)sPtr->node.size()){														//���悪MA��������@MA�̒��S���W���g���ĒʐM���s���D
					pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
					pPtr->setDpos(sPtr->ma[dest - sPtr->node.size()]->getCenter());							//dest��pos��MA�̒��l���W�ɂ���
					pPtr->setTime(now);
//					cout << "com for ma --- " << now.dtime() << " \t" << id << endl;
				}
				else{																						//���悪MA�ȊO��������
					if(path[dest].size() <= 1){																//path�Ɍo�H��񂪓����Ă��Ȃ��ꍇ
						if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//�ΏۂɊւ��郋�[�g���N�G�X�g�̍ŋ߂̑��M�������ꍇ 
							pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
							pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
							pPtr->setDpos(sPtr->ma[0]->getCenter());							//dest��pos��MA�̒��l���W�ɂ���						
							pPtr->setReqDest(dest);
							pPtr->increSize(4);
							pPtr->queue(sPtr, true);																//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
							requestTime[dest] = now;													//���[�g���N�G�X�g���M�����̐ݒ�
							cout << "TCP transmission start." << endl; 
							setPath1SegSize((short)TCPDEFAULTSIZE);											//path1��path2��TCP�����Z�O�����g�T�C�Y��ݒ�
							setPath2SegSize((short)TCPDEFAULTSIZE);
							cout << getPath1SegSize() << " " << getPath2SegSize() <<endl;
					//		//cout << "make packet --- " << now.dtime() << "\t" << id << "\t" << type << "\t" << dest << endl;
						}
						else
							return;
					}
					else{
						//path1�Cpath2�Ŋ���U�鏈���@��������̂��Q�l�ɂ���
						if(path[dest].size() == 2 ){
							if(pPtr->path.size() > 0){																	//�}���`�p�X���Ȃ��V���O���p�X������ꍇ
								pPtr->path.clear();																		//�p�P�b�g�̌o�H����������
							}
							for(char i = 0; i < (char)path[dest].size(); i++)										//�o�H���̐ݒ�
								pPtr->path.push_back(path[dest][i]);
							pPtr->increSize(path[dest].size() * 4);
							bufferPtr->increLength(path[dest].size() * 4);
						}
						else if(path1[dest].size() > 1 && path2[dest].size() > 1){								//�}���`�p�X���ǂ��������ꍇ
							//�m�[�h�ɒ��O�܂Ŏg���Ă��o�H���o��������(pathNum)
							if(pathNum[dest] == 2){
								if(pPtr->path.size() > 0){														//�}���`�p�X���Ȃ��V���O���p�X������ꍇ
									pPtr->path.clear();																				//�p�P�b�g�̌o�H����������
								}
								for(char i = 0; i < (char)path1[dest].size(); i++)										//�o�H���̐ݒ�
									pPtr->path.push_back(path1[dest][i]);
								pPtr->increSize(path1[dest].size() * 4);
								bufferPtr->increLength(path1[dest].size() * 4);
								pathNum[dest] = 1;
								pPtr->mpath_check = 1;
								cout << "path1 Seg Size:" << getPath1SegSize() << endl;
							//	cout << "path 1 ���p�I--" << now.dtime() <<  endl;
							}
							else if(pathNum[dest] == 1){
								if(pPtr->path.size() > 0){														//�}���`�p�X���Ȃ��V���O���p�X������ꍇ
									pPtr->path.clear();																				//�p�P�b�g�̌o�H����������
								}
								for(char i = 0; i < (char)path2[dest].size(); i++)										//�o�H���̐ݒ�
									pPtr->path.push_back(path2[dest][i]);
								pPtr->increSize(path2[dest].size() * 4);
								bufferPtr->increLength(path2[dest].size() * 4);
								pathNum[dest] = 2;
								pPtr->mpath_check = 2;
								cout << "path2 Seg Size:" << getPath2SegSize() << endl;
							//	cout << "path 2 ���p�I--" << now.dtime() << "\tdest:" << dest << "\ttype:" << getType() << endl;
							}
							else{
								cout << "pathNum error!!!" << endl, exit(1);
							}
						}
						else if(path[dest].size() > 1){																		//�����Ƃ��́@�V���O���p�X���݂�
							if(timeCompare(now, requestTime[dest] + REQUEST_INT)){
								pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
								pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
								pPtr->setDpos(sPtr->ma[0]->getCenter());							//dest��pos��MA�̒��l���W�ɂ���						
								pPtr->setReqDest(dest);
								pPtr->increSize(4);
								pPtr->queue(sPtr, true);																//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
								requestTime[dest] = now;													//���[�g���N�G�X�g���M�����̐ݒ�
								sPtr->list.insert(pPtr);
								return;
							}
							else{
								if(pPtr->path.size() > 0){																	//�}���`�p�X���Ȃ��V���O���p�X������ꍇ
									pPtr->path.clear();																		//�p�P�b�g�̌o�H����������
								}
								for(char i = 0; i < (char)path[dest].size(); i++)										//�o�H���̐ݒ�
									pPtr->path.push_back(path[dest][i]);
								pPtr->increSize(path[dest].size() * 4);
								bufferPtr->increLength(path[dest].size() * 4);
								pPtr->mpath_check = 0;
							}
							//cout << "�V���O���p�X�I--" << now.dtime() << endl;
						}
						else
							cout << "path error!!!!" << endl, exit(1);

						pPtr->setTime(now);																				//���M�����̍X�V
						pPtr->setDpos(nodePos[dest]);
					}
				}
			}
			else{																//���g�����M���łȂ��ꍇ
//				cout << "not make packet --- " << now.dtime() << endl;
				pPtr->setTime(now);
			}
			break;
			//if(pPtr->getSource() == id){											//���g�����M���̏ꍇ
			//	short dest = pPtr->getDest();
			//	if(path1[dest].size() > 1 && path2[dest].size() > 1){			//�Q�̌o�H������ꍇ
			//		if(pathNum[dest] == 1){
			//			for(char i = 0; i < (char)path1[dest].size(); i++)		//�o�H���̐ݒ�
			//				pPtr->path.push_back(path[dest][i]);					//�m�[�h�̌o�H�����p�P�b�g�ɃR�s�[
			//			pPtr->setTime(now);										//�o�H�\�z���Ԃ̍X�V
			//			pPtr->pathNum = 1;
			//			pathNum[dest] = 2;										//���̌o�H���Q�ɐݒ�
			//		}
			//		else{
			//			for(char i = 0; i < (char)path2[dest].size(); i++)		//�o�H���̐ݒ�
			//				pPtr->path.push_back(path[dest][i]);					//�m�[�h�̌o�H�����p�P�b�g�ɃR�s�[
			//			pPtr->setTime(now);										//�o�H�\�z���Ԃ̍X�V
			//			pPtr->pathNum = 2;
			//			pathNum[dest] = 1;										//���̌o�H���P�ɐݒ�
			//		}
			//	}
			//	else if(path[dest].size() == 2){								//dest���אڃm�[�h�̏ꍇ
			//		for(char i = 0; i < (char)path[dest].size(); i++)			//�o�H���̐ݒ�
			//			pPtr->path.push_back(path[dest][i]);
			//		pPtr->setTime(now);
			//	}
			//	else{															//�ǂ��炩�̌o�H���Ȃ��ꍇ
			//		if(path[dest].size() * path1[dest].size() * path2[dest].size() == 1){ //�S�Ă̌o�H���Ȃ��ꍇ
			//			//�p�P�b�g�̃T�C�Y��RREQDSR���g�p���Ă����̂�
			//			//�p�P�b�g�̍����͂����Ă��邩BOSS�Ɋm�F �����Ă���
			//			pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
			//			pPtr->path.push_back(id);																	//�p�P�b�g�̌o�H���̍쐬
			//			pPtr->increSize(4);
			//			pPtr->queue(sPtr, true);																		//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
			//			requestTime[sPtr->node.size()] = now;														//���[�g���N�G�X�g���M�����̐ݒ�
			//		}
			//		else{														//�ǂ����̌o�H������Ȃ�
			//			if(path[dest].size() > 1){								//path������ꍇ�i�}���`�p�X����Ȃ��o�H�j
			//				for(char i = 0; i < (char)path[dest].size(); i++)			//�o�H���̐ݒ�
			//					pPtr->path.push_back(path[dest][i]);
			//				pPtr->setTime(now);
			//			}
			//			//�V���O���p�X�����݂����C�}���`�p�X�̕Е������݂���ꍇ�͌o�H�\�z�v��������
			//			else if(path1[dest].size() > 1 || path2[dest].size() > 1){
			//				pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
			//				pPtr->path.push_back(id);																	//�p�P�b�g�̌o�H���̍쐬
			//				pPtr->increSize(4);
			//				pPtr->queue(sPtr, true);																	//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
			//				requestTime[sPtr->node.size()] = now;														//���[�g���N�G�X�g���M�����̐ݒ�
			//			}
			//			else{
			//				cout << "???CheckPacket" << endl;
			//			}
			//		}
			//	}
			//}
			//else																//���g�����M���łȂ��ꍇ
			//	pPtr->setTime(now);
			//break;
		//****************************************//
		// MAMR�̏ꍇ�@�ȏ�
		//****************************************//
		}
		case NODE::DSR:																					//DSR�̏ꍇ
			if(pPtr->getSource() == id){																	//���g�����M���̏ꍇ
				short dest = pPtr->getDest();																	//���M����
				if(path[dest].size() == 1){																	//�o�H�����̏ꍇ
					if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//�ΏۂɊւ��郋�[�g���N�G�X�g�̍ŋ߂̑��M�������ꍇ 
						pPtr = new PACKET(sPtr, now, PACKET::RreqDsr, RREQDSR, id, id, dest, -1);		//���[�g���N�G�X�g�p�P�b�g�̍쐬
//						cout << now.dtime() << " route request " << id << endl;
						pPtr->path.push_back(id);																		//�p�P�b�g�̌o�H���̍쐬
						pPtr->increSize(4);
						pPtr->queue(sPtr, true);																		//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
						requestTime[dest] = now;																		//���[�g���N�G�X�g���M�����̐ݒ�
					}
					else																									//���ꈶ��ōŋߌo�H�T�������Ă���ꍇ
						return;																								//���������������I��
				}
				else{																									//�o�H�L��̏ꍇ
					//if(path[dest].size() == 0)
					//	cout << id << "\t" << dest << endl;
					if(pPtr->path.size() > 0)
						pPtr->path.clear();																				//�p�P�b�g�̌o�H����������
					for(char i = 0; i < (char)path[dest].size(); i++)										//�o�H���̐ݒ�
						pPtr->path.push_back(path[dest][i]);														//�m�[�h�̌o�H�����p�P�b�g�ɃR�s�[
					pPtr->increSize(path[dest].size() * 4);
					bufferPtr->increLength(path[dest].size() * 4);
					pPtr->setTime(now);																				//���M�����̍X�V
				}
			}
			else																									//���g�����M���łȂ��ꍇ
				pPtr->setTime(now);																				//���M�����̍X�V
			break;
		case NODE::AODV:																					//AODV�̏ꍇ
			if(pPtr->getSource() == id){																	//���g�����M���̏ꍇ
				short dest = pPtr->getDest();																	//���M���� 
				if(routing[dest]->getNext() == -1){															//�o�H�����̏ꍇ
					if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//�ΏۂɊւ��郋�[�g���N�G�X�g�̍ŋ߂̑��M�������ꍇ 
						//cout << id << " ���[�g���N�G�X�g���M" << aodvSeq << endl;
						pPtr = new PACKET(sPtr, now, PACKET::RreqAodv, RREQAODV, id, id, dest, -1);	//���[�g���N�G�X�g�p�P�b�g�̍쐬
						pPtr->setAodvSeqS(aodvSeq);																	//AODV�p�V�[�P���X�i���M���j�̐ݒ�
						pPtr->setAodvSeqD(routing[dest]->getSeq());												//AODV�p�V�[�P���X�i���M����j�̐ݒ�
						pPtr->queue(sPtr, true);																		//�p�P�b�g���o�b�t�@�̐擪�֑}�� 
						requestTime[dest] = now;																		//���[�g���N�G�X�g���M�����̐ݒ�
					}
					else																									//���ꈶ��ōŋߌo�H�T�������Ă���ꍇ								
						return;																								//���������������I��
				}
				else																									//�o�H�L��̏ꍇ
					pPtr->setTime(now);																		//���M�����̍X�V
			}
			else																									//���g�����M���łȂ��ꍇ
				pPtr->setTime(now);																		//���M�����̍X�V
			break;
		case NODE::PRO:																					//�v���A�N�e�B�u���[�e�B���O�̏ꍇ
//			cout << "check packet  " << getNid() << endl;
			pPtr->setTime(now);																				//���M�����̍X�V
			break;
		case NODE::GEDIR:
			//cout << "GIDER --- " << sPtr->getNow().dtime() << endl;
			if(pPtr->getSource() == id){																	//���g�����M���̏ꍇ
				short dest = pPtr->getDest();																	//���M����
				if(dest >= (short)sPtr->node.size()){
					pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
					pPtr->setDpos(sPtr->ma[dest - sPtr->node.size()]->getCenter());
					pPtr->setTime(now);
				}
				else{

					if(sPtr->node[id]->nodePos[dest].getX() < 0)
						cout << now.dtime() << "\t" << dest << "\t" << (int)sPtr->node[id]->routing[dest]->getHop() << "\t" << pPtr->getType() << " dest position error" << endl, exit(1);
					else{
						pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
						pPtr->setDpos(sPtr->node[id]->nodePos[dest]);
						pPtr->setTime(now);																				//���M�����̍X�V
					}
				}
			}
			else																									//���g�����M���łȂ��ꍇ
				pPtr->setTime(now);																				//���M�����̍X�V
			break;
		}
	}
	else																									//���̑��̏ꍇ
		pPtr->setTime(now);																				//���M�����̍X�V
	sPtr->list.insert(pPtr);																		//�p�P�b�g�������C�x���g���X�g�֒ǉ�
}

//�p�P�b�g���p����
//�����i���̃p�P�b�g�I�u�W�F�N�g�j
//�߂�l�i���p�p�P�b�g�I�u�W�F�N�g�j
PACKET* NODE::relayPacket(PACKET* pPtr, short increSize){
	//if(pPtr->getType() == PACKET::RerrDsr)
		//cout << getSimu()->getNow().dtime() << " relay at " << getNid() << "\t" << pPtr->getType() 
		//	<< "--" << pPtr->getSeq() << " size " << pPtr->getSize()  << endl; 
//	cout << now.dtime() << " relay inform " << endl;
	bool duplicateFlag = false;
	RECEIVED_PACKET rPacket = RECEIVED_PACKET(pPtr->getType(), pPtr->getSource(), pPtr->getDest(), pPtr->getSeq(), pPtr->getSendStart());
	for(short i = 0; i < (short)receivedPacketList.size(); i++){
		RECEIVED_PACKET packet = receivedPacketList[i];
		if(packet.getType() == rPacket.getType() && packet.getSource() == rPacket.getSource() 
			&& packet.getDest() == rPacket.getDest() && packet.getSeq() == rPacket.getSeq()
			&& packet.getTime().dtime() == rPacket.getTime().dtime()){
			duplicateFlag = true;
			break;
		}
	}
	if(duplicateFlag){
		//static int cnt;
		//cout << "relay�@duplicate receive " << ++cnt << endl;
		return false;
	}
	else{
		receivedPacketList.push_back(rPacket);
		receivedPacketList.erase(receivedPacketList.begin());
	}
	PACKET* newPtr = new PACKET();																//�����p�p�P�b�g
	*newPtr = *pPtr;																					//���̃p�P�b�g���𕡐��p�P�b�g�փR�s�[
	newPtr->setHere(getNid());																		//�p�P�b�g�̌��݈ʒu�͎����ɕύX
	newPtr->increSize(increSize);
	//if(newPtr->getType() == PACKET::DummyBroadcast)
	//	cout << getNid() << " �� " << newPtr->getDest() << " �s���̃p�P�b�g�𒆌p " << endl; 
	if(!newPtr->queue(getSimu(), false)){														//�p�P�b�g���o�b�t�@�֑}��
		delete newPtr;																						//�}���Ɏ��s������폜
		newPtr = NULL;
	}
	return newPtr;
}

//���[�g�G���[�p�P�b�g���M����
//�����i�C�x���g���X�g�C�m�[�h�I�u�W�F�N�g�C�p�P�b�g�I�u�W�F�N�g�j
//�߂�l�F�Ȃ�
void NODE::sendError(PACKET* pPtr){
	//pPtr->showPath();
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	//cout << now.dtime() << "\t" << getNid() << "  ���[�g�G���[���� " << pPtr->getType() << "\t" << pPtr->getSeq() << endl;
	//queueShow();
	//cout << "�G���[�p�P�b�g�͕K�v�H" << endl;
	//mPtr->show(0);
	if(routeId == NODE::PRO){																		//�v���A�N�e�B�u���[�e�B���O�̏ꍇ
		checkPacket();																						//�p�P�b�g�`�F�b�N����
		return;																								//�������Ȃ��ŏI��
	}
	PACKET::ptype type = pPtr->getType();														//�p�P�b�g�^�C�v
	if(type != PACKET::Tcp && type != PACKET::Ack && type != PACKET::Udp){			//�p�P�b�g���f�[�^�p�P�b�g�ȊO�̏ꍇ
		checkPacket();																						//�p�P�b�g�`�F�b�N����
		return;																								//�������Ȃ��ŏI��
	}
	short id = getNid();																				//���g�̃m�[�hID
	short sid = pPtr->getSource();																//���M���m�[�hID
	switch(routeId){																					//���[�e�B���O�^�C�v�ɂ��ꍇ����
	case NODE::DSR:	case NODE::MAMR:																	//DSR�̏ꍇ
		if(id == sid){																						//���g���p�P�b�g�̑��M���̏ꍇ
			path[pPtr->path.back()].clear();																//����܂ł̌o�H�����N���A
			path[pPtr->path.back()].push_back(id);														//�o�H���̏�����
		}
		else{																									//���g���p�P�b�g�̑��M���łȂ��ꍇ
			if(timeCompare(requestTime[sid] + REQUEST_INT, now)){									//�Ώۂփ��[�g�G���[�������̓��N�G�X�g�p�P�b�g�̍ŋ߂̑��M������ꍇ
				checkPacket();																						//�p�P�b�g�`�F�b�N����
				return;																								//�������Ȃ��ŏI��
			}
			requestTime[sid] = now;																			//���[�g�G���[���M�����̐ݒ�
			PACKET* newPtr = new PACKET(sPtr, now, PACKET::RerrDsr, RERRDSR, id, id, sid);//���[�g�G���[�p�P�b�g�̍쐬
			newPtr->setErrDest(pPtr->getDest());														//�G���[�Ώۃm�[�h�̐ݒ�
			path[pPtr->path[0]].clear();
			char cnt;
			if(routeId == NODE::DSR){
				for(cnt = 0; pPtr->path[cnt] != id; cnt++);												//�p�P�b�g�̌o�H��񂩂玩�M��T��
				for(char i = cnt; i >= 0; i--)																//���[�g�G���[�p�P�b�g�֌o�H���̓o�^
					path[pPtr->path[0]].push_back(pPtr->path[i]);	
			}
			else{
				cout << pPtr->getType() << "\tpath" << pPtr->mamrPath.size() << endl; 
				newPtr->mpath_check = pPtr->mpath_check;
				for(cnt = 0; pPtr->mamrPath[cnt] != id; cnt++);												//�p�P�b�g�̌o�H��񂩂玩�M��T��
				for(char i = cnt; i >= 0; i--)																//���[�g�G���[�p�P�b�g�֌o�H���̓o�^
					path[pPtr->mamrPath[0]].push_back(pPtr->mamrPath[i]);	
			}
			//			cout << "�G���[�p�P�b�g���M" << endl;
			if(!newPtr->queue(sPtr, false))																//�o�b�t�@�ւ̃p�P�b�g�}��
				delete newPtr;																						//�}���Ɏ��s���������
		}
		break;
		case NODE::AODV:																				//AODV�̏ꍇ
			if(id == sid){																					//���g���p�P�b�g�̑��M���̏ꍇ
				routing[pPtr->getDest()]->setNext(-1);													//�]����̏�����											
				routing[pPtr->getDest()]->setHop(-1);													//�z�b�v���̏�����
			}
			else{																								//���g�����M���łȂ��ꍇ
				if(timeCompare(requestTime[sid] + REQUEST_INT, now)){								//�Ώۂփ��[�g�G���[�������̓��N�G�X�g�p�P�b�g�̍ŋ߂̑��M������ꍇ
				checkPacket();																						//�p�P�b�g�`�F�b�N����
				return;																								//�������Ȃ��ŏI��
			}
			requestTime[sid] = now;																			//���[�g�G���[���M�����̐ݒ�
			PACKET* newPtr = new PACKET(sPtr, now, PACKET::RerrAodv, RERRAODV, id, id, sid);//���[�g�G���[�p�P�b�g�̍쐬
			newPtr->setErrDest(pPtr->getDest());														//�G���[�Ώۃm�[�h�̐ݒ�
//			cout << "�G���[�p�P�b�g���M" << endl;
			if(!newPtr->queue(sPtr, false))																//�o�b�t�@�ւ̃p�P�b�g�}��
				delete newPtr;																						//�}���Ɏ��s���������
		}
		break;
	}
}

//�o�b�t�@�\��
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::queueShow(void){
	cout << "node" << getNid() << " size= " << bufferPtr->getLength() << " ";
	PACKET* pPtr;
	if(getMAC()->getFrame()){
		pPtr = getMAC()->getFrame()->getPacket();
		cout << "f" << pPtr << "-T" << pPtr->getType() << "-Se" << pPtr->getSeq() << "-So" << pPtr->getSource() << "-D" << pPtr->getDest();
		if(pPtr->getType() == PACKET::Ack)
			cout << "-" << pPtr->getSeg()->getTcp() << " ";
		else
			cout << " ";
		cout << "\t";
	}
	if(bufferPtr->queue.size()){
		if(bufferPtr->getLength() <= 0)
			cout << "queue show error " << bufferPtr->queue.size() << "\t" << bufferPtr->getLength() << endl, exit(1);
		for(short i = 0; i < (short)bufferPtr->queue.size(); i++){	
			pPtr = bufferPtr->queue[i];
			cout << pPtr << "-T" << pPtr->getType() << "-Se" << pPtr->getSeq() << "-So" << pPtr->getSource() << "-D" << pPtr->getDest()
				<< "-Si" << pPtr->getSize();
			if(pPtr->getType() == PACKET::Ack)
				cout << "-" << pPtr->getSeg()->getTcp() << " ";
			else
				cout << " ";
			cout << "\t";
		}
	}
	cout << endl;
}

//LAB���̑��M
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::sendLab(short staId, short mapId){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	short id = getNid();																				//�m�[�hID
	LAB lab = LAB(id, now);																			//LAB���
	gab[staId - sPtr->getMAP()] = lab;															//���M��GAB���ɊY��LAB����o�^
	PACKET* pPtr;																						//LAB�p�P�b�g�p�I�u�W�F�N�g
	switch(sPtr->getMesh()){																		//�Ǘ������ɂ��ꍇ����
	case SIMU::CENTRAL:																				//�W���Ǘ������̏ꍇ
		pPtr = new PACKET(sPtr, now, PACKET::LabCenter, LABPACKET, id, id, sPtr->getCenter(), -1);//LAB�p�P�b�g�̍쐬
		pPtr->setSTA(staId);																				//STAID�̓o�^
		pPtr->setLAB(lab);																				//LAB���̓o�^
		pPtr->setSeq(seq++);																				//�V�[�P���X�ԍ��̐ݒ�
		if(!pPtr->queue(sPtr, false))																	//�o�b�t�@�ւ̃p�P�b�g�}��
			delete pPtr;																						//�}���Ɏ��s���������
		break;
	case SIMU::RAOLSR:																				//RA-OLSR�̏ꍇ
		pPtr = new PACKET(sPtr, now, PACKET::LabRa, LABPACKET, id, id, -1, -1);			//LAB�p�P�b�g�̍쐬
		pPtr->setSTA(staId);																				//STAID�̓o�^
		pPtr->setLAB(lab);																				//LAB���̓o�^
		pPtr->setSeq(seq++);																				//�V�[�P���X�ԍ��̐ݒ�
		if(!pPtr->queue(sPtr, false))																	//�o�b�t�@�ւ̃p�P�b�g�}��
			delete pPtr;																						//�}���Ɏ��s���������
		break;
	case SIMU::NEIGHBOR:	case SIMU::AGENT:														//�אڒʒB������G�[�W�F���g�̏ꍇ
		if(mapId == -1)
			break;
		pPtr = new PACKET(sPtr, now, PACKET::LabNeighbor, LABPACKET, id, id, mapId, -1);	//LAB�p�P�b�g�̍쐬
		pPtr->setSTA(staId);																				//STAID�̓o�^
		pPtr->setLAB(lab);																				//LAB���̓o�^
		pPtr->setSeq(seq++);																				//�V�[�P���X�ԍ��̐ݒ�
		if(!pPtr->queue(sPtr, false))																	//�o�b�t�@�ւ̃p�P�b�g�}��
			delete pPtr;																						//�}���Ɏ��s���������
		break;
	}
}

//���[�e�B���O�e�[�u���X�V����
//�����i�Ώۃm�[�hID�j
//�߂�l�F�i�Ȃ��j
void NODE::makeRoutingTable(short you){
	SIMU* sPtr = getSimu();
	for(int i = 0; i < (int)sPtr->node.size(); i++){										//�S�Ẵm�[�h�ɑ΂��čX�V������`�F�b�N
		ROUTING_DATA* routeMe = routing[i];															//���M�����Ώۂ̃��[�e�B���O���
		ROUTING_DATA* routeYou = sPtr->node[you]->routing[i];									//���肪���Ώۂ̃��[�e�B���O���
		if(routeYou->getNext() == -1){																//���肪���������ĂȂ����
			if(routeMe->getNext() == you){															//���肪�]����ɐݒ肳��Ă�����
				routeMe->setNext(-1);																		//�]����������
				routeMe->setHop(-1);
			}
			continue;																						//���肪�ݒ肳��Ă��Ȃ���Ζ���
		}
		if(routeYou->getNext() == getNid()){														//����̓]���悪�����̏ꍇ
			if(routeMe->getNext() == you){															//�����̓]���悪����Ȃ�
				routeMe->setNext(-1);																		//�]����������
				routeMe->setHop(-1);
			}
			continue;																						//�����̓]���悪����łȂ���Ζ���
		}
		//����������̓]���悪�����łȂ��ꍇ������
		if(routeMe->getNext() == -1){																//�������]�����������ĂȂ����
			routeMe->setNext(you);																		//�]�������X�V
			routeMe->setHop(routeYou->getHop() + 1);
			continue;
		}
		if(routeYou->getHop() + 1 < routeMe->getHop()){										//������o�R�����ق����ŒZ�̏ꍇ
			routeMe->setNext(you);																		//�]�������X�V
			routeMe->setHop(routeYou->getHop() + 1);
			continue;
		}
		if(routeMe->getNext() == you)																//���肪�]����̏ꍇ
			routeMe->setHop(routeYou->getHop() + 1);												//�]�����̃z�b�v�����X�V		
	}
}


//����d�͂̌v�Z
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::calcPower(void){
	SIMUTIME now = getSimu()->getNow();															//���ݎ���
	if(timeCompare(calcPowerTime, now))
		return;
	double consume;																				//�P�ʎ��ԓ�����̏���d��
	switch(pmode){																					//�m�[�h�̏�Ԃɂ�����d�͂�����
	case SLEEP:
		consume = SLEEP_POWER;
		break;
	case WAIT:
		consume = WAIT_POWER;
		break;
	case SEND:
		consume = SEND_POWER;
		break;
	case RECEIVE:
		consume = RECEIVE_POWER;
		break;
	}
	usedPower += consume * (now - calcPowerTime).dtime();								//���ݎ����ƌv�Z�J�n�������c���d�͂��X�V
	calcPowerTime = now;
}


//�ʒu���p�P�b�g�̑��M
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void NODE::sendInfoemLoc(void){
	SIMU* sPtr = getSimu();
//	if(dist(derivePos[0] , lastInformPos) > sPtr->getRate()){
	if(dist(derivePos[0] , lastInformPos) >= 2000){							//�ʒu���X�V�̂������l�icm)
		sPtr->counter1++;
//		cout << now.dtime() << "\t" << "send inform " << cntP << "\t" << (double)sPtr->counter2 / sPtr->counter1 << endl;
		lastInformPos = derivePos[0];
		PACKET* pPtr = new PACKET(sPtr, now, PACKET::InformLoc, INFORM, getNid(), getNid(), sPtr->node.size() + 0, -1);//InformLoc�p�P�b�g�I�u�W�F�N�g�̍쐬
//		pPtr->setSeq(seq++);
		if(!pPtr->queue(sPtr, false))														//�o�b�t�@�ւ̃p�P�b�g�}��
			delete pPtr;																				//�}���Ɏ��s���������
	}
}

////AODVRREQ��M���̌o�H���`�F�b�N
////�����i�p�P�b�g�I�u�W�F�N�g�C�]����j
////�߂�l�F�i�Ȃ��j
//void NODE::aodvRreqCheck(PACKET* pPtr, int next){
//	bool flag = false;																				//�o�H���X�V�t���O�i�����l��X�V�j
//	AODV_RREQ* rPtr = pPtr->getAodvRreq();														//�p�P�b�g������RREQ�I�u�W�F�N�g
//	int sid = pPtr->getSource();																	//���M���m�[�h
//	if(sid == id)																						//���M���������Ȃ�Ή������Ȃ�
//		return;
//	if(routing[sid]->getSeq() < rPtr->getSourceSeq())										//����V�[�P���X���Â��ꍇ
//		flag = true;																						//�X�V�t���O�𗧂Ă�
//	else if(routing[sid]->getSeq() == rPtr->getSourceSeq())								//����V�[�P���X�������ꍇ
//		if(routing[sid]->getHop() > pPtr->getHop())												//�p�P�b�g�̎��z�b�v���ȏ�Ȃ��
//			flag = true;																						//�X�V�t���O�𗧂Ă�
//	if(flag == true){																					//�X�V�t���O�������Ă���ꍇ
//		routing[sid]->setNext(next);																	//�l�N�X�g�z�b�v�̍X�V
//		routing[sid]->setHop(pPtr->getHop());														//�z�b�v���̍X�V
//		routing[sid]->setSeq(rPtr->getSourceSeq());												//�V�[�P���X�̍X�V
//	}
//}
//
////AODVRREP��M���̌o�H���`�F�b�N
////�����i�p�P�b�g�I�u�W�F�N�g�C�]����j
////�߂�l�F�i�Ȃ��j
//void NODE::aodvRrepCheck(PACKET* pPtr, int next){
//	bool flag = false;																				//�o�H���X�V�t���O�i�����l��X�V�j
//	AODV_RREP* rPtr = pPtr->getAodvRrep();														//�p�P�b�g������RREP�I�u�W�F�N�g
//	int sid = rPtr->getDest();																		//���v���C���w�肷�鑗�M���m�[�h
//	if(sid == id)																						//���M���������Ȃ�Ή������Ȃ�
//		return;
//	if(routing[sid]->getSeq() < rPtr->getDestSeq())											//����V�[�P���X���Â��ꍇ
//		flag = true;																						//�X�V�t���O�𗧂Ă�
//	else if(routing[sid]->getSeq() == rPtr->getDestSeq())									//����V�[�P���X�������ꍇ
//		if(routing[sid]->getHop() > pPtr->getHop())												//�p�P�b�g�̎��z�b�v���ȏ�Ȃ��
//			flag = true;																						//�X�V�t���O�𗧂Ă�
//	if(flag == true){																					//�X�V�t���O�������Ă���ꍇ
//		routing[sid]->setNext(next);																	//�l�N�X�g�z�b�v�̍X�V
//		routing[sid]->setHop(pPtr->getHop());														//�z�b�v���̍X�V
//		routing[sid]->setSeq(rPtr->getDestSeq());													//�V�[�P���X�̍X�V
//	}
//	requestTime[sid] = -2 * REQUEST_INT;														//���[�g���N�G�X�g�^�C���������̏�����
//}
//
////�m�[�h�ԋ����`�F�b�N
////�����i�m�[�h�I�u�W�F�N�g�C�`�F�b�N�����j
////�߂�l�i�`�F�b�N�����ȓ��Ȃ�^�C�ȏ�Ȃ�U�j
//bool NODE::distCheck(NODE* nPtr, double range){
//	if(dist(position, nPtr->getPos()) <= range)
//		return true;
//	return false;
//}
//

////�o�H�\��
////�����i����m�[�h�j
////�߂�l�i�Ȃ��j
//void NODE::pathShow(_NODE node, int dest){
//	int cnt = 1;
//	int next = routing[dest]->getNext();
//	cout << id << "-";
//	while(next != -1 && next != dest && cnt < 20){
//		cout << next << "-";
//		next = node[next]->routing[dest]->getNext();
//		cnt++;
//	}
//	cout << next << endl;
//}
//
