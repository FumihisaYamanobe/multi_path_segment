#include "class.h"
#include "mobileAgent.h"

//�s�b�o�I�u�W�F�N�g�̃R���X�g���N�^
TCP::TCP(SIMU* ptr, short nid, SIMUTIME tval, int sval):AGENT(ptr, nid, tval, EVENT::Tcp){
	size = sval;																						//�s�b�o�t���[�̃T�C�Y
	byte = 0;																							//���M�����o�C�g���i�����l0�j
	startTime = tval;																					//���M�J�n����
	rtt = SIMUTIME(0, 100000);																		//���E���h�g���b�v�^�C��
	D = 0;																								//�^�C���A�E�g�ݒ�p�΍�
	windowSize = 1;																					//�E�B���h�E�T�C�Y
	windowThreshold = 10000.0;																		//�E�B���h�E����p臒l
	makeSegNum = 0;																					//���ɍ쐬����Z�O�����g�ԍ��̏������i�l��0�j
	lastSendSeq = -1;																					//���O�ɑ��M�����Z�O�����g�̃V�[�P���X�ԍ��i�����M�Ȃ̂Œl��-1�j
	nextSendSeq = 0;																					//���̑��M�Z�O�����g�̃V�[�P���X�ԍ��̏������i�l��0�j	
	lastReqSeq = 0;																					//���O��MACK�̗v���V�[�P���X�ԍ��i�l��0�j
	buffering = false;																				//�o�b�t�@�����O��ԃt���O�i�����l�͔�o�b�t�@�����O�j
	maPtr = NULL;																						//�Ή����o�C���G�[�W�F���g�I�u�W�F�N�g�i�����l�Ȃ��j
	abort = false;																						//���f�t���O
	finish = false;
//	init = true;
//	finishTime = -1;																					//�I�������i�����l-1�j
//	finishNum = 0;																						//�E�B���h�E�x�N�^�ɑ��݂��鑗�M�I���Z�O�����g��
//	windowNum = 0;																						//�E�B���h�E�ɑ��݂���Z�O�����g��
//	segPacketNum = 0;																					//�Z�O�����g�p�P�b�g��
//	sendSeq = 0;																						//���ɑ���ׂ��Z�O�����g�p�P�b�g�̃V�[�P���X
//	seq = 0;																								//�V�[�P���X�ԍ�
//	ack = 0;																								//ACK�ɂ�鑗�M�v���V�[�P���X
//	lastack = -1;																						//���O�̑��M�v���V�[�P���X
//	ackRepeatNum = 1;																					//����ACK�̘A����M��
//	finish = false;																					//�I���t���O
//	timeOut = false;																					//�^�C���A�E�g�t���O
//	measurement = false;	
}

TCP::~TCP(void){
	while(segCash.size() > 0){																		//�Z�O�����g�L���b�V�������݂������
		delete segCash[0];																				//�L���b�V���擪�̃Z�O�����g������
		segCash.erase(segCash.begin());																//�L���b�V���̐擪�v�f���폜
	}
	if(maPtr)																							//���o�C���G�[�W�F���g�Ή�TCP�Ȃ�
		delete maPtr;																						//���o�C���G�[�W�F���g���폜
}

//�s�b�o�G�[�W�F���g�̏���
//�����i�Ȃ��j
//�߂�l�F�i�^�U�l�i�_�~�[�j�j
bool TCP::process(void){
//	cout << "TCP process in" << endl;
	makeSegment();																						//�Z�O�����g�̍쐬
	return true;																						//�I�u�W�F�N�g�͔j�����Ȃ�
}

//SINK�Ƃ̌�������
//�����iSINK�I�u�W�F�N�g�j
//�߂�l�i�Ȃ��j
void TCP::connectSink(TCPSINK* sPtr){
	objectPtr = sPtr;
	sPtr->setObject(this);
}

