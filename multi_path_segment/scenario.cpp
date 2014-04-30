#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																		//メルセンヌツイスタ乱数発生（ノード移動以外用）
unsigned long genrand2();																		//メルセンヌツイスタ乱数発生（ノード移動用）
double rand_d();																					//メルセンヌツイスタ乱数発生
double rand2_d();																					//メルセンヌツイスタ乱数発生
void sgenrand(unsigned long);																	//メルセンヌツイスタ乱数初期化関数の外部宣言
void sgenrand2(unsigned long);																//メルセンヌツイスタ乱数初期化関数の外部宣言
void scenario(SIMU*);																			//シナリオ作成関数の外部宣言
void result(SIMU*);																				//結果表示関数の外部宣言
void result(const char*, SIMU*);																//結果表示関数の外部宣言

int poison(double rate);

void main(int argc, char *argv[]){
	//sgenrand((unsigned long)time(NULL));													//ノード移動以外用乱数初期化（時間変動させたい場合）
	//sgenrand2((unsigned long)time(NULL));												//ノード移動用乱数初期化（時間変動させたい場合）
	sgenrand(1);																					//ノード移動以外用乱数初期化（時間変動させたくない場合）
	sgenrand2(1);																					//ノード移動用乱数初期化（時間変動させたくない場合）
	//if(argc == 2)
	//	sgenrand2((atoi(argv[1]) - 1) % 20 + 1);																					//ノード移動用乱数初期化（時間変動させたくない場合）
	//cout << atoi(argv[1]) << endl;
	cout.setf(ios::fixed);																		//表示用プロパティ
	cout << setprecision(6);																	//現在時刻

	SIMU simu(SIMU::SQUARE, SIMU::NORMAL);													//シミュレータオブジェクトの作成（正方形エリア）
	//if(argc == 2)
	//	simu.setRate(atoi(argv[1]) / 40.0);
	//else
	//	simu.setRate(1.0);


	simu.setRate(2.5);	


//	SIMU simu(SIMU::CIRCLE, SIMU::NORMAL);													//シミュレータオブジェクトの作成（円形エリア）
//	SIMU simu(SIMU::MESH, SIMU::NORMAL);													//シミュレータオブジェクトの作成（メッシュネットワーク）

	simu.setMAloc();																				//MAが位置情報管理を行う場合に必要（使わない場合にはコメントアウト）
	
	//メッシュネットワーク環境作成（方式，MAP数，STA数，UDP負荷（秒あたりのフロー数），滞在時間（μ秒））
	//simu.makeMesh(SIMU::NEIGHBOR, 1000, 300, 1.0, 100000);

	//ノードの作成 simu.newNode（活動開始時刻，活動終了時刻，位置情報, 移動モデル，ルーティングタイプ）
	//LOCATION pos(AREA / 2, AREA / 2, 0);														//初期値を指定する場合の位置情報宣言
	//SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);									//活動開始時刻，活動終了時刻の初期値設定
	//simu.newNode(activeTime, offTime, pos, NODE::NO, NODE::AODV);				//ノードオブジェクトの作成
	
	//for(short i = 0; i < NODENUM; i++){
	//	LOCATION pos;																				//初期値なしの位置情報宣言は値がランダムになる
	////	//LOCATION pos(i * 6000, 0, 0);														//初期値を指定する場合の位置情報宣言
	//	SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);									//活動開始時刻，活動終了時刻の初期値設定
	//	simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::GEDIR);				//ノードオブジェクトの作成
	//}


	//ノードを作成
	//コマンドラインから入力する場合
	//for(short i = 0; i < atoi(argv[1]) * 100 + 300; i++){
	//	LOCATION pos;																//ノードの初期値はランダムとする
	//	SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);								//活動開始時刻，活動終了時刻の初期値設定
	//	simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::MAMR);				//ノードオブジェクトを作成　ランダムに移動し，ルーティングはMAMRとする
	//}

	for(short i = 0; i < NODENUM; i++){
		LOCATION pos;
		SIMUTIME activeTime, offTime(TIMELIMIT + 1, 0);
		simu.newNode(activeTime, offTime, pos, NODE::RWP, NODE::MAMR);
	}
	for(int i = 0; i < simu.node.size(); i++){
		for(int j = 0; j < simu.node.size(); j++){
			simu.node[i]->pathNum.push_back(1);
			simu.node[i]->path1SegSize.push_back(TCPDEFAULTSIZE);
			simu.node[i]->path2SegSize.push_back(TCPDEFAULTSIZE);
			simu.node[i]->path1[j].push_back(i);
			simu.node[i]->path2[j].push_back(i);
		}
	}

	//for(short i = 1; i < NODENUM; i++)
	//	simu.node[i]->nodePos[0] = simu.node[0]->getPos();
	scenario(&simu);																				//通信シナリオの作成(コマンドライン引数不使用時）
	while(timeCompare(SIMUTIME(TIMELIMIT, 0), simu.getNow()))						//設定時刻に達するまでの処理を以下に記述
		simu.processEvent();																			//イベント処理
	result(&simu);																					//結果表示
}

