#include "class.h"
#include "mobileAgent.h"

extern double rand_d();																				//[0,1)�̎�����������

//��M����
//�����i���X�g�I�u�W�F�N�g�C�m�[�h�I�u�W�F�N�g�j
//�߂�l�i�^�U�l�i�_�~�[�j�j
bool RECEIVE::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	//if(now.dtime() > 9.000106)
	//	cout << "type=" << getType() << endl;
	short id = getNid();																			//��M�m�[�hID
	short sid = pPtr->getSource();																//���M���m�[�hID
	short did = pPtr->getDest();																	//���M����m�[�hID
	PACKET::ptype tid = pPtr->getType();
	NODE* nPtr = sPtr->node[id];																//�m�[�h�I�u�W�F�N�g
	PACKET* newPtr;																				//�V�K�쐬�p�P�b�g
	MAC* mPtr = nPtr->getMAC();																//�����Ώ�MAC�I�u�W�F�N�g
	if(pPtr->getSpos().getX() >= 0){
		if(timeCompare(pPtr->getSpos().getT(), nPtr->nodePos[sid].getT()))
			nPtr->nodePos[sid] = pPtr->getSpos();
	}
	bool duplicateFlag = false;																//�d���p�P�b�g��M�`�F�b�N�t���O
	for(short i = 0; i < (short)nPtr->receivedPacketList.size(); i++){			//��M�p�P�b�g���X�g���`�F�b�N
		NODE::RECEIVED_PACKET packet = nPtr->receivedPacketList[i];						//���X�g���̎�M�p�P�b�g�I�u�W�F�N�g
		if(packet.getType() == tid && packet.getSource() == sid 
			&& packet.getDest() == did && packet.getSeq() == pPtr->getSeq()
			&& packet.getTime().dtime() == pPtr->getSendStart().dtime()){			//���X�g���̃I�u�W�F�N�g�Ɠ���̃p�P�b�g�Ȃ�
				duplicateFlag = true;																	//�d����M�t���O�𗧂Ă�
				break;																						//�`�F�b�N�͏I��
		}
	}
	if(duplicateFlag){																			//�d����M�t���O�������Ă�����
//		static int cnt;
//		cout << now.dtime() << "\t" << pPtr->getType() << " from " << pPtr->getSource()  << " receive duplicate packet " << ++cnt <<endl;
		return false;																					//��M�����͂����ɏI��
	}	
	else{																								//�d����M�t���O�������Ă��Ȃ����
		NODE::RECEIVED_PACKET rPacket 
			= NODE::RECEIVED_PACKET(tid, sid, did, pPtr->getSeq(), pPtr->getSendStart());						//��M�p�P�b�g�I�u�W�F�N�g
		nPtr->receivedPacketList.push_back(rPacket);											//��M�p�P�b�g���X�g�ɒǉ�
		nPtr->receivedPacketList.erase(nPtr->receivedPacketList.begin());				//�ł��Â��d����M�p�P�b�g��������
	}
	switch(tid){																					//��M�p�P�b�g�^�C�v�ɂ��ꍇ����
		case PACKET::Routing:																	//���[�e�B���O�p�P�b�g�̏ꍇ
			if(sPtr->getArea() == SIMU::MESH && id >= sPtr->getMAP())					//��M�m�[�h�����b�V��STA�Ȃ�
				break;																						//���������ɏI��
			if(nPtr->getRoute() == NODE::GEDIR)												//GEDIR���[�e�B���O�Ȃ�
				break;
			if(nPtr->routing[sid]->getHop() == 1)												//���[�e�B���O�f�[�^�̑��M�����אڒ[���Ȃ� 
				nPtr->makeRoutingTable(sid);															//���[�e�B���O�e�[�u���쐬
			break;
		case PACKET::RreqDsr:																	//���[�g���N�G�X�g�iDSR�j�̏ꍇ
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid)						//���Ɏ�M�����p�P�b�g�̏ꍇ
				break;																						//��M�����������ɔj��
			nPtr->floodSeq[sid] = pPtr->getSeq();												//��M�t���b�f�B���O�V�[�P���X�̍X�V
			if(id == did){																				//�������g������Ȃ�
				//cout << now.dtime() << " ���[�g���N�G�X�g��M " << id << endl;
				newPtr = new PACKET(sPtr, now, PACKET::RrepDsr, RREPDSR, id, id, sid, -1);//���[�g���v���C�p�P�b�g�쐬
				newPtr->setSendStart(pPtr->getSendStart());										//�p�P�b�g���M������v���p�P�b�g�̎��ԂɕύX
				nPtr->path[sid].clear();																//�ߋ��̌o�H�����폜
				nPtr->path[sid].push_back(id);														//�m�[�h�̌o�H���̏�����
				newPtr->path.push_back(id);															//���v���C�p�P�b�g�̌o�H��񏉊���
				for(short i = (short)pPtr->path.size() - 1; i >= 0; i--){
					//cout << pPtr->path[i] << "->";
					nPtr->path[sid].push_back(pPtr->path[i]);											//�m�[�h�̌o�H���̏�������	
					newPtr->path.push_back(pPtr->path[i]);												//���v���C�p�P�b�g�̌o�H���̏�������
				}
				//cout << endl;
				newPtr->increSize(pPtr->path.size() * 4);											//�o�H��񕪂����p�P�b�g�T�C�Y�𑝉�
				newPtr->setSeq(nPtr->getSeq());														//�V�[�P���X�ԍ��̕t�^
				nPtr->increSeq();																			//�V�[�P���X�ԍ��̃C���N�������g
				if(!newPtr->queue(sPtr, false))														//�p�P�b�g���o�b�t�@�֑}��
					delete newPtr;																				//�}���Ɏ��s������폜
			}
			else{																							//�������g������łȂ��ꍇ
				//cout << now.dtime() << " ���[�g���N�G�X�g���p " << id << endl;
				nPtr->receivedPacketList.pop_back();												//���p�����ōēx�d����M���������邽�߃��X�g����폜
				newPtr = nPtr->relayPacket(pPtr, 4);												//�p�P�b�g���p����
				if(newPtr)																					//���p���������������ꍇ
					newPtr->path.push_back(id);															//���g���o�H���ɒǉ�
			}
			break;
		case PACKET::RrepDsr:																	//���[�g���v���C�iDSR�j�̏ꍇ
			//cout << now.dtime() << " ���[�g���v���C��M " << pPtr << "\t" << id << "\t" << sid << "\t" << pPtr->getSeq() << "\t" << pPtr->path.size() << endl;
			nPtr->requestTime[pPtr->getSource()] = SIMUTIME(-10,0);					//���[�g���N�G�X�g�^�C���������̏�����
			nPtr->path[pPtr->getSource()].clear();											//�Â��o�H������U�폜
			for(char i = (char)pPtr->path.size() - 1; i >= 0; i--)
				nPtr->path[pPtr->getSource()].push_back(pPtr->path[i]);
			break;
		case PACKET::RerrDsr:																	//���[�g�G���[�iDSR�j�̏ꍇ
