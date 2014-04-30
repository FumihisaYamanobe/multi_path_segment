#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <math.h>
#include "list.h"
#include "scenario.h" 


using namespace std;

class SIMU;
class CHANNEL;
class MAC;
class NODE;
class UDP;
class UDPSINK;
class SEGMENT;
class TCP;
class TCPSINK;
class MA;
class MAREP;
class LAB;

typedef vector<NODE*> _NODE;

//�V�~�����[�V���������N���X
class SIMUTIME{
	int sec;																								//�b��\���l
	int lessSec;																						//�b������\���l(�ŏ��l��1��s�j
	short lessMuSec;																					//�ʕb������\���l�i�ŏ��l��0.001��s�j
public:
	SIMUTIME(int s, int mu)					{ sec = s, lessSec = mu, lessMuSec = 0; }	//�R���X�g���N�^�i�����l��2����ꍇ)
	SIMUTIME(int mu)							{ if(mu < 1000000){ sec = 0, lessSec = mu, lessMuSec = 0;}
														else{sec = mu / 1000000, lessSec = mu % 1000000, lessMuSec = 0;}}	//�R���X�g���N�^�i�����l��1����ꍇ)
	SIMUTIME(void)								{ sec = 0, lessSec = 0; lessMuSec = 0; }	//�R���X�g���N�^�i�����l���Ȃ��ꍇ�i0�ŏ������j�j
	SIMUTIME operator+(const SIMUTIME& t){ if(lessSec + t.lessSec < 1000000)
															return SIMUTIME(sec + t.sec, 
																			 lessSec + t.lessSec);
														else
															return SIMUTIME(sec + t.sec + 1, 
																			 lessSec + t.lessSec - 1000000);}	//�����̉��Z���Z�q�Ɋւ����`
	SIMUTIME operator-(const SIMUTIME& t){ if(lessSec >= t.lessSec)
															return SIMUTIME(sec - t.sec, 
																			 lessSec - t.lessSec);
														else
															return SIMUTIME(sec - t.sec -1, 
																			 lessSec + 1000000 - t.lessSec);}	//�����̌��Z���Z�q�Ɋւ����`
	void setSec(int val)						{ sec = val; }										//�b�̐ݒ�
	int getSec(void)							{ return sec; }									//�b�̎擾
	void setLessSec(int val)				{ lessSec = val; }								//�ʕb�̐ݒ�
	int getLessSec(void)						{ return lessSec; }								//�ʕb�̎擾
	void setLessMuSec(short val)			{ lessMuSec = val; }								//�ʕb�����̐ݒ�
	short getLessMuSec(void)				{ return lessMuSec; }							//�ʕb�����̎擾
	void setTime(int s, int mu)			{ sec = s, lessSec = mu; }						//�����̐ݒ�i���l�Ŏw��j
	void setTime(SIMUTIME t)				{ sec = t.getSec(), lessSec = t.getLessSec(), lessMuSec = t.getLessMuSec(); }	//�����̐ݒ�i�I�u�W�F�N�g�Ŏw��j
	void addTime(SIMUTIME t)				{ sec += t.getSec(), lessSec += t.getLessSec(); 
														if(lessSec >= 1000000)sec++, lessSec -= 1000000;}
	void subTime(SIMUTIME t)				{ if(lessSec >= t.lessSec)
															sec - t.sec, lessSec - t.lessSec;
														else
															sec - t.sec -1, lessSec + 1000000 - t.lessSec;}
	bool isPositive(void)					{ if(sec < 0 || lessSec < 0 || lessMuSec < 0)return false; return true; }
	void showSec(void)						{ cout << sec; }
	void showMuSec(void)						{ cout << sec << "."; 
													  if(lessSec == 0) cout << "00000";
													  else for(int i = 0; i < 5 - (int)log10((double)lessSec); i++) cout << "0";
													  cout << lessSec;
		                                   if(lessMuSec > 999) cout << lessMuSec;
		                                   else if(lessMuSec > 99) cout << "0" << lessMuSec;
													  else if(lessMuSec > 9) cout << "00" << lessMuSec;
													  else cout << "000" << lessMuSec;}
	void show(void)							{ cout << sec << ".";
													  if(lessSec == 0) cout << "00000";
													  else for(int i = 0; i < 5 - (int)log10((double)lessSec); i++) cout << "0";
													  cout << lessSec << "_";
	                                      if(lessMuSec > 999) cout << lessMuSec << endl;
													  else if(lessMuSec > 99) cout << "0" << lessMuSec << endl;
													  else if(lessMuSec > 9) cout << "00" << lessMuSec << endl;
													  else cout << "000" << lessMuSec << endl;}
	void showtab(void)						{ cout << sec << ".";
													  if(lessSec == 0) cout << "00000";
													  else for(int i = 0; i < 5 - (int)log10((double)lessSec); i++) cout << "0";
													  cout << lessSec << "_";
	                                      if(lessMuSec > 999) cout << lessMuSec << "\t";
													  else if(lessMuSec > 99) cout << "0" << lessMuSec << "\t";
													  else if(lessMuSec > 9) cout << "00" << lessMuSec << "\t";
													  else cout << "000" << lessMuSec << "\t";}
	double dtime(void)						{ return sec + lessSec / 1000000.0; }
};

//������r�֐�
//�����i����a ����b)
//�߂�l�ia�������Ȃ�^�C�ߋ��Ȃ�U�j
inline bool timeCompare(SIMUTIME a, SIMUTIME b){
	if(a.getSec() > b.getSec())
		return true;
	if(a.getSec() < b.getSec())
		return false;
	if(a.getLessSec() > b.getLessSec())
		return true;
	if(a.getLessSec() < b.getLessSec())
		return false;
	if(a.getLessMuSec() > b.getLessMuSec())
		return true;
	return false;
}

//������r�֐�
//�����i����a ����b)
//�߂�l�ia�������Ȃ�^�C�ߋ��Ȃ�U�j
inline bool timeCompareMu(SIMUTIME a, SIMUTIME b){
	if(a.getSec() > b.getSec())
		return true;
	if(a.getSec() < b.getSec())
		return false;
	if(a.getLessSec() > b.getLessSec())
		return true;
	return false;
}

//�ʒu���N���X
class LOCATION{
	int x;
	int y;
	SIMUTIME t;
public:
	LOCATION(int xx, int yy, SIMUTIME tt)	{ x = xx, y = yy, t = tt; }
	LOCATION(int xx, int yy)					{ x = xx, y = yy; }
	LOCATION(void)									{ x = 0, y = 0, t.setSec(-1); }
	void setX(int val)							{ x = val; }
	int getX(void)									{ return x; }
	void setY(int val)							{ y = val; }
	int getY(void)									{ return y; }
	void setT(SIMUTIME val)						{ t = val; }
	SIMUTIME getT(void)							{ return t; }
	void set(int xx, int yy, SIMUTIME tt)	{ x = xx, y = yy, t = tt; }
	void set(int xx, int yy)					{ x = xx, y = yy; }
	void show(void)								{ cout << "(" << x << "," << y << ")" << " -- " << t.dtime() << endl; }
};


inline int min(int x, int y){																		//�ŏ��l�̎擾
	if(x < y)
		return x;
	return y;
}

inline int max(int x, int y){																		//�ŏ��l�̎擾
	if(x > y)
		return x;
	return y;
}

inline double max(double x, double y){															//�ő�l�̎擾
	if(x > y)
		return x;
	return y;
}

inline void check(void){																			//�f�o�b�O�p�`�F�b�N�֐�
	std::cout << "check" << std::endl;
}

inline double dist(LOCATION me, LOCATION you){
	return sqrt(pow((double)me.getX() - you.getX(), 2.0) + pow((double)me.getY() - you.getY(), 2.0));
}

