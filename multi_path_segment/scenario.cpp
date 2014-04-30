#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																		//�����Z���k�c�C�X�^���������i�m�[�h�ړ��ȊO�p�j
unsigned long genrand2();																		//�����Z���k�c�C�X�^���������i�m�[�h�ړ��p�j
double rand_d();																					//�����Z���k�c�C�X�^��������
double rand2_d();																					//�����Z���k�c�C�X�^��������
void sgenrand(unsigned long);																	//�����Z���k�c�C�X�^�����������֐��̊O���錾
void sgenrand2(unsigned long);																//�����Z���k�c�C�X�^�����������֐��̊O���錾
void scenario(SIMU*);																			//�V�i���I�쐬�֐��̊O���錾
void result(SIMU*);																				//���ʕ\���֐��̊O���錾
void result(const char*, SIMU*);																//���ʕ\���֐��̊O���錾

int poison(double rate);

void main(int argc, char *argv[]){
	//sgenrand((unsigned long)time(NULL));													//�m�[�h�ړ��ȊO�p�����������i���ԕϓ����������ꍇ�j
	//sgenrand2((unsigned long)time(NULL));												//�m�[�h�ړ��p�����������i���ԕϓ����������ꍇ�j
	sgenrand(1);																					//�m�[�h�ړ��ȊO�p�����������i���ԕϓ����������Ȃ��ꍇ�j
	sgenrand2(1);																					//�m�[�h�ړ��p�����������i���ԕϓ����������Ȃ��ꍇ�j
	//if(argc == 2)
	//	sgenrand2((atoi(argv[1]) - 1) % 20 + 1);																					//�m�[�h�ړ��p�����������i���ԕϓ����������Ȃ��ꍇ�j
	//cout << atoi(argv[1]) << endl;
	cout.setf(ios::fixed);																		//�\���p�v���p�e�B
	cout << setprecision(6);																	//���ݎ���

	SIMU simu(SIMU::SQUARE, SIMU::NORMAL);													//�V�~�����[�^�I�u�W�F�N�g�̍쐬�i�����`�G���A�j
	//if(argc == 2)
	//	simu.setRate(atoi(argv[1]) / 40.0);
	//else
	//	simu.setRate(1.0);


	simu.setRate(2.5);	


//	SIMU simu(SIMU::CIRCLE, SIMU::NORMAL);													//�V�~�����[�^�I�u�W�F�N�g�̍쐬�i�~�`�G���A�j
//	SIMU simu(SIMU::MESH, SIMU::NORMAL);													//�V�~�����[�^�I�u�W�F�N�g�̍쐬�i���b�V���l�b�g���[�N�j

	simu.setMAloc();																				//MA���ʒu���Ǘ����s���ꍇ�ɕK�v�i�g��Ȃ��ꍇ�ɂ̓R�����g�A�E�g�j
	
	//���b�V���l�b�g���[�N���쐬�i�����CMAP���CSTA���CUDP���ׁi�b������̃t���[���j�C�؍ݎ��ԁi�ʕb�j�j
	//simu.makeMesh(SIMU::NEIGHBOR, 1000, 300, 1.0, 100000);

	//�m�[�h�̍쐬 simu.newNode�i�����J�n�����C�����I�������C�ʒu���, �ړ����f���C���[�e�B���O�^�C�v�j
	//LOCATION pos(AREA / 2, AREA / 2, 0);														//�����l���w�肷��ꍇ�̈ʒu���錾
	//SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);									//�����J�n�����C�����I�������̏����l�ݒ�
	//simu.newNode(activeTime, offTime, pos, NODE::NO, NODE::AODV);				//�m�[�h�I�u�W�F�N�g�̍쐬
	
	//for(short i = 0; i < NODENUM; i++){
	//	LOCATION pos;																				//�����l�Ȃ��̈ʒu���錾�͒l�������_���ɂȂ�
	////	//LOCATION pos(i * 6000, 0, 0);														//�����l���w�肷��ꍇ�̈ʒu���錾
	//	SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);									//�����J�n�����C�����I�������̏����l�ݒ�
	//	simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::GEDIR);				//�m�[�h�I�u�W�F�N�g�̍쐬
	//}


	//�m�[�h���쐬
	//�R�}���h���C��������͂���ꍇ
	//for(short i = 0; i < atoi(argv[1]) * 100 + 300; i++){
	//	LOCATION pos;																//�m�[�h�̏����l�̓����_���Ƃ���
	//	SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);								//�����J�n�����C�����I�������̏����l�ݒ�
	//	simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::MAMR);				//�m�[�h�I�u�W�F�N�g���쐬�@�����_���Ɉړ����C���[�e�B���O��MAMR�Ƃ���
	//}

	for(short i = 0; i < NODENUM; i++){
		LOCATION pos;
		SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);
		simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::MAMR);
	}
	for(int i = 0; i < simu.node.size(); i++){
		for(int j = 0; j < simu.node.size(); j++){
			simu.node[i]->pathNum.push_back(1);
			simu.node[i]->path1SegSize.push_back(TCPDEFAULTSIZE);
			simu.node[i]->path2SegSize.push_back(TCPDEFAULTSIZE);
			simu.node[i]->path1[j].push_back(i);
			simu.node[i]->path2[j].push_back(i);
		}
	}

	//for(short i = 1; i < NODENUM; i++)
	//	simu.node[i]->nodePos[0] = simu.node[0]->getPos();
	scenario(&simu);																				//�ʐM�V�i���I�̍쐬(�R�}���h���C�������s�g�p���j
	while(timeCompare(SIMUTIME(TIMELIMIT, 0), simu.getNow()))						//�ݒ莞���ɒB����܂ł̏������ȉ��ɋL�q
		simu.processEvent();																			//�C�x���g����
	result(&simu);																					//���ʕ\��
}