//			cout << now.dtime() << "\t" << getNid() << " ���[�g�G���[��M from " << pPtr->getSource() << endl;
			if(nPtr->getRoute() == NODE::MAMR){
				switch (pPtr->mpath_check)
				{
				case 0:
					nPtr->path[pPtr->getErrDest()].clear();											//�o�H��������
					nPtr->path[pPtr->getErrDest()].push_back(id);									//�o�H���̏�����
					break;
				case 1:
					nPtr->path1[pPtr->getErrDest()].clear();											//�o�H��������
					nPtr->path1[pPtr->getErrDest()].push_back(id);									//�o�H���̏�����
					break;
				case 2:
					nPtr->path2[pPtr->getErrDest()].clear();											//�o�H��������
					nPtr->path2[pPtr->getErrDest()].push_back(id);									//�o�H���̏�����
					break;
				}
			}
			else{
				nPtr->path[pPtr->getErrDest()].clear();											//�o�H��������
				nPtr->path[pPtr->getErrDest()].push_back(id);									//�o�H���̏�����
			}
			break;
		case PACKET::RreqAodv:																	//���[�g���N�G�X�g�iAODV�j�̏ꍇ
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid)						//���Ɏ�M�����p�P�b�g�̏ꍇ
				break;																						//��M�����������ɔj��
			nPtr->floodSeq[sid] = pPtr->getSeq();												//��M�t���b�f�B���O�V�[�P���X�̍X�V
			if(nPtr->routing[sid]->getSeq() >= pPtr->getAodvSeqS())						//���M���̏�񂪎��g�̃��[�e�B���O�e�[�u�����Â��ꍇ
				break;																						//��M�����������ɔj��
			//if(nPtr->routing[sid]->getSeq() == pPtr->getAodvSeqS() && nPtr->routing[sid]->getHop() >= pPtr->getHop())
			//	break;
			nPtr->routing[sid]->setNext(object);												//���[�e�B���O�e�[�u���̓]����̍X�V
			nPtr->routing[sid]->setHop(pPtr->getHop());										//���[�e�B���O�e�[�u���̃z�b�v���̍X�V
			nPtr->routing[sid]->setSeq(pPtr->getAodvSeqS());								//���[�e�B���O�e�[�u���̃V�[�P���X�ԍ��̍X�V
			if(id == did){																				//�������g������Ȃ�
				//cout << now.dtime() << " AODV���[�g���N�G�X�g��M " << id << " from " << sid << endl;
				newPtr = new PACKET(sPtr, now, PACKET::RrepAodv, RREPAODV, id, id, sid, -1);//���[�g���v���C�p�P�b�g�쐬
				newPtr->setReqDest(id);																	//�v������m�[�hID��ݒ�
				newPtr->setAodvSeqS(nPtr->getAodvSeq());											//���g��AODV�p�V�[�P���X��ݒ�
				newPtr->setSendStart(pPtr->getSendStart());										//�p�P�b�g���M������v���p�P�b�g�̎��ԂɕύX
				newPtr->setSeq(nPtr->getSeq());														//�V�[�P���X�ԍ��̕t�^
				nPtr->increSeq();																			//�V�[�P���X�ԍ��̃C���N�������g
				if(!newPtr->queue(sPtr, false))														//�p�P�b�g���o�b�t�@�֑}��
					delete newPtr;																				//�}���Ɏ��s������폜
			}
			else{																							//�������g������łȂ��ꍇ
//				if(id == 0){
				//cout << now.dtime() << "���p�m�[�hAODV���[�g���N�G�X�g��M " << id  << " from " << sid 
				//	<< "\t" << mPtr->getNid() << "\t" << object << endl;
				//cout << nPtr->routing[sid]->getNext() << endl;
//				}
				if(nPtr->routing[did]->getSeq() > pPtr->getAodvSeqD()){						//�v���p�P�b�g���V�������������Ă���ꍇ
					newPtr = new PACKET(sPtr, now, PACKET::RrepAodv, RREPAODV, id, id, sid, -1);//���[�g���v���C�p�P�b�g�쐬
					newPtr->setReqDest(did);																//�v������m�[�hID��ݒ�
					newPtr->setAodvSeqS(nPtr->routing[did]->getSeq());								//���g��AODV�p�V�[�P���X��ݒ�
					newPtr->setSendStart(pPtr->getSendStart());										//�p�P�b�g���M������v���p�P�b�g�̎��ԂɕύX
					newPtr->setSeq(nPtr->getSeq());														//�V�[�P���X�ԍ��̕t�^
					nPtr->increSeq();																			//�V�[�P���X�ԍ��̃C���N�������g
					if(!newPtr->queue(sPtr, false))														//�p�P�b�g���o�b�t�@�֑}��
						delete newPtr;																				//�}���Ɏ��s������폜
				}
				else{																							//�v���p�P�b�g���V�������������Ă��Ȃ��ꍇ
					//cout << now.dtime() << " ���[�g���N�G�X�g���p " << id << endl;
					nPtr->receivedPacketList.pop_back();												//���p�����ōēx�d����M���������邽�߃��X�g����폜
					nPtr->relayPacket(pPtr, 0);															//�p�P�b�g���p����
				}
			}
			break;
		case PACKET::RrepAodv:																	//���[�g���v���C�iAODV�j�̏ꍇ
