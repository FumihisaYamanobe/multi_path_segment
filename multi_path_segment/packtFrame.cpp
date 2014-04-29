#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//�����Z���k�c�C�X�^�����O���錾�i�m�[�h�ړ��ȊO�p�j

//�Z�O�����g�I�u�W�F�N�g�̃R���X�g���N�^
SEGMENT::SEGMENT(SIMU* ptr, SIMUTIME tval, TCP* tPtr, short sizeval, int seqval, short nid):EVENT(ptr, tval, EVENT::Segment, nid){
	tcpPtr = tPtr;
	size = sizeval;																					//�Z�O�����g�T�C�Y
	seq = seqval;																						//�V�[�P���X�ԍ�
	ackRepeat = 0;																						//�d����MACK�񐔂�0
	toPtr = NULL;																						//�^�C���A�E�g�I�u�W�F�N�g�͂Ȃ�
	NAretrans = false;																				//�d��ACK��M�ɂ��đ��ς݃t���O�����낷
}

SEGMENT::~SEGMENT(){
	//static int cnt;
	//if(cnt++ % 10 == 0)
	//	cout << "delete " << cnt << endl;
}

int cntP = 0;
int cntF = 0;
//�p�P�b�g�I�u�W�F�N�g�̃R���X�g���N�^
PACKET::PACKET(SIMU* ptr, SIMUTIME tval, ptype pval, short sval, short sid, short hid, short did, int ttlval)	
:EVENT(ptr, tval, EVENT::Packet, hid){
	//cout << hid << " create " << this << "\t" << pval << endl; 
	if(did == -1 && pval != Null && pval != Routing && pval != MapReqB && pval != MigReq && pval != DummyBroadcast){
		if(ptr != NULL)
			cout << "dest error " << ptr->getNow().dtime() << "\t" << pval << "\t" << sid << endl, exit(1);
		else
			cout << "dest error " << pval << "\t" << sid << endl, exit(1);
	}
	sendStartTime = tval;			
	type = pval;																						//�p�P�b�g�^�C�v
	size = sval;																						//�T�C�Y
	if(size > 1500 || size <= -2 || size == 0)																	//�p�P�b�g�T�C�Y��1�`1500�łȂ����
		cout << "packet size error" << endl, exit(1);											//�G���[
	source = sid;																						//���M���m�[�h
	if(source != -1){
		seq = ptr->node[source]->getSeq();
		ptr->node[source]->increSeq();
	}
	here = hid;																							//���݂̃m�[�h
	dest = did;																							//����m�[�h
	hop = 0;
	if(ttlval == -1)
		ttl = MAXHOP;
	else
		ttl = ttlval;
	sPos = LOCATION(-1, -1);
	dPos = LOCATION(-1, -1);
	cntP++;
	//cout << type << " create " << ++cnt << endl;
}

PACKET::~PACKET(){
	//cout << here << " delete " << this << endl; 
//	switch(getType()){
//		case RreqAodv:
//			delete getAodvRreq();
//			break;
//		case RrepAodv:
//			delete getAodvRrep();
//			break;
//		case MigRep:
//			delete getMigRep();
//			break;
//	}
	//if(type == Tcp)
//	cout << "delete " << cnt-- << endl;
//	cntP--;
}