//�Z�O�����g�쐬
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void TCP::makeSegment(void){
//	cout << "test makeSegment --- " << getSimu()->getNow().dtime() << endl;
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	short nid = getNid();																			//�m�[�hID
	short wSize = window.size();
	for(char i = 0; i < (char)(windowSize - wSize); i++){									//�E�B���h�E�T�C�Y�̋󂫕������V���ɃZ�O�����g�쐬
		if(byte < size){																					//�쐬�ς݃Z�O�����g�T�C�Y��TCP�T�C�Y��菬�������
			short segSize = min(size - byte, TCPSIZE);												//���M�Z�O�����g�T�C�Y
			SEGMENT* segPtr = new SEGMENT(sPtr, now, this, segSize, makeSegNum++, nid);	//�Z�O�����g�I�u�W�F�N�g�̍쐬	
			window.push_back(segPtr);																		//�E�B���h�E�ɃZ�O�����g��o�^
			segCash.push_back(segPtr);																		//�L���b�V���ɂ��Z�O�����g��o�^
			if(segCash.size() > 1000){
				delete segCash[0];
				segCash.erase(segCash.begin());
			}
			byte += segSize;																					//�쐬�Z�O�����g�T�C�Y�̍X�V
//�������������������O�\������������������
//�s�b�o�Z�O�����g���x���̑��M���O���s�K�v�Ȃ�R�����g�A�E�g����	
			//cout << now.dtime() << "\tS\t" << nid << "\tTCP\t" << this << "\t" << segPtr->getSeq()
			//<< "\t" << windowSize << "\t" << byte << endl;
//����������������������������������������
		}
		else																									//�쐬�ς݃Z�O�����g�T�C�Y��TCP�T�C�Y�ȏ�Ȃ�
			window.push_back(NULL);
	}
//	cout << "last " << lastSendSeq << "\tnext " << nextSendSeq << endl;  
	if(lastSendSeq < nextSendSeq)																	//�E�B���h�E�̐擪�Z�O�����g�����̑��M�Z�O�����g�Ȃ�
		makePacket();																						//���M�p�P�b�g�쐬����
}

//���M�p�P�b�g�쐬
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void TCP::makePacket(void){
	if(buffering)																						//���Ƀo�b�t�@�����O����Ă���p�P�b�g������Ȃ�
		return;																								//�������Ȃ�
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	short nid = getNid();																			//�m�[�hID
	for(short i = 0; i < (short)windowSize; i++){											//�E�B���h�E�T�C�Y�����E�B���h�E�̒��g���`�F�b�N
		if(!window[i])
			break;
		if(window[i]->getSeq() == nextSendSeq){													//���̑��M�Z�O�����g�����݂���Ȃ�
			SEGMENT* segPtr = window[i];																	//���M�ΏۃZ�O�����g
			PACKET* pPtr																						//�p�P�b�g�I�u�W�F�N�g�̍쐬
				= new PACKET(sPtr, now, PACKET::Tcp, segPtr->getSize() + 40, nid, nid, objectPtr->getNid(), -1);
//			cout << "make packet " << segPtr->getSeq() << endl;
			pPtr->setSeg(segPtr);																			//�p�P�b�g�̑Ή��Z�O�����g�o�^
			pPtr->setSeq(segPtr->getSeq());																//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			if(!pPtr->queue(sPtr, false)){																//�o�b�t�@�ւ̃p�P�b�g�}��
				delete pPtr;																						//�}���Ɏ��s���������
				setTime(now + 100000);																			//���̃p�P�b�g�������Ԃ�ݒ�
				sPtr->list.insert(this);																		//�C�x���g���X�g�֒ǉ�
			}
			else{																									//�}���ɐ���������
				buffering = true;																					//�o�b�t�@�����O�t���O�𗧂Ă�
				nextSendSeq++;																						//���̑��M�Z�O�����g�V�[�P���X�ԍ���ݒ�
			}
			break;
		}
	}
}