//			cout << id << "\t" << nPtr->routing[pPtr->getReqDest()]->getSeq() << "\t" << pPtr->getAodvSeqS() << endl;
			if(nPtr->routing[pPtr->getReqDest()]->getSeq() >= pPtr->getAodvSeqS())	//���M���̏�񂪎��g�̃��[�e�B���O�e�[�u�����Â��ꍇ
				break;																						//��M�����������ɔj��
			nPtr->routing[pPtr->getReqDest()]->setNext(object);
			nPtr->routing[sid]->setHop(pPtr->getHop());										//���[�e�B���O�e�[�u���̃z�b�v���̍X�V
			nPtr->routing[pPtr->getReqDest()]->setSeq(pPtr->getAodvSeqS());
			if(id != did){
//				cout << "���p�m�[�hAODV���[�g���v���C��M\t" << id << endl;
				nPtr->receivedPacketList.pop_back();												//���p�����ōēx�d����M���������邽�߃��X�g����폜
				nPtr->relayPacket(pPtr, 0);															//�p�P�b�g���p����
			}
			//else
			//	cout << "���M��AODV���[�g���v���C��M\t" << id << endl;
			break;
		case PACKET::RerrAodv:																	//���[�g���v���C�iAODV�j�̏ꍇ
			nPtr->routing[pPtr->getErrDest()]->setNext(-1);													//�]����̏�����											
			nPtr->routing[pPtr->getErrDest()]->setHop(-1);													//�z�b�v���̏�����
			if(id != did){
				nPtr->receivedPacketList.pop_back();												//���p�����ōēx�d����M���������邽�߃��X�g����폜
				nPtr->relayPacket(pPtr, 0);															//�p�P�b�g���p����
			}
			//else
			//	cout << "���M��AODV���[�g�G���[��M\t" << id << endl;
			break;
		case PACKET::Udp:																			//UDP�f�[�^�̏ꍇ
			pPtr->getUdp()->getObject()->increByte(pPtr->getSize() - 28);				//�f�[�^�O������M�o�C�g���C���N�������g
			sPtr->increTransDelay(now - pPtr->getSendStart());
			sPtr->increTransDelayCnt();