//�ʐM�V�i���I�ݒ�֐�
void scenario(SIMU* sPtr){
	//for(int i = 0; i < 10; i++){																//UDP�t���[�쐬�T���v��	
	//	UDP* udp = new UDP(sPtr, i, SIMUTIME(0,0), 1 * 100, 10 * 1000 * 1000);	//UDP�G�[�W�F���g�i�����FsPtr, ID�C�J�n�����Ckbps�C���M�o�C�g���j
	//	UDPSINK* udpsink = new UDPSINK(sPtr, i + 15);									//UDP�V���N�i�����FsPtr, ID�j
	//	udp->connectSink(udpsink);																//�G�[�W�F���g�ƃV���N�̌���
	//	sPtr->list.insert(udp);																	//UDP�G�[�W�F���g���C�x���g���X�g�֒ǉ�
	//}
	//short source = genrand2() % (NODENUM - 1) + 1;
	//double rate = sPtr->getRate();
	//if(sPtr->getRate() > 0){
	//	TIMEOUT* toPtr = new TIMEOUT(sPtr, SIMUTIME(0, 0), TIMEOUT::MakeUdp, source);
	//	toPtr->setRate(rate);
	//	sPtr->list.insert(toPtr);
	//}

	//for(int i = 0; i < 1; i++){																//TCP�t���[�쐬�T���v��
	//	TCP* tcp = new TCP(sPtr, 1, SIMUTIME(1,0), 500 * 1000 * 1000);				//TCP�G�[�W�F���g�i�����FID�C�J�n�����C���M�o�C�g���j
	//	TCPSINK* tcpsink = new TCPSINK(sPtr, 0);											//TCP�V���N�i�����FID�j
	//	tcp->connectSink(tcpsink);																//�G�[�W�F���g�ƃV���N�̌���
	//	sPtr->list.insert(tcp);																	//TCP�G�[�W�F���g���C�x���g���X�g�֒ǉ�
	//}

	TIMEOUT* toPtr = new TIMEOUT(sPtr, SIMUTIME(30, 0), TIMEOUT::MakeTcp, genrand2() % (sPtr->node.size() - 1));
	toPtr->setRate(sPtr->getRate()); 
	sPtr->list.insert(toPtr);

	////�R�O�b��Ɉ�x����TCP�ʐM�𔭐�������D
	//for(int i = 0; i < 1; i++){
	//	TCP* tcp = new TCP(sPtr, 1, SIMUTIME(30,0), 1000 * 1000);						//TCP�G�[�W�F���g�i�����FID�C�J�n�����C���M�o�C�g���j
	//	TCPSINK* tcpsink = new TCPSINK(sPtr, 0);											//TCP�V���N�i�����FID�j
	//	tcp->connectSink(tcpsink);															//�G�[�W�F���g�ƃV���N�̌���
	//	sPtr->list.insert(tcp);																//TCP�G�[�W�F���g���C�x���g���X�g�֒ǉ�
	//}

	//
	//���b�V���l�b�g���[�N��STA�v���쐬�i���ԁC1�b������̗v���񐔁j
	//sPtr->makeStaReq(100, 10.0);
	//TIMEOUT* toPtr = new TIMEOUT(sPtr, poison(sPtr->getRate()), TIMEOUT::BackGround, genrand2() % NODENUM);
	//toPtr->setRate(sPtr->getRate());
	//sPtr->list.insert(toPtr);

	if(sPtr->getMAloc())
		MA* maPtr = new MA(sPtr, SIMUTIME(0,0), LOCATION(AREA / 2, AREA / 2), 50 * 100, MA::Distance, 0, -1);
//	sPtr->list.orderShow();
}

