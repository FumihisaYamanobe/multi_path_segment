
#define MASIZE 30
#define MARANGE 50 * 100																				//MAエリア半径
#define MACHECKINTERVAL 1000000																		//MA移動判定間隔(musec)
#define NOMIGRATION 50 * 100																			//非移動エリア半径
//
//#define DISSEMINATION_SIZE 10240
//#define DISSEMINATION_INTERVAL 1 * 1000000.0
//
#define MIGREQ 24																						//MA移動要求パケットサイズ
#define MIGREP 48																						//MA移動応答パケットサイズ

#define MAREQTTL 100000																					//MA移動要求のタイムアウト時間
//
//
//class MA;
//
////情報配信クラス
//class DISSEMINATION:public EVENT{
//	MA* maPtr;
//	int size;
//public:
//	DISSEMINATION(double, int, MA*, LIST<EVENT>*);
//	bool process(LIST<EVENT>*, _NODE);															//イベント処理
//};
//
//MA用MIGREPパケットクラス
class MAREP{
	LOCATION position;																				//位置
	short difX;																							//移動差（ｘ座標）
	short difY;																							//移動差（ｙ座標）
	SIMUTIME interval;																					//測定間隔
	double power;																						//残存電力
	MA* maPtr;																							//対応MAオブジェクト
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
	enum mtype {Distance, Predict, PredictDistance};	//イベントタイプ識別子
private:
	short id;																							//ID
	LOCATION center;																					//中心位置
	double radius;																						//半径
	int size;																							//データとしてのサイズ
	short migrationNumber;																			//移動回数
	bool migrationFlag;
//	double outTime;
	TCP* tPtr;																							//移動の際のTCPオブジェクト
	TIMEOUT* toPtr;																					//移動要求タイムアウトオブジェクト
	short candidate;																					//移動先候補
	double myDist;
	double candDist;																					//移動先候補の中心からの距離
//	double candSurvive;
	mtype mode;															
	short meshRouteOrder;																			//メッシュネットワーク巡回経路の順序番号
	SIMUTIME stayTime;																				//滞在時間														
public:
	MA(SIMU*, SIMUTIME, LOCATION, double, mtype, short, short);
	MA(SIMU*);
	bool process(void);																				//イベント処理

	short getId(void)				{ return id; }													//MAのIDの取得
	LOCATION getCenter(void)	{ return center; }											//中心位置の取得
	void setSize(int val)		{ size = val; }												//サイズの設定
	int getSize(void)				{ return size; }												//サイズの取得
	void setStayTime(SIMUTIME tval){ stayTime = tval; }									//滞在時間の設定
	SIMUTIME getStayTime(void)	{ return stayTime; }											//滞在時間の取得
	short increRouteOrder(void){ return ++meshRouteOrder; }								//巡回経路の順序番号のインクリメント
	void resetRouteOrder(void) { meshRouteOrder = 0; }										//巡回経路の順序番号の初期化
	short getRouteOrder(void)	{ return meshRouteOrder; }									//巡回経路の順序番号の取得
	void increMigNum(void)		{ migrationNumber++; }										//移動回数のインクリメント
	short getMigNum(void)		{ return migrationNumber; }								//移動回数の取得
	void resetMigration(void)	{ migrationFlag = false; }									//移動中フラグの初期化
	bool getMigration(void)		{ return migrationFlag; }									//移動中フラグ取得
	TCP* getTCP(void)				{ return tPtr; }												//移動用TCPオブジェクトの取得
	TIMEOUT* getTimeout(void)	{ return toPtr; }												//タイムアウトオブジェクトの取得
	short getCandidate(void)	{ return candidate; }										//移動候補の取得

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
	
	vector<LAB> gab;																					//GAB情報
	vector<short> meshRoute;																		//メッシュ用巡回エージェントの経路情報
	vector<LOCATION> nodePos;																		//ノードの位置情報(v7だとnowPos)
	vector<LOCATION> estimatePos;																	//ノードの存在推定場所
	vector<LOCATION> lastPos;																		//ノードが前いた場所
	vector<int> route;
	vector<int> route1;
	vector<int> route2;
};

//
//