//			if(sid == 19 && id == 77)
////�������������������O�\������������������
////����m�[�h��UDP��M���O���s�K�v�Ȃ�R�����g�A�E�g����
//			pPtr->showLog(id, "R", now);
////����������������������������������������
			break;
		case PACKET::Tcp:																			//TCP�p�P�b�g�̏ꍇ
			pPtr->getSeg()->getTcp()->getObject()->sendAck(pPtr);							//ACK���M����
			break;
		case PACKET::Ack:																			//TCPACK�p�P�b�g�̏ꍇ
			pPtr->getSeg()->getTcp()->getTcpAck(pPtr);										//ACK��M����
			break;
		case PACKET::LabCenter: case PACKET::LabRa: case PACKET::LabNeighbor: 	//Lab�p�P�b�g�̏ꍇ
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid 
																  || id >= sPtr->getMAP())			//���Ɏ�M�����p�P�b�g��STA����M�����ꍇ
				break;																						//��M�����������ɔj��
			nPtr->floodSeq[sid] = pPtr->getSeq();												//��M�t���b�f�B���O�V�[�P���X�̍X�V
//			cout << now.dtime() << " LabRa��M " << id << "\t" << sid << endl;
			if(timeCompare(pPtr->getLAB().getTime(), nPtr->gab[pPtr->getSTA() - sPtr->getMAP()].getTime()))//�V����LAB���ł����
				nPtr->gab[pPtr->getSTA() - sPtr->getMAP()] = pPtr->getLAB();				//LAB�̍X�V
			if(pPtr->getType() == PACKET::LabRa)												//RA-OLSR�̏ꍇ
				nPtr->relayPacket(pPtr, 0);															//�p�P�b�g���p����
			break;
		case PACKET::StaReq:{																	//StaReq�p�P�b�g�̏ꍇ
			cout << now.dtime() << "\t" << id << " StaReq ��M" << endl;
			short sSta = pPtr->getSTA();															//�v��STA
			short dSta = pPtr->getReqDest();														//����STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//����STA�̐ڑ�MAP���
			if(sPtr->getMesh() == SIMU::IDEAL)
				dMap = sPtr->gab[dSta].getMap();
			cout << sSta << "\t" << dSta << "\t" << dMap << endl;
			if(dMap == id)																				//����STA�̐ڑ��悪�����Ȃ�
				newPtr = new PACKET(sPtr, now, PACKET::StaRep, REP, id, id, sSta, -1);//StaRep�p�P�b�g�I�u�W�F�N�g�̍쐬
			else{																							//����STA�̐ڑ��悪�����łȂ��Ȃ�
				if(sPtr->getMesh() == SIMU::NONE){													//��Ǘ������Ȃ�
					newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReq�p�P�b�g�I�u�W�F�N�g�̍쐬					
				}
				else if(sPtr->getMesh() == SIMU::CENTRAL)											//�W���Ǘ������Ȃ�												
					newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, sPtr->getCenter(), -1);	//MapReq�p�P�b�g�I�u�W�F�N�g�̍쐬
				else{																							//���̑��̕����Ȃ�
					if(dMap != -1)																				//�ڑ�MAP��񂪑��݂���Ȃ�
						newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, dMap, -1);	//MapReq�p�P�b�g�I�u�W�F�N�g�̍쐬
					else																							//�ڑ�MAP��񂪑��݂��Ȃ��Ȃ�
						newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReqB�p�P�b�g�I�u�W�F�N�g�̍쐬
				}
			}
			newPtr->setTimeout(pPtr->getTimeout());											//�v���^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
			newPtr->setSTA(sSta);																	//�v��STA�̓o�^
			newPtr->setReqSource(id);
			newPtr->setReqDest(dSta);																//����STA�̓o�^
			newPtr->setSeq(nPtr->getSeq());														//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			nPtr->increSeq();																			//�m�[�h�V�[�P���X�̃C���N�������g
			if(!newPtr->queue(sPtr, false))														//�o�b�t�@�ւ̃p�P�b�g�}��
				delete newPtr;																				//�}���Ɏ��s���������
			break;
								  }
		case PACKET::MapReq:{																	//MapReq�p�P�b�g�̏ꍇ
			cout << id << " MapReq ��M  " << pPtr->getReqDest() << "\t" << nPtr->gab[pPtr->getReqDest() - sPtr->getMAP()].getMap() <<  endl;
			short sSta = pPtr->getSTA();															//�v��STA
			short dSta = pPtr->getReqDest();														//����STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//����STA�̐ڑ�MAP���
			if(dMap == id){																				//����STA�̐ڑ��悪�����Ȃ�
				//cout << id << " �� " << sid << " ��MapRep �ԐM  " <<  endl;
				newPtr = new PACKET(sPtr, now, PACKET::MapRep, REP, id, id, pPtr->getReqSource(), -1);	//MapRep�p�P�b�g�I�u�W�F�N�g�̍쐬
				newPtr->setLAB(nPtr->gab[dSta - sPtr->getMAP()]);								//LAB���̓o�^
			}
			else if(dMap != -1)																		//����MAP�ɐڑ����Ă���Ƃ�����񂪂���Ȃ�
				newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, dMap, -1);	//MapReq�p�P�b�g�I�u�W�F�N�g�̍쐬
			else{																							//�ڑ����������Ă��Ȃ��Ȃ�
				newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReqB�p�P�b�g�I�u�W�F�N�g�̍쐬
				//cout << sid << " �s���̂��߃u���[�h�L���X�g" << endl;
			}
			newPtr->setTimeout(pPtr->getTimeout());											//�v���^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
			newPtr->setSTA(sSta);																	//�v��STA�̓o�^
			newPtr->setReqSource(pPtr->getReqSource());
			newPtr->setReqDest(dSta);																//����STA�̓o�^
			newPtr->setSeq(nPtr->getSeq());														//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			nPtr->increSeq();																			//�m�[�h�V�[�P���X�̃C���N�������g
			if(!newPtr->queue(sPtr, false))														//�o�b�t�@�ւ̃p�P�b�g�}��
				delete newPtr;																				//�}���Ɏ��s���������
			break;
								  }
		case PACKET::MapReqB:{
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid 
																  || id >= sPtr->getMAP())			//���Ɏ�M�����p�P�b�g��STA����M�����ꍇ
				break;																						//��M�����������ɔj��
			//if(sid == 112)
			//	cout << id << " MapReqB ��M " << pPtr->getReqDest() << "\t" << (int)pPtr->getHop() << endl;
			nPtr->floodSeq[sid] = pPtr->getSeq();												//��M�t���b�f�B���O�V�[�P���X�̍X�V
			short sSta = pPtr->getSTA();															//�v��STA
			short dSta = pPtr->getReqDest();														//����STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//����STA�̐ڑ�MAP���
			if(dMap == id){																				//����STA�̐ڑ��悪�����Ȃ�
			//	cout << "MapRep ���M" << endl;
				newPtr = new PACKET(sPtr, now, PACKET::MapRep, REP, id, id, pPtr->getReqSource(), -1);	//MapRep�p�P�b�g�I�u�W�F�N�g�̍쐬
				newPtr->setLAB(nPtr->gab[dSta - sPtr->getMAP()]);								//LAB���̓o�^
				newPtr->setTimeout(pPtr->getTimeout());											//�v���^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
				newPtr->setSTA(sSta);																	//�v��STA�̓o�^
				newPtr->setReqDest(dSta);																//����STA�̓o�^
				newPtr->setSeq(nPtr->getSeq());														//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
				nPtr->increSeq();																			//�m�[�h�V�[�P���X�̃C���N�������g
				if(!newPtr->queue(sPtr, false))														//�o�b�t�@�ւ̃p�P�b�g�}��
					delete newPtr;																				//�}���Ɏ��s���������
			}
			else																								//����STA�̐ڑ��悪�����łȂ��Ȃ�		
				nPtr->relayPacket(pPtr, 0);																//�p�P�b�g���p����
			break;
									}
		case PACKET::MapRep:{																	//MapRep�p�P�b�g�̏ꍇ
//			cout << id << " MapRep ��M from " << sid << endl;
			short sSta = pPtr->getSTA();															//�v��STA
			short dSta = pPtr->getReqDest();														//����STA
			nPtr->gab[dSta - sPtr->getMAP()] = pPtr->getLAB();								//LAB���̐ݒ�
			newPtr = new PACKET(sPtr, now, PACKET::StaRep, REP, id, id, sSta, -1);	//StaRep�p�P�b�g�I�u�W�F�N�g�̍쐬
			newPtr->setTimeout(pPtr->getTimeout());											//�v���^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
			newPtr->setSTA(sSta);																	//�v��STA�̓o�^
			newPtr->setReqDest(dSta);																//����STA�̓o�^
			newPtr->setSeq(nPtr->getSeq());														//�p�P�b�g�̃V�[�P���X�ԍ��ݒ�
			nPtr->increSeq();																			//�m�[�h�V�[�P���X�̃C���N�������g
			if(!newPtr->queue(sPtr, false))														//�o�b�t�@�ւ̃p�P�b�g�}��
				delete newPtr;																				//�}���Ɏ��s���������
			break;
								  }
		case PACKET::StaRep:
			cout << now.dtime() << "\t" << id << " StaRep��M " << endl;
			cout << "�x��" << (now - pPtr->getTimeout()->getTime()).dtime() << endl;
			if(timeCompare(now - pPtr->getTimeout()->getTime(), SIMUTIME(5, 0)))
				break;
			if(sPtr->list.deleteEvent(pPtr->getTimeout()) != NULL){
				sPtr->increDelay((now - pPtr->getTimeout()->getTime()).dtime());
				delete pPtr->getTimeout();
			}
			break;
		case PACKET::MigReq:{
			//cout << "get migreq at " << id << " from " << sid << endl;
			LOCATION nowLoc = nPtr->getDerivePos(0);
			LOCATION lastLoc = nPtr->getDerivePos(1);
			MAREP* repPtr = new MAREP(nowLoc, nowLoc.getX() - lastLoc.getX(),
				nowLoc.getY() - lastLoc.getY(), SIMUTIME(1,0), nPtr->getRemainPower(), pPtr->getMa());
			newPtr = new PACKET(sPtr, now, PACKET::MigRep, MIGREP, id, id, sid, -1);//MA�ړ������p�P�b�g�̍쐬
			newPtr->setMigRep(repPtr);
			newPtr->setDpos(nPtr->nodePos[sid]);
			if(!newPtr->queue(sPtr, false))															//�p�P�b�g���o�b�t�@�֑}��
				delete newPtr;																				//�}���Ɏ��s������폜
			break;								  								  }
		case PACKET::MigRep:{
			//cout << now.dtime() << " get migrep from " << sid << "\t" << dist(sPtr->node[sid]->getDerivePos(0), sPtr->ma[0]->getCenter()) << endl;
			MA* mPtr = pPtr->getMigRep()->getMa();
			if(mPtr->getTimeout() && !mPtr->getMigration())
				mPtr->decideMigratingNode(sid, pPtr->getMigRep());
			break;
								  }
		case PACKET::InformLoc:								//�ʒu����MA�ɑ����Ă���
			//cout << "get informlog --- " << now.dtime() << "\t" << id << "\t" << sid << "\t" << did << endl;
			//cout << now.dtime() << "\t" << id << "   informloc " << endl;
			sPtr->counter2++;
			//cout << now.dtime() << "," << sPtr->counter1 << "," << sPtr->counter2 << "," << (double)sPtr->counter2 / sPtr->counter1 << endl;
			sPtr->ma[0]->lastPos[sid] = sPtr->ma[0]->nodePos[sid];									//���݂̈ʒu���𒼋߂̈ʒu���֕ύX
			sPtr->ma[did - sPtr->node.size()]->nodePos[sid] = pPtr->getSpos();						//MA�������Ă���m�[�h�̈ʒu���Ƀp�P�b�g�̑��M���̈ʒu��������	
			//pPtr->getSpos().show();
			break;
		case PACKET::DummyBroadcast:
			//cout << now.dtime() << "\t" << id << "  receive flooding " << endl;
			nPtr->receivedPacketList.pop_back();												//���p�����ōēx�d����M���������邽�߃��X�g����폜
			nPtr->relayPacket(pPtr, 0);															//�p�P�b�g���p����
			break;


		//*****************************************************************//
		// �}���`�p�X���v���C�p�P�b�g
		//*****************************************************************//
		case PACKET::MrRep:{														//�}���`�p�X���v���C�p�P�b�g����
			//cout << " get MrRep --- " << now.dtime() << " from " << sid << endl;
			int dest = pPtr->reqPath[0];
			nPtr->requestTime[dest] = -2 * REQUEST_INT;										//���[�g���N�G�X�g�^�C���������̓o�^
			nPtr->path[dest].clear();														//�Â��o�H������U�폜
			nPtr->path1[dest].clear();
			nPtr->path2[dest].clear();
			nPtr->nodePos[dest] = pPtr->getReqDPos();
			//cout << "reqpath size " << pPtr->reqPath.size() << endl;
			if(pPtr->reqPath.size() > 1){
				for(int i = pPtr->reqPath.size() -1; i >= 0; i--){
					nPtr->path[dest].push_back(pPtr->reqPath[i]);
				}
			}
			if(pPtr->reqPath1.size() > 1){
				for(int i = pPtr->reqPath1.size() -1; i >= 0; i--){
					nPtr->path1[dest].push_back(pPtr->reqPath1[i]);
				}
			}
			if(pPtr->reqPath2.size() > 1){
				for(int i = pPtr->reqPath2.size() -1; i >= 0; i--){
					nPtr->path2[dest].push_back(pPtr->reqPath2[i]);
				}
			}
			//cout << "path[" << dest << "] size after input:" << nPtr->path[dest].size() << endl;
			//cout << "path ::: ";
			//for(int i = 0; i < (int)nPtr->path[dest].size(); i++ ){
			//	cout << "->" << nPtr->path[dest][i];
			//}cout << endl;
			//cout << "path1 ::: ";
			//for(int i = 0; i < (int)nPtr->path1[dest].size(); i++ ){
			//	cout << "->" << nPtr->path1[dest][i];
			//}cout << endl;
			//cout << "path2 ::: ";
			//for(int i = 0; i < (int)nPtr->path2[dest].size(); i++ ){
			//	cout << "->" << nPtr->path2[dest][i];
			//}cout << endl;
			//cout << "path pos ::: " << endl;
			//for(int i = 0; i < nPtr->path[dest].size(); i++ ){
			//	cout << nPtr->path[dest][i] << "\t";
			//	sPtr->node[nPtr->path[dest][i]]->getPos().show();
			//}
			//for(int i = 0; i < nPtr->path[dest].size()-1; i++ ){
			//	cout << nPtr->path[dest][i] << "-" << nPtr->path[dest][i+1] << "\t" << dist(sPtr->node[nPtr->path[dest][i]]->getPos(),sPtr->node[nPtr->path[dest][i+1]]->getPos()) << endl;;
			//}
			break;
						   }
		//*****************************************************************//
		// �}���`�p�X���N�G�X�g�p�P�b�g
		//*****************************************************************//
		case PACKET::MrReq:														//�}���`�p�X���N�G�X�g�p�P�b�g����
			//cout << " get MrReq --- " << now.dtime() << "\tsid:" << sid << "\tdid:" << did << "\tid:" << id <<  "\treqdest\t" << pPtr->getReqDest() << endl;

			//MA�̊֐����Ăяo���ăp�P�b�g�ɒ��g����
			newPtr = nPtr->ma[0]->makeMultiRoute(sPtr, sid, did, now, pPtr, id);
			newPtr->setReqDPos(nPtr->ma[0]->nodePos[pPtr->getReqDest()]);
			//cout << "test out " << endl;
			//cout << "path size " << newPtr->reqPath.size() << endl;
			//cout << "path1 size " << newPtr->reqPath1.size() << endl;
			//cout << "path2 size " << newPtr->reqPath2.size() << endl;
			for(int i = (int)newPtr->reqPath.size() - 1; i >= 0; i--){						//�v���p�P�b�g�̓��B�o�H���t�����ɉ����p�P�b�g�ԐM�o�H�ɐݒ�
				sPtr->node[id]->path[sid].push_back(newPtr->reqPath[i]);
				//cout << "test out  2 " << endl;
				//cout << newPtr->reqPath[i] << endl;
			}
			newPtr->setSize(newPtr->getSize() + 4 * (int)newPtr->reqPath.size() + 4 * (int)newPtr->reqPath1.size() + 4 * (int)newPtr->reqPath2.size());
			if(!newPtr->queue(sPtr, false)){														//�p�P�b�g���o�b�t�@�֑}��
				cout << "packet delete at makeMultiRoute" << endl;
				delete newPtr;																				//�}���Ɏ��s������폜
			}
			break;


