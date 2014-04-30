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

//シミュレーション時刻クラス
class SIMUTIME{
	int sec;																								//秒を表す値
	int lessSec;																						//秒未満を表す値(最小値は1μs）
	short lessMuSec;																					//μ秒未満を表す値（最小値は0.001μs）
public:
	SIMUTIME(int s, int mu)					{ sec = s, lessSec = mu, lessMuSec = 0; }	//コンストラクタ（初期値が2つある場合)
	SIMUTIME(int mu)							{ if(mu < 1000000){ sec = 0, lessSec = mu, lessMuSec = 0;}
														else{sec = mu / 1000000, lessSec = mu % 1000000, lessMuSec = 0;}}	//コンストラクタ（初期値が1つある場合)
	SIMUTIME(void)								{ sec = 0, lessSec = 0; lessMuSec = 0; }	//コンストラクタ（初期値がない場合（0で初期化））
	SIMUTIME operator+(const SIMUTIME& t){ if(lessSec + t.lessSec < 1000000)
															return SIMUTIME(sec + t.sec, 
																			 lessSec + t.lessSec);
														else
															return SIMUTIME(sec + t.sec + 1, 
																			 lessSec + t.lessSec - 1000000);}	//時刻の加算演算子に関する定義
	SIMUTIME operator-(const SIMUTIME& t){ if(lessSec >= t.lessSec)
															return SIMUTIME(sec - t.sec, 
																			 lessSec - t.lessSec);
														else
															return SIMUTIME(sec - t.sec -1, 
																			 lessSec + 1000000 - t.lessSec);}	//時刻の減算演算子に関する定義
	void setSec(int val)						{ sec = val; }										//秒の設定
	int getSec(void)							{ return sec; }									//秒の取得
	void setLessSec(int val)				{ lessSec = val; }								//μ秒の設定
	int getLessSec(void)						{ return lessSec; }								//μ秒の取得
	void setLessMuSec(short val)			{ lessMuSec = val; }								//μ秒未満の設定
	short getLessMuSec(void)				{ return lessMuSec; }							//μ秒未満の取得
	void setTime(int s, int mu)			{ sec = s, lessSec = mu; }						//時刻の設定（数値で指定）
	void setTime(SIMUTIME t)				{ sec = t.getSec(), lessSec = t.getLessSec(), lessMuSec = t.getLessMuSec(); }	//時刻の設定（オブジェクトで指定）
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

//時刻比較関数
//引数（時刻a 時刻b)
//戻り値（aが未来なら真，過去なら偽）
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

//時刻比較関数
//引数（時刻a 時刻b)
//戻り値（aが未来なら真，過去なら偽）
inline bool timeCompareMu(SIMUTIME a, SIMUTIME b){
	if(a.getSec() > b.getSec())
		return true;
	if(a.getSec() < b.getSec())
		return false;
	if(a.getLessSec() > b.getLessSec())
		return true;
	return false;
}

//位置情報クラス
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


inline int min(int x, int y){																		//最小値の取得
	if(x < y)
		return x;
	return y;
}

inline int max(int x, int y){																		//最小値の取得
	if(x > y)
		return x;
	return y;
}

inline double max(double x, double y){															//最大値の取得
	if(x > y)
		return x;
	return y;
}

inline void check(void){																			//デバッグ用チェック関数
	std::cout << "check" << std::endl;
}

inline double dist(LOCATION me, LOCATION you){
	return sqrt(pow((double)me.getX() - you.getX(), 2.0) + pow((double)me.getY() - you.getY(), 2.0));
}

//イベントクラス
class EVENT{
public:
	enum type {Node, Channel, Mac, Frame, Packet, Beacon, Udp, Tcp, Sink, Segment, Receive, Timeout, Ma, Abort};	//イベントタイプ識別子
private:
	SIMU* sPtr;
	SIMUTIME time;																						//イベント発生時刻
	type typeId;																						//イベントのタイプ
	short nodeId;																						//イベント所持ノードのID
public:
	EVENT(SIMU* ptr, SIMUTIME val, type id, short nid) 
	{ sPtr = ptr, time = val, typeId = id, nodeId = nid, time.setLessMuSec(nodeId); }//コンストラクタ
	virtual ~EVENT(){ ; }																			//デストラクタ（仮想デストラクタ）
	SIMU* getSimu(void)					{ return sPtr; }
	void setTime(SIMUTIME val)			{ time = val, time.setLessMuSec(nodeId); }	//イベント発生時刻の設定（オブジェクト利用）
	void addTime(SIMUTIME val)			{ time.addTime(val); }								//イベント発生時刻の更新（オブジェクト利用）
	SIMUTIME getEventTime(void)		{ return time; }										//イベント発生時刻の取得
	type getType(void)					{ return typeId; }									//イベントタイプの取得
	void setNid(short id)				{ nodeId = id; }										//イベント所持ノードIDの設定
	short getNid(void)					{ return nodeId; }									//イベント所持ノードIDの取得
	virtual bool process(void) = 0;																//イベント処理（仮想関数）
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

//信号クラス
class SIGNAL{
public:
	enum type { RTS, CTS, DATA, ACK, BDATA, ORACK };										//信号タイプの列挙
private:
	CHANNEL* chPtr;																					//チャネルオブジェクトポインタ
	type typeId;																						//信号タイプ
	short length;																						//信号長
	short dataTime;																					//送信したいデータ信号長
	short duration;																					//回線使用時間
	short orSource;																					//ORRTS用送信元ノードID
public:
	SIGNAL(CHANNEL*);																					//コンストラクタ
	~SIGNAL();
	CHANNEL* getChannel(){ return chPtr; }														//チャネルオブジェクトの取得
	type getType(void)	{ return typeId; }													//信号タイプの取得
	short getLength(void){ return length; }													//信号長の取得
	short getDataTime(void)	{ return dataTime; }												//送信したいデータ信号長の取得
	short getDuration(void) { return duration; }												//回線使用時間の取得
	void setOrSource(short id) { orSource = id; }											//ORRTS用送信元ノードIDの設定
	short getOrSource(void)	{ return orSource; }												//ORRTS用送信元ノードIDの取得
};

//チャネルクラス
class CHANNEL:public EVENT{
public:
private:
	MAC* mPtr;																							//MACオブジェクト
	bool sendFlag;																						//送信状態フラグ
	SIGNAL* sigPtr;																					//受信信号オブジェクト
	short dataTime;																					//受信データの受信所要時間
public:
	CHANNEL(SIMU*, MAC*, SIMUTIME, short);																		//コンストラクタ
	bool process(void);																				//イベント処理
	MAC* getMAC()						{ return mPtr; }											//MACオブジェクトの取得
	void setSendFlag(bool flag)	{ sendFlag = flag; }										//送信状態フラグの設定
	bool getSendFlag(void)			{ return sendFlag; }										//送信状態フラグの取得
	void setSignal(SIGNAL* ptr)	{ sigPtr = ptr; }											//受信信号の設定
	SIGNAL* getSignal(void)			{ return sigPtr; }										//受信信号の取得
	void setDataTime(short val)	{ dataTime = val; }											//データ受信所要時間の取得
	short getDataTime(void)			{ return dataTime; }										//データ受信所要時間の取得
	class RX_SIGNAL_LIST{																			//受信信号情報クラス
		short id;																							//ノードID
		double rxPower;																					//受信電力（未受信は0）
		SIMUTIME rxFinTime;																				//受信完了時刻
	public:
		RX_SIGNAL_LIST(short ival, double pval){ id = ival, rxPower = pval, 
																rxFinTime.setLessMuSec(id); }			//コンストラクタ
		short getId(void)						{ return id; }											//ノードIDの取得
		double getRxPower(void)				{ return rxPower; }									//受信電力の取得
		void setRxFinTime(SIMUTIME t)		{ rxFinTime.setSec(t.getSec()),
													  rxFinTime.setLessSec(t.getLessSec()); }		//受信完了時刻の設定
		SIMUTIME getRxFnTime(void)			{ return rxFinTime; }								//受信完了時刻の取得
	};
	vector<RX_SIGNAL_LIST> signalList;															//受信信号情報リスト
	void sendSignal(SIMUTIME);
};

//タイムアウトクラス（イベントクラスの継承）
class TIMEOUT:public EVENT{
public:
	enum type { Tcp, Udp, Ma, Segment, RouteAodv, Mareq, StaReq, MakeUdp, MeshMa, BackGround, MakeTcp };
private:
	type typeId;																	//タイムアウトのタイプ
	bool flag;																		//フラグ（様々な処理に使用）
	SIMUTIME timeVal;																//時刻情報（様々な処理に使用）
	double rate;																	//発生レート（負荷発生時などで使用）
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
	bool process(void);															//イベント処理
//	void setType(type tid)			{ typeId = tid; }
//	type getType(void)				{ return typeId; }
	void setFlag(bool val)			{ flag = val; }						//フラグの設定
	bool getFlag(void)				{ return flag; }						//フラグの取得
	void setTime(SIMUTIME tval)	{ timeVal = tval; }					//時刻の設定
	SIMUTIME getTime(void)			{ return timeVal; }					//時刻の取得
	void setRate(double val)		{ rate = val; }						//負荷レートの設定
	double getRate(void)				{ return rate; }						//負荷レートの取得
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

//パケットクラス（イベントクラスの継承）
class PACKET:public EVENT{	
public:
	enum ptype {Udp, Tcp, Ack, Null, Routing, Flood, 
		RreqDsr, RrepDsr, RerrDsr, RreqAodv, RrepAodv, RerrAodv, Segment,
		MigReq, MigRep, Dissemination, LabCenter, LabRa, LabNeighbor, StaReq, 
		StaRep, MapReq, MapReqB, MapRep, InformLoc, DummyBroadcast, MrReq, MrRep};					//パケットタイプ識別子
																									//MrReq　MrRep：マルチパスルーティング用パケット
private:
	ptype type;																							//パケットタイプ
	short size;																							//パケットサイズ
	short source;																						//送信元ノード
	LOCATION sPos;																						//送信元ノード位置
	short here;																							//存在ノード
	short dest;																							//送信宛先ノード
	LOCATION dPos;																						//送信宛先ノード位置								
	short reqSource;																					//要求発生ノード
	short reqDest;																						//要求宛先ノード
	short errDest;																						//エラーパケットの対象宛先
	int seq;																								//シーケンス番号
	int aodvSeqS;																						//AODV用シーケンス（送信元）
	int aodvSeqD;																						//AODV用シーケンス（送信宛先）
	char hop;																							//経由ホップ数
	char ttl;																							//生存ホップ数
	int requestSeq;																					//TCPACKだった場合の期待受信シーケンス番号
	SIMUTIME sendStartTime;																			//送信元出発時刻
	LAB lab;																								//LAB情報
	short sta;																							//STA情報
	union TYPEPTR{																						//パケットタイプごとの関連情報
		UDP* udpPtr;																						//対応UDPオブジェクト（UDPパケットの場合）
		SEGMENT* segPtr;																					//対応セグメントオブジェクト（TCPパケットの場合）
		MA* maPtr;																							//対応MAオブジェクト（MAパケットの場合）
		MAREP* maRepPtr;																					//対応MA応答パケット（MA応答パケットの場合）
		TIMEOUT* toPtr;																					//対応タイムアウトオブジェクト
	}typePtr;
	int category;																						//カテゴリ
	LOCATION sourcePos;																				//送信元端末位置
	LOCATION lastNodePos;																			//直前送信端末位置
	LOCATION destPos;																					//宛先端末位置
	LOCATION reqDestPos;
	LOCATION reqSourcePos;
	int requestDest;
	int requestSource;
	ptype packetForGDtype;
public:
   PACKET(SIMU* = NULL, SIMUTIME = 0, ptype = Null, short = -1, short = -1, short = -1, short = -1, int = -1);
	~PACKET();																							//デストラクタ
	bool process(void);																				//イベント処理
	ptype getType(void)			{ return type; }												//タイプの取得
	void setSize(short val)		{ size = val; }												//サイズの設定
	void increSize(short val)	{ size += val; }												//サイズのインクリメント
	short getSize(void)			{ return size; }												//サイズの取得
	short getSource(void)		{ return source; }											//送信元ノードの取得
	void setSpos(LOCATION pos)	{ sPos = pos; }												//送信元ノード位置の設定
	LOCATION getSpos(void)		{ return sPos; }												//送信元ノード位置の設定
	void setHere(short val)		{ here = val; }												//存在ノードの設定
	short getHere(void)			{ return here; }												//存在ノードの取得
	short getDest(void)			{ return dest; }												//宛先ノードの取得
	void setDpos(LOCATION pos)	{ dPos = pos; }												//送信宛先ノード位置の設定
	LOCATION getDpos(void)		{ return dPos; }												//送信宛先ノード位置の設定
	void setReqSource(short val){ reqSource = val; }										//要求発生ノードの設定
	short getReqSource(void)	{ return reqSource; }										//要求発生ノードの取得
	void setReqDest(short val)	{ reqDest = val; }											//要求宛先ノードの設定
	short getReqDest(void)		{ return reqDest; }											//要求宛先ノードの取得
	void setErrDest(short val)	{ errDest = val; }											//エラーパケットの対象宛先の設定
	short getErrDest(void)		{ return errDest; }											//エラーパケットの対象宛先の取得
	void setSeq(int val)			{ seq = val; }													//シーケンス番号の設定
	int getSeq(void)				{ return seq; }												//シーケンス番号の取得
	void setAodvSeqS(int val)	{ aodvSeqS = val; }											//AODV用シーケンス番号（送信元）の設定
	int getAodvSeqS(void)		{ return aodvSeqS; }											//AODV用シーケンス番号（送信元）の取得
	void setAodvSeqD(int val)	{ aodvSeqD = val; }											//AODV用シーケンス番号（送信宛先）の設定
	int getAodvSeqD(void)		{ return aodvSeqD; }											//AODV用シーケンス番号（送信宛先）の取得
	char getHop(void)				{ return hop; }												//経由ホップ数の取得
	char getTtl(void)				{ return ttl; }												//生存ホップ数の取得
	void setReqSeq(int val)		{ requestSeq = val; }										//ACKだった場合の期待受信シーケンス番号の設定
	int getReqSeq(void)			{ return requestSeq; }										//ACKだった場合の期待受信シーケンス番号の取得
	void setSendStart(SIMUTIME val)	{ sendStartTime = val; }							//送信時刻の設定
	SIMUTIME getSendStart(void){ return sendStartTime; }									//送信時刻の取得
	void setLAB(LAB val)			{ lab = val; }													//LAB情報の設定
	LAB getLAB(void)				{ return lab; }												//LAB情報の取得
	void setSTA(short val)		{ sta = val; }													//STA情報の設定
	short getSTA(void)			{ return sta; }												//STA情報の取得
	void setUdp(UDP* ptr)		{ typePtr.udpPtr = ptr; }									//対応UDPオブジェクトの設定
	UDP* getUdp(void)				{ return typePtr.udpPtr; }									//対応UDPオブジェクトの取得
	void setSeg(SEGMENT* ptr)	{ typePtr.segPtr = ptr; }									//対応セグメントオブジェクトの設定
	SEGMENT* getSeg(void)		{ return typePtr.segPtr; }									//対応セグメントオブジェクトの取得
	void setMa(MA* ptr)			{ typePtr.maPtr = ptr; }									//対応MAオブジェクトの設定
	MA* getMa(void)				{ return typePtr.maPtr; }									//対応MAオブジェクトの取得
	void setMigRep(MAREP* ptr)	{ typePtr.maRepPtr = ptr; }								//対応MAREPオブジェクトの設定
	MAREP* getMigRep(void)		{ return typePtr.maRepPtr; } 								//対応MAREPオブジェクトの取得
	void setTimeout(TIMEOUT* ptr){ typePtr.toPtr = ptr; }									//対応タイムアウトオブジェクトの設定
	TIMEOUT* getTimeout(void)	{ return typePtr.toPtr; }									//対応タイムアウトオブジェクトの取得
	bool queue(SIMU*, bool);																		//バッファへの挿入
	void showLog(short, char*, SIMUTIME);														//パケット情報表示
	void showPath(void);																				//経路情報表示
	vector<int> path;																					//経路情報
	vector<int> path1;																					//マルチパス１
	vector<int> path2;																					//マルチパス２
	short pathNum;																		//経路判断用
	vector<int> reqPath;																				//要求経路情報（提案方式用）
	vector<int> reqPath1;
	vector<int> reqPath2;
	vector<int> mamrPath;
	void setReqDPos(LOCATION loc)		{ reqDestPos = loc; }								//宛先位置の設定
	LOCATION getReqDPos(void)			{ return reqDestPos; }								//宛先位置の取得
	void setReqSPos(LOCATION loc)		{ reqSourcePos = loc; }								//宛先位置の設定
	LOCATION getReqSPos(void)			{ return reqSourcePos; }								//宛先位置の取得
	short mpath_check;
};

//フレームクラス（イベントクラスの継承）
class FRAME:public EVENT{
public:
	enum castType {Uni, Broad, Null};															//フレームタイプ識別子
private:
	castType cast;																						//フレームタイプ
	short source;																						//送信元ノード
	short dest;																							//宛先ノード
	short size;																							//フレームサイズ（ビット）
	int seq;																								//シーケンス番号
	PACKET* packetPtr;																				//対応パケットオブジェクト
public:
	FRAME(SIMU*, SIMUTIME, castType, short, short, int, PACKET*);						//コンストラクタ
	~FRAME();																							//デストラクタ
	bool process(void);																				//イベント処理
	void setCast(castType val)	{ cast = val; }												//フレームタイプの設定
	castType getCast(void)		{ return cast; }												//フレームタイプの取得
	short getSize(void)			{ return size; }												//フレームサイズの取得
	short getSource(void)		{ return source; }											//送信元ノードの取得
	short getDest(void)			{ return dest; }												//宛先ノードの取得
	int getSeq(void)				{ return seq; }												//シーケンス番号の取得
	void setPacket(PACKET* ptr){ packetPtr = ptr; }											//対応パケットオブジェクトの設定
	PACKET* getPacket(void)		{ return packetPtr; }
};

//エージェントクラス（イベントクラスの継承）
class AGENT:public EVENT{
public:
	AGENT(SIMU* ptr, short nid, SIMUTIME tval, EVENT::type tid):EVENT(ptr, tval, tid, nid){;}	//コンストラクタ
	bool process(void){ return true; }															//イベント処理
};

//ビーコンクラス（エージェントクラスの継承）
class BEACON:public AGENT{
public:
	BEACON(SIMU* ptr, short nid, SIMUTIME tval):AGENT(ptr, nid, tval, EVENT::Beacon){};					//コンストラクタ
	bool process(void);															//イベント処理
};

//受信クラス（エージェントクラスの継承）
class RECEIVE:public AGENT{
	PACKET* pPtr;																				//パケットオブジェクト
	short object;																							//受信イベントの対象
public:
	RECEIVE(SIMU* ptr, short nid, SIMUTIME tval, PACKET* Ptr, short oval):AGENT(ptr, nid, tval, EVENT::Receive)
	{ object = oval, pPtr = Ptr; }														//コンストラクタ
	~RECEIVE(){ ; }
	bool process(void);																				//イベント処理
	void setPacket(PACKET* ptr)	{ pPtr = ptr; }											//パケットオブジェクトの設定
	PACKET* getPacket(void)			{ return pPtr; }											//パケットオブジェクトの取得
	short getObject(void)			{ return object; }
};

//UDPクラス（エージェントクラスの継承）
class UDP:public AGENT{
	UDPSINK* objectPtr;																				//対応シンクオブジェクト
	double rate;																						//送信レート
	int size;																							//総送信サイズ
	int byte;																							//送信済みサイズ
	int seq;																								//シーケンス番号
public:
	UDP(SIMU* ptr, short nid, SIMUTIME tval, double rval, int sval):AGENT(ptr, nid, tval, EVENT::Udp)	//コンストラクタ
	{ rate = rval, size = sval, byte = 0, seq = 0; }
	~UDP(){ ; }
	bool process(void);																				//イベント処理
	UDPSINK* getObject(void)		{ return objectPtr; }									//シンクオブジェクトの取得
	void setRate(double val)		{ rate = val; }											//送信レートの設定
	double getRate(void)				{ return rate; }											//送信レートの取得
	int getSize(void)					{ return size; }											//総送信サイズの取得
	int increByte(int val)			{ byte += val; return byte; }							//送信済みサイズの設定
	int getByte(void)					{ return byte; }											//送信済みサイズの取得
	void increSeq(void)				{ seq++; }													//シーケンス番号のインクリメント
	int getSeq(void)					{ return seq; }											//シーケンス番号の取得
	void connectSink(UDPSINK*);																	//SINKとの結合
};

//UDPシンククラス（エージェントクラスの継承）
class UDPSINK:public AGENT{
	UDP* objectPtr;																					//対応UDPオブジェクト
	int byte;																							//受信バイト
public:
	UDPSINK(SIMU* ptr, short nid):AGENT(ptr, nid, 0, EVENT::Sink){ byte = 0; }		//コンストラクタ
	void setObject(UDP* val)		{ objectPtr = val; }										//対応UDPオブジェクトの設定
	UDP* getObject(void)				{ return objectPtr; }									//対応UDPオブジェクトの取得
	int increByte(int val)			{ byte += val; return byte; }							//受信バイトのインクリメント
	int getByte(void)					{ return byte; }											//受信バイトの取得
};

//セグメントクラス（イベントクラスの継承）
class SEGMENT:public EVENT{
	TCP* tcpPtr;																						//対応TCPオブジェクト
	short size;																							//サイズ
	int seq;																								//シーケンス番号
	SIMUTIME sendStartTime;																			//送信開始時刻
	char ackRepeat;																					//重複ACK受信回数
	TIMEOUT* toPtr;																					//タイムアウトオブジェクト
	bool NAretrans;																					//重複ACK受信による再送済みフラグ
public:
	SEGMENT(SIMU*, SIMUTIME, TCP*, short, int, short);										//コンストラクタ
	virtual ~SEGMENT();																				//デストラクタ
	bool process(void){ return true; }															//イベント処理
	void setTcp(TCP* ptr)			{ tcpPtr = ptr; }											//対応TCPオブジェクトの設定
	TCP* getTcp(void)					{ return tcpPtr; }										//対応TCPオブジェクトの取得
	short getSize(void)				{ return size; }											//サイズの取得
	int getSeq(void)					{ return seq; }											//シーケンス番号の取得
	void setSendStart(SIMUTIME t) { sendStartTime = t; }									//送信開始時刻の設定
	SIMUTIME getSendStart(void)	{ return sendStartTime; }								//送信開始時刻の取得
	void resetAckRepeat(void)		{ ackRepeat = 0; }										//重複ACK受信回数の初期化 
	void increAckRepeat(void)		{ ackRepeat++; }											//重複ACK受信回数のインクリメント 
	char getAckRepeat(void)			{ return ackRepeat; }									//重複ACK受信回数の取得 
	void setTimeout(TIMEOUT* ptr)	{ toPtr = ptr; }											//タイムアウトオブジェクトの設定
	TIMEOUT* getTimeout(void)		{ return toPtr; }											//タイムアウトオブジェクトの取得
	void setNAretrans(bool flag)	{ NAretrans = flag; }									//重複ACK受信による再送済みフラグの設定
	bool getNAretrans(void)			{ return NAretrans; }									//重複ACK受信による再送済みフラグの取得
};


////TCPクラス（エージェントクラスの継承）
class TCP:public AGENT{
	TCPSINK* objectPtr;																				//対応TCPシンクオブジェクト
	int size;																							//総送信サイズ
	int byte;																							//送信済みバイト
	SIMUTIME startTime;																				//送信開始時刻
	SIMUTIME finishTime;																				//送信完了時刻
	SIMUTIME rtt;																						//ラウンドトリップ時間
	SIMUTIME nowRtt;																					//直近パケットのラウンドトリップ時間
	double D;																							//タイムアウト設定用偏差
	double windowSize;																				//ウィンドウサイズ
	double windowThreshold;																			//ウィンドウ制御用閾値
	int makeSegNum;																					//次に作成するセグメント番号
	int lastSendSeq;																					//直前に送信したセグメントのシーケンス番号
	int nextSendSeq;																					//次に送信するセグメントのシーケンス番号
	int lastReqSeq;																					//直前の期待シーケンス番号																						
	bool buffering;																					//パッケトのバッファリング状態
	MA* maPtr;																							//モバイルエージェントオブジェクト
	bool abort;																							//中断フラグ
	bool finish;																						//終了フラグ
//	bool init;																							//対象セグメントが先頭かを示すフラグ
//	int finishNum;																						//ウィンドウベクタに存在する送信終了セグメント数
//	int windowNum;																						//ウィンドウに存在するセグメント数
//	int segPacketNum;																					//セグメントパケット数（TCPパケット含む）
//	int sendSeq;																						//次に送信処理を行うセグメントのシーケンス
//	int receiveSeq;																					//直近受信ACKの対象シーケンス
//	int seq;																								//直近送信セグメントのシーケンス番号
//	int id;																								//直近受信ACKのシーケンス番号
//	int ack;																								//受信ACK番号
//	int lastack;																						//直前受信ACK番号
//	bool timeOut;																						//タイムアウトフラグ
//	int ackRepeatNum;																					//同一セグメント要求ACKの数
//	TIMEOUT* toPtr;																					//タイムアウトオブジェクト
//	bool finish;																						//終了フラグ
//	bool measurement;
public:
	TCP(SIMU*, short, SIMUTIME, int);															//コンストラクタ
	~TCP();
	bool process(void);																					//イベント処理
	TCPSINK* getObject(void)		{ return objectPtr; }									//対応TCPシンクオブジェクトの取得
	int getSize(void)					{ return size; }											//総送信サイズの取得
	SIMUTIME getRtt(void)			{ return rtt; }											//ラウンドトリップタイムの取得
	double getD(void)					{ return D; }												//平均ラウンドトリップタイムの取得
	void setLastSendSeq(int val)	{ lastSendSeq = val; }									//直前送信セグメントシーケンス番号の設定
	void resetBuffer(void)			{ buffering = false; }									//バッファリングフラグのリセット
	void setMA(MA* ptr)				{ maPtr = ptr; }											//モバイルエージェントオブジェクトの設定
	MA* getMA(void)					{ return maPtr; }											//モバイルエージェントオブジェクトの取得
	void setAbort(void)				{ abort = true; }											//中断フラグ

//	void setInit(bool val)			{ init = val; }											//イニシャルフラグの設定
//	bool getInit(void)				{ return init; }											//イニシャルフラグの取得
//	void setSize(int val)			{ size = val; }											//総送信サイズの設定
//	int increByte(int val)			{ byte += val; return byte; }							//送信済みバイトの設定
//	int getByte(void)					{ return byte; }											//送信済みバイトの取得
//	void setStartTime(double val)	{ startTime = val; }										//送信開始時刻の設定
//	double getStartTime(void)		{ return startTime; }									//送信開始時刻の取得
//	void setFinishTime(double val){ finishTime = val; }									//送信終了時刻の設定
//	double getFinishTime(void)		{ return finishTime; }									//送信終了時刻の取得
//	void setRtt(double val)			{ rtt = val; }												//ラウンドトリップタイムの設定
//	double getNowRtt(void)			{ return nowRtt; }										//直近パケットのラウンドトリップタイムの取得
//	void setNowRtt(double val)		{ nowRtt = val; }											//直近パケットのラウンドトリップタイムの設定
//	void setD(double val)			{ D = val; }												//平均ラウンドトリップタイムの設定
//	void setWindow(double val)		{ windowSize = val; }									//ウィンドウサイズの設定
//	double getWindow(void)			{ return windowSize; }									//ウィンドウサイズの取得
//	void setWindowNum(int val)		{ windowNum = val; }										//ウィンドウサイズの設定
//	int getWindowNum(void)			{ return windowNum; }									//ウィンドウサイズの取得
//	void decreSegPacketNum(void)	{ segPacketNum--; }										//セグメントパケット数のデクリメント
//	int getSegPacketNum(void)		{ return segPacketNum; }
//	void setSendSeq(int val)		{ sendSeq = val; }										//次に送信処理を行うセグメントのシーケンスの設定
//	int getSendSeq(void)				{ return sendSeq; }										//次に送信処理を行うセグメントのシーケンスの取得
//	void setThreshold(double val)	{ windowThreshold = val; }								//ウィンドウサイズの設定
//	double getThreshold(void)		{ return windowThreshold; }							//ウィンドウサイズの取得
//	void increSeq(void)				{ seq++; }													//直近送信セグメントシーケンス番号のインクリメント
//	int getSeq(void)					{ return seq; }											//直近送信セグメントシーケンス番号の取得
//	void setId(int val)				{ id = val; }												//直近受信ACKのシーケンス番号の設定
//	int getId(void)					{ return id; }												//直近受信ACKのシーケンス番号の取得
//	void setAck(int val)				{ ack = val; }												//受信ACK番号の設定
//	int getAck(void)					{ return ack; }											//受信ACK番号の取得
//	void setLastack(int val)		{ lastack = val; }										//直前受信ACK番号の設定
//	int getLastack(void)				{ return lastack; }										//直前受信ACK番号の取得
//	void setTimeOut(bool val)		{ timeOut = val; }										//タイムアウトフラグの設定
//	bool getTimeOut(void)			{ return timeOut; }										//タイムアウトフラグの取得
//	void initAckRepeat(void)		{ ackRepeatNum = 1; }									//同一セグメント要求ACK数の初期化
//	void increAckRepeat(void)		{ ackRepeatNum++; }										//同一セグメント要求ACK数のインクリメント
//	int getAckRepeat(void)			{ return ackRepeatNum; }								//同一セグメント要求ACK数の取得						
//	void setTObject(TIMEOUT* ptr)	{ toPtr = ptr; }											//タイムアウトオブジェクトの設定
//	TIMEOUT* getTObject(void)		{ return toPtr; }											//タイムアウトオブジェクトの取得
//	void setFinish(void)				{ finish = true; }										//終了フラグの設定
//	bool getFinish(void)				{ return finish; }										//終了フラグの取得
//	void setMeasurement(void)		{ measurement = true; }									//中断フラグの設定
//	bool getMeasurement(void)		{ return measurement; }									//中断フラグの取得
	void connectSink(TCPSINK*);																	//SINKとの結合
	void makeSegment(void);																			//セグメント作成
	void makePacket(void);																			//送信パケット作成
	void getTcpAck(PACKET*);																		//ACK受信処理
	void retransmission(void);																		//再送
	void abortProcess(NODE*);																		//TCPの中断処理

//	int renewWindow(LIST<EVENT>*, NODE*);														//輻輳ウィンドウの更新
//	bool checkWindow(void);																			//輻輳ウィンドウチェック
//	int eraseSegPacket(NODE*, int, int);																//セグメントパケットの削除
//	void windowShow(void);																			//輻輳ウィンドウの表示
	vector<SEGMENT*> window;																		//送信ウィンドウ
	vector<SEGMENT*> segCash;																		//セグメントキャッシュ
};

//TCPシンクオブジェクトクラス（エージェントクラスの継承）
class TCPSINK:public AGENT{
	TCP* objectPtr;																					//対応TCPオブジェクト
	int byte;																							//受信バイト
	int lastSeq;																						//直近受信セグメントのシーケンス番号
public:
	TCPSINK(SIMU* ptr, short nid):AGENT(ptr, nid, 0, EVENT::Sink){ byte = 0, lastSeq = -1; }			//コンストラクタ
	bool process(void){ return true; }										//イベント処理
	void setObject(TCP* val)		{ objectPtr = val; }										//対応TCPオブジェクトの設定
//	TCP* getObject(void)				{ return objectPtr; }									//対応TCPオブジェクトの取得
//	double increByte(double val)	{ byte += val; return byte; }							//受信バイトのインクリメント
	int getByte(void)					{ return byte; }											//受信バイトの取得
//	void setLastseq(int val)		{ lastseq = val; }										//直近受信セグメントのシーケンス番号の設定
//	int getLastseq(void)				{ return lastseq; }										//直近受信セグメントのシーケンス番号の取得
	void sendAck(PACKET*);																			//TCPACK送信処理
	vector<int> cashSeq;																				//受信不連続セグメントのシーケンスキャッシュ
};

////MACクラス（イベントクラスの継承）
class MAC:public EVENT{
public:
//	enum castType {Uni, Broad};
	enum stateType {Idle, Difs, Backoff, Rts, Cts, 
		Waitcts, Data, BData, Waitdata, Ack, Waitack, Nav, NavFin, WaitOrAck, WaitOtherAck, OrAck};		//フレームの状態識別子（列挙）
private:
	int id;																								//ノード番号
	stateType state;																					//ノードの状態
	short objectId;																					//対象相手ノード
	char rtsTime;																						//RTS信号送信所要時間
	char ctsTime;																						//CTS信号送信所要時間
	short dataTime;																					//データ送信所要時間
	char ackTime;																						//ACK信号送信所要時間
	FRAME* framePtr;																					//処理対象フレームオブジェクト
	short contWindow;																					//コンテンションウィンドウサイズ
	SIMUTIME backoffTime;																			//バックオフ時間
	char retrans;																						//再送回数
	bool listInsert;																					//イベントリストに登録されているかを示すフラグ
	bool orFlag;																						//ORをするかを示すフラグ
	PACKET* orPtr;																						//OR用パケット
	short orSource;																					//OR方式の場合の本来の送信元ノード
	short orDest;																						//OR方式の場合の本来の送信宛先ノード
public:
	MAC(SIMU*, SIMUTIME, short);																	//コンストラクタ
//	~MAC();																								//デストラクタ
	void show(double val);																			//状態の表示
	bool process(void);																				//イベント処理
	void setState(stateType val)		{ state = val; }										//ノード状態の設定
	stateType getState(void)			{ return state; }										//ノード状態の取得
	void setObject(short id)			{ objectId = id; }									//対象相手ノードの設定
	short getObject(void)				{ return objectId; }									//対象相手ノードの取得
	char getRtsTime(void)				{ return rtsTime; }									//RTS信号送信所要時間の取得
	char getCtsTime(void)				{ return ctsTime; }									//CTS信号送信所要時間の取得 
	short getDatatime(void)				{ return dataTime; }									//データ送信所要時間の取得
	char getAckTime(void)				{ return ctsTime; }									//ACK信号送信所要時間の取得 
	void setFrame(FRAME* ptr)        { framePtr = ptr; }									//処理対象フレームオブジェクトの設定
	FRAME* getFrame(void)				{ return framePtr; }									//処理対象フレームオブジェクトの取得
	void setBackoff(SIMUTIME t)		{ backoffTime = t; }
	SIMUTIME getBackoff(void)			{ return backoffTime; }
	void increRetrans(void)				{ retrans++; }											//再送回数のインクリメント
	char getRetrans(void)				{ return retrans; }									//再送回数の取得
	void setListInsert(bool flag)		{ listInsert = flag; }								//リスト登録フラグの設定
	bool getListInsert(void)			{ return listInsert; }								//リスト登録フラグの取得
	void setOrFlag(bool flag)			{ orFlag = flag; }									//OR用フラグの設定
	bool getOrFlag(void)					{ return orFlag; }									//OR用フラグの取得
	PACKET* getOrPtr(void)				{ return orPtr; }										//OR用パケットの取得
	void setOrSource(short id)			{ orSource = id; }									//OR用送信元ノードの設定
	short getOrSource(void)				{ return orSource; }									//OR用送信元ノードの取得
	void setOrDest(short id)			{ orDest = id; }										//OR用送信宛先ノードの設定
	short getOrDest(void)				{ return orDest; }									//OR用送信宛先ノードの取得