//�C�x���g�N���X
class EVENT{
public:
	enum type {Node, Channel, Mac, Frame, Packet, Beacon, Udp, Tcp, Sink, Segment, Receive, Timeout, Ma, Abort};	//�C�x���g�^�C�v���ʎq
private:
	SIMU* sPtr;
	SIMUTIME time;																						//�C�x���g��������
	type typeId;																						//�C�x���g�̃^�C�v
	short nodeId;																						//�C�x���g�����m�[�h��ID
public:
	EVENT(SIMU* ptr, SIMUTIME val, type id, short nid) 
	{ sPtr = ptr, time = val, typeId = id, nodeId = nid, time.setLessMuSec(nodeId); }//�R���X�g���N�^
	virtual ~EVENT(){ ; }																			//�f�X�g���N�^�i���z�f�X�g���N�^�j
	SIMU* getSimu(void)					{ return sPtr; }
	void setTime(SIMUTIME val)			{ time = val, time.setLessMuSec(nodeId); }	//�C�x���g���������̐ݒ�i�I�u�W�F�N�g���p�j
	void addTime(SIMUTIME val)			{ time.addTime(val); }								//�C�x���g���������̍X�V�i�I�u�W�F�N�g���p�j
	SIMUTIME getEventTime(void)		{ return time; }										//�C�x���g���������̎擾
	type getType(void)					{ return typeId; }									//�C�x���g�^�C�v�̎擾
	void setNid(short id)				{ nodeId = id; }										//�C�x���g�����m�[�hID�̐ݒ�
	short getNid(void)					{ return nodeId; }									//�C�x���g�����m�[�hID�̎擾
	virtual bool process(void) = 0;																//�C�x���g�����i���z�֐��j
	void show(double val)
	{
		int sec = (int)floor(val);
		int musec = (int)((val - sec) * 1000000);
		short lessMuSec = time.getLessMuSec();
		char* id[14] = { "Node", "Channel", "Mac", "Frame", "Packet", "Beacon", "Udp", "Tcp", "Sink", "Segment", "Receive", 
			"Timeout", "Ma", "Abort"};
		if(timeCompare(time, SIMUTIME(sec, musec)) || (time.getSec() == sec && time.getLessSec() == musec)){
			cout << time.dtime() << "_";
			if(lessMuSec > 999)
				cout << lessMuSec;
			else if(lessMuSec > 99)
				cout << "0" << lessMuSec;
			else if(lessMuSec > 9) 
				cout << "00" << lessMuSec;
			else cout << "000" << lessMuSec;
			cout << " --- " << id[(int)getType()] << endl;
		}
	}
	void show(void)
	{
		char* id[14] = { "Node", "Channel", "Mac", "Frame", "Packet", "Beacon", "Udp", "Tcp", "Sink", "Segment", "Receive", 
			"Timeout", "Ma", "Abort"};
			cout << " --- " << id[(int)getType()] << endl;
	}
};

//�M���N���X
class SIGNAL{
public:
	enum type { RTS, CTS, DATA, ACK, BDATA, ORACK };										//�M���^�C�v�̗�
private:
	CHANNEL* chPtr;																					//�`���l���I�u�W�F�N�g�|�C���^
	type typeId;																						//�M���^�C�v
	short length;																						//�M����
	short dataTime;																					//���M�������f�[�^�M����
	short duration;																					//����g�p����
	short orSource;																					//ORRTS�p���M���m�[�hID
public:
	SIGNAL(CHANNEL*);																					//�R���X�g���N�^
	~SIGNAL();
	CHANNEL* getChannel(){ return chPtr; }														//�`���l���I�u�W�F�N�g�̎擾
	type getType(void)	{ return typeId; }													//�M���^�C�v�̎擾
	short getLength(void){ return length; }													//�M�����̎擾
	short getDataTime(void)	{ return dataTime; }												//���M�������f�[�^�M�����̎擾
	short getDuration(void) { return duration; }												//����g�p���Ԃ̎擾
	void setOrSource(short id) { orSource = id; }											//ORRTS�p���M���m�[�hID�̐ݒ�
	short getOrSource(void)	{ return orSource; }												//ORRTS�p���M���m�[�hID�̎擾
};

//�`���l���N���X
class CHANNEL:public EVENT{
public:
private:
	MAC* mPtr;																							//MAC�I�u�W�F�N�g
	bool sendFlag;																						//���M��ԃt���O
	SIGNAL* sigPtr;																					//��M�M���I�u�W�F�N�g
	short dataTime;																					//��M�f�[�^�̎�M���v����
public:
	CHANNEL(SIMU*, MAC*, SIMUTIME, short);																		//�R���X�g���N�^
	bool process(void);																				//�C�x���g����
	MAC* getMAC()						{ return mPtr; }											//MAC�I�u�W�F�N�g�̎擾
	void setSendFlag(bool flag)	{ sendFlag = flag; }										//���M��ԃt���O�̐ݒ�
	bool getSendFlag(void)			{ return sendFlag; }										//���M��ԃt���O�̎擾
	void setSignal(SIGNAL* ptr)	{ sigPtr = ptr; }											//��M�M���̐ݒ�
	SIGNAL* getSignal(void)			{ return sigPtr; }										//��M�M���̎擾
	void setDataTime(short val)	{ dataTime = val; }											//�f�[�^��M���v���Ԃ̎擾
	short getDataTime(void)			{ return dataTime; }										//�f�[�^��M���v���Ԃ̎擾
	class RX_SIGNAL_LIST{																			//��M�M�����N���X
		short id;																							//�m�[�hID
		double rxPower;																					//��M�d�́i����M��0�j
		SIMUTIME rxFinTime;																				//��M��������
	public:
		RX_SIGNAL_LIST(short ival, double pval){ id = ival, rxPower = pval, 
																rxFinTime.setLessMuSec(id); }			//�R���X�g���N�^
		short getId(void)						{ return id; }											//�m�[�hID�̎擾
		double getRxPower(void)				{ return rxPower; }									//��M�d�͂̎擾
		void setRxFinTime(SIMUTIME t)		{ rxFinTime.setSec(t.getSec()),
													  rxFinTime.setLessSec(t.getLessSec()); }		//��M���������̐ݒ�
		SIMUTIME getRxFnTime(void)			{ return rxFinTime; }								//��M���������̎擾
	};
	vector<RX_SIGNAL_LIST> signalList;															//��M�M����񃊃X�g
	void sendSignal(SIMUTIME);
};

//�^�C���A�E�g�N���X�i�C�x���g�N���X�̌p���j
class TIMEOUT:public EVENT{
public:
	enum type { Tcp, Udp, Ma, Segment, RouteAodv, Mareq, StaReq, MakeUdp, MeshMa, BackGround, MakeTcp };
private:
	type typeId;																	//�^�C���A�E�g�̃^�C�v
	bool flag;																		//�t���O�i�l�X�ȏ����Ɏg�p�j
	SIMUTIME timeVal;																//�������i�l�X�ȏ����Ɏg�p�j
	double rate;																	//�������[�g�i���ה������ȂǂŎg�p�j
	union TYPEPTR{
		SEGMENT* sPtr;
		TCP* tPtr;
		UDP* uPtr;
		MA* maPtr;
	}typePtr;
	short dest;
public:
	TIMEOUT(SIMU*, SIMUTIME, type, short);
//	~TIMEOUT();
	bool process(void);															//�C�x���g����
//	void setType(type tid)			{ typeId = tid; }
//	type getType(void)				{ return typeId; }
	void setFlag(bool val)			{ flag = val; }						//�t���O�̐ݒ�
	bool getFlag(void)				{ return flag; }						//�t���O�̎擾
	void setTime(SIMUTIME tval)	{ timeVal = tval; }					//�����̐ݒ�
	SIMUTIME getTime(void)			{ return timeVal; }					//�����̎擾
	void setRate(double val)		{ rate = val; }						//���׃��[�g�̐ݒ�
	double getRate(void)				{ return rate; }						//���׃��[�g�̎擾
	void setSegment(SEGMENT* ptr)	{ typePtr.sPtr = ptr; }
	SEGMENT* getSegment(void)		{ return typePtr.sPtr; }
	void setTcp(TCP* ptr)			{ typePtr.tPtr = ptr; }
	TCP* getTcp(void)					{ return typePtr.tPtr; }
	void setUdp(UDP* ptr)			{ typePtr.uPtr = ptr; }
//	UDP* getUdp(void)					{ return typePtr.uPtr; }
	void setMa(MA* ptr)				{ typePtr.maPtr = ptr; }
//	MA* getMa(void)					{ return typePtr.maPtr; }
	void setDest(short id)			{ dest = id; }
	short getDest(void)				{ return dest; }
};

class LAB{
	short mapId;
	SIMUTIME t;
public:
	LAB(short id , SIMUTIME tval)	{ mapId = id, t = tval; }
	LAB(short id)				{ mapId = id, t = 0; }
	LAB()							{ mapId = -1, t = 0; }
	void setMap(short id)	{ mapId = id; }
	short getMap(void)		{ return mapId; }
	void setTime(SIMUTIME tval)	{ t = tval; }
	SIMUTIME getTime(void)	{ return t; }
};