//			newPtr = new PACKET(sPtr, now, PACKET::MrRep, RREQDSR, id, id , sid, -1);
//			nPtr->path[sid].clear();
//			nPtr->path[sid].push_back(id);
//			newPtr->path.push_back(id);
//			for(short i = (short)pPtr->path.size() - 1; i >= 0; i--){
//				//cout << pPtr->path[i] << "->";
//				nPtr->path[sid].push_back(pPtr->path[i]);											//�m�[�h�̌o�H���̏�������	
//				newPtr->path.push_back(pPtr->path[i]);												//���v���C�p�P�b�g�̌o�H���̏�������
//			}
//			newPtr->increSize(pPtr->path.size() * 4);											//�o�H��񕪂����p�P�b�g�T�C�Y�𑝉�
//			newPtr->setSeq(nPtr->getSeq());														//�V�[�P���X�ԍ��̕t�^
//			nPtr->increSeq();																			//�V�[�P���X�ԍ��̃C���N�������g
//			if(!newPtr->queue(sPtr, false))														//�p�P�b�g���o�b�t�@�֑}��
//				delete newPtr;																				//�}���Ɏ��s������폜
//			break;



			//	//MA�̊֐����Ăяo���Čo�H�\�z������

			//	//�֐��Ăяo��!!!

			//}
			//else{																//�������g������łȂ��ꍇ
			//	nPtr->receivedPacketList.pop_back();								//���p�����ōēx�d����M���������邽�߃��X�g����폜
			//	newPtr = nPtr->relayPacket(pPtr, 4);								//�p�P�b�g���p����
			//	if(newPtr)														//���p���������������ꍇ
			//		newPtr->path.push_back(id);									//���g���o�H���ɒǉ�
			//}
			//break;
		//******************************************************************//
		// �}���`�p�X���N�G�X�g�p�P�b�g�@�ȏ�
		//******************************************************************//