//�p�P�b�g����
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�i�^�U�l�i�_�~�[�j�j
bool PACKET::process(void){	
	//if(getSimu()->getNow().dtime() >= 89.635987)
	//	cout << getSimu()->getNow().dtime() << "\t" << getNid() << " �p�P�b�g�v���Z�X " <<  type 
	//	<< "\t" << path.size() << "\t" << size << "\t" << seq << "\t" << this << "\t" << getSimu()->node[getNid()]->getBuffer()->getLength() << endl;
	//if(getSimu()->getNow().dtime() >= 89.635987){
	//	cout << "dtime--" << getSimu()->getNow().dtime() << "\ttype--" << type << "\tsize--" << size << endl; 
	//}
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	FRAME* fPtr = NULL;																				//�t���[���I�u�W�F�N�g�|�C���^
	NODE* nPtr = sPtr->node[here];																	//�m�[�h�I�u�W�F�N�g
	MAC* mPtr = nPtr->getMAC();																	//MAC�I�u�W�F�N�g
	BUFFER* bPtr = nPtr->getBuffer();															//�o�b�t�@�I�u�W�F�N�g
	//if(type == MrReq)
	//	cout << "test --- " << now.dtime() << "\t" << getSource() << "\t" << getDest() << endl;
	if(!nPtr->getIsActive())																		//�m�[�h��������~��ԂȂ�
		cout << "off node sends packet!!!" << endl, exit(1);									//�G���[
	if(mPtr->getFrame() != NULL)																	//������MAC�����Ώۃt���[�������݂���Ȃ�
		return true;																						//���������ɏI��
	switch(type){																						//�p�P�b�g�^�C�v�ɂ��ꍇ����
	case Routing: case RreqDsr: case RreqAodv: case LabRa: case MapReqB:	case MigReq:
	case DummyBroadcast:																				//�u���[�h�L���X�g�p�P�b�g�̏ꍇ
		fPtr = new FRAME(sPtr, now, FRAME::Broad, here, -1, seq, this);					//�u���[�h�L���X�g�t���[�����쐬
		break;

	default:																								//���̑��̏ꍇ
		if(type == Tcp && source == here){															//TCP�p�P�b�g�����g�����M���̏ꍇ
			SEGMENT* segPtr = getSeg();																	//�Ή��Z�O�����g�I�u�W�F�N�g
			TCP* tPtr = segPtr->getTcp();																	//�Ή�TCP�I�u�W�F�N�g
			tPtr->resetBuffer();																				//�o�b�t�@�����O�t���O�����Z�b�g
			if(tPtr->window[0] == NULL || segPtr->getSeq() < tPtr->window[0]->getSeq()){	//����ACK��M�ς݂ő���K�v�̂Ȃ��p�P�b�g�̏ꍇ
				bPtr->decreLength(size);																		//�s�񒷂��p�P�b�g���������炷
				bPtr->queue.erase(bPtr->queue.begin());													//�o�b�t�@�̐擪�p�P�b�g���폜
				tPtr->makePacket();																				//���̃p�P�b�g�쐬
				return false;																						//�I�u�W�F�N�g��j�����ďI��
			}
			segPtr->setSendStart(now);																		//�Z�O�����g�̑��M���Ԃ����ݎ����ɂ���
			tPtr->setLastSendSeq(segPtr->getSeq());
			if(segPtr->getTimeout() == NULL){															//�Z�O�����g�̃^�C���A�E�g���쐬����Ă��Ȃ��ꍇ
				TIMEOUT* ptr = new TIMEOUT(sPtr, now + tPtr->getRtt() + int(tPtr->getD() * 4), 
																	TIMEOUT::Segment, here);					//�^�C���A�E�g�I�u�W�F�N�g�̍쐬	
				segPtr->setTimeout(ptr);																		//�Z�O�����g�Ƀ^�C���A�E�g��o�^
				ptr->setSegment(segPtr);																		//�^�C���A�E�g�ɑΉ��Z�O�����g��o�^
				sPtr->list.insert(ptr);																			//�^�C���A�E�g���C�x���g�ɒǉ�
			}
			else{																									//�Z�O�����g�̃^�C���A�E�g�����ɍ쐬����Ă���ꍇ
				segPtr->getTimeout()->setTime(now + tPtr->getRtt() + int(tPtr->getD() * 4));	//�^�C���A�E�g�̌���
				sPtr->list.replace(segPtr->getTimeout());													//�C�x���g�����X�g���ōĔz�u
			}
		}
		if(nPtr->getRoute() == NODE::DSR){															//���[�e�B���O��DSR�̏ꍇ
			if(path.size() == 0){
				cout << this << "\t" << type << "\t" << here << "\t" << now.dtime() << endl;
				cout << "DSR route error" << endl, exit(1); 
			}
			for(short i = 0; i < (short)path.size(); i++){											//�o�H���̌���
				if(path[i] == here){																				//�o�H���Ɏ��g������������
					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);//�o�H�����t���[�����쐬
					break;
				}
			}
		}
		else if(nPtr->getRoute() == NODE::GEDIR ){													//���[�e�B���O��GEDIR�̏ꍇ
			short next = -1;																					//�]���m�[�hID�i�����l-1�j
			double minDist = AREA * 2;																		//���Đ�܂ł̍ŏ������i�����l�̓G���A��2�{�j
			short i = 0;																						//�J�E���g�p�ϐ�
			vector<short> candidate;																		//�]���m�[�h���o�^�p�x�N�^
			while(nPtr->neighborList[i].getDist() < RANGE)											//���g�̗אڃm�[�h���`�F�b�N
			{
				short checkId = nPtr->neighborList[i].getId();											//�`�F�b�N�Ώۃm�[�hID
				if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){	//�Ώۂ�����Ȃ�
					next = checkId;																					//�K���]����ɂ���
					break;																								//�`�F�b�N�I��
				}
				bool flag = true;																					//�Ώۂ��]���o�H�Ɋ܂܂�Ă邩�������t���O
				for(char j = 0; j < (char)path.size(); j++){												//�p�P�b�g�̓]���o�H���`�F�b�N
					if(path[j] == checkId){																			//�Ώۂ��o�H�Ɋ܂܂�Ă�����
						flag = false;																						//�t���O������
						break;																								//�`�F�b�N�I��
					}
				}
				if(flag == true){																					//�Ώۂ��o�H�Ɋ܂܂�Ă��Ȃ��Ȃ�
					candidate.push_back(checkId);																	//���x�N�^�֓o�^
					if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){//���Đ�܂ł̋������ŒZ�Ȃ�
						minDist = dist(dPos, nPtr->nodePos[checkId]);											//�ŒZ�������X�V
						next = checkId;																					//�]�����Ώۃm�[�h�ɂ���
					}
				}
				i++;																									//���̃`�F�b�N�Ώۂ�
			}
			if(next == dest || candidate.size() > 0){													//�]���悪�m�肵�Ă��邩���x�N�^�ւ̓o�^������Ȃ�
				if(next == -1)																						//�]���悪�m�肵�Ă��Ȃ��ꍇ
					next = candidate[genrand() % (int)candidate.size()];									//���x�N�^���烉���_���ɓ]���������
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//�t���[�����쐬
//				cout << now.dtime() << "  make frame in gedir "<< here << "\t" << next  << endl;
			}
			else{																									//�]���悪�m�肹���������݂��Ȃ��ꍇ
				bPtr->decreLength(size);																		//�s�񒷂��p�P�b�g���������炷
				bPtr->queue.erase(bPtr->queue.begin());													//�o�b�t�@�̐擪�p�P�b�g���폜
//				cout << "no next " << endl;
				return false;																						//�I�u�W�F�N�g��j�����ďI��
			}
		}
		//************************************************//
		// MA�}���`�p�X���[�e�B���O�̎�
		// �o�H��񂪂Ȃ��ꍇ��GEDIR�Ɠ�������������
		// ����ȊO��������DSR�Ɠ�����������
		//************************************************//
		else if( nPtr->getRoute() == NODE::MAMR ){													//MAMR�̏ꍇ
			short next = -1;																		//�]���m�[�hID�i�����l-1�j
			double minDist = AREA * 2;																//���Đ�܂ł̍ŏ������i�����l�̓G���A��2�{�j
			short i = 0;																			//�J�E���g�p�ϐ�
			vector<short> candidate;																//�]���m�[�h���o�^�p�x�N�^
			//if(type  == 14 )
			//	cout << "test -1 --- " << now.dtime() << endl;
			bool flag = false;
			if(path.size() >= 2 && type != MrReq){																	//path������ꍇ
				for(i = 0; i < (short)path.size() - 1; i++){										//�o�H���̌���
					if(path[i] == here){																//�o�H���Ɏ��g������������
						if(nPtr->routing[path[i + 1]]->getHop() == 1 /*|| destPos.getT().dtime()  + 0.1 <  now.dtime()*/)	//���̃m�[�h���אڒ[���Ȃ�
							flag = true;
						break;
					}
				}
			}
			if(flag == true){
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);	//�o�H�����t���[�����쐬
			}
			else{
				i = 0;
				while(nPtr->neighborList[i].getDist() < RANGE){										//���g�̗אڃm�[�h���`�F�b�N
					static int cnt;
 					//if(type == MrReq)
					//	cout << "test 6 " << now.dtime() << endl;
					short checkId = nPtr->neighborList[i].getId();									//�`�F�b�N�Ώۃm�[�hID
					if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //�Ώۃm�[�h������Ȃ�
						next = checkId;																//���ɓ]������m�[�h��Ώۃm�[�h�Ƃ���
						break;
					}
					bool flag = true;																//�Ώۂ��]���o�H�Ɋ܂܂�Ă��邩�������t���O
					for(char j = 0; j < (char)mamrPath.size(); j++){									//�]���o�H�����J��Ԃ�
						if(mamrPath[j] == checkId){														//�]���o�H�ɑΏۂ��܂܂�Ă���ꍇ
							flag = false;															//�t���O�����낷
							break;
						}
					}
					if(flag == true){																//�Ώۂ��o�H�Ɋ܂܂�Ă��Ȃ��ꍇ
						candidate.push_back(checkId);												//���x�N�^�֓o�^
						if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //����܂ł̌o�H���ŒZ�Ȃ�
							minDist = dist(dPos, nPtr->nodePos[checkId]);							//�ŒZ�o�H���X�V
							next = checkId;
						}
					}
					i++;
				}
				//if(type == MrReq)
				//	cout << "test 7 " << now.dtime() << endl; 
			
				if(next != -1 || candidate.size() > 0){											//�]���悪�m�肵�Ă��邩�@���x�N�^�ւ̓o�^������Ȃ��
					if(next == -1)
						next = candidate[genrand() % (int)candidate.size()];						//��₩�烉���_���ɓ]�����I��
					//if(type == MrReq)
					//	cout << "test 8 " << now.dtime() << endl;
					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//�t���[�����쐬
				}
				else{																				//�]������m�肹���@�������݂��Ȃ��ꍇ
					bPtr->decreLength(size);														//�s�񒷂��p�P�b�g���������炷
					bPtr->queue.erase(bPtr->queue.begin());											//�o�b�t�@�̐擪�p�P�b�g���폜
					if(type == Tcp || type == Udp)
						nPtr->sendError(this);													//���[�g�G���[���M����
					return false;
				}
			}




			//	//cout << "path ����" << path.size() << endl;
			//	for(short i = 0; i < (short)path.size(); i++){										//�o�H���̌���
			//		if(path[i] == here){															//�o�H���Ɏ��g������������
			//			if(nPtr->routing[path[i + 1]]->getHop() == 1){
			//				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);	//�o�H�����t���[�����쐬
			//				break;
			//			}
			//			else{
			//				while(nPtr->neighborList[i].getDist() < RANGE){										//���g�̗אڃm�[�h���`�F�b�N
			//					//if(type == MrReq)
			//					//	cout << "test 6 " << now.dtime() << endl;
			//					short checkId = nPtr->neighborList[i].getId();									//�`�F�b�N�Ώۃm�[�hID
			//					if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //�Ώۃm�[�h������Ȃ�
			//						next = checkId;																//���ɓ]������m�[�h��Ώۃm�[�h�Ƃ���
			//						break;
			//					}
			//					bool flag = true;																//�Ώۂ��]���o�H�Ɋ܂܂�Ă��邩�������t���O
			//					for(char j = 0; j < (char)path.size(); j++){									//�]���o�H�����J��Ԃ�
			//						if(path[j] == checkId){														//�]���o�H�ɑΏۂ��܂܂�Ă���ꍇ
			//							flag = false;															//�t���O�����낷
			//							break;
			//						}
			//					}
			//					if(flag == true){																//�Ώۂ��o�H�Ɋ܂܂�Ă��Ȃ��ꍇ
			//						candidate.push_back(checkId);												//���x�N�^�֓o�^
			//						if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //����܂ł̌o�H���ŒZ�Ȃ�
			//							minDist = dist(dPos, nPtr->nodePos[checkId]);							//�ŒZ�o�H���X�V
			//							next = checkId;
			//						}
			//					}
			//					i++;
			//				}
			//				//if(type == MrReq)
			//				//	cout << "test 7 " << now.dtime() << endl; 
			//
			//				if(next == dest || candidate.size() > 0){											//�]���悪�m�肵�Ă��邩�@���x�N�^�ւ̓o�^������Ȃ��
			//					if(next == -1)
			//						next = candidate[genrand() % (int)candidate.size()];						//��₩�烉���_���ɓ]�����I��
			//					//if(type == MrReq)
			//					//	cout << "test 8 " << now.dtime() << endl;
			//					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//�t���[�����쐬
			//		
			//				}
			//				else{																				//�]������m�肹���@�������݂��Ȃ��ꍇ
			//					bPtr->decreLength(size);														//�s�񒷂��p�P�b�g���������炷
			//					bPtr->queue.erase(bPtr->queue.begin());											//�o�b�t�@�̐擪�p�P�b�g���폜
			//					return false;
			//				}
			//			}
			//		}
			//	}
			//}else{																					//path���Ȃ��ꍇ�@�ȉ�GEDIR�Ɠ����������g��
			//	while(nPtr->neighborList[i].getDist() < RANGE){										//���g�̗אڃm�[�h���`�F�b�N
			//		//if(type == MrReq)
			//		//	cout << "test 6 " << now.dtime() << endl;
			//		short checkId = nPtr->neighborList[i].getId();									//�`�F�b�N�Ώۃm�[�hID
			//		if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //�Ώۃm�[�h������Ȃ�
			//			next = checkId;																//���ɓ]������m�[�h��Ώۃm�[�h�Ƃ���
			//			break;
			//		}
			//		bool flag = true;																//�Ώۂ��]���o�H�Ɋ܂܂�Ă��邩�������t���O
			//		for(char j = 0; j < (char)path.size(); j++){									//�]���o�H�����J��Ԃ�
			//			if(path[j] == checkId){														//�]���o�H�ɑΏۂ��܂܂�Ă���ꍇ
			//				flag = false;															//�t���O�����낷
			//				break;
			//			}
			//		}
			//		if(flag == true){																//�Ώۂ��o�H�Ɋ܂܂�Ă��Ȃ��ꍇ
			//			candidate.push_back(checkId);												//���x�N�^�֓o�^
			//			if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //����܂ł̌o�H���ŒZ�Ȃ�
			//				minDist = dist(dPos, nPtr->nodePos[checkId]);							//�ŒZ�o�H���X�V
			//				next = checkId;
			//			}
			//		}
			//		i++;
			//	}
			//	//if(type == MrReq)
			//	//	cout << "test 7 " << now.dtime() << endl; 
			//
			//	if(next == dest || candidate.size() > 0){											//�]���悪�m�肵�Ă��邩�@���x�N�^�ւ̓o�^������Ȃ��
			//		if(next == -1)
			//			next = candidate[genrand() % (int)candidate.size()];						//��₩�烉���_���ɓ]�����I��
			//		//if(type == MrReq)
			//		//	cout << "test 8 " << now.dtime() << endl;
			//		fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//�t���[�����쐬
			//		
			//	}
			//	else{																				//�]������m�肹���@�������݂��Ȃ��ꍇ
			//		bPtr->decreLength(size);														//�s�񒷂��p�P�b�g���������炷
			//		bPtr->queue.erase(bPtr->queue.begin());											//�o�b�t�@�̐擪�p�P�b�g���폜
			//		return false;
			//	}
			//}
			//else{																					//MA�ȊO�̏ꍇ
			//	if(path.size() <= 1){																//path���Ȃ��ꍇ
			//		if(dPos.getX() >= 0){
			//			while(nPtr->neighborList[i].getDist() < RANGE){										//���g�̗אڃm�[�h���`�F�b�N
			//				short checkId = nPtr->neighborList[i].getId();									//�`�F�b�N�Ώۃm�[�hID
			//				if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //�Ώۃm�[�h������Ȃ�
			//					next = checkId;																//���ɓ]������m�[�h��Ώۃm�[�h�Ƃ���
			//					break;
			//				}
			//				i++;
			//			}
			//			if(next == dest)
			//				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//�t���[�����쐬
			//			else
			//				cout << "next error at PACKET process" << endl,exit(1);
			//		}
			//		else
			//			cout << "dPos error at PACKET process" << endl, exit(1);
			//	}
			//	else{																				//path������ꍇ
			//		cout << "test --- " << now.dtime() << endl;
			//	}
			//}
			//if(path.size() == 0){																	//�o�H��񂪖��ߍ��܂�Ă��Ȃ��ꍇ
			//	//// ���O�\��
			//	//cout << "MAMR�o�H���Ȃ�" << endl;
			//	//cout << this << "\t" << type << "\t" << here << "\t" << now.dtime() << endl;
			//	short next = -1;																	//�]���m�[�hID�i�����l-1�j
			//	double minDist = AREA * 2;															//���Đ�܂ł̍ŏ������i�����l�̓G���A��2�{�j
			//	short i = 0;																		//�J�E���g�p�ϐ�
			//	vector<short> candidate;															//�]���m�[�h���o�^�p�x�N�^
			//else{																						//�o�H����ێ����Ă���ꍇ
			//	}
			//}
		}
		//*************************************************************//
		// �}���`�p�X�����ȏ�
		//*************************************************************//

		else{																									//���̑��̃��[�e�B���O�̏ꍇ
			//if(source == 50 && dest == 76)
			//cout << here << "->" << dest << "\t" << nPtr->routing[dest]->getNext() << "\t" << (int)nPtr->routing[dest]->getHop() << endl;
			if(nPtr->routing[dest]->getNext() != -1)											//�]���悪���݂���Ȃ�
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, 
								nPtr->routing[dest]->getNext(), nPtr->getSeq(), this);		//���[�e�B���O�e�[�u�����t���[�����쐬
			else																							//�]���悪���݂��Ȃ��Ȃ�
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, dest, nPtr->getSeq(), this);//���Đ���t���[�����쐬
		}
		break;
	}
	nPtr->increSeq();
	bPtr->decreLength(size);																		//�s�񒷂��p�P�b�g���������炷
	bPtr->queue.erase(bPtr->queue.begin());													//�o�b�t�@�̐擪�p�P�b�g���폜
	//if(here == 248)
	//	nPtr->queueShow();
	if(fPtr != NULL){																					//�t���[�����쐬���ꂽ�ꍇ
		sPtr->increPacket(type, size);
		sPtr->list.insert(fPtr);																		//�t���[���������C�x���g���X�g�֒ǉ�
		return true;
//�������������������O�\������������������
//�m�[�h���x���̃p�P�b�g���M���O���s�K�v�Ȃ�R�����g�A�E�g����	
	//if(type == Udp || type == Tcp || type == Ack){
	//	cout << now.dtime() << "\ts\t" << here << "\t";
	//	switch(type){																					//�p�P�b�g�^�C�v����уV�[�P���X�ԍ�
	//		case PACKET::Udp:
	//			cout << "UDP\t" << getUdp() << "\t" << getSeq() << "\t--------\t--------" << endl;
	//			break;
	//		case PACKET::Tcp:
	//			cout << "TCP\t" << getSeg()->getTcp() << "\t" << getSeq() << "\t--------\t--------" << endl;
	//			break;
	//		case PACKET::Ack:
	//			cout << "ACK\t" << getSeg()->getTcp() << "\t" << getSeq() << "\t--------\t--------" << endl;
	//			break;
	//	}
	//}
//����������������������������������������
	}
	else{																									//�t���[���쐬����Ȃ������Ȃ�
//		cout << "packet process error " << endl, exit(1);
		mPtr->setTime(now);																				//MAC�̃C�x���g���������݂ɐݒ�
		nPtr->checkPacket();																				//�o�b�t�@���̃p�P�b�g�`�F�b�N
	}
	return true;
}