//�p�P�b�g�N���X�i�C�x���g�N���X�̌p���j
class PACKET:public EVENT{	
public:
	enum ptype {Udp, Tcp, Ack, Null, Routing, Flood, 
		RreqDsr, RrepDsr, RerrDsr, RreqAodv, RrepAodv, RerrAodv, Segment,
		MigReq, MigRep, Dissemination, LabCenter, LabRa, LabNeighbor, StaReq, 
		StaRep, MapReq, MapReqB, MapRep, InformLoc, DummyBroadcast, MrReq, MrRep};					//�p�P�b�g�^�C�v���ʎq
																									//MrReq�@MrRep�F�}���`�p�X���[�e�B���O�p�p�P�b�g
private:
	ptype type;																							//�p�P�b�g�^�C�v
	short size;																							//�p�P�b�g�T�C�Y
	short source;																						//���M���m�[�h
	LOCATION sPos;																						//���M���m�[�h�ʒu
	short here;																							//���݃m�[�h
	short dest;																							//���M����m�[�h
	LOCATION dPos;																						//���M����m�[�h�ʒu								
	short reqSource;																					//�v�������m�[�h
	short reqDest;																						//�v������m�[�h
	short errDest;																						//�G���[�p�P�b�g�̑Ώۈ���
	int seq;																								//�V�[�P���X�ԍ�
	int aodvSeqS;																						//AODV�p�V�[�P���X�i���M���j
	int aodvSeqD;																						//AODV�p�V�[�P���X�i���M����j
	char hop;																							//�o�R�z�b�v��
	char ttl;																							//�����z�b�v��
	int requestSeq;																					//TCPACK�������ꍇ�̊��Ҏ�M�V�[�P���X�ԍ�
	SIMUTIME sendStartTime;																			//���M���o������
	LAB lab;																								//LAB���
	short sta;																							//STA���
	union TYPEPTR{																						//�p�P�b�g�^�C�v���Ƃ̊֘A���
		UDP* udpPtr;																						//�Ή�UDP�I�u�W�F�N�g�iUDP�p�P�b�g�̏ꍇ�j
		SEGMENT* segPtr;																					//�Ή��Z�O�����g�I�u�W�F�N�g�iTCP�p�P�b�g�̏ꍇ�j
		MA* maPtr;																							//�Ή�MA�I�u�W�F�N�g�iMA�p�P�b�g�̏ꍇ�j
		MAREP* maRepPtr;																					//�Ή�MA�����p�P�b�g�iMA�����p�P�b�g�̏ꍇ�j
		TIMEOUT* toPtr;																					//�Ή��^�C���A�E�g�I�u�W�F�N�g
	}typePtr;
	int category;																						//�J�e�S��
	LOCATION sourcePos;																				//���M���[���ʒu
	LOCATION lastNodePos;																			//���O���M�[���ʒu
	LOCATION destPos;																					//����[���ʒu
	LOCATION reqDestPos;
	LOCATION reqSourcePos;
	int requestDest;
	int requestSource;
	ptype packetForGDtype;
public:
   PACKET(SIMU* = NULL, SIMUTIME = 0, ptype = Null, short = -1, short = -1, short = -1, short = -1, int = -1);
	~PACKET();																							//�f�X�g���N�^
	bool process(void);																				//�C�x���g����
	ptype getType(void)			{ return type; }												//�^�C�v�̎擾
	void setSize(short val)		{ size = val; }												//�T�C�Y�̐ݒ�
	void increSize(short val)	{ size += val; }												//�T�C�Y�̃C���N�������g
	short getSize(void)			{ return size; }												//�T�C�Y�̎擾
	short getSource(void)		{ return source; }											//���M���m�[�h�̎擾
	void setSpos(LOCATION pos)	{ sPos = pos; }												//���M���m�[�h�ʒu�̐ݒ�
	LOCATION getSpos(void)		{ return sPos; }												//���M���m�[�h�ʒu�̐ݒ�
	void setHere(short val)		{ here = val; }												//���݃m�[�h�̐ݒ�
	short getHere(void)			{ return here; }												//���݃m�[�h�̎擾
	short getDest(void)			{ return dest; }												//����m�[�h�̎擾
	void setDpos(LOCATION pos)	{ dPos = pos; }												//���M����m�[�h�ʒu�̐ݒ�
	LOCATION getDpos(void)		{ return dPos; }												//���M����m�[�h�ʒu�̐ݒ�
	void setReqSource(short val){ reqSource = val; }										//�v�������m�[�h�̐ݒ�
	short getReqSource(void)	{ return reqSource; }										//�v�������m�[�h�̎擾
	void setReqDest(short val)	{ reqDest = val; }											//�v������m�[�h�̐ݒ�
	short getReqDest(void)		{ return reqDest; }											//�v������m�[�h�̎擾
	void setErrDest(short val)	{ errDest = val; }											//�G���[�p�P�b�g�̑Ώۈ���̐ݒ�
	short getErrDest(void)		{ return errDest; }											//�G���[�p�P�b�g�̑Ώۈ���̎擾
	void setSeq(int val)			{ seq = val; }													//�V�[�P���X�ԍ��̐ݒ�
	int getSeq(void)				{ return seq; }												//�V�[�P���X�ԍ��̎擾
	void setAodvSeqS(int val)	{ aodvSeqS = val; }											//AODV�p�V�[�P���X�ԍ��i���M���j�̐ݒ�
	int getAodvSeqS(void)		{ return aodvSeqS; }											//AODV�p�V�[�P���X�ԍ��i���M���j�̎擾
	void setAodvSeqD(int val)	{ aodvSeqD = val; }											//AODV�p�V�[�P���X�ԍ��i���M����j�̐ݒ�
	int getAodvSeqD(void)		{ return aodvSeqD; }											//AODV�p�V�[�P���X�ԍ��i���M����j�̎擾
	char getHop(void)				{ return hop; }												//�o�R�z�b�v���̎擾
	char getTtl(void)				{ return ttl; }												//�����z�b�v���̎擾
	void setReqSeq(int val)		{ requestSeq = val; }										//ACK�������ꍇ�̊��Ҏ�M�V�[�P���X�ԍ��̐ݒ�
	int getReqSeq(void)			{ return requestSeq; }										//ACK�������ꍇ�̊��Ҏ�M�V�[�P���X�ԍ��̎擾
	void setSendStart(SIMUTIME val)	{ sendStartTime = val; }							//���M�����̐ݒ�
	SIMUTIME getSendStart(void){ return sendStartTime; }									//���M�����̎擾
	void setLAB(LAB val)			{ lab = val; }													//LAB���̐ݒ�
	LAB getLAB(void)				{ return lab; }												//LAB���̎擾
	void setSTA(short val)		{ sta = val; }													//STA���̐ݒ�
	short getSTA(void)			{ return sta; }												//STA���̎擾
	void setUdp(UDP* ptr)		{ typePtr.udpPtr = ptr; }									//�Ή�UDP�I�u�W�F�N�g�̐ݒ�
	UDP* getUdp(void)				{ return typePtr.udpPtr; }									//�Ή�UDP�I�u�W�F�N�g�̎擾
	void setSeg(SEGMENT* ptr)	{ typePtr.segPtr = ptr; }									//�Ή��Z�O�����g�I�u�W�F�N�g�̐ݒ�
	SEGMENT* getSeg(void)		{ return typePtr.segPtr; }									//�Ή��Z�O�����g�I�u�W�F�N�g�̎擾
	void setMa(MA* ptr)			{ typePtr.maPtr = ptr; }									//�Ή�MA�I�u�W�F�N�g�̐ݒ�
	MA* getMa(void)				{ return typePtr.maPtr; }									//�Ή�MA�I�u�W�F�N�g�̎擾
	void setMigRep(MAREP* ptr)	{ typePtr.maRepPtr = ptr; }								//�Ή�MAREP�I�u�W�F�N�g�̐ݒ�
	MAREP* getMigRep(void)		{ return typePtr.maRepPtr; } 								//�Ή�MAREP�I�u�W�F�N�g�̎擾
	void setTimeout(TIMEOUT* ptr){ typePtr.toPtr = ptr; }									//�Ή��^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
	TIMEOUT* getTimeout(void)	{ return typePtr.toPtr; }									//�Ή��^�C���A�E�g�I�u�W�F�N�g�̎擾
	bool queue(SIMU*, bool);																		//�o�b�t�@�ւ̑}��
	void showLog(short, char*, SIMUTIME);														//�p�P�b�g���\��
	void showPath(void);																				//�o�H���\��
	vector<int> path;																					//�o�H���
	vector<int> path1;																					//�}���`�p�X�P
	vector<int> path2;																					//�}���`�p�X�Q
	short pathNum;																		//�o�H���f�p
	vector<int> reqPath;																				//�v���o�H���i��ĕ����p�j
	vector<int> reqPath1;
	vector<int> reqPath2;
	vector<int> mamrPath;
	void setReqDPos(LOCATION loc)		{ reqDestPos = loc; }								//����ʒu�̐ݒ�
	LOCATION getReqDPos(void)			{ return reqDestPos; }								//����ʒu�̎擾
	void setReqSPos(LOCATION loc)		{ reqSourcePos = loc; }								//����ʒu�̐ݒ�
	LOCATION getReqSPos(void)			{ return reqSourcePos; }								//����ʒu�̎擾
	short mpath_check;
};

//�t���[���N���X�i�C�x���g�N���X�̌p���j
class FRAME:public EVENT{
public:
	enum castType {Uni, Broad, Null};															//�t���[���^�C�v���ʎq
private:
	castType cast;																						//�t���[���^�C�v
	short source;																						//���M���m�[�h
	short dest;																							//����m�[�h
	short size;																							//�t���[���T�C�Y�i�r�b�g�j
	int seq;																								//�V�[�P���X�ԍ�
	PACKET* packetPtr;																				//�Ή��p�P�b�g�I�u�W�F�N�g
public:
	FRAME(SIMU*, SIMUTIME, castType, short, short, int, PACKET*);						//�R���X�g���N�^
	~FRAME();																							//�f�X�g���N�^
	bool process(void);																				//�C�x���g����
	void setCast(castType val)	{ cast = val; }												//�t���[���^�C�v�̐ݒ�
	castType getCast(void)		{ return cast; }												//�t���[���^�C�v�̎擾
	short getSize(void)			{ return size; }												//�t���[���T�C�Y�̎擾
	short getSource(void)		{ return source; }											//���M���m�[�h�̎擾
	short getDest(void)			{ return dest; }												//����m�[�h�̎擾
	int getSeq(void)				{ return seq; }												//�V�[�P���X�ԍ��̎擾
	void setPacket(PACKET* ptr){ packetPtr = ptr; }											//�Ή��p�P�b�g�I�u�W�F�N�g�̐ݒ�
	PACKET* getPacket(void)		{ return packetPtr; }
};

