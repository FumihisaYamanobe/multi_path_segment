//#include "class.h"
//#include "mobileAgent.h"


//
////AODV_RREQ�̃R���X�g���N�^�i�V�K�쐬�j
////�����i���g�̃V�[�P���X�ԍ��C����̃V�[�P���X�ԍ��C���ݎ����C���M��ID�C���݃m�[�hID�C����ID�C����t���O�j
//AODV_RREQ::AODV_RREQ(int sSeq, int dSeq, double tval, int sid, int hid, int did, bool flag){	
//	PACKET* ptr = new PACKET(tval, PACKET::RreqAodv, RREQAODV, sid, hid, did, -1, 0);//���N�G�X�g�p�P�b�g�쐬
//	pPtr = ptr;																					//���N�G�X�g�I�u�W�F�N�g�ɍ쐬�p�P�b�g��o�^
//	ptr->setAodvRreq(this);																	//�p�P�b�g�Ƀ��N�G�X�g�I�u�W�F�N�g��o�^
//	sourceSeq = sSeq;																			//���M����ݒ�
//	destSeq = dSeq;																			//�����ݒ�
//	destFlag = flag;																			//����t���O��ݒ�
//}
//
////AODV_RREQ�̃R���X�g���N�^�i�����j
////�����i�������I�u�W�F�N�g�|�C���^�j
//AODV_RREQ::AODV_RREQ(AODV_RREQ* rPtr){
//	PACKET* ptr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//�����p�p�P�b�g�̂̍쐬
//	*ptr =*(rPtr->getPacket());															//���������N�G�X�g�̑Ή��p�P�b�g�����R�s�[
//	pPtr = ptr;																					//���N�G�X�g�I�u�W�F�N�g�ɍ쐬�p�P�b�g��o�^
//	ptr->setAodvRreq(this);																	//�p�P�b�g�Ƀ��N�G�X�g�I�u�W�F�N�g��o�^
//	sourceSeq = rPtr->getSourceSeq();													//���M���𕡐�
//	destSeq = rPtr->getDestSeq();															//����𕡐�
//	destFlag = rPtr->getDestFlag();														//����t���O�𕡐�
//}
//
////AODV_RREP�̃R���X�g���N�^�i�V�K�쐬�j
////�����i���g�̃V�[�P���X�ԍ��C����̃V�[�P���X�ԍ��C���ݎ����C���M��ID�C���݃m�[�hID�C����ID�C����t���O�j
//AODV_RREP::AODV_RREP(int sSeq, int dSeq, double tval, int sid, int hid, int did){
//	PACKET* ptr = new PACKET(tval, PACKET::RrepAodv, RREPAODV, hid, hid, did, -1, 0);
//	pPtr = ptr;																					//���v���C�I�u�W�F�N�g�ɍ쐬�p�P�b�g��o�^
//	ptr->setAodvRrep(this);																	//�p�P�b�g�Ƀ��v���C�I�u�W�F�N�g��o�^
//	dest = sid;
//	sourceSeq = sSeq;																			//���M����ݒ�
//	destSeq = dSeq;																			//�����ݒ�
//}
//
////AODV_RREP�̃R���X�g���N�^�i�����j
////�����i�������I�u�W�F�N�g�|�C���^�j
//AODV_RREP::AODV_RREP(AODV_RREP* rPtr){
//	PACKET* ptr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//�����p�p�P�b�g�̂̍쐬
//	*ptr =*(rPtr->getPacket());															//���������v���C�̑Ή��p�P�b�g�����R�s�[
//	pPtr = ptr;																					//���v���C�I�u�W�F�N�g�ɍ쐬�p�P�b�g��o�^
//	ptr->setAodvRrep(this);																	//�p�P�b�g�Ƀ��v���C�I�u�W�F�N�g��o�^
//	dest = rPtr->getDest();
//	sourceSeq = rPtr->getSourceSeq();													//���M���𕡐�
//	destSeq = rPtr->getDestSeq();															//����𕡐�
//}
//	