//通信シナリオ設定関数
void scenario(SIMU* sPtr){
	//for(int i = 0; i < 10; i++){																//UDPフロー作成サンプル	
	//	UDP* udp = new UDP(sPtr, i, SIMUTIME(0,0), 1 * 100, 10 * 1000 * 1000);	//UDPエージェント（引数：sPtr, ID，開始時刻，kbps，送信バイト数）
	//	UDPSINK* udpsink = new UDPSINK(sPtr, i + 15);									//UDPシンク（引数：sPtr, ID）
	//	udp->connectSink(udpsink);																//エージェントとシンクの結合
	//	sPtr->list.insert(udp);																	//UDPエージェントをイベントリストへ追加
	//}
	//short source = genrand2() % (NODENUM - 1) + 1;
	//double rate = sPtr->getRate();
	//if(sPtr->getRate() > 0){
	//	TIMEOUT* toPtr = new TIMEOUT(sPtr, SIMUTIME(0, 0), TIMEOUT::MakeUdp, source);
	//	toPtr->setRate(rate);
	//	sPtr->list.insert(toPtr);
	//}

	//for(int i = 0; i < 1; i++){																//TCPフロー作成サンプル
	//	TCP* tcp = new TCP(sPtr, 1, SIMUTIME(1,0), 500 * 1000 * 1000);				//TCPエージェント（引数：ID，開始時刻，送信バイト数）
	//	TCPSINK* tcpsink = new TCPSINK(sPtr, 0);											//TCPシンク（引数：ID）
	//	tcp->connectSink(tcpsink);																//エージェントとシンクの結合
	//	sPtr->list.insert(tcp);																	//TCPエージェントをイベントリストへ追加
	//}

	TIMEOUT* toPtr = new TIMEOUT(sPtr, SIMUTIME(30, 0), TIMEOUT::MakeTcp, genrand2() % (sPtr->node.size() - 1));
	toPtr->setRate(sPtr->getRate()); 
	sPtr->list.insert(toPtr);

	////３０秒後に一度だけTCP通信を発生させる．
	//for(int i = 0; i < 1; i++){
	//	TCP* tcp = new TCP(sPtr, 1, SIMUTIME(30,0), 1000 * 1000);						//TCPエージェント（引数：ID，開始時刻，送信バイト数）
	//	TCPSINK* tcpsink = new TCPSINK(sPtr, 0);											//TCPシンク（引数：ID）
	//	tcp->connectSink(tcpsink);															//エージェントとシンクの結合
	//	sPtr->list.insert(tcp);																//TCPエージェントをイベントリストへ追加
	//}

	//
	//メッシュネットワークのSTA要求作成（時間，1秒あたりの要求回数）
	//sPtr->makeStaReq(100, 10.0);
	//TIMEOUT* toPtr = new TIMEOUT(sPtr, poison(sPtr->getRate()), TIMEOUT::BackGround, genrand2() % NODENUM);
	//toPtr->setRate(sPtr->getRate());
	//sPtr->list.insert(toPtr);

	if(sPtr->getMAloc())
		MA* maPtr = new MA(sPtr, SIMUTIME(0,0), LOCATION(AREA / 2, AREA / 2), 50 * 100, MA::Distance, 0, -1);
//	sPtr->list.orderShow();
}

//結果表示関数
void result(SIMU* sPtr){
	//全ての送信パケットサイズを表示したい場合には下のコメントアウトを解除（iの上限値はclass.hで設定したパケットの種類数）
	cout << "rate=" << sPtr->getRate() << "の時の結果" << endl;

	for(char i = 0; i < 24; i++)
		cout << sPtr->getPacket(i) << ",";

	//特定の送信パケットサイズを表示したい場合は以下のようにする
	//cout << sPtr->getPacket(PACKET::RreqDsr) << ",";
	//cout << sPtr->getPacket(PACKET::RrepDsr) << ",";
	//cout << sPtr->getPacket(PACKET::RerrDsr) << ",";
	//cout << sPtr->getPacket(PACKET::LabCenter) << ",";
	//cout << sPtr->getPacket(PACKET::LabRa) << ",";
	//cout << sPtr->getPacket(PACKET::LabNeighbor) << ",";
	//cout << sPtr->getPacket(PACKET::MapReq) << ",";
	//cout << sPtr->getPacket(PACKET::MapReqB) << ",";
	//cout << sPtr->getPacket(PACKET::MapRep) << ",";
	//cout << sPtr->getPacket(PACKET::Udp) << ",";
	//cout << sPtr->getPacket(PACKET::Tcp) << ",";
	//cout << sPtr->getPacket(PACKET::Ack) << ",";

	//経路構築失敗率と回数
	cout << (double)sPtr->getReqFail() / sPtr->getReqNum() << ",";
	cout << sPtr->getReqFail() << ",";
	cout << sPtr->getReqNum() << ",";

	//平均遅延（合計遅延を経路構築成功回数で割り算）
	cout << sPtr->getDlay() / (sPtr->getReqNum() - sPtr->getReqFail()) << ",";

	//UDPデータの到達率
	cout << (double)sPtr->getReceiveUdp() / sPtr->getSendUdp() << ",";
	cout << sPtr->getReceiveUdp() << ",";
	cout << sPtr->getSendUdp() << ",";

	//TCPデータのスループット
	cout << sPtr->getTcpData() * 8 / sPtr->getTcpTime().dtime() / 1000 /1000  << ",";

	//パケットの送信遅延
	cout << sPtr->getTransDelay() / sPtr->getTransDelayCnt() << ",";


	//消費電力の計算
	double totalUsedPower = 0;
	for(short i = 0; i < (short)sPtr->node.size(); i++)
		totalUsedPower += sPtr->node[i]->getUsedPower();
	cout << totalUsedPower / sPtr->node.size() << ",";
	if(sPtr->getMAloc())
		cout << sPtr->ma[0]->getMigNum() << ",";
	cout << endl;
}