	void calcDataTime(FRAME*);																		//データフレームの送信所用時間
	void receiveSignal(SIGNAL*, MAC*);															//信号受信処理
	void sendFinSignal(SIGNAL*, MAC*);															//信号送信完了処理
	void makeBackoff(SIMUTIME);																	//バックオフ時間の設定
	bool judgeSendable(void);																		//送信可能かどうかの判断
	void reset(void);																					//送信処理のリセット
	void checkFrame(void);																			//フレームの存在チェック
	bool checkIdling(void);																			//信号レベルでのアイドリングチェック
	bool judgeOpportunistic(MAC*);																//ORをするかの判断
	vector<int> last;																					//直近受信フレーム																				
};

//バッファクラス
class BUFFER{
	int size;																							//パケットバッファサイズ
	int length;																							//パケット行列長
	short segLength;																					//セグメント行列長
public:
	BUFFER(int val)					{ size = val, length = 0, segLength; }				//コンストラクタ
	int getSize(void)					{ return size; }											//サイズの取得
	void increLength(int val)		{ length += val; }										//行列長のインクリメント
	void decreLength(int val)		{ length -= val; }										//行列長のデクリメント
	int getLength(void)				{ return length; }										//行列長の取得
	vector<PACKET*> queue;																			//パケット用バッファ本体
	vector<SEGMENT*> squeue;																		//TCPセグメント用バッファ本体
};

//ルーティングデータクラス
class ROUTING_DATA{
	short next;																							//転送先
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


//ノードクラス（イベントクラスの継承）
class NODE:public EVENT{
public:
	enum move {NO, RWP, GRID};																		//移動タイプ識別子
	enum route {PRO, DSR, AODV, GEDIR, MAMR};														//ルーティングタイプ識別子
																									//MAMRはモバイルエージェント利用型マルチパスルーティング
	enum powerMode { SLEEP, WAIT, SEND, RECEIVE};											//消費電力計算用状態識別子
	enum mrgroup { S_PATH, M_PATH1, M_PATH2, NONE}  ;										  //マルチパス生成時のグループ

private:
	BUFFER* bufferPtr;																				//バッファオブジェクト
	MAC* mPtr;																							//MACオブジェクト
	CHANNEL* cPtr;																						//チャネルオブジェクト
	SIMUTIME now;																						//現在時刻
	short id;																							//識別子
	bool isActive;																						//使用状況
	SIMUTIME activeTime;																				//ノード活動開始時刻
	SIMUTIME offTime;																					//ノード活動停止時刻
	LOCATION position;																				//現在位置
	LOCATION derivePos[2];																			//測定位置
	char deriveTime;																					//測定タイミング
	bool neighborLocEnable;																			//隣接ノード情報を別手段で取得できるかを表すフラグ	
	LOCATION moveDestPos;																			//移動目的地
	short destId;																						//目的地の辺ID（SQUAR，GRIDエリアモデルのみ使用）
	short gridId;																						//移動先交点ID（GRID移動タイプのみ使用）
	double speed;																						//速度
	SIMUTIME  pause;																					//停止時間
	SIMUTIME  pauseReleaseTime;																	//停止解除時刻
	move moveId;																						//移動タイプ
	bool isRecieveingSignal;																		//信号受信中であるかを示すフラグ																						//																					
	route routeId;																						//ルーティングタイプ
	short mapId;																						//接続MAPID
	int seq;																								//シーケンス番号
	int aodvSeq;																						//AODV用シーケンス番号
	int overflow;																						//オーバフローによる棄却フレーム数
	LAB a;	
	double remainPower;																				//残存電力
	double usedPower;																					//消費電力
	powerMode pmode;																					//消費電力計算用状態
	SIMUTIME calcPowerTime;																			//消費電力計算用時刻
	LOCATION lastInformPos;																			//最近情報広告した位置
	