//		case PACKET::Dissemination:{
//			bool flag = false;
////			cout << "receive dissmination " << id << endl;
//			if(dist(node[id]->getPos(), pPtr->getMa()->getCenter()) < MARANGE)
//				flag = true;
//			else{
//				int lid = pPtr->getLast();
//				if(dist(node[id]->getDerivePos(0), pPtr->getMa()->getCenter())
//				< dist(node[lid]->getDerivePos(0), pPtr->getMa()->getCenter()))
//					flag = true;
//			}
//			if(flag){
//				PACKET* newPtr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//�����p�p�P�b�g
//				*newPtr = *pPtr;																				//���̃p�P�b�g���𕡐��p�P�b�g�փR�s�[
////				cout << "relay " << id << endl;
//				if(newPtr->queue(list, node[id], false, true, getTime()) == false)							//�o�b�t�@�֑}��
//					delete newPtr;																					//�}���Ɏ��s���������
//				newPtr->setHere(id);																				//�p�P�b�g�̌��݈ʒu�������ɐݒ�
//				newPtr->setLast(id);
//			}
//			break;
//											}
	}
	return false;																					//��M�I�u�W�F�N�g�͏���
}

//void result(SIMU* sPtr){
//	double sumPow = 0;
//	double outTime = 0;
//	int migNum = 0;
//
//	for(int i = 0; i < (int)sPtr->getNode().size(); i++)
//		sumPow += 100000000 - sPtr->getNode()[i]->getRemainPower();
//	for(int i = 0; i < (int)sPtr->ma.size(); i++){
//		outTime += sPtr->ma[i]->getOutTime();
//		migNum += sPtr->ma[i]->getMigNum();
//	}
//	cout << sumPow << "," << outTime << "," << migNum << endl;
//}
//
//void result(const char* fname, SIMU* sPtr){
//	fstream fstr (fname, ios::app);	
//	double sumPow = 0;
//	double outTime = 0;
//	int migNum = 0;
//	double max = 0;
//	for(int i = 0; i < (int)sPtr->getNode().size(); i++){
//		if(max < 100000000 - sPtr->getNode()[i]->getRemainPower())
//			max = 100000000 - sPtr->getNode()[i]->getRemainPower();
//		sumPow += 100000000 - sPtr->getNode()[i]->getRemainPower();
//	}
//	for(int i = 0; i < (int)sPtr->ma.size(); i++){
//		outTime += sPtr->ma[i]->getOutTime() / 1000000.0;
//		migNum += sPtr->ma[i]->getMigNum();
//	}
////	fstr << load << "," << sumPow / 1000.0 / (int)sPtr->node.size() << "," << max / 1000.0 << "," << outTime / (TIMELIMIT) << "," << migNum << endl;
//}