//���ʕ\���֐�
void result(SIMU* sPtr){
	//�S�Ă̑��M�p�P�b�g�T�C�Y��\���������ꍇ�ɂ͉��̃R�����g�A�E�g�������ii�̏���l��class.h�Őݒ肵���p�P�b�g�̎�ސ��j
	cout << "rate=" << sPtr->getRate() << "�̎��̌���" << endl;

	for(char i = 0; i < 24; i++)
		cout << sPtr->getPacket(i) << ",";

	//����̑��M�p�P�b�g�T�C�Y��\���������ꍇ�͈ȉ��̂悤�ɂ���
	//cout << sPtr->getPacket(PACKET::RreqDsr) << ",";
	//cout << sPtr->getPacket(PACKET::RrepDsr) << ",";
	//cout << sPtr->getPacket(PACKET::RerrDsr) << ",";
	//cout << sPtr->getPacket(PACKET::LabCenter) << ",";
	//cout << sPtr->getPacket(PACKET::LabRa) << ",";
	//cout << sPtr->getPacket(PACKET::LabNeighbor) << ",";
	//cout << sPtr->getPacket(PACKET::MapReq) << ",";
	//cout << sPtr->getPacket(PACKET::MapReqB) << ",";
	//cout << sPtr->getPacket(PACKET::MapRep) << ",";
	//cout << sPtr->getPacket(PACKET::Udp) << ",";
	//cout << sPtr->getPacket(PACKET::Tcp) << ",";
	//cout << sPtr->getPacket(PACKET::Ack) << ",";

	//�o�H�\�z���s���Ɖ�
	cout << (double)sPtr->getReqFail() / sPtr->getReqNum() << ",";
	cout << sPtr->getReqFail() << ",";
	cout << sPtr->getReqNum() << ",";

	//���ϒx���i���v�x�����o�H�\�z�����񐔂Ŋ���Z�j
	cout << sPtr->getDlay() / (sPtr->getReqNum() - sPtr->getReqFail()) << ",";

	//UDP�f�[�^�̓��B��
	cout << (double)sPtr->getReceiveUdp() / sPtr->getSendUdp() << ",";
	cout << sPtr->getReceiveUdp() << ",";
	cout << sPtr->getSendUdp() << ",";

	//TCP�f�[�^�̃X���[�v�b�g
	cout << sPtr->getTcpData() * 8 / sPtr->getTcpTime().dtime() / 1000 /1000  << ",";

	//�p�P�b�g�̑��M�x��
	cout << sPtr->getTransDelay() / sPtr->getTransDelayCnt() << ",";


	//����d�͂̌v�Z
	double totalUsedPower = 0;
	for(short i = 0; i < (short)sPtr->node.size(); i++)
		totalUsedPower += sPtr->node[i]->getUsedPower();
	cout << totalUsedPower / sPtr->node.size() << ",";
	if(sPtr->getMAloc())
		cout << sPtr->ma[0]->getMigNum() << ",";
	cout << endl;
}