	mrgroup mgroup;    //マルチパス生成時のグループ

//	int block;																							//通信切断による棄却フレーム数
//	int retransfail;																					//再送上限による棄却フレーム数
//	ANTENNAE* antPtr;																					//アンテナオブジェクト
//	double powerDecreTime;																			//消費電力計算開始時刻
//	double receivePower;																				//受信電力（未受信の場合0）
//	double relayRate;																					//中継拒否率（通常ノードなら0）
//	vector<NODESET*> neighbor;																		//隣接ノードオブジェクト
public:
//	NODE(LIST<EVENT>*, short, double, double, move, LOCATION, route, char, ANTENNAE::antenna, double, bool = false);//コンストラクタ
	NODE(SIMU*, short, SIMUTIME, SIMUTIME, LOCATION, move, route);
	~NODE();																								//デストラクタ
	bool process(void);																				//イベント処理
	BUFFER* getBuffer(void)				{ return bufferPtr; }								//バッファオブジェクトの取得
	MAC* getMAC(void)						{ return mPtr; }										//MACオブジェクトの取得
	CHANNEL* getChannel(void)			{ return cPtr; }										//チャネルオブジェクトの取得
	void setNow(SIMUTIME t)				{ now = t; }											//現在時刻の設定
	short getId(void)						{ return id; }											//識別子の取得
	void setIsActive(bool flag)		{ isActive = flag; }									//使用状況の設定
	bool getIsActive(void)				{ return isActive; }									//使用状況の取得
	void setActiveTime(SIMUTIME tval){ activeTime = tval; }								//ノード活動開始時刻の設定
	SIMUTIME getActiveTime(void)		{ return activeTime; }								//ノード活動開始時刻の取得
	void setOffTime(SIMUTIME tval)	{ offTime = tval; }									//ノード活動終了時刻の設定
	SIMUTIME getOffTime(void)			{ return offTime; }									//ノード活動終了時刻の取得
	LOCATION getPos(void)				{ return position; }									//現在位置の取得
	LOCATION getDerivePos(int val)	{ return derivePos[val]; }							//測定位置の取得
	char getDeriveTime(void)			{ return deriveTime; }								//測定タイミングの取得
	void setNeighLocEnable(bool flag){ neighborLocEnable = flag; }						//隣接ノード情報取得可フラグの設定
	bool getNeighborLocEnable(void)	{ return neighborLocEnable; }						//隣接ノード情報取得可フラグの取得
	SIMUTIME getPause(void)				{ return pause; }										//停止時間の取得
	SIMUTIME getPauseRelease(void)	{ return pauseReleaseTime; }						//停止解除時刻の取得
	move getMove(void)					{ return moveId; }									//移動タイプの取得
	void setReceiveSignal(bool flag)	{ isRecieveingSignal = flag; }					//受信中フラグの設定
	bool getReceiveSignal(void)		{ return isRecieveingSignal; }					//受信中フラグの取得
	route getRoute(void)					{ return routeId; }									//ルーティングタイプの取得
	void setMAP(short id)				{ mapId = id; }										//接続MAPの設定
	short getMAP(void)					{ return mapId; }										//接続MAPの取得
	void increSeq(void)					{ seq++; }												//シーケンス番号のインクリメント
	int getSeq(void)						{ return seq; }										//シーケンス番号の取得
	void increAodvSeq(void)				{ aodvSeq++; }											//AODV用シーケンス番号のインクリメント
	int getAodvSeq(void)					{ return aodvSeq; }									//AODV用シーケンス番号の取得	
	void increOverflow(void)			{ overflow++; }										//再送上限による棄却フレーム数のインクリメント
	int getOverflow(void)				{ return overflow; }									//再送上限による棄却フレーム数の取得
	void setPowerMode(powerMode val)	{ pmode = val; }										//消費電力計算用状態の設定
	double getRemainPower(void)		{ return remainPower; }								//残存電力の取得
	double getUsedPower(void)			{ return usedPower; }								//消費電力の取得
	void setCalcPTime(SIMUTIME t)		{ calcPowerTime = t; }								//消費電力計算用時刻の設定	
	SIMUTIME getCalcPTime(void)		{ return calcPowerTime; }							//消費電力計算用時刻の取得	
	void	setLastInform(LOCATION pos){ lastInformPos = pos; }
	LOCATION	getLastInform(void)		{ return lastInformPos; }
//	void initBlock(void)					{ block = 0; }											//通信切断による棄却フレーム数の初期化
//	void increBlock(void)				{ block++; }											//通信切断による棄却フレーム数のインクリメント
//	int getBlock(void)					{ return block; }										//通信切断による棄却フレーム数の取得
//	void increRetransfail(void)		{ retransfail++; }									//再送上限による棄却フレーム数のインクリメント
//	int getRetransfail(void)			{ return retransfail; }								//再送上限による棄却フレーム数の取得
//	ANTENNAE* getAntenna(void)			{ return antPtr; }									//アンテナオブジェクトの取得
//	void setRxPower(double val)		{ receivePower = val; }								//受信電力の設定
//	double getRxPower(void)				{ return receivePower; }							//受信電力の取得
//	void setRelayRate(double val)		{ relayRate = val; }									//中継拒否率の設定
//	double getRelayRate(void)			{ return relayRate; }								//中継拒否率の取得
//	vector<NODESET*> getNeighbor(void){ return neighbor; }								//隣接ノードオブジェクトベクタの取得