//�o�b�t�@�ւ̃p�P�b�g�}��
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�|�C���^�j
//�߂�l�i�Ȃ��j
bool PACKET::queue(SIMU* ptr, bool front){
	SIMUTIME now = ptr->getNow();																	//���ݎ���
	//if(type != Tcp && type != Ack)
	//cout << now.dtime() << "\t" << here << " �p�P�b�g�L���[ " << endl;
	NODE* nPtr = ptr->node[here];																	//�m�[�h�I�u�W�F�N�g
	setNid(here);																						//�p�P�b�g�C�x���g�����m�[�h�����g�ɕύX
	bool flag = true;																					//�o�b�t�@�}���������������������t���O�i�����l�͐����j
	if(ttl == 0){																						//TTL��0�i�z�b�v���̏���I�[�o�j�̏ꍇ
		flag = false;																					//�}�������t���O�����낷
//		cout << now.dtime() << "\tHD\t" << here << "\t";					//���O�\��
//		switch(getType()){																			//�p�P�b�g�^�C�v����уV�[�P���X�ԍ�
//			case PACKET::Udp:
//				cout << "UDP\t" << getUdp() << "\t" << getSeq() << "\t--------\t--------" << endl;
//				break;
//			case PACKET::Tcp:
//				cout << "TCP\t" << getSeg()->getTcp() << "\t" << getSeq() << "\t--------\t--------" << endl;
//				break;
//			case PACKET::Ack:
//				cout << "ACK\t" << getSeg()->getTcp() << "\t" << getAckSeq() << "\t--------\t--------" << endl;
//				break;
//			default:
//				cout << "OTHER\t" << "--------" << "\t" << "------" << "\t--------\t--------" << endl;
//				break;
//		}
	}
	else{																								//TTL��0�łȂ��ꍇ
		if(nPtr->getRoute() == NODE::GEDIR || nPtr->getRoute() == NODE::MAMR){														//GEDIR�̏ꍇ
			mamrPath.push_back(here);																		//�o�H���Ƃ��Ď��g��}������
			size += 4;
		}
		ttl--;																							//TTL�̃f�N�������g
		hop++;																							//�z�b�v���̃C���N�������g
		BUFFER* bPtr = nPtr->getBuffer();														//�o�b�t�@�I�u�W�F�N�g�|�C���^
		if(front){																						//�擪�}���t���O�������Ă�����
			bPtr->queue.insert(bPtr->queue.begin(), this);										//�o�b�t�@�̐擪�ɒǉ�
			bPtr->increLength(size);
			if(bPtr->getLength() > bPtr->getSize()){
//				cout << getNid() << "  bPtrGetSize " << bPtr->getSize() << endl;
				bPtr->decreLength(bPtr->queue[bPtr->queue.size() - 1]->getSize());
				delete bPtr->queue[bPtr->queue.size() - 1];
				bPtr->queue.pop_back();
			}
		}
		else{																								//�擪�}���t���O�������Ă��Ȃ��Ȃ�
			if(bPtr->getLength() + size < bPtr->getSize()){										//�p�P�b�g��}�����Ă��s�񒷂��T�C�Y��菬�����Ȃ�
				bPtr->queue.push_back(this);																//�s��̍Ō�Ƀp�P�b�g��ǉ�
				bPtr->increLength(size);																	//�s�񒷂��p�P�b�g�T�C�Y�������₷
			}
			else{																									//�p�P�b�g�}���̗]�T���Ȃ��ꍇ
				nPtr->increOverflow();																			//�o�b�t�@�I�[�o�t���[�J�E���^���C���N�������g
				flag = false;																						//�}�������t���O�����낷
				//static int cnt;
				//cout << now.dtime() << "  overflow at  " << getNid() << "\t" << seq << "\t" << type << endl;
//				nPtr->queueShow();
//				cout << now.dtime() << "\tOD\t" << getHere() << "\t";
//			switch(type){																							//�p�P�b�g�^�C�v����уV�[�P���X�ԍ�
//				case PACKET::Udp:
//					cout << "UDP\t" << getUdp() << "\t" << getSeq() << "\t--------\t--------" << endl;
//					break;
//				case PACKET::Tcp:
//					cout << "TCP\t" << getSeg()->getTcp() << "\t" << getSeq() << "\t--------\t--------" << endl;
//					break;
//				case PACKET::Ack:
//					cout << "ACK\t" << getSeg()->getTcp() << "\t" << getAckSeq() << "\t--------\t--------" << endl;
//					break;
//				default:
//					cout << "OTHER\t" << "--------" << "\t" << "------" << "\t--------\t--------" << endl;
//					break;
//			}
			}
		}
	}
	//if(here == 248)
	//	nPtr->queueShow();
	MAC* mPtr = nPtr->getMAC();
	//if(type == RrepDsr)
	//	mPtr->show(0);
	if(front == false && mPtr->checkIdling() && (mPtr->getState() == MAC::Idle || mPtr->getState() == MAC::NavFin)){								//�o�b�t�@����ւ̑}���ł��m�[�h��MAC��Ԃ�Idle�Ȃ�
		mPtr->setTime(now);																		//MAC�̃C�x���g���������݂ɐݒ�
		nPtr->checkPacket();																		//�o�b�t�@���̃p�P�b�g�`�F�b�N
	}
	return flag;																					//�}�������t���O��Ԃ�
}