//�G�[�W�F���g�N���X�i�C�x���g�N���X�̌p���j
class AGENT:public EVENT{
public:
	AGENT(SIMU* ptr, short nid, SIMUTIME tval, EVENT::type tid):EVENT(ptr, tval, tid, nid){;}	//�R���X�g���N�^
	bool process(void){ return true; }															//�C�x���g����
};

//�r�[�R���N���X�i�G�[�W�F���g�N���X�̌p���j
class BEACON:public AGENT{
public:
	BEACON(SIMU* ptr, short nid, SIMUTIME tval):AGENT(ptr, nid, tval, EVENT::Beacon){};					//�R���X�g���N�^
	bool process(void);															//�C�x���g����
};

//��M�N���X�i�G�[�W�F���g�N���X�̌p���j
class RECEIVE:public AGENT{
	PACKET* pPtr;																				//�p�P�b�g�I�u�W�F�N�g
	short object;																							//��M�C�x���g�̑Ώ�
public:
	RECEIVE(SIMU* ptr, short nid, SIMUTIME tval, PACKET* Ptr, short oval):AGENT(ptr, nid, tval, EVENT::Receive)
	{ object = oval, pPtr = Ptr; }														//�R���X�g���N�^
	~RECEIVE(){ ; }
	bool process(void);																				//�C�x���g����
	void setPacket(PACKET* ptr)	{ pPtr = ptr; }											//�p�P�b�g�I�u�W�F�N�g�̐ݒ�
	PACKET* getPacket(void)			{ return pPtr; }											//�p�P�b�g�I�u�W�F�N�g�̎擾
	short getObject(void)			{ return object; }
};

//UDP�N���X�i�G�[�W�F���g�N���X�̌p���j
class UDP:public AGENT{
	UDPSINK* objectPtr;																				//�Ή��V���N�I�u�W�F�N�g
	double rate;																						//���M���[�g
	int size;																							//�����M�T�C�Y
	int byte;																							//���M�ς݃T�C�Y
	int seq;																								//�V�[�P���X�ԍ�
public:
	UDP(SIMU* ptr, short nid, SIMUTIME tval, double rval, int sval):AGENT(ptr, nid, tval, EVENT::Udp)	//�R���X�g���N�^
	{ rate = rval, size = sval, byte = 0, seq = 0; }
	~UDP(){ ; }
	bool process(void);																				//�C�x���g����
	UDPSINK* getObject(void)		{ return objectPtr; }									//�V���N�I�u�W�F�N�g�̎擾
	void setRate(double val)		{ rate = val; }											//���M���[�g�̐ݒ�
	double getRate(void)				{ return rate; }											//���M���[�g�̎擾
	int getSize(void)					{ return size; }											//�����M�T�C�Y�̎擾
	int increByte(int val)			{ byte += val; return byte; }							//���M�ς݃T�C�Y�̐ݒ�
	int getByte(void)					{ return byte; }											//���M�ς݃T�C�Y�̎擾
	void increSeq(void)				{ seq++; }													//�V�[�P���X�ԍ��̃C���N�������g
	int getSeq(void)					{ return seq; }											//�V�[�P���X�ԍ��̎擾
	void connectSink(UDPSINK*);																	//SINK�Ƃ̌���
};

//UDP�V���N�N���X�i�G�[�W�F���g�N���X�̌p���j
class UDPSINK:public AGENT{
	UDP* objectPtr;																					//�Ή�UDP�I�u�W�F�N�g
	int byte;																							//��M�o�C�g
public:
	UDPSINK(SIMU* ptr, short nid):AGENT(ptr, nid, 0, EVENT::Sink){ byte = 0; }		//�R���X�g���N�^
	void setObject(UDP* val)		{ objectPtr = val; }										//�Ή�UDP�I�u�W�F�N�g�̐ݒ�
	UDP* getObject(void)				{ return objectPtr; }									//�Ή�UDP�I�u�W�F�N�g�̎擾
	int increByte(int val)			{ byte += val; return byte; }							//��M�o�C�g�̃C���N�������g
	int getByte(void)					{ return byte; }											//��M�o�C�g�̎擾
};

//�Z�O�����g�N���X�i�C�x���g�N���X�̌p���j
class SEGMENT:public EVENT{
	TCP* tcpPtr;																						//�Ή�TCP�I�u�W�F�N�g
	short size;																							//�T�C�Y
	int seq;																								//�V�[�P���X�ԍ�
	SIMUTIME sendStartTime;																			//���M�J�n����
	char ackRepeat;																					//�d��ACK��M��
	TIMEOUT* toPtr;																					//�^�C���A�E�g�I�u�W�F�N�g
	bool NAretrans;																					//�d��ACK��M�ɂ��đ��ς݃t���O
public:
	SEGMENT(SIMU*, SIMUTIME, TCP*, short, int, short);										//�R���X�g���N�^
	virtual ~SEGMENT();																				//�f�X�g���N�^
	bool process(void){ return true; }															//�C�x���g����
	void setTcp(TCP* ptr)			{ tcpPtr = ptr; }											//�Ή�TCP�I�u�W�F�N�g�̐ݒ�
	TCP* getTcp(void)					{ return tcpPtr; }										//�Ή�TCP�I�u�W�F�N�g�̎擾
	short getSize(void)				{ return size; }											//�T�C�Y�̎擾
	int getSeq(void)					{ return seq; }											//�V�[�P���X�ԍ��̎擾
	void setSendStart(SIMUTIME t) { sendStartTime = t; }									//���M�J�n�����̐ݒ�
	SIMUTIME getSendStart(void)	{ return sendStartTime; }								//���M�J�n�����̎擾
	void resetAckRepeat(void)		{ ackRepeat = 0; }										//�d��ACK��M�񐔂̏����� 
	void increAckRepeat(void)		{ ackRepeat++; }											//�d��ACK��M�񐔂̃C���N�������g 
	char getAckRepeat(void)			{ return ackRepeat; }									//�d��ACK��M�񐔂̎擾 
	void setTimeout(TIMEOUT* ptr)	{ toPtr = ptr; }											//�^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
	TIMEOUT* getTimeout(void)		{ return toPtr; }											//�^�C���A�E�g�I�u�W�F�N�g�̎擾
	void setNAretrans(bool flag)	{ NAretrans = flag; }									//�d��ACK��M�ɂ��đ��ς݃t���O�̐ݒ�
	bool getNAretrans(void)			{ return NAretrans; }									//�d��ACK��M�ɂ��đ��ς݃t���O�̎擾
};