	void posInit(void);																				//ノード配置
	void replace(short);																				//ノード再配置
	void posDerive(void);																			//位置情報の取得
	void destInit(void);																				//目的地の初期化
	void speedSet(void);																				//速度の設定
	bool toDestination(LOCATION);																	//目的地への移動処理
	void noMove(void);																				//非移動処理（時刻更新等）
	void randomWayPoint(void);																		//ランダムウェイポイントの移動処理
	void gridWayPoint(void);																		//格子移動処理
	void calcDistance(void);																		//ノード間距離の決定
	void checkPacket(void);																			//送信パケットが存在するかのチェック
	PACKET* relayPacket(PACKET*, short);														//パケット中継処理
	void sendError(PACKET*);																		//送信エラー処理
	void queueShow(void);																			//バッファ内のパケット表示
	bool migrationCheck();																			//MAの移動が必要かのチェック
	void sendLab(short, short);																	//LAB情報の送信
	void makeRoutingTable(short);																	//ルーティングテーブル作成
	void calcPower(void);																			//消費電力計算
	void sendInfoemLoc(void);																		//位置情報パケットの送信

	void showPos(void)			{ cout << "(" << position.getX() << "," << position.getY() << ")" 
		<< "\tdest " <<  "(" << moveDestPos.getX() << "," << moveDestPos.getY() << ")" << endl; }//位置表示
//	void pathShow(_NODE, int);																		//経路表示