//�`�b�j��M����
//�����i�p�P�b�g�I�u�W�F�N�g�C���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�F�i�Ȃ��j
void TCP::getTcpAck(PACKET* pPtr){
	if(abort)
		return;
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	SEGMENT* segPtr = pPtr->getSeg();															//�m�[�hID
//�������������������O�\������������������
//���M���m�[�h��ACK��M���O���s�K�v�Ȃ�R�����g�A�E�g����
//	pPtr->showLog(getNid(), "R", now);
//����������������������������������������
	nowRtt = now - segPtr->getSendStart();														//�����RTT�𑪒�
	int reqSeq = pPtr->getReqSeq();																//ACK�������҃V�[�P���X�ԍ�
	int nowRttVal = nowRtt.getSec() * 1000000 + nowRtt.getLessSec();					//����RTT�̐����l
	int rttVal = rtt.getSec() * 1000000 + rtt.getLessSec();								//����RTT�̐����l
	rtt = int(rttVal * RTT_ALPHA + nowRttVal * (1 - RTT_ALPHA));						//�X�V����TT�̌v�Z 
	int diff = timeCompare(rtt, nowRtt)															//����RTT�Ɛ���RTT�̍��̐����l�̐�Βl
		? (rtt - nowRtt).getSec() * 1000000 + (rtt - nowRtt).getLessSec()
		: (nowRtt -rtt).getSec() * 1000000 + (nowRtt - rtt).getLessSec();
	D = D * RTT_ALPHA + diff * (1 - RTT_ALPHA);												//�^�C���A�E�g�p�΍��̌v�Z
//	cout << "receiveSeq " << reqSeq << "\tlastAckSeq " << lastReqSeq << endl;
	if(windowSize <= windowThreshold)															//�E�B���h�E�T�C�Y�̍X�V
		windowSize += 1;																					//�ʏ탂�[�h
	else
		windowSize += 1 / windowSize;																	//�t�s������[�h
	if(reqSeq == lastReqSeq && reqSeq < pPtr->getSeq()){									//�V����ACK��M�Ŋ��҃V�[�P���X���O��Ɠ����Ȃ��
		if(window[0]->getAckRepeat() == 2 && window[0]->getNAretrans() == false){		//�d��ACK��M�đ��͂��ĂȂ�����2��d��ACK��M���Ă����ꍇ
////�������������������O�\������������������
////�`�b�j�ɂ��đ��̑��M���O���s�K�v�Ȃ�R�����g�A�E�g����	
			//cout << now.dtime() << "\tNA\t" << getNid() << "\tTCP\t" << this << "\t" << reqSeq
			//				<< "\t--------\t--------" << endl;
////����������������������������������������
			window[0]->setNAretrans(true);																//�d��ACK��M�ɂ��đ��ς݃t���O�𗧂Ă�
			retransmission();																					//�đ�����
			return;																								//�֐������I��
		}
		else																									//�d���񐔂�1�̏ꍇ
			window[0]->increAckRepeat();																	//�d���񐔂̃C���N�������g
	}
	else if(reqSeq > lastReqSeq){																	//�����M�̊��҃V�[�P���X���O����傫�����
		while(window.size() > 0 && window[0] != NULL){											//�E�B���h�E�T�C�Y��0�łȂ�����
			if(window[0]->getSeq() < reqSeq){															//�E�B���h�E�擪�̃V�[�P���X�����Ғl��菬�������
				sPtr->list.remove(window[0]->getTimeout());
				window[0]->setTimeout(NULL);
				window.erase(window.begin());																	//�E�B���h�E����폜				
			}	
			else{																									//�E�B���h�E�̐擪�����҃V�[�P���X�ƂȂ�����
				window[0]->increAckRepeat();																	//�d���񐔂̃C���N�������g
				break;																								//�I��
			}
		}
		lastReqSeq = reqSeq;
		nextSendSeq = max(reqSeq, nextSendSeq);
	}
	makeSegment();																						//�Z�O�����g�̍쐬					
	if(reqSeq * TCPSIZE >= size && finish == false){																	//�������M����Z�O�����g���Ȃ��ꍇ
		finish = true;
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(20,0), TIMEOUT::Tcp, getNid());//TCP�����p�^�C���A�E�g�I�u�W�F�N�g���쐬
		toPtr->setTcp(this);																				//�I�u�W�F�N�g�ɑΉ�TCP��o�^
		sPtr->list.insert(toPtr);																		//�����^�C���A�E�g�I�u�W�F�N�g���C�x���g�ɒǉ�
		if(!maPtr){
			sPtr->increTcpData(size);																		//�����M�f�[�^�T�C�Y�̃C���N�������g
			sPtr->increTcpTime(now - startTime);														//�����M���Ԃ̃C���N�������g
			//cout << maPtr << "\tTCP throughput " << now.dtime() << "\t" << (int)pPtr->getHop() << "\t" << size * 8  / (now - startTime).dtime()  / 1000 / 1000 << endl;
		}
		if(maPtr){																							//�I�u�W�F�N�g�����݂���ꍇ
			NODE* nPtr = sPtr->node[getNid()];															//�m�[�h�I�u�W�F�N�g
			if(nPtr->ma.size() > 0){																		//MA��ێ����Ă���Ȃ�
				vector<MA*>::iterator i = nPtr->ma.begin();	
				while(*i != maPtr && i != nPtr->ma.end())												//����ړ�����MA�����`�F�b�N
					i++;
				if(i != nPtr->ma.end()){																		//�ړ�����MA������������
					MA* mPtr = *i;																					//�폜�Ώۂ�MA�I�u�W�F�N�g
					sPtr->list.remove(mPtr->getTimeout());													//�ړ��p�^�C���A�E�g�I�u�W�F�N�g�����X�g����O��
					sPtr->list.remove(mPtr);																	//�C�x���g���X�g����MA���O��
					nPtr->ma.erase(i);																			//����MA�����X�g����͂���
					i = sPtr->ma.begin();																		//�V�~�����[�^�I�u�W�F�N�g����MA��T��
					while(*i != maPtr && i != sPtr->ma.end())												//����ړ�����MA�����`�F�b�N
						i++;							
					sPtr->ma.erase(i);																			//MA�����X�g����O��
				}
			}
		}
	}
}