////TCP�N���X�i�G�[�W�F���g�N���X�̌p���j
class TCP:public AGENT{
	TCPSINK* objectPtr;																				//�Ή�TCP�V���N�I�u�W�F�N�g
	int size;																							//�����M�T�C�Y
	int byte;																							//���M�ς݃o�C�g
	SIMUTIME startTime;																				//���M�J�n����
	SIMUTIME finishTime;																				//���M��������
	SIMUTIME rtt;																						//���E���h�g���b�v����
	SIMUTIME nowRtt;																					//���߃p�P�b�g�̃��E���h�g���b�v����
	double D;																							//�^�C���A�E�g�ݒ�p�΍�
	double windowSize;																				//�E�B���h�E�T�C�Y
	double windowThreshold;																			//�E�B���h�E����p臒l
	int makeSegNum;																					//���ɍ쐬����Z�O�����g�ԍ�
	int lastSendSeq;																					//���O�ɑ��M�����Z�O�����g�̃V�[�P���X�ԍ�
	int nextSendSeq;																					//���ɑ��M����Z�O�����g�̃V�[�P���X�ԍ�
	int lastReqSeq;																					//���O�̊��҃V�[�P���X�ԍ�																						
	bool buffering;																					//�p�b�P�g�̃o�b�t�@�����O���
	MA* maPtr;																							//���o�C���G�[�W�F���g�I�u�W�F�N�g
	bool abort;																							//���f�t���O
	bool finish;																						//�I���t���O
//	bool init;																							//�ΏۃZ�O�����g���擪���������t���O
//	int finishNum;																						//�E�B���h�E�x�N�^�ɑ��݂��鑗�M�I���Z�O�����g��
//	int windowNum;																						//�E�B���h�E�ɑ��݂���Z�O�����g��
//	int segPacketNum;																					//�Z�O�����g�p�P�b�g���iTCP�p�P�b�g�܂ށj
//	int sendSeq;																						//���ɑ��M�������s���Z�O�����g�̃V�[�P���X
//	int receiveSeq;																					//���ߎ�MACK�̑ΏۃV�[�P���X
//	int seq;																								//���ߑ��M�Z�O�����g�̃V�[�P���X�ԍ�
//	int id;																								//���ߎ�MACK�̃V�[�P���X�ԍ�
//	int ack;																								//��MACK�ԍ�
//	int lastack;																						//���O��MACK�ԍ�
//	bool timeOut;																						//�^�C���A�E�g�t���O
//	int ackRepeatNum;																					//����Z�O�����g�v��ACK�̐�
//	TIMEOUT* toPtr;																					//�^�C���A�E�g�I�u�W�F�N�g
//	bool finish;																						//�I���t���O
//	bool measurement;
public:
	TCP(SIMU*, short, SIMUTIME, int);															//�R���X�g���N�^
	~TCP();
	bool process(void);																					//�C�x���g����
	TCPSINK* getObject(void)		{ return objectPtr; }									//�Ή�TCP�V���N�I�u�W�F�N�g�̎擾
	int getSize(void)					{ return size; }											//�����M�T�C�Y�̎擾
	SIMUTIME getRtt(void)			{ return rtt; }											//���E���h�g���b�v�^�C���̎擾
	double getD(void)					{ return D; }												//���σ��E���h�g���b�v�^�C���̎擾
	void setLastSendSeq(int val)	{ lastSendSeq = val; }									//���O���M�Z�O�����g�V�[�P���X�ԍ��̐ݒ�
	void resetBuffer(void)			{ buffering = false; }									//�o�b�t�@�����O�t���O�̃��Z�b�g
	void setMA(MA* ptr)				{ maPtr = ptr; }											//���o�C���G�[�W�F���g�I�u�W�F�N�g�̐ݒ�
	MA* getMA(void)					{ return maPtr; }											//���o�C���G�[�W�F���g�I�u�W�F�N�g�̎擾
	void setAbort(void)				{ abort = true; }											//���f�t���O

//	void setInit(bool val)			{ init = val; }											//�C�j�V�����t���O�̐ݒ�
//	bool getInit(void)				{ return init; }											//�C�j�V�����t���O�̎擾
//	void setSize(int val)			{ size = val; }											//�����M�T�C�Y�̐ݒ�
//	int increByte(int val)			{ byte += val; return byte; }							//���M�ς݃o�C�g�̐ݒ�
//	int getByte(void)					{ return byte; }											//���M�ς݃o�C�g�̎擾
//	void setStartTime(double val)	{ startTime = val; }										//���M�J�n�����̐ݒ�
//	double getStartTime(void)		{ return startTime; }									//���M�J�n�����̎擾
//	void setFinishTime(double val){ finishTime = val; }									//���M�I�������̐ݒ�
//	double getFinishTime(void)		{ return finishTime; }									//���M�I�������̎擾
//	void setRtt(double val)			{ rtt = val; }												//���E���h�g���b�v�^�C���̐ݒ�
//	double getNowRtt(void)			{ return nowRtt; }										//���߃p�P�b�g�̃��E���h�g���b�v�^�C���̎擾
//	void setNowRtt(double val)		{ nowRtt = val; }											//���߃p�P�b�g�̃��E���h�g���b�v�^�C���̐ݒ�
//	void setD(double val)			{ D = val; }												//���σ��E���h�g���b�v�^�C���̐ݒ�
//	void setWindow(double val)		{ windowSize = val; }									//�E�B���h�E�T�C�Y�̐ݒ�
//	double getWindow(void)			{ return windowSize; }									//�E�B���h�E�T�C�Y�̎擾
//	void setWindowNum(int val)		{ windowNum = val; }										//�E�B���h�E�T�C�Y�̐ݒ�
//	int getWindowNum(void)			{ return windowNum; }									//�E�B���h�E�T�C�Y�̎擾
//	void decreSegPacketNum(void)	{ segPacketNum--; }										//�Z�O�����g�p�P�b�g���̃f�N�������g
//	int getSegPacketNum(void)		{ return segPacketNum; }
//	void setSendSeq(int val)		{ sendSeq = val; }										//���ɑ��M�������s���Z�O�����g�̃V�[�P���X�̐ݒ�
//	int getSendSeq(void)				{ return sendSeq; }										//���ɑ��M�������s���Z�O�����g�̃V�[�P���X�̎擾
//	void setThreshold(double val)	{ windowThreshold = val; }								//�E�B���h�E�T�C�Y�̐ݒ�
//	double getThreshold(void)		{ return windowThreshold; }							//�E�B���h�E�T�C�Y�̎擾
//	void increSeq(void)				{ seq++; }													//���ߑ��M�Z�O�����g�V�[�P���X�ԍ��̃C���N�������g
//	int getSeq(void)					{ return seq; }											//���ߑ��M�Z�O�����g�V�[�P���X�ԍ��̎擾
//	void setId(int val)				{ id = val; }												//���ߎ�MACK�̃V�[�P���X�ԍ��̐ݒ�
//	int getId(void)					{ return id; }												//���ߎ�MACK�̃V�[�P���X�ԍ��̎擾
//	void setAck(int val)				{ ack = val; }												//��MACK�ԍ��̐ݒ�
//	int getAck(void)					{ return ack; }											//��MACK�ԍ��̎擾
//	void setLastack(int val)		{ lastack = val; }										//���O��MACK�ԍ��̐ݒ�
//	int getLastack(void)				{ return lastack; }										//���O��MACK�ԍ��̎擾
//	void setTimeOut(bool val)		{ timeOut = val; }										//�^�C���A�E�g�t���O�̐ݒ�
//	bool getTimeOut(void)			{ return timeOut; }										//�^�C���A�E�g�t���O�̎擾
//	void initAckRepeat(void)		{ ackRepeatNum = 1; }									//����Z�O�����g�v��ACK���̏�����
//	void increAckRepeat(void)		{ ackRepeatNum++; }										//����Z�O�����g�v��ACK���̃C���N�������g
//	int getAckRepeat(void)			{ return ackRepeatNum; }								//����Z�O�����g�v��ACK���̎擾						
//	void setTObject(TIMEOUT* ptr)	{ toPtr = ptr; }											//�^�C���A�E�g�I�u�W�F�N�g�̐ݒ�
//	TIMEOUT* getTObject(void)		{ return toPtr; }											//�^�C���A�E�g�I�u�W�F�N�g�̎擾
//	void setFinish(void)				{ finish = true; }										//�I���t���O�̐ݒ�
//	bool getFinish(void)				{ return finish; }										//�I���t���O�̎擾
//	void setMeasurement(void)		{ measurement = true; }									//���f�t���O�̐ݒ�
//	bool getMeasurement(void)		{ return measurement; }									//���f�t���O�̎擾
	void connectSink(TCPSINK*);																	//SINK�Ƃ̌���
	void makeSegment(void);																			//�Z�O�����g�쐬
	void makePacket(void);																			//���M�p�P�b�g�쐬
	void getTcpAck(PACKET*);																		//ACK��M����
	void retransmission(void);																		//�đ�
	void abortProcess(NODE*);																		//TCP�̒��f����

//	int renewWindow(LIST<EVENT>*, NODE*);														//�t�s�E�B���h�E�̍X�V
//	bool checkWindow(void);																			//�t�s�E�B���h�E�`�F�b�N
//	int eraseSegPacket(NODE*, int, int);																//�Z�O�����g�p�P�b�g�̍폜
//	void windowShow(void);																			//�t�s�E�B���h�E�̕\��
	vector<SEGMENT*> window;																		//���M�E�B���h�E
	vector<SEGMENT*> segCash;																		//�Z�O�����g�L���b�V��
};

//TCP�V���N�I�u�W�F�N�g�N���X�i�G�[�W�F���g�N���X�̌p���j
class TCPSINK:public AGENT{
	TCP* objectPtr;																					//�Ή�TCP�I�u�W�F�N�g
	int byte;																							//��M�o�C�g
	int lastSeq;																						//���ߎ�M�Z�O�����g�̃V�[�P���X�ԍ�
public:
	TCPSINK(SIMU* ptr, short nid):AGENT(ptr, nid, 0, EVENT::Sink){ byte = 0, lastSeq = -1; }			//�R���X�g���N�^
	bool process(void){ return true; }										//�C�x���g����
	void setObject(TCP* val)		{ objectPtr = val; }										//�Ή�TCP�I�u�W�F�N�g�̐ݒ�
//	TCP* getObject(void)				{ return objectPtr; }									//�Ή�TCP�I�u�W�F�N�g�̎擾
//	double increByte(double val)	{ byte += val; return byte; }							//��M�o�C�g�̃C���N�������g
	int getByte(void)					{ return byte; }											//��M�o�C�g�̎擾
//	void setLastseq(int val)		{ lastseq = val; }										//���ߎ�M�Z�O�����g�̃V�[�P���X�ԍ��̐ݒ�
//	int getLastseq(void)				{ return lastseq; }										//���ߎ�M�Z�O�����g�̃V�[�P���X�ԍ��̎擾
	void sendAck(PACKET*);																			//TCPACK���M����
	vector<int> cashSeq;																				//��M�s�A���Z�O�����g�̃V�[�P���X�L���b�V��
};