	void setGroupe(mrgroup val)					{ mgroup = val; }								//グループを設定
	short getGroupe(void)						{ return mgroup; }							//グループを取得
	void showGroupe(void){																		//グループを表示
		if(mgroup == NONE ) cout << "group is none" << endl;
		else if(mgroup == M_PATH1) cout << "group is multipath1" << endl;
		else if(mgroup == M_PATH2) cout << "group is multipath2" << endl;
		else if(mgroup == S_PATH) cout << "group is singlepath" << endl;
		else cout << "??? LOCATION showGroupe at class.h" << endl,exit(1);
	}
	

//
//	//_NODESET conNode;																					//隣接ノード
//	//_NODESET carNode;																					//キャリアセンスノード
	class NEIGHBOR_LIST{																				//隣接ノード情報クラス
		short id;																							//ノードID
		double distance;																					//距離
	public:
		NEIGHBOR_LIST(short ival, double dval){ id = ival, distance = dval; }							//コンストラクタ
//		~NEIGHBOR_LIST(){ cout << "bad " << this << endl; }
		short getId(void)						{ return id; }											//ノードIDの取得
		double getDist(void)					{ return distance; }									//距離の取得
	};
	vector<NEIGHBOR_LIST> neighborList;															//隣接ノードリスト

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