//�đ�����
//�����i�Ȃ��j
//�߂�l�F�i�Ȃ��j
void TCP::retransmission(void){
	//cout << "test1" << endl;
	windowThreshold = windowSize / 2.0;															//臒l�͌��݂̃E�B���h�E�T�C�Y�̔���
	windowSize = 1;																					//�E�B�h�E�T�C�Y��1�ɏ�����
	for(short i = 0; i < (short)window.size(); i++){											//�E�B���h�E�x�N�^���`�F�b�N
		if(!window[i])
			break;
		getSimu()->list.remove(window[i]->getTimeout());										//�^�C���A�E�g�I�u�W�F�N�g���폜����
		window[i]->setTimeout(NULL);
	}
	nextSendSeq = window[0]->getSeq();															//���̑��M�Z�O�����g���E�B���h�E�̐擪�ɖ߂�
	lastSendSeq = nextSendSeq - 1;																//���O���M�Z�O�����g�V�[�P���X��������
	window[0]->resetAckRepeat();																	//�d��ACK��M���̏�����
		
	//	cout << "retransmission " << lastSendSeq << "\t" << nextSendSeq << endl;
	makePacket();
}

//�`�b�j���M����
//�����i�p�P�b�g�I�u�W�F�N�g�C���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�i�Ȃ��j
void TCPSINK::sendAck(PACKET* pPtr){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	SEGMENT* segPtr = pPtr->getSeg();															//�Z�O�����g�I�u�W�F�N�g�|�C���^
	int id = getNid();																				//�m�[�hID
	bool newSegFlag = true;																			//�V�����Z�O�����g��M���������t���O�i�^�ŏ������j
	int receiveSeq = segPtr->getSeq();															//��M�Z�O�����g�̃V�[�P���X�ԍ�
	int requestSeq = lastSeq + 1;																	//��M���҃V�[�P���X�ԍ�
//	cout << "last " << lastSeq << "\trequest " << requestSeq << "\treceive " << receiveSeq << endl;
	if(receiveSeq > lastSeq){																		//�d����M�Z�O�����g�łȂ��ꍇ
		if(receiveSeq > requestSeq){																	//��M�Z�O�����g�̃V�[�P���X�����Ғl���傫���ꍇ
			bool flag = true;																					//�p�P�b�g�̃L���b�V�����݂������t���O�i�񑶍݂ŏ������j
			for(short i = 0; i < (short)cashSeq.size(); i++){										//��M�s�A���Z�O�����g�̃L���b�V���V�[�P���X���`�F�b�N
				if(cashSeq[i] == receiveSeq){																	//�L���b�V�����Ɏ�M�Z�O�����g������ꍇ
					flag = false;																						//�L���b�V�����݃t���O�����낷
					newSegFlag = false;																				//�V�K��M�Z�O�����g�t���O�͋U
					break;
				}
			}
			if(flag)																								//�L���b�V���Ɏ�M�Z�O�����g�������ꍇ
				cashSeq.push_back(receiveSeq);																//��M�Z�O�����g�̒ǉ��o�^
		}
		else{																									//��M�Z�O�����g�����҃V�[�P���X�����ꍇ
			requestSeq++;																						//��M���҃V�[�P���X�̃C���N�������g
			bool flag = true;																					//�A����M�p�P�b�g�`�F�b�N�p�t���O
			while(cashSeq.size() != 0 && flag == true){												//�A����M�p�P�b�g�`�F�b�N�̑Ώۂ��������
				flag = false;																						//�L���b�V���Ɋ��҃Z�O�����g�����݂��邩�������t���O	
				for(vector<int>::iterator i = cashSeq.begin(); i != cashSeq.end();){				//�L���b�V�������`�F�b�N
					if(*i == requestSeq){																			//���҃Z�O�����g�����݂�����
						requestSeq++;																						//���҃V�[�P���X�ԍ����C���N�������g
						cashSeq.erase(i);																					//�ΏۃZ�O�����g���폜
						flag = true;																						//���̑Ώۂ��`�F�b�N���邽�߂̃t���O�ݒ�
						break;																								//�`�F�b�N�͏I��
					}
					else
						i++;
				}
			}
		}
	}
	else																									//��M�Z�O�����g�V�[�P���X�����Ғl�ȉ��̏ꍇ
		newSegFlag = false;																				//�V�K��M�t���O������
	if(newSegFlag == true)																			//�V�K��M�t���O���^�Ȃ�
		byte += segPtr->getSize();																		//�Z�O�����g�̑���M�o�C�g���C���N�������g
			
//�������������������O�\������������������
//����m�[�h��TCP��M���O���s�K�v�Ȃ�R�����g�A�E�g����
//	pPtr->showLog(id, "r", now);
//����������������������������������������
	if(byte == objectPtr->getSize()){															//TCP�̍Ō�̃p�P�b�g����M������
		MA* maPtr = objectPtr->getMA();																//MA�I�u�W�F�N�g
		NODE* nPtr = sPtr->node[id];																	//�m�[�h�I�u�W�F�N�g
		if(maPtr){																							//MA�p��TCP�ł������Ȃ�
			bool flag = true;																					//���ł�MA���ړ����Ă��邩��\���t���O�i���Ȃ���ΐ^�j
			for(char i = 0; i < (char)nPtr->ma.size(); i++){											//���g���ێ����Ă���MA���`�F�b�N
				if(nPtr->ma[i]->getId() == maPtr->getId()){												//TCP�ɑΉ�����MA��ID�������Ȃ��
					flag = false;																						//�t���O�����낷
					break;																								//�`�F�b�N�͏I��
				}
			}
			if(flag){																							//MA���܂��ړ����Ă��Ȃ����
				MA* newMaPtr = new MA(sPtr);																	//�R�s�[�p�I�u�W�F�N�g�̍쐬
				*newMaPtr = *maPtr;																				//MA�I�u�W�F�N�g�̃R�s�[
				newMaPtr->setNid(id);																			//MA�̑؍݃m�[�hID�����g�ɕύX
				newMaPtr->setTime(now + MACHECKINTERVAL);													//�����̏�����
				newMaPtr->resetMigration();																	//�ړ����t���O������
				nPtr->ma.push_back(newMaPtr);																	//�m�[�h�̕ێ�MA���X�g�ɒǉ�
				sPtr->ma.push_back(newMaPtr);																	//�V�~�����[�^�̕ێ�MA���X�g�ɒǉ�
				newMaPtr->increMigNum();																		//�ړ��񐔂̃C���N�������g
				if(sPtr->getArea() == SIMU::MESH){
					TIMEOUT* toPtr = new TIMEOUT(sPtr, now + newMaPtr->getStayTime(), TIMEOUT::MeshMa, id);	//�G�[�W�F���g�ړ��^�C���A�E�g�I�u�W�F�N�g�̍쐬
					toPtr->setMa(newMaPtr);																			//�Ή�MA�̓o�^
					sPtr->list.insert(toPtr);																		//�C�x���g���X�g�֓o�^
				}
				else
					sPtr->list.insert(newMaPtr);
//				cout << "node " << id << " receive MA" << maPtr->getId() << " at " << now.dtime() << " ---- " << endl; 
			}
		}
	}
	PACKET* packetPtr																					//ACK�I�u�W�F�N�g�̍쐬
		= new PACKET(sPtr, now, PACKET::Ack, TCPACKSIZE, id, id, objectPtr->getNid(), -1);
	packetPtr->setSeg(segPtr);																		//�p�P�b�g�̑Ή��Z�O�����g�o�^
	packetPtr->setReqSeq(requestSeq);															//��M���҃Z�O�����g�̃V�[�P���X�ԍ��o�^
	packetPtr->setSeq(receiveSeq);																//ACK�ΏۃZ�O�����g�̃V�[�P���X�ԍ��ݒ�
	if(!packetPtr->queue(sPtr, false))															//�o�b�t�@�ւ̃p�P�b�g�}��
		delete packetPtr;																					//�}���Ɏ��s���������
	lastSeq = requestSeq - 1;																		//���ߎ�M�Z�O�����g�̃V�[�P���X�ԍ��ݒ�

//�������������������O�\������������������
//����m�[�h��ACK���M���O���s�K�v�Ȃ�R�����g�A�E�g����
//	packetPtr->showLog(id, "S", now);
//����������������������������������������
}
extern int cntP;

