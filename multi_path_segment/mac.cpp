#include "class.h"
#include "mobileAgent.h"

extern unsigned long genrand();																	//�����Z���k�c�C�X�^��������
static int cntTry, cntSuc;


//MAC�I�u�W�F�N�g�̃R���X�g���N�^ 
MAC::MAC(SIMU* ptr, SIMUTIME tval, short idval):EVENT(ptr, tval, EVENT::Mac, idval){
	rtsTime = 16 + (1 + (16 + RTSSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	ctsTime = 16 + (1 + (16 + CTSSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	ackTime = 16 + (1 + (16 + ACKSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	contWindow = 16;
	backoffTime = SIMUTIME(-2, 0);
	retrans = 0;
	framePtr = NULL;																					//�Ή��t���[���I�u�W�F�N�g�̏������iNULL�j
	listInsert = false;
	state = Idle;																						//MAC��Ԃ̏������iIdle�j
	for(short i = 0; i < 200; i++)																	//��M�t���[���L���b�V���̏�����
		last.push_back(-1);
	orFlag = false;																					//OR�t���O�̏�����
	orPtr = NULL;																						//OR�p�P�b�g�̏�����
}

////�l�`�b�I�u�W�F�N�g�̃f�R���X�g���N�^
//MAC::~MAC(){
//	if(getFrame() != NULL){																			//�t���[���I�u�W�F�N�g�����݂��Ă���ꍇ
//		if(getFrame()->getPacket() != NULL)															//�Ή��p�P�b�g�I�u�W�F�N�g�����݂��Ă���ꍇ
//			delete getFrame()->getPacket();																//�p�P�b�g�I�u�W�F�N�g������
//		delete getFrame();																				//�t���[���I�u�W�F�N�g������
//	}
//}

void MAC::show(double val = 0){
		int sec = (int)floor(val);
		int musec = (int)((val - sec) * 1000000);
		short lessMuSec = getEventTime().getLessMuSec();
		char* type[20] = {"Idle", "Difs", "Backoff", "Rts", "Cts", 
		"Waitcts", "Data", "BData", "Waitdata", "Ack", "Waitack", "Nav", "NavFin", "WaitOrAck", "WaitOtherAck", "OrAck"};

		if(timeCompare(getEventTime(), SIMUTIME(sec, musec))){
			cout << getEventTime().dtime() << "_";
			if(lessMuSec > 99)
				cout << lessMuSec;
			else if(lessMuSec > 9) 
				cout << "0" << lessMuSec;
			else cout << "00" << lessMuSec;
			cout << " --- " << type[(int)getState()] << "\t";
			if(getSimu()->node[getNid()]->getChannel()->getSignal())
				if(getSimu()->node[getNid()]->getChannel()->getSignal()->getChannel() == getSimu()->node[getNid()]->getChannel())
					cout << "sending" << "\t";
				else
					cout << "receiving" << "\t";
			else
				if(getSimu()->node[getNid()]->getChannel()->signalList.size())
					cout << "carrier sensing" << "\t";
				else
					cout << "idling" << "\t";
			cout << listInsert << endl;
		}
}

//MAC�C�x���g����
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�F�i�u�[���ϐ��j
bool MAC::process(void){
	listInsert = false;																				//���X�g�o�^�t���O�����낷
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	NODE* nPtr = sPtr->node[getNid()];															//�m�[�h�I�u�W�F�N�g
	CHANNEL* chPtr = nPtr->getChannel();														//�`���l���I�u�W�F�N�g
	switch(state){																						//MAC��Ԃɂ��ꍇ����
	case Idle:																							//Idle�Ȃ�
		cout << now.dtime() << "\t" << getNid() <<  " mac idle error " << endl, exit(1);
		break;
	case Difs:																							//Difs�Ȃ�
		if(chPtr->signalList.size())																	//��M�M�������݂���Ȃ�
			cout << "difs error " << endl, exit(1);													//�G���[
		makeBackoff(now);																					//�o�b�N�I�t����
		break;
	case Backoff:																						//Backoff�Ȃ�
		if(chPtr->signalList.size())																	//��M�M�������݂���Ȃ�
			cout << "backoff error " << endl, exit(1);												//�G���[
		backoffTime.setTime(-2, 0);																	//�o�b�N�I�t���Ԃ̏�����
		objectId = framePtr->getDest();																//���M�Ώۑ�����t���[���̈���ɐݒ�
		if(framePtr->getCast() == FRAME::Uni){														//���j�L���X�g�t���[���Ȃ�
//			cout << now.dtime() << "\t" << getNid() << "\tRTS���M" << endl;
			if(!judgeSendable()){																			//���M�s�Ȃ�
				nPtr->sendError(framePtr->getPacket());													//���[�g�G���[���M����
				reset();																								//���M�����̏�����
				if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
					checkFrame();																						//�t���[���`�F�b�N				
				break;
			}
			state = Rts;																						//Rts��Ԃ֑J��
			retrans++;																							//���M�@����C���N�������g
		}
		else{																									//�u���[�h�L���X�g�t���[���Ȃ�
			state = BData;																						//BData��Ԃ֑J��				
//			cout << now.dtime() << "\t" << getNid() << "\tBDATA���M" << endl;		
		}
		chPtr->sendSignal(now);																			//�`���l���̐M�����M����
		chPtr->setTime(now);																				//�`���l���̎��������݂ɐݒ�
		sPtr->list.insert(chPtr);																		//�`���l�����C�x���g���X�g�֓o�^
		break;
	case Cts: case Data: case Ack: case OrAck:													//Cts�CData�CAck, OrAck�Ȃ�
		//if(state == Cts)
		//	cout << now.dtime() << "\t" << getNid() << "\tCTS���M" << endl;
		//if(state == Data)
		//	cout << now.dtime() << "\t" << getNid() << "\tDATA���M" << endl;
		//if(state == Ack)
		//	cout << now.dtime() << "\t" << getNid() << "\tACK���M" << endl;
		if(state == OrAck)
			objectId = orSource;
		if(state == Data){																				//Data���M�̏ꍇ
			if(!judgeSendable()){																			//���M�s�Ȃ�
				nPtr->sendError(framePtr->getPacket());													//���[�g�G���[���M����
				reset();																								//���M�����̏�����
				if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
					checkFrame();																						//�t���[���`�F�b�N				
				break;
			}
		}
		chPtr->sendSignal(now);																			//�`���l���̐M�����M����
		chPtr->setTime(now);																				//�`���l���̎��������݂ɐݒ�
		sPtr->list.insert(chPtr);																		//�`���l�����C�x���g���X�g�֓o�^
		break;
	case Nav: case Waitcts: case Waitdata:														//Nav, Waitcts, Waitdata�Ȃ�
		//if(state == Waitcts)
		//	cout << "waitcts�ɂ��đ�" << endl;
		//if(state == Waitdata)
		//	cout << "waitdata�ɂ��đ�" << endl;
		//if(state == Waitack)
		//	cout << "waitack�ɂ��đ�" << endl;
		state = Idle;																						//Idle��ԂɕύX
		if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
			checkFrame();																						//�t���[���`�F�b�N
		break;
	case Waitack:																						//Waitack�Ȃ�
		if(sPtr->getMac() == SIMU::OR_JOJI_ACK){														//JOJI�����Ȃ�
			state = WaitOrAck;																				//OR�pACK��M�ҋ@��ԂɑJ��
			addTime(ORACK_RANGE + ackTime + 1);																		//�C�x���g�����̐ݒ�
			sPtr->list.insert(this);																		//���g���C�x���g���X�g�֓o�^
		}
		else{																									//�ʏ�̕����Ȃ�
			//cout << now.dtime() << "\t" << (double)++cntSuc / cntTry << " rr " << endl;			
			state = Idle;																						//Idle��ԂɕύX
			if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
				checkFrame();																						//�t���[���`�F�b�N
		}
		break;
	case WaitOtherAck:
		if(sPtr->getMac() == SIMU::OR_JOJI_ACK){														//ORACK�����̏ꍇ	
			state = OrAck;																						//OR�pACK���M�҂���ԂɑJ��
			addTime(genrand() % ORACK_RANGE);															//�C�x���g�����̐ݒ�
			sPtr->list.insert(this);																		//���g���C�x���g���X�g�֓o�^		
			listInsert = true;
		}
		else{																									//ORRTS�����̏ꍇ
//**			orFlag = true;																						//OR�p�t���O�𗧂Ă�
			//cout << now.dtime() << "\t" << getNid() << "\tOR begin" << orPtr << "\t" << orPtr->getSize() << endl;
			//nPtr->queueShow();
			PACKET* pPtr = nPtr->relayPacket(orPtr, 0);												//�p�P�b�g���p����
			//nPtr->queueShow();
			//cout << now.dtime() << "\t" << getNid() << " wait other ack " << orPtr << endl;
			delete orPtr;																						//OR�p�p�P�b�g�I�u�W�F�N�g�̔p��
			orPtr = pPtr;																						//���M�o�b�t�@�ɗ��߂��p�P�b�g��V�KOR�p�p�P�b�g�ɂ���										
			//nPtr->queueShow();
			state = Idle;																						//Idle��ԂɕύX
			if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
				checkFrame();																						//�t���[���`�F�b�N
		}
		break;
	case WaitOrAck:
		//cout << now.dtime() << "\t" << (double)++cntSuc / cntTry << endl;			
		state = Idle;																						//Idle��ԂɕύX
		if(!chPtr->signalList.size())																	//��M�M�������݂��Ȃ��Ȃ�
			checkFrame();																						//�t���[���`�F�b�N
		break;
	default:
		cout << now.dtime() << " mac process error" << endl, show(0), exit(1);
	}
	return true;
}

//�f�[�^�t���[�����M���v���Ԃ̌v�Z
//�����i�t���[���I�u�W�F�N�g�j
//�߂�l�i�Ȃ��j
void MAC::calcDataTime(FRAME* fPtr){
	short size = fPtr->getSize();
	dataTime = 16 + (1 + (size + 1) / (PHYSPEED * 4)) * 4;
}


//�M����M����
//�����i�M���I�u�W�F�N�g�C���M�m�[�h��MAC�I�u�W�F�N�g�j
//�߂�l�i�Ȃ��j
void MAC::receiveSignal(SIGNAL* sigPtr, MAC* omPtr){
	//if(getNid() == 229)
	//cout << getSimu()->getNow().dtime() << "\t" << getNid() << " �M����M" << sigPtr->getType() 
	//	<< "\t" << sigPtr->getChannel()->getNid() << "\t" << state << endl; 
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	NODE* nPtr = sPtr->node[getNid()];															//�m�[�h�I�u�W�F�N�g 
	CHANNEL* cPtr = nPtr->getChannel();															//�`���l���I�u�W�F�N�g
	switch(sigPtr->getType()){																		//�M���^�C�v�ɂ��ꍇ����
	case SIGNAL::RTS:																					//RTS�M���̏ꍇ
		if(omPtr->getObject() == getNid()){															//RTS�̑Ώۂ��������g�Ȃ�
			if(state == Nav)																					//NAV��ԂȂ�
				break;																								//�������Ȃ�
			if(cPtr->signalList.size()){																	//�܂���M�M�������݂���Ȃ�
				state = Idle;																						//Idle��Ԃ֑J��
				sPtr->list.remove(this);																		//���X�g�ɓo�^����Ă���C�x���g������
				listInsert = false;																				//�C�x���g�o�^�t���O�����낷
				break;
			}
//			cout << now.dtime() << "\t" << getNid() << "\tRTS��M�@�|���@CTS���M" << endl;
			cPtr->setDataTime(omPtr->getDatatime());													//��M�f�[�^���v���Ԃ̐ݒ�
			state = Cts;																						//CTS��Ԃ֑J��
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS���Ԃ����ҋ@
			objectId = omPtr->getNid();																	//����M�Ώۃm�[�hID�̐ݒ�
		}
		else{																									//RTS�̑Ώۂ������łȂ�������
			if(orFlag && sigPtr->getOrSource() == orSource){										//OR�t���O���o���Ă��ē���ORRTS���󂯎�����ꍇ
				//cout << getNid() << " delete by ORRTS " << (int)retrans << endl;
				if(framePtr && framePtr->getPacket() == orPtr){
					reset();																								//���M�����̏�����
					if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
						checkFrame();																						//���M�t���[���̃`�F�b�N			
				}
				else{
					vector<PACKET*>::iterator i = nPtr->getBuffer()->queue.begin();					//�o�b�t�@����̃p�P�b�g�폜�p�C�e���[�^
					while(i != nPtr->getBuffer()->queue.end() && (*i) != orPtr)							//���M�o�b�t�@���瓯��OR�p�P�b�g��T��
						i++;
					//if(i == nPtr->getBuffer()->queue.end())													//����OR�p�P�b�g���Ȃ�������G���[
					//	cout << "OR Packet error " << endl, exit(1);
					nPtr->getBuffer()->queue.erase(i);															//�������p�P�b�g���폜
					cout << "receive rts " << orPtr << endl;
					delete orPtr;																						//OR�p�P�b�g���폜
				}
				orPtr = NULL;																						//OR�p�P�b�g��������
				orFlag = false;																					//OR�t���O�����낷
			}
			else if(sigPtr->getOrSource() == getNid()){												//ORRTS�̑Ώۂ�������������
				cntSuc--;
				if(framePtr)																						//�t���[�������݂���Ȃ�
					reset();																								//���M�����̏�����
			}
			state = Nav;																						//Nav��Ԃ֑J��
			setTime(now + SIMUTIME(sigPtr->getDuration()));											//Nav���Ԃ����ҋ@
		}
		sPtr->list.replace(this);																		//�C�x���g���X�g�֒ǉ�
		break;
	case SIGNAL::CTS:																					//CTS�M���̏ꍇ
		if(omPtr->getObject() == getNid() && state == Waitcts){								//CTS�̑Ώۂ��������g�Ȃ�
			if(cPtr->signalList.size()){																	//�܂���M�M�������݂���Ȃ�
				state = Idle;																						//Idle��Ԃ֑J��
				if(listInsert){																					//MAC�C�x���g�����X�g�o�^����Ă���Ȃ�
					sPtr->list.remove(this);																		//���X�g�ɓo�^����Ă���C�x���g������
					listInsert = false;																				//�C�x���g�o�^�t���O�����낷
				}
				break;
			}
//			cout << now.dtime() << "\t" << getNid() << " \tCTS��M�@�[���@DATA���M" << endl;
			state = Data;																						//��M�M�������݂��Ȃ��Ȃ�DATA��Ԃ֑J��
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS���Ԃ����ҋ@
		}
		else{																									//CTS�̑Ώۂ������łȂ�������
//			cout << getNid() << " NAV�֑J��" << endl;
			state = Nav;																						//Nav��Ԃ֑J��
			setTime(now + SIMUTIME(sigPtr->getDuration()));											//Nav���Ԃ����ҋ@
		}
		sPtr->list.replace(this);																		//�C�x���g���X�g�֒ǉ�
		break;
	case SIGNAL::DATA:																				//DATA�M���̏ꍇ
		if(omPtr->getObject() == getNid() && state == Waitdata){								//DATA�̑Ώۂ��������g�Ȃ�
//			cout << now.dtime() << "\t" << getNid() << "\tDATA��M�@�|���@ACK���M  " << omPtr->getNid() << "\t" << omPtr->getFrame()->getPacket() << "\t" << omPtr->getFrame()->getSeq() << endl;
			FRAME* fPtr = omPtr->getFrame();																//��M�t���[���I�u�W�F�N�g
			bool flag = true;																					//����t���[���Ď�M�`�F�b�N�p�t���O
			for(short i = 0; i < (short)last.size(); i++){												//���ߎ�M�t���[�����`�F�b�N
				if(last[i] == fPtr->getSeq() + fPtr->getSource() * 1000000){						//����t���[�������łɎ�M���Ă�����
					flag = false;																						//�t���O���U�ɂ����[�v�I��
					break;
				}
			}
			if(flag == true){																					//�t���O���^�i����t���[���̎�M�Ȃ��j�̏ꍇ
				last.push_back(fPtr->getSeq() + fPtr->getSource() * 1000000);						//���ߎ�M�t���[���ɓo�^
				last.erase(last.begin());																		//�Â���M�t���[�������폜
				PACKET* pPtr = omPtr->getFrame()->getPacket();											//��M�p�P�b�g�I�u�W�F�N�g
				if(pPtr->getDest() == getNid() || pPtr->getType() == PACKET::RrepAodv || pPtr->getType() == PACKET::RerrAodv
					|| ((pPtr->getType() == PACKET::InformLoc || pPtr->getType() == PACKET::MrReq) && sPtr->ma[pPtr->getDest() - sPtr->node.size()]->getNid() == getNid())){//�p�P�b�g�̈��悪������AODV�p����p�P�b�g�Ȃ�
					RECEIVE* rPtr = new RECEIVE(sPtr, getNid(), now, pPtr, omPtr->getNid());		//��M�I�u�W�F�N�g�̍쐬
					rPtr->setTime(now);																				//�C�x���g�����̐ݒ�
					sPtr->list.insert(rPtr);																		//�C�x���g���X�g�֒ǉ�
				}
				else																									//����ȊO�̃p�P�b�g�Ȃ�
					nPtr->relayPacket(pPtr, 0);																	//�p�P�b�g���p����
			}
			if(cPtr->signalList.size()){																	//�܂���M�M�������݂���Ȃ�
				state = Idle;																						//Idle��Ԃ֑J��
				sPtr->list.remove(this);																		//���X�g�ɓo�^����Ă���C�x���g������
				listInsert = false;																				//�C�x���g�o�^�t���O�����낷
				break;
			}
			state = Ack;																						//��M�M�������݂��Ȃ��Ȃ�ACK��Ԃ֑J��
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS���Ԃ����ҋ@
			sPtr->list.replace(this);																		//�C�x���g���X�g�֒ǉ�
		}
		else{																									//DATA�̑Ώۂ������łȂ�������
			if(judgeOpportunistic(omPtr)){																//OR���\�ȏꍇ
				state = WaitOtherAck;																			//�Ώۂ�ACK��M�ҋ@��ԂɑJ��
				if(orPtr != NULL){																				//OR�p�P�b�g�����݂���Ȃ�
					//cout << now.dtime() << "\t" << getNid() << " duplicate opportunistic " << orPtr << endl;
					delete orPtr;																						//OR�p�P�b�g�̍폜
					orPtr = NULL;																						//OR�p�P�b�g�̏�����
				}
				orPtr = new PACKET();																			//OR�p�P�b�g�I�u�W�F�N�g
				//cout << now.dtime() << " opportunistic !!!!"  << getNid() << "\t" << orPtr << endl;
				*orPtr = *(omPtr->getFrame()->getPacket());												//��M�p�P�b�g��OR�p�P�b�g�ɃR�s�[
				orPtr->setHere(getNid());
				orSource = omPtr->getNid();																	//OR�p�t���[���̑��M��
				orDest = omPtr->getFrame()->getDest();														//OR�p�t���[���̑��M����
				setTime(now + SIMUTIME(3 * DELAY + SIFSTIME + ackTime));							//ACK�҂����Ԃ����ҋ@
				sPtr->list.replace(this);																		//�C�x���g���X�g�֒ǉ�	
				listInsert = true;																				//�x���g�o�^�t���O�𗧂Ă�
			}
			else{																									//OR���s�̏ꍇ
				if(state == Nav)																					//NAV��ԂȂ�
					break;																								//���̂܂ܑҋ@
				state = Idle;																						//NAV��ԂłȂ��Ȃ�Idle��Ԃ֑J��
				if(listInsert){																					//�C�x���g�o�^���Ă���Ȃ�
					sPtr->list.remove(this);																		//���X�g����O��
					listInsert = false;																				//�o�^�t���O�����낷
				}
				if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
					checkFrame();																						//���M�t���[���L���̃`�F�b�N	
			}
		}
		break;
	case SIGNAL::ACK:																					//ACK�M���̏ꍇ
		if(omPtr->getObject() == getNid() && state == Waitack){								//ACK�̑Ώۂ��������g�Ȃ�
//			cout << now.dtime() << "\t" << getNid() << "\tACK��M�@�|���@�t���[�����M����" << endl;
			reset();																								//���M�����̏�����
			if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
				checkFrame();																						//���M�t���[���̃`�F�b�N
		}
		else{																									//ACK�̑Ώۂ��������g�łȂ��Ȃ�
			//cout << getNid() << "\t" << "OTHER ACK ��M" << endl;
			if(state == WaitOtherAck &&  orSource == omPtr->getObject()){												//				
				//cout << "OTHER ACK ��M" << endl;
				//cout << now.dtime() << "\t" << getNid() << " receive ack " << orPtr << endl;
				delete orPtr;
				orPtr = NULL;
			}
			if(state == Nav)																					//NAV��ԂȂ�
				break;																								//���̂܂ܑҋ@
			state = Idle;																						//NAV��ԂłȂ��Ȃ�Idle��Ԃ֑J��
			if(listInsert){																					//�C�x���g�o�^���Ă���Ȃ�
				sPtr->list.remove(this);																		//���X�g����O��
				listInsert = false;																				//�o�^�t���O�����낷
			}
			if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
				checkFrame();																						//���M�t���[���L���̃`�F�b�N
		}
		break;
	case SIGNAL::BDATA:{																				//BDATA�M���̏ꍇ
		//cout << getNid() << "\tBDATA��M" << endl;
		PACKET* pPtr = omPtr->getFrame()->getPacket();											//��M�p�P�b�g�I�u�W�F�N�g		
		RECEIVE* rPtr = new RECEIVE(sPtr, getNid(), now, pPtr, omPtr->getNid());		//��M�I�u�W�F�N�g�̍쐬
		rPtr->setTime(now);																				//�C�x���g�����̐ݒ�
		sPtr->list.insert(rPtr);																		//�C�x���g���X�g�֒ǉ�
		if(state == Nav)																					//NAV��ԂȂ�
			break;																								//���̂܂ܑҋ@
		state = Idle;																						//NAV��ԂłȂ��Ȃ�IDLE��Ԃ֑J��
		if(listInsert){																					//�C�x���g�o�^���Ă���Ȃ�
			sPtr->list.remove(this);																		//���X�g����O��
			listInsert = false;																				//�o�^�t���O�����낷
		}
		if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
			checkFrame();																						//���M�t���[���L���̃`�F�b�N
		break;
							 }
	case SIGNAL::ORACK:
//		cout << now.dtime() << "\t" << omPtr->getNid() << "\t" << omPtr->getOrSource() << "\t", show();
		if(state == WaitOrAck){
			reset();																								//���M�����̏�����
			if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
				checkFrame();																						//���M�t���[���̃`�F�b�N			
		}
		else{
			if(state == Nav)																					//NAV��ԂȂ�
				break;																								//���̂܂ܑҋ@
			if(state == OrAck){																				//ORACK���M�҂��Ȃ�
				cout << "receive orack " << orPtr << endl;
				delete orPtr;																						//OR�p�P�b�g�̍폜
				orPtr = NULL;																						//OR�p�P�b�g�̏�����
			}
			state = Idle;																						//NAV��ԂłȂ��Ȃ�Idle��Ԃ֑J��
			if(listInsert){																					//�C�x���g�o�^���Ă���Ȃ�
				sPtr->list.remove(this);																		//���X�g����O��
				listInsert = false;																				//�o�^�t���O�����낷
			}
			if(!cPtr->signalList.size())																	//���̎�M�M�����Ȃ��Ȃ�
				checkFrame();																						//���M�t���[���L���̃`�F�b�N			
		}
		break;
	}
}

//�M�����M��������
//�����i�M���I�u�W�F�N�g�C���M�m�[�h��MAC�I�u�W�F�N�g�j
//�߂�l(�Ȃ�)
void MAC::sendFinSignal(SIGNAL* sigPtr, MAC* objectPtr){
	SIMUTIME now = getSimu()->getNow();															//���ݎ���	
//	cout << now.dtime() << "\t" << getNid() << "\t���M����" << sigPtr->getChannel()->getNid() << "\t" << sigPtr->getType() << endl;
	switch(sigPtr->getType()){																		//�M���^�C�v�ɂ��ꍇ����
	case SIGNAL::RTS:																					//RTS�M���̏ꍇ
		if(orFlag){																							//OR�t���O�������Ă���Ȃ�
			sigPtr->setOrSource(orSource);																//OR����ǉ�
			orPtr = NULL;																						//OR�p�P�b�g�̏�����
			orFlag = false;																					//OR�t���O�����낷
		}
		state = Waitcts;																					//CTS�҂���Ԃ֑J��
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + ctsTime));								//�ҋ@���Ԃ̐ݒ�
		getSimu()->list.insert(this);																	//�C�x���g���X�g�֒ǉ�
		listInsert = true;																				//�C�x���g�o�^�t���O�𗧂Ă�
		break;
	case SIGNAL::CTS:																					//CTS�M���̏ꍇ
		state = Waitdata;																					//DATA�҂���Ԃ֑J��
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + objectPtr->getDatatime()));		//�ҋ@���Ԃ̐ݒ�
		getSimu()->list.insert(this);																	//�C�x���g���X�g�֒ǉ�
		listInsert = true;																				//�C�x���g�o�^�t���O�𗧂Ă�
		break;
	case SIGNAL::DATA:																				//DATA�M���̏ꍇ
		cntTry++;
		state = Waitack;																					//ACK�҂���Ԃ֑J��
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + ackTime));								//�ҋ@���Ԃ̐ݒ�
		getSimu()->list.insert(this);																	//�C�x���g���X�g�֒ǉ�
		listInsert = true;																				//�C�x���g�o�^�t���O�𗧂Ă�
		break;
	case SIGNAL::BDATA:																				//BDATA�M���̏ꍇ
		delete framePtr->getPacket();																	//���M�p�P�b�g���폜
		delete framePtr;																					//���M�t���[�����폜
		framePtr = NULL;																					//�o�^�t���[���̃��Z�b�g
	case SIGNAL::ACK:																					//ACK�M���̏ꍇ
		objectId = -1;																						//���M�Ώۂ̉���
		if(backoffTime.getLessSec() > 0 || backoffTime.getSec() > 0)						//�o�b�N�I�t�c�����Ԃ�����Ȃ�
			backoffTime = backoffTime - 1;																//1�ʕb�������炷
		state = Idle;																						//IDLE��Ԃ֑J��
		if(!getSimu()->node[getNid()]->getChannel()->signalList.size())					//���̎�M�M�����Ȃ��Ȃ�
			checkFrame();																						//���M�t���[�������邩�̃`�F�b�N		
		break;
	case SIGNAL::ORACK:																				//ORACK�M���̏ꍇ
		objectId = -1;
		getSimu()->node[getNid()]->relayPacket(orPtr, 0);										//�p�P�b�g���p����
		cout << "sendfin orack " << orPtr << endl;
		delete orPtr;																						//OR�p�P�b�g�̍폜	
		orPtr = NULL;																						//OR�p�P�b�g�̏�����
		break;
	}
}

//�o�b�N�I�t���Ԃ̐ݒ�
//�����i�Ȃ��j
//�߂�l�i�����j
void MAC::makeBackoff(SIMUTIME now){
	if(timeCompare(backoffTime, now + (1023 * SLOTTIME)))									//�o�b�N�I�t�^�C���̏���𒴂��Ă�����
		cout << "backoff error " << endl, backoffTime.show(), exit(1);						//�G���[
	if(backoffTime.getSec() == -2)																//�o�b�N�I�t�^�C�}���N�����Ă��Ȃ����
		backoffTime = now + (genrand() % contWindow) * SLOTTIME;								//�E�B���h�E�T�C�Y����o�b�N�I�t���Ԃ��쐬
	else																									//�o�b�N�I�t�^�C�}���N�����Ă���ꍇ
		backoffTime = now + backoffTime;																//�^�C�}�c�莞�Ԃ���o�b�N�I�t���Ԃ��쐬
	state = Backoff;
	setTime(backoffTime);																			//�o�b�N�I�t���Ԃ̐ݒ�
	getSimu()->list.insert(this);
	listInsert = true;																				//���X�g�o�^�t���O�𗧂Ă�
}

//���M�\���ǂ����̔���
//�����i���g�̃m�[�h�I�u�W�F�N�g�|�C���^�j
//�߂�l�i���M�Ȃ�^�A���M�s�Ȃ�U�j
bool MAC::judgeSendable(void){
	NODE* nPtr = getSimu()->node[getNid()];													//�m�[�h�I�u�W�F�N�g
	if(nPtr->routing[objectId]->getNext() != objectId){
//		cout << getNid() << "cannot send to " << objectId << endl; 
		return false;
	}
	return true;
}

//���M�����̃��Z�b�g
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void MAC::reset(void){
	contWindow = 16;																					//�R���e���V�����E�B���h�E��������
	retrans = 0;																						//�đ��񐔂�������
	//cout << "reset " << framePtr << "\t" << framePtr->getPacket() << endl;
	delete framePtr->getPacket();																	//�p�P�b�g�I�u�W�F�N�g������
	delete framePtr;																					//���M�t���[���I�u�W�F�N�g������
	framePtr = NULL;																					//�t���[���I�u�W�F�N�g�o�^��������
	objectId = -1;																						//����M�Ώۂ�������
	state = Idle;																						//IDLE��Ԃ֑J��
	getSimu()->list.remove(this);																	//MAC�C�x���g�����X�g����O��
	listInsert = false;																				//���X�g�o�^�t���O�����낷
}

//�t���[���̃`�F�b�N����
//�����i���X�g�I�u�W�F�N�g�A�m�[�h�I�u�W�F�N�g�|�C���^�j
//�߂�l�i�Ȃ��j
void MAC::checkFrame(void){

//	cout << getNid() << "\t�`�F�b�N�t���[��" << endl;
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	NODE* nPtr = sPtr->node[getNid()];															//�m�[�h�I�u�W�F�N�g
	CHANNEL* cPtr = nPtr->getChannel();															//�`���l���I�u�W�F�N�g
	if(nPtr->getChannel()->signalList.size() != 0)											//���̐M������M���̏ꍇ
		cout << "check frame error " << endl, exit(1);											//�G���[
	if(framePtr != NULL){																			//���M�҂��̃t���[�������݂���ꍇ
		if(retrans == RETRANS_MAX){																	//�đ��񐔂�����ɒB���Ă�����
			//cout << "�t���[���đ����s���� " << framePtr->getPacket()->getSeq() << "\t" << endl;	
			reset();																								//���M�����̏�����
			nPtr->checkPacket();																				//���M�p�P�b�g�̗L���̃`�F�b�N
			return;																								//���͉��������ɏI��
		}
//		cout << "�V�t���[��" << endl;
		if(retrans > 0){																					//�đ��t���[���̏ꍇ
//			cout << getNid() << "  �t���[���đ����� " << endl;
			contWindow = min(1024, contWindow * 2);													//�R���e���V�����E�B���h�E�̍X�V
		}
		state = Difs;																						//��Ԃ�Difs�ҋ@�ɐݒ�
		setTime(now + DIFSTIME);																		//DIFS���Ԃ����ҋ@
		sPtr->list.insert(this);																		//�C�x���g���X�g�֓o�^
	}
	else																									//���M�҂��̃t���[�������݂��Ȃ��ꍇ
		nPtr->checkPacket();																				//���M�o�b�t�@�̃p�P�b�g�L�����`�F�b�N
}

//�M�����x���̃A�C�h�����O�`�F�b�N
//�����i�Ȃ��j
//�߂�l�i�^�U�l�C�A�C�h�����O�Ȃ�^�j
bool MAC::checkIdling(void){
	if(getSimu()->node[getNid()]->getChannel()->getSignal() == NULL)
		return true;
	return false;
}

//OR�����邩�̔��f
//�����i���M���m�[�h��MAC�I�u�W�F�N�g�j
//�߂�l�i�^�U�l�COR����Ȃ�^�j
bool MAC::judgeOpportunistic(MAC* omPtr){
	if(getSimu()->getMac() == SIMU::NORMAL)													//�ʏ��MAC�Ȃ�
		return false;																						//OR�͂��Ȃ�
	if(orPtr)																							//OR���Ȃ�
		return false;																						//���炽��OR�͂��Ȃ�
	FRAME* fPtr = omPtr->getFrame();																//�t���[���I�u�W�F�N�g
	if(fPtr->getDest() == fPtr->getPacket()->getDest())									//�t���[���ƑΉ��p�P�b�g�̈��悪�����Ȃ�				
		return false;																						//OR�͂��Ȃ�
	NODE* nPtr = getSimu()->node[getNid()];													//�m�[�h�I�u�W�F�N�g
	if(dist(nPtr->getPos(), getSimu()->node[fPtr->getDest()]->getPos()) > RANGE / 2.0)	//�{���̑��M����m�[�h�Ƃ̋������ʐM�͈͂̔����ȏ�Ȃ�
		return false;																						//OR�͂��Ȃ�
	return true;																						//��L�̂�����ɂ����Ă͂܂�Ȃ��Ȃ�OR������
}