	vector<ROUTING_DATA*> routing;																//ルーティングテーブル
	vector<LOCATION> nodePos;																		//ノード位置情報
	vector<vector<short>> path;																	//DSR用経路情報　　//MA利用型のマルチパスルーティングではシングルパス用になる
	vector<vector<short>> path1;																//マルチパス用１
	vector<vector<short>> path2;																//マルチパス用２
	vector<short> pathNum;																		//経路判断用
	vector<short> path1SegSize;																				//path1の現在のセグメントサイズ
	vector<short> path2SegSize;																				//path2の現在のセグメントサイズ
	vector<SIMUTIME> requestTime;																	//各ノード宛ての経路探索パケット送信時刻
	vector<int> floodSeq;																			//各ノードからのフラッディングシーケンス
	vector<MA*> ma;																					//保持中モバイルエージェント
	vector<LAB> gab;
};

//シミュレーションクラス
class SIMU{
public:
	enum area { CIRCLE, SQUARE, GRID, MESH };
	enum mesh { IDEAL, NONE, CENTRAL, RAOLSR, NEIGHBOR, AGENT };
	enum mac {NORMAL, OR_JOJI_ACK, OR_JOJI_RTS};												//MACの種類
private:
	SIMUTIME now;																						//現在時刻
	SIMUTIME last;																						//前回処理時刻
	area areaId;																						//シミュレーションエリアタイプ
	mesh meshId;																						//メッシュ方式
	mac macId;																							//MACタイプ
	int areaSize;																						//エリアサイズ
	char gridLine;																						//格子線数
	LOCATION gridPoint[GRIDNUM + 1][GRIDNUM + 1];											//格子の交差点座標
	short mapNum;																						//MAP数
	short staNum;																						//STA数
	short interval;																					//発生時間（メッシュネットワークで使用: 単位秒）
	double staReqRate;																				//STA要求発生レート
	short staReqNum;																					//STA要求数（メッシュネットワークで使用）
	short staReqFail;																					//要求失敗回数
	double packetSize[50];																			//総送信パケットサイズ（パケットの種別ごと）
	double delay;																						//経路構築遅延
	double transDelay;																				//送信遅延
	int transDelayCnt;																				//送信遅延計測パケット数
	short centerId;																					//エリアの最中心ノード座標
	int totalSendUDP;																					//総送信UDPデータ
	int totalReceiveUDP;																				//送受信UDPデータ
	int totalTcpData;																					//総TCPデータ
	SIMUTIME totalTcpTime;																			//総TCP通信時間																					//
	bool MAlocation;																					//MAが位置情報管理を行うかを表すフラグ
	
