#include "class.h"
#include "mobileAgent.h"

int cntS = 0;

SIGNAL::SIGNAL(CHANNEL* cPtr){																	//�R���X�g���N�^
//	cout << "signal make " << ++cntS << endl;
	chPtr = cPtr;
	MAC* mPtr = chPtr->getSimu()->node[chPtr->getNid()]->getMAC();
	orSource = -1;
	switch(mPtr->getState()){
	case MAC::Rts:
		typeId = RTS;
		if(mPtr->getOrFlag())
			orSource = mPtr->getOrSource(), cout << cPtr->getSimu()->getNow().dtime() << "  fheroirgei "  << cPtr->getNid() << endl;
		length = mPtr->getRtsTime() + DELAY;
		if(chPtr->getSimu()->getMac() != SIMU::OR_JOJI_ACK)
			duration = 3 * (SIFSTIME + DELAY) + mPtr->getCtsTime() + mPtr->getDatatime() + mPtr->getAckTime();
		else
			duration = 3 * (SIFSTIME + DELAY) + mPtr->getCtsTime() + mPtr->getDatatime() + mPtr->getAckTime() + ORACK_RANGE;
		break;
	case MAC::Cts:
		typeId = CTS;
		length = mPtr->getCtsTime() + DELAY;
		if(chPtr->getSimu()->getMac() != SIMU::OR_JOJI_ACK)
			duration = 2 * (SIFSTIME + DELAY) + cPtr->getDataTime() + mPtr->getAckTime();
		else
			duration = 2 * (SIFSTIME + DELAY) + cPtr->getDataTime() + mPtr->getAckTime() + ORACK_RANGE;
		break;
	case MAC::Data:
		typeId = DATA;
		length = mPtr->getDatatime() + DELAY;
		break;
	case MAC::Ack:
		typeId = ACK;
		length = mPtr->getAckTime() + DELAY;
		break;
	case MAC::BData:
		typeId = BDATA;
		length = mPtr->getDatatime() + DELAY;
		break;
	case MAC::OrAck:
		typeId = ORACK;
		length = mPtr->getAckTime() + DELAY;
		break;
	default:
		cout << "WHAT" << endl;
		length = 100;
		break;
	}
}

SIGNAL::~SIGNAL(){
//	cout << "signal delete " << --cntS << endl;
}

CHANNEL::CHANNEL(SIMU* ptr, MAC* mac, SIMUTIME tval, short idval):EVENT(ptr, tval, EVENT::Channel, idval){	//�R���X�g���N�^{
	mPtr = mac;
	sigPtr = NULL;																						//��M�M���I�u�W�F�N�g�i�����l��NULL�j
	sendFlag = false;
}