//���O�\���֐�
//�����i�p�P�b�g�I�u�W�F�N�g�j
//�߂�l�F�i�Ȃ��j
void PACKET::showLog(short nid, char* id, SIMUTIME now){
	cout << now.dtime() << "\t";																//���ݎ���
	cout << id << "\t";																			//��ԁi��M�C���M�Ȃǁj
	cout << nid << "\t";																			//�m�[�h�ԍ�
	switch(type){																					//�p�P�b�g�^�C�v����уV�[�P���X�ԍ�
		case PACKET::Udp:
			cout << "UDP\t" << typePtr.udpPtr << "\t" << seq << "\t";
			break;
		case PACKET::Tcp:
			cout << "TCP\t" << typePtr.segPtr->getTcp() << "\t" << seq << "\t";
			break;
		case PACKET::Ack:
			cout << "ACK\t" << typePtr.segPtr->getTcp() << "\t" << seq << "(" << requestSeq << ")" << "\t";
			break;
	}
	cout << "--------" << "\t";
	if(dest == nid && type == PACKET::Tcp)													//��M�o�C�g���i����m�[�h�̂ݕ\���j
		cout << typePtr.segPtr->getTcp()->getObject()->getByte() << endl;
	else if(dest == nid && type == PACKET::Udp)
		cout << typePtr.udpPtr->getObject()->getByte() << endl;
	else
		cout << "--------" << endl;
}