	double rate; 
public:
	SIMU(area, mac = NORMAL); 																						//コンストラクタ
	~SIMU();																								//デストラクタ
	LIST<EVENT> list;																					//イベントリストオブジェクト
	void processEvent();																				//イベント処理
	SIMUTIME getNow(void)	{ return now; }													//現在時刻の取得
	area getArea(void)		{ return areaId; }												//エリアタイプの取得
	mesh getMesh(void)		{ return meshId; }												//メッシュ方式の取得
	mac getMac(void)			{ return macId; }													//MACタイプの取得
	int getAreaSize(void)	{ return areaSize; }												//エリアサイズの取得
	char getGridLine(void)	{ return gridLine; }												//格子線数の取得
	LOCATION getGridPoint(char x, char y)	{ return gridPoint[x][y]; }
	short getMAP(void)		{ return mapNum; }												//MAP数の取得
	short getSTA(void)		{ return staNum; }												//STA数の取得
	short getInterval(void)	{ return interval; }												//発生間隔の取得
	double getReqRate(void)	{ return staReqRate; }											//STA要求発生レートの取得
	void increReqNum(void)	{ staReqNum++; }													//STA要求発生数のインクリメント
	short getReqNum(void)	{ return staReqNum; }											//STA要求数の取得
	void increReqFail(void)	{ staReqFail++; }													//要求失敗回数のインクリメント
	short getReqFail(void)	{ return staReqFail; }											//要求失敗回数の取得
	void increPacket(char id, short size){ packetSize[id] += size; }					//総送信パケットサイズのインクリメント
	double getPacket(char id)	{ return packetSize[id]; }									//総送信パケットサイズの取得
	void increDelay(double val){ delay += val; }												//遅延のインクリメント
	double getDlay(void)		{ return delay; }													//遅延の取得
	void increTransDelay(SIMUTIME t)	{ transDelay = transDelay + t.dtime(); }		//転送遅延のインクリメント
	double getTransDelay(void)	{ return transDelay; }										//転送遅延の取得
	void increTransDelayCnt(void)	{ transDelayCnt++; }										//転送遅延計測パケット数のインクリメント
	int getTransDelayCnt(void)	{ return transDelayCnt; }									//転送遅延計測パケット数の取得
	short getCenter(void)	{ return centerId; }												//中心ノードの取得
	void increSendUdp(int val) { totalSendUDP += val; }									//総送信UDPデータのインクリメント
	int getSendUdp(void)	{ return totalSendUDP; }											//総送信UDPデータの取得
	void increReceiveUdp(int val) { totalReceiveUDP += val; }							//総受信UDPデータのインクリメント
	int getReceiveUdp(void)	{ return totalReceiveUDP; }									//総受信UDPデータの取得
	void increTcpData(int val)	{ totalTcpData += val; }									//総TCPデータのインクリメント
	int getTcpData(void)		{ return totalTcpData; }										//総TCPデータの取得
	void increTcpTime(SIMUTIME t) { totalTcpTime = totalTcpTime + t; }				//総TCP通信時間の更新
	SIMUTIME getTcpTime(void){ return totalTcpTime; }										//総TCP通信時間の取得
	void setMAloc(void)			{ MAlocation = true; }										//MAが位置情報管理をするためのフラグを立てる
	bool getMAloc(void)			{ return MAlocation; }										//MAが位置情報管理をするか表すフラグの取得

	void setRate(double val)	{ rate = val; }
	double getRate(void)			{ return rate; }

	void newNode(SIMUTIME, SIMUTIME, LOCATION, NODE::move, NODE::route);				//ノードの作成
	void makeStaReq(short, double);																//STA要求の作成
	void makeMesh(mesh, short, short, double, SIMUTIME);												//メッシュネットワークの作成
	void checkConnectMap(void);																	//接続MAPの確認
	
	_NODE node;
	vector<MA*> ma;																					//存在中モバイルエージェント
	vector<LAB> gab;

	int counter1, counter2;
};