////MAC�N���X�i�C�x���g�N���X�̌p���j
class MAC:public EVENT{
public:
//	enum castType {Uni, Broad};
	enum stateType {Idle, Difs, Backoff, Rts, Cts, 
		Waitcts, Data, BData, Waitdata, Ack, Waitack, Nav, NavFin, WaitOrAck, WaitOtherAck, OrAck};		//�t���[���̏�Ԏ��ʎq�i�񋓁j
private:
	int id;																								//�m�[�h�ԍ�
	stateType state;																					//�m�[�h�̏��
	short objectId;																					//�Ώۑ���m�[�h
	char rtsTime;																						//RTS�M�����M���v����
	char ctsTime;																						//CTS�M�����M���v����
	short dataTime;																					//�f�[�^���M���v����
	char ackTime;																						//ACK�M�����M���v����
	FRAME* framePtr;																					//�����Ώۃt���[���I�u�W�F�N�g
	short contWindow;																					//�R���e���V�����E�B���h�E�T�C�Y
	SIMUTIME backoffTime;																			//�o�b�N�I�t����
	char retrans;																						//�đ���
	bool listInsert;																					//�C�x���g���X�g�ɓo�^����Ă��邩�������t���O
	bool orFlag;																						//OR�����邩�������t���O
	PACKET* orPtr;																						//OR�p�p�P�b�g
	short orSource;																					//OR�����̏ꍇ�̖{���̑��M���m�[�h
	short orDest;																						//OR�����̏ꍇ�̖{���̑��M����m�[�h
public:
	MAC(SIMU*, SIMUTIME, short);																	//�R���X�g���N�^
//	~MAC();																								//�f�X�g���N�^
	void show(double val);																			//��Ԃ̕\��
	bool process(void);																				//�C�x���g����
	void setState(stateType val)		{ state = val; }										//�m�[�h��Ԃ̐ݒ�
	stateType getState(void)			{ return state; }										//�m�[�h��Ԃ̎擾
	void setObject(short id)			{ objectId = id; }									//�Ώۑ���m�[�h�̐ݒ�
	short getObject(void)				{ return objectId; }									//�Ώۑ���m�[�h�̎擾
	char getRtsTime(void)				{ return rtsTime; }									//RTS�M�����M���v���Ԃ̎擾
	char getCtsTime(void)				{ return ctsTime; }									//CTS�M�����M���v���Ԃ̎擾 
	short getDatatime(void)				{ return dataTime; }									//�f�[�^���M���v���Ԃ̎擾
	char getAckTime(void)				{ return ctsTime; }									//ACK�M�����M���v���Ԃ̎擾 
	void setFrame(FRAME* ptr)        { framePtr = ptr; }									//�����Ώۃt���[���I�u�W�F�N�g�̐ݒ�
	FRAME* getFrame(void)				{ return framePtr; }									//�����Ώۃt���[���I�u�W�F�N�g�̎擾
	void setBackoff(SIMUTIME t)		{ backoffTime = t; }
	SIMUTIME getBackoff(void)			{ return backoffTime; }
	void increRetrans(void)				{ retrans++; }											//�đ��񐔂̃C���N�������g
	char getRetrans(void)				{ return retrans; }									//�đ��񐔂̎擾
	void setListInsert(bool flag)		{ listInsert = flag; }								//���X�g�o�^�t���O�̐ݒ�
	bool getListInsert(void)			{ return listInsert; }								//���X�g�o�^�t���O�̎擾
	void setOrFlag(bool flag)			{ orFlag = flag; }									//OR�p�t���O�̐ݒ�
	bool getOrFlag(void)					{ return orFlag; }									//OR�p�t���O�̎擾
	PACKET* getOrPtr(void)				{ return orPtr; }										//OR�p�p�P�b�g�̎擾
	void setOrSource(short id)			{ orSource = id; }									//OR�p���M���m�[�h�̐ݒ�
	short getOrSource(void)				{ return orSource; }									//OR�p���M���m�[�h�̎擾
	void setOrDest(short id)			{ orDest = id; }										//OR�p���M����m�[�h�̐ݒ�
	short getOrDest(void)				{ return orDest; }									//OR�p���M����m�[�h�̎擾

	void calcDataTime(FRAME*);																		//�f�[�^�t���[���̑��M���p����
	void receiveSignal(SIGNAL*, MAC*);															//�M����M����
	void sendFinSignal(SIGNAL*, MAC*);															//�M�����M��������
	void makeBackoff(SIMUTIME);																	//�o�b�N�I�t���Ԃ̐ݒ�
	bool judgeSendable(void);																		//���M�\���ǂ����̔��f
	void reset(void);																					//���M�����̃��Z�b�g
	void checkFrame(void);																			//�t���[���̑��݃`�F�b�N
	bool checkIdling(void);																			//�M�����x���ł̃A�C�h�����O�`�F�b�N
	bool judgeOpportunistic(MAC*);																//OR�����邩�̔��f
	vector<int> last;																					//���ߎ�M�t���[��																				
};

//�o�b�t�@�N���X
class BUFFER{
	int size;																							//�p�P�b�g�o�b�t�@�T�C�Y
	int length;																							//�p�P�b�g�s��
	short segLength;																					//�Z�O�����g�s��
public:
	BUFFER(int val)					{ size = val, length = 0, segLength; }				//�R���X�g���N�^
	int getSize(void)					{ return size; }											//�T�C�Y�̎擾
	void increLength(int val)		{ length += val; }										//�s�񒷂̃C���N�������g
	void decreLength(int val)		{ length -= val; }										//�s�񒷂̃f�N�������g
	int getLength(void)				{ return length; }										//�s�񒷂̎擾
	vector<PACKET*> queue;																			//�p�P�b�g�p�o�b�t�@�{��
	vector<SEGMENT*> squeue;																		//TCP�Z�O�����g�p�o�b�t�@�{��
};

//���[�e�B���O�f�[�^�N���X
class ROUTING_DATA{
	short next;																							//�]����
	char hop;																								
	int seq;
	TIMEOUT* toPtr;
public:
	ROUTING_DATA(short nval, char hval){ next = nval, hop = hval, seq = -1; }
	void setNext(short val)			{ next = val; }
	short getNext(void)				{ return next; }
	void setHop(char val)			{ hop = val; }
	char getHop(void)					{ return hop; }
	void setSeq(int val)				{ seq = val; }
	int increSeq(void)				{ seq++; return seq; }
	int getSeq(void)					{ return seq; }
	void setTimeout(TIMEOUT* ptr)	{ toPtr = ptr; }
	TIMEOUT* getTimeout(void)		{ return toPtr; }
};


//�m�[�h�N���X�i�C�x���g�N���X�̌p���j
class NODE:public EVENT{
public:
	enum move {NO, RWP, GRID};																		//�ړ��^�C�v���ʎq
	enum route {PRO, DSR, AODV, GEDIR, MAMR};														//���[�e�B���O�^�C�v���ʎq
																									//MAMR�̓��o�C���G�[�W�F���g���p�^�}���`�p�X���[�e�B���O
	enum powerMode { SLEEP, WAIT, SEND, RECEIVE};											//����d�͌v�Z�p��Ԏ��ʎq
	enum mrgroup { S_PATH, M_PATH1, M_PATH2, NONE}  ;										  //�}���`�p�X�������̃O���[�v

private:
	BUFFER* bufferPtr;																				//�o�b�t�@�I�u�W�F�N�g
	MAC* mPtr;																							//MAC�I�u�W�F�N�g
	CHANNEL* cPtr;																						//�`���l���I�u�W�F�N�g
	SIMUTIME now;																						//���ݎ���
	short id;																							//���ʎq
	bool isActive;																						//�g�p��
	SIMUTIME activeTime;																				//�m�[�h�����J�n����
	SIMUTIME offTime;																					//�m�[�h������~����
	LOCATION position;																				//���݈ʒu
	LOCATION derivePos[2];																			//����ʒu
	char deriveTime;																					//����^�C�~���O
	bool neighborLocEnable;																			//�אڃm�[�h����ʎ�i�Ŏ擾�ł��邩��\���t���O	
	LOCATION moveDestPos;																			//�ړ��ړI�n
	short destId;																						//�ړI�n�̕�ID�iSQUAR�CGRID�G���A���f���̂ݎg�p�j
	short gridId;																						//�ړ����_ID�iGRID�ړ��^�C�v�̂ݎg�p�j
	double speed;																						//���x
	SIMUTIME  pause;																					//��~����
	SIMUTIME  pauseReleaseTime;																	//��~��������
	move moveId;																						//�ړ��^�C�v
	bool isRecieveingSignal;																		//�M����M���ł��邩�������t���O																						//																					
	route routeId;																						//���[�e�B���O�^�C�v
	short mapId;																						//�ڑ�MAPID
	int seq;																								//�V�[�P���X�ԍ�
	int aodvSeq;																						//AODV�p�V�[�P���X�ԍ�
	int overflow;																						//�I�[�o�t���[�ɂ����p�t���[����
	LAB a;	
	double remainPower;																				//�c���d��
	double usedPower;																					//����d��
	powerMode pmode;																					//����d�͌v�Z�p���
	SIMUTIME calcPowerTime;																			//����d�͌v�Z�p����
	LOCATION lastInformPos;																			//�ŋߏ��L�������ʒu
	