//�o�H�\��
//�����i�Ȃ��j
//�߂�l�i�Ȃ��j
void PACKET::showPath(void){
	for(char i = 0; i < (char)path.size() - 1; i++)
		cout << path[i] << "->";
	if(path.size() > 1)
		cout << path[path.size() - 1] << endl;
}

//�t���[���I�u�W�F�N�g�̃R���X�g���N�^
FRAME::FRAME(SIMU* ptr, SIMUTIME tval, castType cid, short sid, short did, int seqid, PACKET* pPtr):EVENT(ptr, tval, EVENT::Frame, sid){ 
	//cout << "create frame " << this << endl;
	cast = cid;																						//�L���X�g�^�C�v
	source = sid;																					//�]�����m�[�h
	dest = did;																						//�]����m�[�h
	seq = seqid;																					//�V�[�P���X�ԍ�
	packetPtr = pPtr;																				//�Ή��p�P�b�g�I�u�W�F�N�g
	size = 16 + (pPtr->getSize() + 36) * 8 + 6;											//�t���[���T�C�Y
	//cout << "frame " << cnt++ << endl;
	cntF++;
}

FRAME::~FRAME(){
	//cout << "delete frame " << this << endl;
	//cout << "frame " << cnt-- << endl;
	cntF--;
}