//�t�c�o�G�[�W�F���g�̏���
//�����i�Ȃ��j
//�߂�l�F(�G�[�W�F���g���p������Ȃ�^�C�I������Ȃ�U�j
bool UDP::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	short here = getNid();
	int dSize = min(size - byte, UDPSIZE);											//�f�[�^�O�����T�C�Y
	PACKET* pPtr																				//�p�P�b�g�I�u�W�F�N�g�̍쐬
		= new PACKET(sPtr, now, PACKET::Udp, dSize + 28, here, here, objectPtr->getNid(), -1);
	pPtr->setUdp(this);																	//�p�P�b�g�̑Ή��t�c�o�G�[�W�F���g�o�^
	pPtr->setSeq(seq++);																	//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�ƃC���N�������g
	if(!pPtr->queue(sPtr, false))															//�o�b�t�@�ւ̃p�P�b�g�}��
		delete pPtr;																				//�}���Ɏ��s���������

//�������������������O�\������������������
//�t�c�o���x���̑��M���O���s�K�v�Ȃ�R�����g�A�E�g����	
	//cout << now.dtime() << "\tS\t" << here << "\tUDP\t" << this << "\t" << getSeq() - 1
	//<< "\t--------\t" << byte + dSize << "\t" << cntP << endl;
//����������������������������������������

	if(increByte(dSize) < size){														//���M�o�C�g�������T�C�Y�ɒB���Ă��Ȃ����
		setTime(now + int(8 * dSize / (1000 * rate) * 1000000.0));						//���[�g���玟�̃p�P�b�g�������Ԃ�ݒ�
		sPtr->list.insert(this);																			//�C�x���g���X�g�֒ǉ�
	}
	else{																								//���M�o�C�g�������T�C�Y�ɒB���Ă�����
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(20,0), TIMEOUT::Udp, getNid());//UDP�����p�^�C���A�E�g�I�u�W�F�N�g���쐬
		toPtr->setUdp(this);																			//�I�u�W�F�N�g�ɑΉ�UDP��o�^
		sPtr->list.insert(toPtr);																			//�C�x���g�ɒǉ�
	}
	return true;
}

//SINK�Ƃ̌�������
//�����iSINK�I�u�W�F�N�g�j
//�߂�l�F�i�Ȃ��j
void UDP::connectSink(UDPSINK* sPtr){
	objectPtr = sPtr;
	sPtr->setObject(this);
}