	mrgroup mgroup;    //�}���`�p�X�������̃O���[�v

//	int block;																							//�ʐM�ؒf�ɂ����p�t���[����
//	int retransfail;																					//�đ�����ɂ����p�t���[����
//	ANTENNAE* antPtr;																					//�A���e�i�I�u�W�F�N�g
//	double powerDecreTime;																			//����d�͌v�Z�J�n����
//	double receivePower;																				//��M�d�́i����M�̏ꍇ0�j
//	double relayRate;																					//���p���ۗ��i�ʏ�m�[�h�Ȃ�0�j
//	vector<NODESET*> neighbor;																		//�אڃm�[�h�I�u�W�F�N�g
public:
//	NODE(LIST<EVENT>*, short, double, double, move, LOCATION, route, char, ANTENNAE::antenna, double, bool = false);//�R���X�g���N�^
	NODE(SIMU*, short, SIMUTIME, SIMUTIME, LOCATION, move, route);
	~NODE();																								//�f�X�g���N�^
	bool process(void);																				//�C�x���g����
	BUFFER* getBuffer(void)				{ return bufferPtr; }								//�o�b�t�@�I�u�W�F�N�g�̎擾
	MAC* getMAC(void)						{ return mPtr; }										//MAC�I�u�W�F�N�g�̎擾
	CHANNEL* getChannel(void)			{ return cPtr; }										//�`���l���I�u�W�F�N�g�̎擾
	void setNow(SIMUTIME t)				{ now = t; }											//���ݎ����̐ݒ�
	short getId(void)						{ return id; }											//���ʎq�̎擾
	void setIsActive(bool flag)		{ isActive = flag; }									//�g�p�󋵂̐ݒ�
	bool getIsActive(void)				{ return isActive; }									//�g�p�󋵂̎擾
	void setActiveTime(SIMUTIME tval){ activeTime = tval; }								//�m�[�h�����J�n�����̐ݒ�
	SIMUTIME getActiveTime(void)		{ return activeTime; }								//�m�[�h�����J�n�����̎擾
	void setOffTime(SIMUTIME tval)	{ offTime = tval; }									//�m�[�h�����I�������̐ݒ�
	SIMUTIME getOffTime(void)			{ return offTime; }									//�m�[�h�����I�������̎擾
	LOCATION getPos(void)				{ return position; }									//���݈ʒu�̎擾
	LOCATION getDerivePos(int val)	{ return derivePos[val]; }							//����ʒu�̎擾
	char getDeriveTime(void)			{ return deriveTime; }								//����^�C�~���O�̎擾
	void setNeighLocEnable(bool flag){ neighborLocEnable = flag; }						//�אڃm�[�h���擾�t���O�̐ݒ�
	bool getNeighborLocEnable(void)	{ return neighborLocEnable; }						//�אڃm�[�h���擾�t���O�̎擾
	SIMUTIME getPause(void)				{ return pause; }										//��~���Ԃ̎擾
	SIMUTIME getPauseRelease(void)	{ return pauseReleaseTime; }						//��~���������̎擾
	move getMove(void)					{ return moveId; }									//�ړ��^�C�v�̎擾
	void setReceiveSignal(bool flag)	{ isRecieveingSignal = flag; }					//��M���t���O�̐ݒ�
	bool getReceiveSignal(void)		{ return isRecieveingSignal; }					//��M���t���O�̎擾
	route getRoute(void)					{ return routeId; }									//���[�e�B���O�^�C�v�̎擾
	void setMAP(short id)				{ mapId = id; }										//�ڑ�MAP�̐ݒ�
	short getMAP(void)					{ return mapId; }										//�ڑ�MAP�̎擾
	void increSeq(void)					{ seq++; }												//�V�[�P���X�ԍ��̃C���N�������g
	int getSeq(void)						{ return seq; }										//�V�[�P���X�ԍ��̎擾
	void increAodvSeq(void)				{ aodvSeq++; }											//AODV�p�V�[�P���X�ԍ��̃C���N�������g
	int getAodvSeq(void)					{ return aodvSeq; }									//AODV�p�V�[�P���X�ԍ��̎擾	
	void increOverflow(void)			{ overflow++; }										//�đ�����ɂ����p�t���[�����̃C���N�������g
	int getOverflow(void)				{ return overflow; }									//�đ�����ɂ����p�t���[�����̎擾
	void setPowerMode(powerMode val)	{ pmode = val; }										//����d�͌v�Z�p��Ԃ̐ݒ�
	double getRemainPower(void)		{ return remainPower; }								//�c���d�͂̎擾
	double getUsedPower(void)			{ return usedPower; }								//����d�͂̎擾
	void setCalcPTime(SIMUTIME t)		{ calcPowerTime = t; }								//����d�͌v�Z�p�����̐ݒ�	
	SIMUTIME getCalcPTime(void)		{ return calcPowerTime; }							//����d�͌v�Z�p�����̎擾	
	void	setLastInform(LOCATION pos){ lastInformPos = pos; }
	LOCATION	getLastInform(void)		{ return lastInformPos; }
//	void initBlock(void)					{ block = 0; }											//�ʐM�ؒf�ɂ����p�t���[�����̏�����
//	void increBlock(void)				{ block++; }											//�ʐM�ؒf�ɂ����p�t���[�����̃C���N�������g
//	int getBlock(void)					{ return block; }										//�ʐM�ؒf�ɂ����p�t���[�����̎擾
//	void increRetransfail(void)		{ retransfail++; }									//�đ�����ɂ����p�t���[�����̃C���N�������g
//	int getRetransfail(void)			{ return retransfail; }								//�đ�����ɂ����p�t���[�����̎擾
//	ANTENNAE* getAntenna(void)			{ return antPtr; }									//�A���e�i�I�u�W�F�N�g�̎擾
//	void setRxPower(double val)		{ receivePower = val; }								//��M�d�͂̐ݒ�
//	double getRxPower(void)				{ return receivePower; }							//��M�d�͂̎擾
//	void setRelayRate(double val)		{ relayRate = val; }									//���p���ۗ��̐ݒ�
//	double getRelayRate(void)			{ return relayRate; }								//���p���ۗ��̎擾
//	vector<NODESET*> getNeighbor(void){ return neighbor; }								//�אڃm�[�h�I�u�W�F�N�g�x�N�^�̎擾

	void posInit(void);																				//�m�[�h�z�u
	void replace(short);																				//�m�[�h�Ĕz�u
	void posDerive(void);																			//�ʒu���̎擾
	void destInit(void);																				//�ړI�n�̏�����
	void speedSet(void);																				//���x�̐ݒ�
	bool toDestination(LOCATION);																	//�ړI�n�ւ̈ړ�����
	void noMove(void);																				//��ړ������i�����X�V���j
	void randomWayPoint(void);																		//�����_���E�F�C�|�C���g�̈ړ�����
	void gridWayPoint(void);																		//�i�q�ړ�����
	void calcDistance(void);																		//�m�[�h�ԋ����̌���
	void checkPacket(void);																			//���M�p�P�b�g�����݂��邩�̃`�F�b�N
	PACKET* relayPacket(PACKET*, short);														//�p�P�b�g���p����
	void sendError(PACKET*);																		//���M�G���[����
	void queueShow(void);																			//�o�b�t�@���̃p�P�b�g�\��
	bool migrationCheck();																			//MA�̈ړ����K�v���̃`�F�b�N
	void sendLab(short, short);																	//LAB���̑��M
	void makeRoutingTable(short);																	//���[�e�B���O�e�[�u���쐬
	void calcPower(void);																			//����d�͌v�Z
	void sendInfoemLoc(void);																		//�ʒu���p�P�b�g�̑��M

	void showPos(void)			{ cout << "(" << position.getX() << "," << position.getY() << ")" 
		<< "\tdest " <<  "(" << moveDestPos.getX() << "," << moveDestPos.getY() << ")" << endl; }//�ʒu�\��
//	void pathShow(_NODE, int);																		//�o�H�\��

