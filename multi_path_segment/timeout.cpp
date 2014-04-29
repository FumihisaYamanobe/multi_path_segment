#include "class.h"
#include "mobileAgent.h"

unsigned long genrand2();																			//�����Z���k�c�C�X�^�����O���錾�i�m�[�h�ړ��p�j
int poison(double);

TIMEOUT::TIMEOUT(SIMU* ptr, SIMUTIME tval, type tid, short idval):EVENT(ptr, tval, EVENT::Timeout, idval){ 
	typeId = tid;
}
//
//TIMEOUT::~TIMEOUT(){ 
//}

//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�F�i�^�U�l�i�_�~�[�j�j
bool TIMEOUT::process(void){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	PACKET* pPtr;																						//�p�P�b�g�I�u�W�F�N�g
	short id = getNid();																				//�m�[�hID
	NODE* nPtr = sPtr->node[id];																	//�m�[�h�I�u�W�F�N�g
	switch(typeId){																					//�^�C���A�E�g�̎�ނɂ��ꍇ����
		case Tcp:																						//TCP�^�C���A�E�g�̏ꍇ
			delete typePtr.tPtr->getObject();														//TCP�V���N������
			delete typePtr.tPtr;																			//TCP�I�u�W�F�N�g������
			return false;																					//�^�C���A�E�g�I�u�W�F�N�g���g������
		case Udp:																						//UDP�^�C���A�E�g�̏ꍇ
			//cout << typePtr.uPtr->getSize() << "," << typePtr.uPtr->getObject()->getByte() << ","
			//	<< typePtr.uPtr->getObject()->getByte() / (double)typePtr.uPtr->getSize() << endl;
			sPtr->increSendUdp(typePtr.uPtr->getSize());
			sPtr->increReceiveUdp(typePtr.uPtr->getObject()->getByte());
			delete typePtr.uPtr->getObject();														//UDP�V���N������
			delete typePtr.uPtr;																			//UDP�I�u�W�F�N�g������
			return false;																					//�^�C���A�E�g�I�u�W�F�N�g���g������
		case Ma:																							//MA�^�C���A�E�g�̏ꍇ
			typePtr.maPtr->getTCP()->setAbort();													//�ړ��pTCP�̒��f�t���O�𗧂Ă�
			typePtr.maPtr->resetMigration();															//�ړ��t���O�����Z�b�g	
			//cout << now.dtime() << " migrate abortion " << typePtr.maPtr << "\t" << typePtr.maPtr->getTCP() << endl;
			return false;																					//�^�C���A�E�g�I�u�W�F�N�g���g������
		case Segment:{																					//TCP�Z�O�����g�^�C���A�E�g�̏ꍇ
			TCP* tcpPtr = typePtr.sPtr->getTcp();													//�^�C���A�E�g�̑Ώ�TCP�I�u�W�F�N�g
////�������������������O�\������������������
////�^�C���A�E�g�đ��̑��M���O���s�K�v�Ȃ�R�����g�A�E�g����	
//			cout << now.dtime() << "\tTO\t" << getNid() << "\tTCP\t" << tcpPtr << "\t" << typePtr.sPtr->getSeq()
//				<< "\t--------\t--------" << endl;
////����������������������������������������
			typePtr.sPtr->setNAretrans(false);
			typePtr.sPtr->setTimeout(NULL);
			tcpPtr->retransmission();
			return false;																					//�I�u�W�F�N�g������
		}
		case Mareq:																						//MA�ړ��v���̏ꍇ
			if(typePtr.maPtr->getCandidate() != -1)												//�ړ���₪���݂���Ȃ�
				typePtr.maPtr->migration();																//�ړ�����
			return true;																					//�֐��𔲂��Ă��I�u�W�F�N�g�͏�������Ȃ�
		case StaReq:{																					//STA�v���̏ꍇ
			if(flag){
				cout << getNid() << " �o�H�\�z���s" << endl;
				sPtr->increReqFail();
				return false;
			}
			cout << now.dtime() << " sta�v���@" << id << "\t" << dest << "\t" << sPtr->gab[dest].getMap() 
				<< "\t" << sPtr->node[dest]->getMAP() << "\t" << nPtr->getMAP() << endl;
			if(nPtr->getMAP() == -1 || sPtr->node[dest]->getMAP() == -1){					//���g������̐ڑ�MAP�����݂��Ȃ��Ȃ�
				cout << "MAP�ɔ�ڑ�" << endl;
				return false;																					//���̂܂܏I�����I�u�W�F�N�g���폜
			}
			sPtr->increReqNum();
			pPtr = new PACKET(sPtr, now, PACKET::StaReq, 
																REQ, id, id, nPtr->getMAP(), 1);		//�p�P�b�g�I�u�W�F�N�g�̍쐬
			pPtr->setTimeout(this);
			pPtr->setSTA(id);																				//�v��STA�̐ݒ�
			pPtr->setReqDest(dest);																		//����STA�̐ݒ�
			pPtr->setSeq(nPtr->getSeq());																//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			nPtr->increSeq();																				//�m�[�h�V�[�P���X�̃C���N�������g
			if(!pPtr->queue(sPtr, false))																//�o�b�t�@�ւ̃p�P�b�g�}��
				delete pPtr;																					//�}���Ɏ��s���������
			timeVal = now;																					//���ݎ������������Ƃ��ċL��
			addTime(SIMUTIME(5, 0));																	//�v�����s���莞���̐ݒ�
			flag = true;																					//���s����p�Ƀt���O��������
			sPtr->list.insert(this);																	//�C�x���g���X�g�ɓo�^
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(poison(sPtr->getReqRate())), 
				TIMEOUT::StaReq, genrand2() % sPtr->getSTA() + sPtr->getMAP());			//����STA�v���̍쐬
			toPtr->setFlag(false);																		//�t���O�͂��낵�Ă���
			short dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();							//STA�v���̈���
			while(dest == toPtr->getNid())																//���悪���g�Ɠ����ł������
				dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();									//������Đݒ�
			toPtr->setDest(dest);																			//����STA�̓o�^
			sPtr->list.insert(toPtr);																		//�C�x���g���X�g�֓o�^
			return true;																					//�^�C���A�E�g�I�u�W�F�N�g�͏������Ȃ�
						}
		//case MakeUdp:{																					//UDP�t���[�쐬�̏ꍇ
		//	cout << now.dtime() << "\t UDP�t���[����" << endl;
		//	UDP* udp = new UDP(sPtr, id, now, 200, 500 * 1000);								//UDP�I�u�W�F�N�g
		//	UDPSINK* udpsink = new UDPSINK(sPtr, 0);												//UDP�V���N
		//	udp->connectSink(udpsink);																	//�G�[�W�F���g�ƃV���N�̌���
		//	sPtr->list.insert(udp);																		//UDP�G�[�W�F���g���C�x���g���X�g�֒ǉ�

		//	//short source = id;
		//	//cout << source << "->";
		//	//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
		//	//	source = sPtr->node[source]->routing[dest]->getNext();
		//	//	cout << source << "->";
		//	//}
		//	//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
		//	
		//	TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeUdp, genrand2() % (NODENUM - 1) + 1);
		//	toPtr->setRate(rate);
		//	sPtr->list.insert(toPtr);
		//	return false;
		//	//setNid(genrand2() % sPtr->getMAP());
		//	//addTime(poison(rate));																		//���̃^�C���A�E�g���������̐ݒ�
		//	//sPtr->list.insert(this);																	//�^�C���A�E�g���C�x���g���X�g�֓o�^
		//	//return true;
		//				 }
		case MakeUdp:{																					//UDP�t���[�쐬�̏ꍇ
			//short dest = genrand2() % NODENUM;
			//while(dest == id)
			//	dest = genrand2() % NODENUM;
			dest = 0;
			//cout << now.dtime() << "\t UDP�t���[���� " << id << "\t" << dest << endl;
			UDP* udp = new UDP(sPtr, id, now, 200, 500 * 1000);								//200kbps, 500KB�̕���
			UDPSINK* udpsink = new UDPSINK(sPtr, dest);											//UDP�V���N
			udp->connectSink(udpsink);																	//�G�[�W�F���g�ƃV���N�̌���
			sPtr->list.insert(udp);																		//UDP�G�[�W�F���g���C�x���g���X�g�֒ǉ�

			//short source = id;
			//cout << source << "->";
			//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
			//	source = sPtr->node[source]->routing[dest]->getNext();
			//	cout << source << "->";
			//}
			//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
			
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeUdp, genrand2() % (NODENUM - 1) + 1);
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
			return false;
						 }
		case MakeTcp:{																					//UDP�t���[�쐬�̏ꍇ
			do{
				dest = genrand2() % (sPtr->node.size() - 1); //dest�̓����_��
			}while(dest == id);
//			cout << id << " TCP start " << now.dtime() << "\tdest " << dest << endl;
			TCP* tcp = new TCP(sPtr, id, now , 1000 * 1000);						//TCP�G�[�W�F���g�i�����FID�C�J�n�����C���M�o�C�g���j
			TCPSINK* tcpsink = new TCPSINK(sPtr, dest);											//TCP�V���N�i�����FID�j
			tcp->connectSink(tcpsink);																	//�G�[�W�F���g�ƃV���N�̌���
			sPtr->list.insert(tcp);																		//UDP�G�[�W�F���g���C�x���g���X�g�֒ǉ�

			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeTcp, genrand2() % (sPtr->node.size() - 1));
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
//			cout << "next TCP " << toPtr->getEventTime().dtime() << endl;
			return false;

			//short source = id;
			//cout << source << "->";
			//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
			//	source = sPtr->node[source]->routing[dest]->getNext();
			//	cout << source << "->";
			//}
			//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
			
						 }
		case MeshMa:{																					//���b�V���l�b�g���[�N�̃G�[�W�F���g����̏ꍇ
			cout << "�G�[�W�F���g����J�n " << getNid() << endl;
			MA* maPtr = typePtr.maPtr;																	//�Ή��G�[�W�F���g												
			short source = maPtr->meshRoute[maPtr->getRouteOrder()];							//�G�[�W�F���g�̑؍݃m�[�h
			if(source != getNid())
				cout << "mesh agent error " << endl, exit(1);
			short dest = maPtr->meshRoute[maPtr->increRouteOrder()];							//�G�[�W�F���g�̈ړ���
			if(maPtr->getRouteOrder() == maPtr->meshRoute.size() - 1)						//�G�[�W�F���g�o�H�\�̍Ō�܂ł�����
				maPtr->resetRouteOrder();																	//�o�H�\�̍ŏ��ɖ߂�
			TCP* tcp = new TCP(sPtr, source, now, maPtr->getSize());							//TCP�t���[�̍쐬
			TCPSINK* tcpsink = new TCPSINK(sPtr, dest);											//TCPSINK�̍쐬
			tcp->connectSink(tcpsink);																	//�R�l�N�V�����̍쐬
			tcp->setMA(maPtr);																			//�Ή�MA��TCP�ɓo�^
			sPtr->list.insert(tcp);																		//�C�x���g���X�g�֓o�^
			return false;																					//�I�u�W�F�N�g�͏���
						}
		case BackGround:{																					//UDP�t���[�쐬�̏ꍇ
			//cout << now.dtime() << "\t UDP�t���[���� " << id << "\t" << dest << endl;
			PACKET* pPtr = new PACKET(sPtr, now, PACKET::DummyBroadcast, 1024, id, id, -1, -1); 
			pPtr->setSeq(nPtr->getSeq());																//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			nPtr->increSeq();																				//�m�[�h�V�[�P���X�̃C���N�������g			
			if(!pPtr->queue(sPtr, false))																//�o�b�t�@�ւ̃p�P�b�g�}��
				delete pPtr;																					//�}���Ɏ��s���������			
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), BackGround, genrand2() % NODENUM);
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
			return false;
						 }
		default:
			return true;
	}
}
