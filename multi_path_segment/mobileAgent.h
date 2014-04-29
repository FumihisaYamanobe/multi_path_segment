
#define MASIZE 30
#define MARANGE 50 * 100																				//MA�G���A���a
#define MACHECKINTERVAL 1000000																		//MA�ړ�����Ԋu(musec)
#define NOMIGRATION 50 * 100																			//��ړ��G���A���a
//
//#define DISSEMINATION_SIZE 10240
//#define DISSEMINATION_INTERVAL 1 * 1000000.0
//
#define MIGREQ 24																						//MA�ړ��v���p�P�b�g�T�C�Y
#define MIGREP 48																						//MA�ړ������p�P�b�g�T�C�Y

#define MAREQTTL 100000																					//MA�ړ��v���̃^�C���A�E�g����
//
//
//class MA;
//
////���z�M�N���X
//class DISSEMINATION:public EVENT{
//	MA* maPtr;
//	int size;
//public:
//	DISSEMINATION(double, int, MA*, LIST<EVENT>*);
//	bool process(LIST<EVENT>*, _NODE);															//�C�x���g����
//};
//
//MA�pMIGREP�p�P�b�g�N���X
class MAREP{
	LOCATION position;																				//�ʒu
	short difX;																							//�ړ����i�����W�j
	short difY;																							//�ړ����i�����W�j
	SIMUTIME interval;																					//����Ԋu
	double power;																						//�c���d��
	MA* maPtr;																							//�Ή�MA�I�u�W�F�N�g
public:
	MAREP(LOCATION loc, int dx, int dy, SIMUTIME val, double p, MA* ma)
	{ position = loc, difX = dx, difY = dy, interval = val, power = p, maPtr = ma; }
	LOCATION getPos(void)	{ return position; }
	int getDifX(void)			{ return difX; }
	int getDifY(void)			{ return difY; }
	SIMUTIME getInterval(void)	{ return interval; }
	double getPower(void)		{ return power; }
	MA* getMa(void)				{ return maPtr; }
};	
//
//typedef vector<MA*>::iterator _MA;
//


class MA:public EVENT{
public:
	enum mtype {Distance, Predict, PredictDistance};	//�C�x���g�^�C�v���ʎq
private:
	short id;																							//ID
	LOCATION center;																					//���S�ʒu
	double radius;																						//���a
	int size;																							//�f�[�^�Ƃ��ẴT�C�Y
	short migrationNumber;																			//�ړ���
	bool migrationFlag;
//	double outTime;
	TCP* tPtr;																							//�ړ��̍ۂ�TCP�I�u�W�F�N�g
	TIMEOUT* toPtr;																					//�ړ��v���^�C���A�E�g�I�u�W�F�N�g
	short candidate;																					//�ړ�����
	double myDist;
	double candDist;																					//�ړ�����̒��S����̋���
//	double candSurvive;
	mtype mode;															
	short meshRouteOrder;																			//���b�V���l�b�g���[�N����o�H�̏����ԍ�
	SIMUTIME stayTime;																				//�؍ݎ���														
public:
	MA(SIMU*, SIMUTIME, LOCATION, double, mtype, short, short);
	MA(SIMU*);
	bool process(void);																				//�C�x���g����

	short getId(void)				{ return id; }													//MA��ID�̎擾
	LOCATION getCenter(void)	{ return center; }											//���S�ʒu�̎擾
	void setSize(int val)		{ size = val; }												//�T�C�Y�̐ݒ�
	int getSize(void)				{ return size; }												//�T�C�Y�̎擾
	void setStayTime(SIMUTIME tval){ stayTime = tval; }									//�؍ݎ��Ԃ̐ݒ�
	SIMUTIME getStayTime(void)	{ return stayTime; }											//�؍ݎ��Ԃ̎擾
	short increRouteOrder(void){ return ++meshRouteOrder; }								//����o�H�̏����ԍ��̃C���N�������g
	void resetRouteOrder(void) { meshRouteOrder = 0; }										//����o�H�̏����ԍ��̏�����
	short getRouteOrder(void)	{ return meshRouteOrder; }									//����o�H�̏����ԍ��̎擾
	void increMigNum(void)		{ migrationNumber++; }										//�ړ��񐔂̃C���N�������g
	short getMigNum(void)		{ return migrationNumber; }								//�ړ��񐔂̎擾
	void resetMigration(void)	{ migrationFlag = false; }									//�ړ����t���O�̏�����
	bool getMigration(void)		{ return migrationFlag; }									//�ړ����t���O�擾
	TCP* getTCP(void)				{ return tPtr; }												//�ړ��pTCP�I�u�W�F�N�g�̎擾
	TIMEOUT* getTimeout(void)	{ return toPtr; }												//�^�C���A�E�g�I�u�W�F�N�g�̎擾
	short getCandidate(void)	{ return candidate; }										//�ړ����̎擾

//	void increMigNum(void)		{ migrationNumber++; }
//	int getMigNum(void)			{ return migrationNumber; }
//	double getOutTime(void)		{ return outTime; }
//	void setTCP(TCP* ptr)		{ tPtr = ptr; }
//	void setTimeout(TIMEOUT* ptr)	{ toPtr = ptr; }
//	mtype getMode(void)		{ return mode; }
	
	void migration(void);
	void decideMigratingNode(short, MAREP*);

	PACKET* makeMultiRoute(SIMU*, short, short, SIMUTIME, PACKET*, short);
	void grouping(SIMU*, short, short);

	void exchangeGab(void);
	
	vector<LAB> gab;																					//GAB���
	vector<short> meshRoute;																		//���b�V���p����G�[�W�F���g�̌o�H���
	vector<LOCATION> nodePos;																		//�m�[�h�̈ʒu���(v7����nowPos)
	vector<LOCATION> estimatePos;																	//�m�[�h�̑��ݐ���ꏊ
	vector<LOCATION> lastPos;																		//�m�[�h���O�����ꏊ
	vector<int> route;
	vector<int> route1;
	vector<int> route2;
};

//
//