//�t���[������
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�F�i�^�U�l�i�_�~�[�j�j
bool FRAME::process(void){
//	cout << getNid() << " �t���[���v���Z�X" << size << "\t" << dest << "\t" << packetPtr->getSeq() << endl;
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();															//���ݎ���
	NODE* nPtr = sPtr->node[getNid()];														//�t���[�������m�[�h�I�u�W�F�N�g
	MAC* mPtr = nPtr->getMAC();																//�t���[�������m�[�h��MAC�I�u�W�F�N�g�|�C���^
	mPtr->setFrame(this);																		//MAC�I�u�W�F�N�g�ɂ��̃t���[���I�u�W�F�N�g��o�^
	mPtr->calcDataTime(this);																	//�f�[�^���M���Ԃ̐ݒ�
	if(nPtr->getChannel()->signalList.size() == 0 && mPtr->getState() != MAC::Nav){	//��M�M�����Ȃ�NAV�ł��Ȃ��Ȃ�
		mPtr->setState(MAC::Difs);																	//��Ԃ�Difs�ҋ@�ɐݒ�
		mPtr->setTime(now + DIFSTIME);															//DIFS���Ԃ����ҋ@
		sPtr->list.insert(mPtr);																	//�C�x���g���X�g�֓o�^
		mPtr->setListInsert(true);																	//���X�g�o�^�t���O�𗧂Ă�
	}
	if(packetPtr->getType() == PACKET::Tcp && packetPtr->getSource() == packetPtr->getHere())
		packetPtr->getSeg()->getTcp()->makePacket();
	return true;																						//�֐��𔲂��Ă��I�u�W�F�N�g�͏������Ȃ�
}

//�r�[�R������
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�i�I�u�W�F�N�g�j���̐^�U�l�j
bool BEACON::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	NODE* nPtr = sPtr->node[getNid()];
	PACKET* packetPtr																					//Routing�p�p�P�b�g�̍쐬
		= new PACKET(sPtr, now, PACKET::Routing, UDPSIZE, getNid(), getNid(), -1, 1);
	packetPtr->setSpos(sPtr->node[getNid()]->getDerivePos(0));
	if(!packetPtr->queue(sPtr, false))															//�o�b�t�@�֑}��
		delete packetPtr;																					//�}���Ɏ��s���������
	addTime(genrand() % (BEACONINT / 100) + BEACONINT - BEACONINT / 200);//���̃r�[�R�����������ݒ�
	sPtr->list.insert(this);																				//�r�[�R�������������C�x���g���X�g�֒ǉ�
	return true;
}