	void setGroupe(mrgroup val)					{ mgroup = val; }								//�O���[�v��ݒ�
	short getGroupe(void)						{ return mgroup; }							//�O���[�v���擾
	void showGroupe(void){																		//�O���[�v��\��
		if(mgroup == NONE ) cout << "group is none" << endl;
		else if(mgroup == M_PATH1) cout << "group is multipath1" << endl;
		else if(mgroup == M_PATH2) cout << "group is multipath2" << endl;
		else if(mgroup == S_PATH) cout << "group is singlepath" << endl;
		else cout << "??? LOCATION showGroupe at class.h" << endl,exit(1);
	}
	

//
//	//_NODESET conNode;																					//�אڃm�[�h
//	//_NODESET carNode;																					//�L�����A�Z���X�m�[�h
	class NEIGHBOR_LIST{																				//�אڃm�[�h���N���X
		short id;																							//�m�[�hID
		double distance;																					//����
	public:
		NEIGHBOR_LIST(short ival, double dval){ id = ival, distance = dval; }							//�R���X�g���N�^
//		~NEIGHBOR_LIST(){ cout << "bad " << this << endl; }
		short getId(void)						{ return id; }											//�m�[�hID�̎擾
		double getDist(void)					{ return distance; }									//�����̎擾
	};
	vector<NEIGHBOR_LIST> neighborList;															//�אڃm�[�h���X�g

	class RECEIVED_PACKET{
		char type;
		short source;
		short dest;
		int seq;
		SIMUTIME sendTime;
	public:
		RECEIVED_PACKET(char t, short s, short d, int sq, SIMUTIME tm){ type = t, source = s, dest = d, seq = sq, sendTime = tm; } 
		char getType(void)					{ return type; }
		short getSource(void)				{ return source; }
		short getDest(void)					{ return dest; }
		int getSeq(void)					{ return seq; }
		SIMUTIME getTime(void)				{ return sendTime; }
	};
	vector<RECEIVED_PACKET> receivedPacketList;

	vector<ROUTING_DATA*> routing;																//���[�e�B���O�e�[�u��
	vector<LOCATION> nodePos;																		//�m�[�h�ʒu���
	vector<vector<short>> path;																	//DSR�p�o�H���@�@//MA���p�^�̃}���`�p�X���[�e�B���O�ł̓V���O���p�X�p�ɂȂ�
	vector<vector<short>> path1;																//�}���`�p�X�p�P
	vector<vector<short>> path2;																//�}���`�p�X�p�Q
	vector<short> pathNum;																		//�o�H���f�p
	vector<short> path1SegSize;																				//path1�̌��݂̃Z�O�����g�T�C�Y
	vector<short> path2SegSize;																				//path2�̌��݂̃Z�O�����g�T�C�Y
	vector<SIMUTIME> requestTime;																	//�e�m�[�h���Ă̌o�H�T���p�P�b�g���M����
	vector<int> floodSeq;																			//�e�m�[�h����̃t���b�f�B���O�V�[�P���X
	vector<MA*> ma;																					//�ێ������o�C���G�[�W�F���g
	vector<LAB> gab;
};

//�V�~�����[�V�����N���X
class SIMU{
public:
	enum area { CIRCLE, SQUARE, GRID, MESH };
	enum mesh { IDEAL, NONE, CENTRAL, RAOLSR, NEIGHBOR, AGENT };
	enum mac {NORMAL, OR_JOJI_ACK, OR_JOJI_RTS};												//MAC�̎��
private:
	SIMUTIME now;																						//���ݎ���
	SIMUTIME last;																						//�O�񏈗�����
	area areaId;																						//�V�~�����[�V�����G���A�^�C�v
	mesh meshId;																						//���b�V������
	mac macId;																							//MAC�^�C�v
	int areaSize;																						//�G���A�T�C�Y
	char gridLine;																						//�i�q����
	LOCATION gridPoint[GRIDNUM + 1][GRIDNUM + 1];											//�i�q�̌����_���W
	short mapNum;																						//MAP��
	short staNum;																						//STA��
	short interval;																					//�������ԁi���b�V���l�b�g���[�N�Ŏg�p: �P�ʕb�j
	double staReqRate;																				//STA�v���������[�g
	short staReqNum;																					//STA�v�����i���b�V���l�b�g���[�N�Ŏg�p�j
	short staReqFail;																					//�v�����s��
	double packetSize[50];																			//�����M�p�P�b�g�T�C�Y�i�p�P�b�g�̎�ʂ��Ɓj
	double delay;																						//�o�H�\�z�x��
	double transDelay;																				//���M�x��
	int transDelayCnt;																				//���M�x���v���p�P�b�g��
	short centerId;																					//�G���A�̍Œ��S�m�[�h���W
	int totalSendUDP;																					//�����MUDP�f�[�^
	int totalReceiveUDP;																				//����MUDP�f�[�^
	int totalTcpData;																					//��TCP�f�[�^
	SIMUTIME totalTcpTime;																			//��TCP�ʐM����																					//
	bool MAlocation;																					//MA���ʒu���Ǘ����s������\���t���O
	
	double rate; 
public:
	SIMU(area, mac = NORMAL); 																						//�R���X�g���N�^
	~SIMU();																								//�f�X�g���N�^
	LIST<EVENT> list;																					//�C�x���g���X�g�I�u�W�F�N�g
	void processEvent();																				//�C�x���g����
	SIMUTIME getNow(void)	{ return now; }													//���ݎ����̎擾
	area getArea(void)		{ return areaId; }												//�G���A�^�C�v�̎擾
	mesh getMesh(void)		{ return meshId; }												//���b�V�������̎擾
	mac getMac(void)			{ return macId; }													//MAC�^�C�v�̎擾
	int getAreaSize(void)	{ return areaSize; }												//�G���A�T�C�Y�̎擾
	char getGridLine(void)	{ return gridLine; }												//�i�q�����̎擾
	LOCATION getGridPoint(char x, char y)	{ return gridPoint[x][y]; }
	short getMAP(void)		{ return mapNum; }												//MAP���̎擾
	short getSTA(void)		{ return staNum; }												//STA���̎擾
	short getInterval(void)	{ return interval; }												//�����Ԋu�̎擾
	double getReqRate(void)	{ return staReqRate; }											//STA�v���������[�g�̎擾
	void increReqNum(void)	{ staReqNum++; }													//STA�v���������̃C���N�������g
	short getReqNum(void)	{ return staReqNum; }											//STA�v�����̎擾
	void increReqFail(void)	{ staReqFail++; }													//�v�����s�񐔂̃C���N�������g
	short getReqFail(void)	{ return staReqFail; }											//�v�����s�񐔂̎擾
	void increPacket(char id, short size){ packetSize[id] += size; }					//�����M�p�P�b�g�T�C�Y�̃C���N�������g
	double getPacket(char id)	{ return packetSize[id]; }									//�����M�p�P�b�g�T�C�Y�̎擾
	void increDelay(double val){ delay += val; }												//�x���̃C���N�������g
	double getDlay(void)		{ return delay; }													//�x���̎擾
	void increTransDelay(SIMUTIME t)	{ transDelay = transDelay + t.dtime(); }		//�]���x���̃C���N�������g
	double getTransDelay(void)	{ return transDelay; }										//�]���x���̎擾
	void increTransDelayCnt(void)	{ transDelayCnt++; }										//�]���x���v���p�P�b�g���̃C���N�������g
	int getTransDelayCnt(void)	{ return transDelayCnt; }									//�]���x���v���p�P�b�g���̎擾
	short getCenter(void)	{ return centerId; }												//���S�m�[�h�̎擾
	void increSendUdp(int val) { totalSendUDP += val; }									//�����MUDP�f�[�^�̃C���N�������g
	int getSendUdp(void)	{ return totalSendUDP; }											//�����MUDP�f�[�^�̎擾
	void increReceiveUdp(int val) { totalReceiveUDP += val; }							//����MUDP�f�[�^�̃C���N�������g
	int getReceiveUdp(void)	{ return totalReceiveUDP; }									//����MUDP�f�[�^�̎擾
	void increTcpData(int val)	{ totalTcpData += val; }									//��TCP�f�[�^�̃C���N�������g
	int getTcpData(void)		{ return totalTcpData; }										//��TCP�f�[�^�̎擾
	void increTcpTime(SIMUTIME t) { totalTcpTime = totalTcpTime + t; }				//��TCP�ʐM���Ԃ̍X�V
	SIMUTIME getTcpTime(void){ return totalTcpTime; }										//��TCP�ʐM���Ԃ̎擾
	void setMAloc(void)			{ MAlocation = true; }										//MA���ʒu���Ǘ������邽�߂̃t���O�𗧂Ă�
	bool getMAloc(void)			{ return MAlocation; }										//MA���ʒu���Ǘ������邩�\���t���O�̎擾

	void setRate(double val)	{ rate = val; }
	double getRate(void)			{ return rate; }

	void newNode(SIMUTIME, SIMUTIME, LOCATION, NODE::move, NODE::route);				//�m�[�h�̍쐬
	void makeStaReq(short, double);																//STA�v���̍쐬
	void makeMesh(mesh, short, short, double, SIMUTIME);												//���b�V���l�b�g���[�N�̍쐬
	void checkConnectMap(void);																	//�ڑ�MAP�̊m�F
	
	_NODE node;
	vector<MA*> ma;																					//���ݒ����o�C���G�[�W�F���g
	vector<LAB> gab;

	int counter1, counter2;
};