bool CHANNEL::process(void){
	SIMU* sPtr = getSimu();																			//�V�~�����[�^�I�u�W�F�N�g
	SIMUTIME now = sPtr->getNow();																//���ݎ���
	NODE* nPtr = sPtr->node[getNid()];															//�m�[�h�I�u�W�F�N�g
	if(sendFlag){																						//�M�����M�����̏ꍇ						
		sendFlag = false;																					//���M�t���O������
		double txPower = pow((RANGE / 100) / 1.5, 4) * RX_THRESHOLD;						//���M�d��
		int interval = sigPtr->getLength() + DELAY;												//�M�����M���Ԃ̐ݒ�
		for(short i = 0; i < (short)nPtr->neighborList.size(); i++){						//�אڃm�[�h�ւ̑��M
			short nid = nPtr->neighborList[i].getId();												//�Ώۂ̃m�[�hID
			CHANNEL* chPtr = sPtr->node[nid]->getChannel();											//�`���l���I�u�W�F�N�g
			double distance = nPtr->neighborList[i].getDist() / 100.0;							//�Ώۂ܂ł̋����i���[�g�����Z�j
			double rxPower = txPower * pow(1.5 / distance, 4.0);									//Two-ray ground�̌������f���ɂ��d�͂��Z�o
			if(rxPower > CS_THRESHOLD){																	//�L�����A�Z���X�\�Ȃ�
				RX_SIGNAL_LIST signal(getNid(), rxPower);													//�M���I�u�W�F�N�g
				signal.setRxFinTime(now + SIMUTIME(interval));											//�M����M���������̐ݒ�
				if(chPtr->signalList.size() == 0){															//��M�M�������݂��Ȃ��Ȃ�
					sPtr->node[nid]->calcPower();
					sPtr->node[nid]->setPowerMode(NODE::RECEIVE);
					MAC* omPtr = chPtr->getMAC();																	//��M�m�[�h��MAC�I�u�W�F�N�g
					if(omPtr->getState() == MAC::Backoff || omPtr->getState() == MAC::Difs){		//�o�b�N�I�t��Difs�ҋ@���Ȃ�
						omPtr->setState(MAC::Idle);																	//Idle��ԂɕύX
						sPtr->list.remove(omPtr);																		//�o�b�N�I�t�C�x���g�����X�g����O��
						omPtr->setListInsert(false);																	//���X�g�o�^�t���O�����낷
						SIMUTIME bTime = omPtr->getBackoff();														//�o�b�N�I�t����
						if(timeCompare(bTime, now) || (bTime.getSec() == now.getSec() && 
																	bTime.getLessSec() == now.getLessSec())		)//�o�b�N�I�t�ҋ@���Ȃ�
							omPtr->setBackoff(bTime - now);																//�c��o�b�N�I�t�^�C���̋L��
					}
					chPtr->signalList.push_back(signal);														//�M�����ݒ�
					chPtr->setTime(now + SIMUTIME(interval));													//�M����M���������̐ݒ�
					sPtr->list.insert(chPtr);																		//�C�x���g���X�g�֒ǉ�
					if(rxPower > RX_THRESHOLD)																		//�M�����ʉ\�d�͂Ȃ�
						chPtr->setSignal(sigPtr);																		//�M���I�u�W�F�N�g�̐ݒ�
				}
				else{																									//��M�M�������݂���Ȃ�
					vector<RX_SIGNAL_LIST>::iterator j = chPtr->signalList.begin();					//�M����񃊃X�g�̃C�e���[�^�i�����l�̓��X�g�̐擪�j
					while(j != chPtr->signalList.end() && (*j).getRxPower() > rxPower)				//���X�g�̓d�͂��M���d�͂�菬�����Ȃ�܂Ō���`�F�b�N
						j++;
					if(j == chPtr->signalList.begin()){															//�����̓d�͂��ő�̏ꍇ
						chPtr->setTime(now + SIMUTIME(interval));													//�M����M���������̐ݒ�
						sPtr->list.replace(chPtr);																		//�C�x���g���X�g�̍Ĕz�u
						if(rxPower > RX_THRESHOLD && rxPower > (*j).getRxPower() * CP_THRESHOLD)		//�M�����ʉ\�d�͂Ȃ�
							chPtr->setSignal(sigPtr);																		//�M���I�u�W�F�N�g�̐ݒ�
						else if(chPtr->getSignal())																	//�M�����ʕs�d�͂ł��łɎ��ʐM����M���Ȃ�
							chPtr->setSignal(NULL);																			//��M�M�������ʕs�ɂ���
					}
					else																									//�����̓d�͂��ő�łȂ��ꍇ
						if(chPtr->getSignal() != NULL && chPtr->getSignal()->getChannel() != chPtr
							&& rxPower * CP_THRESHOLD > chPtr->signalList[0].getRxPower())					//���ʉ\�M�������݂�����Ɋ�����ꍇ
							chPtr->setSignal(NULL);																			//��M�M�������ʕs�ɂ���
					chPtr->signalList.insert(j, signal);														//��M�M�����X�g�ւ̑}��
				}
			}
		}
		addTime(interval + 1);																			//���M���������̐ݒ�
		sPtr->list.insert(this);																		//�C�x���g���X�g�̐ݒ�
	}
	else{																									//�M����M�����̏ꍇ
		if(signalList.size() == 0)																		//��M�M�������݂��Ȃ��Ȃ�														
			cout << now.dtime() << "\t" << getNid() << "  signal receive error " << endl, exit(1);										//��M�M���G���[
		vector<RX_SIGNAL_LIST>::iterator i = signalList.begin();								//�M����񃊃X�g�̃C�e���[�^�i�����l�̓��X�g�̐擪�j
		i = signalList.erase(i);																				//���X�g�̐擪������
		while(i != signalList.end()){																	//���X�g���Ō���܂Ń`�F�b�N
			if(timeCompare(now, (*i).getRxFnTime()))													//���Ɏ�M�������Ă���M��������ꍇ
				i = signalList.erase(i);																				//���X�g�������
			else
				i++;
		}
		if(sigPtr){																							//�M���I�u�W�F�N�g���ݒ肳��Ă�����
			if(sigPtr->getChannel() != this){															//���m�[�h�̑��M�M���ł�������
				mPtr->receiveSignal(sigPtr, sigPtr->getChannel()->getMAC());						//�M����M����
				sigPtr = NULL;																						//�M���o�^�����Z�b�g
			}
			else{																									//���m�[�h�̑��M�M���ł�������
				if(sigPtr->getType() != SIGNAL::BDATA )													//�u���[�h�L���X�g�M���łȂ��Ȃ�											
					mPtr->sendFinSignal(sigPtr, sPtr->node[mPtr->getObject()]->getMAC());			//���j�L���X�g�M�����M��������
				else																									//�u���[�h�L���X�g�M���Ȃ�	
					mPtr->sendFinSignal(sigPtr, NULL);															//�u���[�h�L���X�g�M�����M��������
				delete sigPtr;																						//�M���I�u�W�F�N�g�̏���
				sigPtr = NULL;																						//�M���o�^�����Z�b�g
			}
			if(signalList.size()){																			//��M�M��������ꍇ
				setTime(signalList[0].getRxFnTime());														//�C�x���g������������M���������ɐݒ�
				sPtr->list.insert(this);																		//�C�x���g���X�g�֓o�^
			}
			else{
				getSimu()->node[getNid()]->calcPower();
				getSimu()->node[getNid()]->setPowerMode(NODE::WAIT);
			}
		}
		else{																									//�M���I�u�W�F�N�g���ݒ肳��Ă��Ȃ�������
			//cout << getNid() << " ���ʕs�M����M" << endl;
			if(!signalList.size()){																			//��M�M�����Ȃ��ꍇ
				getSimu()->node[getNid()]->calcPower();
				getSimu()->node[getNid()]->setPowerMode(NODE::WAIT);
				if(mPtr->getState() == MAC::Idle)															//Idle��Ԃ̏ꍇ
					mPtr->checkFrame();																				//�t���[���`�F�b�N
			}
			else{																									//��M�M��������ꍇ
				setTime(signalList[0].getRxFnTime());														//���̎�M���������̐ݒ�
				sPtr->list.insert(this);																		//�C�x���g���X�g�֓o�^
			}
		}
	}
	return true;																						//�֐����I���i�I�u�W�F�N�g�͏������Ȃ��j
}

//�M�����M����
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void CHANNEL::sendSignal(SIMUTIME now){
	getSimu()->node[getNid()]->calcPower();
	getSimu()->node[getNid()]->setPowerMode(NODE::SEND);
	SIGNAL* siPtr = new SIGNAL(this);															//�M���I�u�W�F�N�g�̍쐬
	int interval = siPtr->getLength() + 1;														//�M�����M���Ԃ̐ݒ�
	sigPtr = siPtr;																					//�M���I�u�W�F�N�g�̐ݒ�
	RX_SIGNAL_LIST signal(getNid(), 10000000000000);										//�M���I�u�W�F�N�g
	signal.setRxFinTime(now + SIMUTIME(interval));											//�M����M���������̐ݒ�
	if(signalList.size() > 0)																		//��M�M�������݂���Ȃ�
		getSimu()->list.remove(this);																	//�C�x���g���X�g�����M�������O��
	signalList.insert(signalList.begin(), signal);											//�����̐M������M�M�����X�g�֑}��
	sendFlag = true;
}