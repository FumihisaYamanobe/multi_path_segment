#include <stdlib.h>
#include <math.h>
#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//メルセンヌツイスタ乱数外部宣言（ノード移動以外用）
unsigned long genrand2();																			//メルセンヌツイスタ乱数外部宣言（ノード移動用）

//[0,1)の乱数発生
//引数（なし）
//戻り値：（発生乱数）
double rand_d(void){																						
	return genrand2() / 4294967296.0;
}

//ポアソン分布に従う時間間隔乱数の発生
//引数（1秒あたりの発生レート）
//戻り値：(発生乱数 μ秒)
inline int poison(double rate){
	return (int)(-(log(1 - rand_d()) / rate * 1000000));
}

//コンストラクタ
SIMU::SIMU(area id, mac mid){	
	if(AREA % GRIDNUM != 0)																			//エリアサイズが格子数で割り切れない場合
		cout << "grid size error" << endl, exit(1);												//格子サイズエラーエラー
	last.setTime(-1, 0);																				//直前イベント時刻の初期化
	areaId = id;																						//エリアタイプの初期化
	macId = mid;
	areaSize = AREA;																					//エリアサイズの初期化	
	gridLine = GRIDNUM + 1;																			//格子線の初期化
	for(char i = 0; i < gridLine; i++){															//グリッドポイントの設定
		for(char j = 0; j < gridLine; j++){
			LOCATION gridPos(i * areaSize / GRIDNUM, j * areaSize / GRIDNUM);
			gridPoint[i][j] = gridPos;
		}
	}
	staReqRate = 0;																					//STA要求発生レート
	staReqNum = 0;																						//STA要求数（メッシュネットワークで使用）
	staReqFail = 0;																					//要求失敗回数
	for(char i = 0 ; i < 50; i++)
		packetSize[i] = 0;
	transDelay = 0;
	transDelayCnt = 0;
	delay = 0;
	totalSendUDP = 0;
	totalReceiveUDP = 0;
	totalTcpData = 0;
	totalTcpTime = 0;
	MAlocation = false;																				//MAが位置情報管理をするか（デフォルトはしない）

	rate = 0;

	counter1 = 0;
	counter2 = 0; 
}

//デストラクタ
SIMU::~SIMU(){
	for(short i = 0; i < (short)node.size(); i++)
		delete node[i];
	for(int i = 0; i < (int)ma.size(); i++)
		delete ma[i];
}

//シミュレーションイベント処理
//引数（なし）
//戻り値（なし）
void SIMU::processEvent(){
	LINK<EVENT>* first = list.getFirst();														//リストの先頭
	EVENT* eventPtr = first->getObject();														//先頭に格納されているオブジェクト
	delete list.remove(first);																		//格納していた器を削除
	if(eventPtr->getType() == EVENT::Abort){													//シミュレーション終了イベントの場合
		now.setSec(TIMELIMIT);																		//現在時刻をシミュレーション終了時刻に設定
		return;
	}
	now = eventPtr->getEventTime();																//現在時刻を処理イベントの発生時刻に設定
////デバッグ用
//	eventPtr->show(9.0);																				//処理イベント表示
	//if(now.dtime() > 753.8){
	//	//cout << node[57]->path[37].size() << endl;
	//	if(node[57]->path[37].size() == 0)
	//		cout << " out " << endl, exit(1);
	//}
	//node[263]->getMAC()->show(0.88);
	//if(node[657]->getMAC()->getOrPtr())
	//	cout << node[657]->getMAC()->getOrPtr() << "\t" << node[657]->getMAC()->getOrPtr()->getSize() << endl;
	//if(eventPtr->getType() == EVENT::Packet)
	//	cout << eventPtr << endl;
	//if(now.dtime() > 1.025)
	//	cout << node[679]->getMAC()->getListInsert() << endl;
	if(!now.isPositive())																			//現在時刻がマイナスの場合にはエラー
		cout << "negative time " << eventPtr->getType() << endl, now.show(), exit(1);	
	if(timeCompare(last, now)){																	//現在時刻が直前時刻より小さい場合にもエラー
		cout << eventPtr->getType() << "  back time\nnow\t";
		now.show();
		cout << "previous\t";
		last.show();
		exit(1);
	}
	if(eventPtr->getEventTime().getLessMuSec() != eventPtr->getNid()){				//時刻のマイクロ未満とノードIDが一致していなければエラー				
		cout << "time setting error " << eventPtr->getEventTime().getLessMuSec() 
			<< "\t" << eventPtr->getNid() << endl, exit(1);
	}
	if(!eventPtr->process())																		//時刻エラーでない場合該当イベント処理関数を呼び出す
		delete eventPtr;																							//イベント消去のフラグが返ってきたら消去
	last = now;																									//次のイベントのために現在時刻を直前時刻へ移動
//	eventPtr->show(628.4879);																				//処理イベント表示
	//if(timeCompare(now, SIMUTIME(120,800000)))
	//	list.orderShow();
		//exit(1);
}


//ノードの作成
//引数（作成時刻，消滅時刻，位置情報，移動タイプ，ルーティングタイプ）
//戻り値（なし）
void SIMU::newNode(SIMUTIME active, SIMUTIME off, LOCATION pos, NODE::move mid, NODE::route rid)
{
	NODE* nPtr = new NODE(this, (short)node.size(), active, off, pos, mid, rid);	//ノードオブジェクトの作成
	if(timeCompareMu(nPtr->getActiveTime(), now))											//現在時刻よりも活動開始時刻が後なら
		nPtr->setIsActive(false);																		//活動状態フラグを下す
	else																									//現在時刻よりも活動開始時刻が前なら
		nPtr->setIsActive(true);																		//活動状態フラグを立てる
	if(nPtr->getPos().getT().getSec() == -1)													//位置情報が設定されていないなら
		nPtr->posInit();																					//ノード配置処理	
	if(mid != NODE::NO){																				//静止ノードでなければ
		nPtr->speedSet();																					//速度の設定
		nPtr->destInit();																					//目的地の初期化
	}
	if(rid == NODE::PRO || areaId == MESH 
		|| ((nPtr->getRoute() == NODE::GEDIR || nPtr->getRoute() == NODE::MAMR) && !nPtr->getNeighborLocEnable())){		//プロアクティブルーティングかメッシュネットワークかビーコンによる位置情報取得方式の場合
		SIMUTIME beaconTime = active + genrand() % BEACONINT;									//ビーコンタイミングの決定
		BEACON* beaconPtr = new BEACON(this, (short)node.size(), beaconTime);			//ビーコンオブジェクトの作成		
		list.insert(beaconPtr);																			//作成オブジェクトのイベント登録
	}
	short i;
	for(i = 0; i < (short)node.size(); i++){													//既存ノードのルーティングテーブル追加
		ROUTING_DATA* rPtr = new ROUTING_DATA(-1, -1);											//ルーティングデータオブジェクト
		node[i]->routing.push_back(rPtr);															//ノードにルーティングオブジェクトを追加
		node[i]->nodePos.push_back(LOCATION(-1,-1, -1));												//位置情報テーブルを作成
		short last = (short)node[i]->path.size();													//現在の経路テーブルサイズ数（追加テーブルの要素番号と同じ）
		short last1 = (short)node[i]->path1.size();													//経路テーブルサイズ数（マルチパス１用）
		short last2 = (short)node[i]->path2.size();													//経路テーブルサイズ数（マルチパス２用）
		node[i]->path.push_back(vector<short>(1));													//経路テーブルを追加
		node[i]->path1.push_back(vector<short>(1));													//経路テーブルを追加（マルチパス１）
		node[i]->path2.push_back(vector<short>(1));													//経路テーブルを追加（マルチパス２）
		node[i]->path[last][0] = i;																	//追加テーブル情報の初期化
		node[i]->path1[last][0] = i;																//追加テーブル情報の初期化
		node[i]->path2[last][0] = i;																//追加テーブル情報の初期化
		node[i]->requestTime.push_back(SIMUTIME(-10,0));										//ルートリクエストタイムメモリの登録
		node[i]->floodSeq.push_back(-1);																//フラッディングタイムメモリの登録
	}
	node.push_back(nPtr);																			//ノードリストへの作成ノードの追加
	ROUTING_DATA* rPtr;																				//ルーティングデータオブジェクト
	for(short j = 0; j < (short)node.size(); j++){											//作成ノードのルーティングテーブル作成															
		if(j != i )																							//ルーティングの対象が自分以外なら
			rPtr = new ROUTING_DATA(-1, -1);																//オブジェクトの中身は不明		
		else{																									//対象が自分自身なら
			rPtr = new ROUTING_DATA(j, 0);																//自身を宛先としたオブジェクトを作成
			rPtr->setSeq(0);																					//自身のシーケンス番号初期値は0
		}
		node[i]->routing.push_back(rPtr);															//ルーティングオブジェクトを追加
		node[i]->nodePos.push_back(LOCATION(-1,-1, -1));												//位置情報テーブルを作成
		node[i]->path.push_back(vector<short>(1));												//経路テーブルを追加
		node[i]->path1.push_back(vector<short>(1));												//経路テーブルを追加
		node[i]->path2.push_back(vector<short>(1));												//経路テーブルを追加
		node[i]->path[j][0] = i;																		//経路テーブルの初期化
		node[i]->path1[j][0] = i;																		//経路テーブルの初期化
		node[i]->path2[j][0] = i;																		//経路テーブルの初期化
		node[i]->requestTime.push_back(SIMUTIME(-10,0));										//ルートリクエストタイムメモリの登録
		node[i]->floodSeq.push_back(-1);																//フラッディングタイムメモリの登録
	}
	if(node.size() == 1)																				//最初のノードオブジェクトのみ
		list.insert(nPtr);																				//移動用イベントの登録
}

//STA要求の作成
//引数（発生間隔，要求数）
//戻り値（なし）
void SIMU::makeStaReq(short tval, double rate){
	interval = tval;
	staReqRate = rate;
	staReqNum = 0;
	staReqFail = 0;
}

//メッシュネットワークの作成
//引数（MAP数，STA数）
//戻り値（なし）
void SIMU::makeMesh(mesh id, short mnum, short snum, double rate, SIMUTIME stay){
	meshId = id;
	mapNum = mnum;
	staNum = snum;
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(0, 0), NODE::NO, NODE::PRO);	//左下隅に配置
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(0, AREA), NODE::NO, NODE::PRO);//右下隅に配置
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(AREA, 0), NODE::NO, NODE::PRO);//左上に配置
	newNode(0, SIMUTIME(TIMELIMIT + 1, 0), LOCATION(AREA, AREA), NODE::NO, NODE::PRO);//右上に配置
	LOCATION pos;
	for(short i = 4; i < mapNum; i++)															//その他のMAPはランダムに配置
		newNode(0, SIMUTIME(TIMELIMIT + 1, 0), pos, NODE::NO, NODE::PRO);
	NODE* nPtr;																							//結合チェック元のノードオブジェクト
	char* connectFlag;																				//結合状態を示すフラグ(0は未結合，1は結合＆その先を未チェック，2は結合＆その先をチェック済み）
	connectFlag = (char*)malloc(sizeof(char)*mapNum);										//フラグのメモリ領域を確保
	bool meshFlag = false;																			//メッシュネットワークとして成立してるかを示すフラグ
	short trialCnt = 0;
	while(!meshFlag){																					//メッシュネットワークとして成立してない限り以下を実行
		if(++trialCnt > mapNum)
			cout << "mesh network cannot be generated " << endl, exit(1);
		connectFlag[0] = 1;																				//最初のMAPは結合していると仮定
		for(short i = 1; i < mapNum; i++)															//他のMAPは現時点で未結合
			connectFlag[i] = 0;
		bool checkFlag = true;																			//新規結合MAPがあったかを示すフラグ
		while(checkFlag){																					//新規結合MAPがある限り以下を実行
			checkFlag = false;																				//最初は新規結合がないとする
			for(short i = 0; i < mapNum; i++){
				if(connectFlag[i] == 1){																		//結合MAPでその先が未チェックなら
					connectFlag[i] = 2;																				//チェック済みに変更
					nPtr = node[i];																					//チェック元ノードオブジェクトの設定
					for(short j = 0; j < mapNum; j++){
						if(connectFlag[j] == 0 && dist(nPtr->getPos(), node[j]->getPos()) < RANGE){		//未結合ノードでチェック元の送信範囲にいるなら
							connectFlag[j] = 1;																				//結合状態にする
							checkFlag = true;																					//新規結合MAP存在のフラグを立てる
						}
					}
				}
			}
		}
		meshFlag = true;																					//メッシュネットワーク成立と仮定
		for(short i = 0; i < mapNum; i++){
			if(connectFlag[i] == 0){
				meshFlag = false;
				break;
			}
		}
		if(!meshFlag){																						//メッシュネットワーク成立フラグが下りてる場合
			short disconnectId;																				//非結合MAPのID
			short replaceId;																					//位置変更MAPのID
			char maxConnect = 0;																				//最大結合リンク数
			char minConnect = 100;																			//最少結合リンク数
			for(short i = 0; i < mapNum; i++){															//全てのMAPに対して
				char cnt = 0;
				for(short j = 0; j < mapNum; j++)															//結合リンク数を調べる
					if(i != j && dist(node[i]->getPos(), node[j]->getPos()) < RANGE)
						cnt++;
				if(cnt < minConnect){																			//結合リンク数が最少なら
					minConnect = cnt;
					disconnectId = i;																					//最少結合MAPの更新
				}
				if(cnt > maxConnect){																			//結合リンク数が最大なら
					maxConnect = cnt;
					replaceId = i;																						//最大結合MAPの更新
				}
			}
			node[replaceId]->replace(disconnectId);													//最大結合MAPを最少結合MAPの近くに再配置
		}
	}
	free(connectFlag);
	if(meshId == CENTRAL){																			//集中管理方式の場合
		double minDist = AREA;
		for(short i = 0; i < mapNum; i++){
			if(dist(node[i]->getPos(), LOCATION(AREA / 2, AREA /2)) < minDist){
				minDist = dist(node[i]->getPos(), LOCATION(AREA / 2, AREA /2));
				centerId = i;
			}
		}
	}
	for(short i = 0; i < staNum; i++){															//STAはランダムに配置
		newNode(0, SIMUTIME(TIMELIMIT + 1, 0), pos, NODE::RWP, NODE::PRO);
		double minDist = dist(node[0]->getPos(), node[mapNum + i]->getPos());			//接続MAPとの距離（初期値はMAP0との距離）
		short connectMap = 0;																			//接続MAPID（初期値0）
		for(short j = 1; j < mapNum; j++){															//最も近いMAPを求める
			if(dist(node[j]->getPos(), node[mapNum + i]->getPos()) < minDist){
				minDist = dist(node[j]->getPos(), node[mapNum + i]->getPos());
				connectMap = j;
			}
		}
		if(minDist >= RANGE)																				//最も近いMAPとの距離が通信範囲以上なら
			connectMap = -1;																					//接続MAPは無し
		gab[mapNum + i] = LAB(connectMap);															//理想GABへの登録
		for(short j = 0; j < (short)ma.size(); j++)												//エージェントへのGAB登録
			ma[j]->gab[i] = LAB(connectMap);
		for(short j = 0; j < mapNum; j++)															//各MAPのGABへの登録（最初だけは理想的な値）
			node[j]->gab.push_back(LAB(connectMap));
		node[mapNum + i]->setMAP(connectMap);														//STAに接続MAPを登録
		node[mapNum + i]->routing[connectMap]->setHop(1);										//ルーティング情報の初期化
		node[mapNum + i]->routing[connectMap]->setNext(connectMap);
		node[connectMap]->routing[mapNum + i]->setHop(1);										
		node[connectMap]->routing[mapNum + i]->setNext(mapNum + i);
	}
	short source = genrand2() % mapNum;
	if(rate > 0){
		TIMEOUT* toPtr = new TIMEOUT(this, SIMUTIME(poison(rate)), TIMEOUT::MakeUdp, source);
		toPtr->setRate(rate);
		list.insert(toPtr);
	}
	if(meshId == AGENT){
		//巡回エージェントの作成と経路の設定
		LOCATION pos;																						//ダミー位置情報
		short maId = 0;																					//MAのID（複数のMAを作る場合にはここを調整）
		short nodeId = 0;																					//最初に滞在するノード位置
		MA* maPtr = new MA(this, 0, pos, 0, MA::Distance, maId, nodeId);					//エージェントオブジェクトの作成
		maPtr->setSize(30 * 1000 + 12 * staNum);													//サイズの設定（GABは1ノードあたり12バイトと換算）
		maPtr->setStayTime(stay);																		//各ノードでの滞在時間の設定
		for(short i = 0; i < mapNum; i++){												
			maPtr->meshRoute.push_back(i);															//この例では経路は単純にID順
			maPtr->gab.push_back(LAB());																//GAB情報の初期化
		}
		maPtr->meshRoute.push_back(0);																//経路の最後に最初のノードを入れる
	}
}

//接続MAPの確認
//引数（なし）
//戻り値（なし）
void SIMU::checkConnectMap(void){
	for(short i = mapNum; i < mapNum + staNum; i++){										//各STAについてチェックする
		bool flag = false;																				//接続確認必要性フラグ
		if(node[i]->getMAP() == -1)																	//これまで未接続なら
			flag = true;																						//確認フラグを立てる
		else if(dist(node[i]->getPos(), node[node[i]->getMAP()]->getPos()) >= RANGE){	//接続MAPとの距離が通信範囲を超えた場合
			//if(i == 255)
			//	cout << "before " << node[i]->getMAP() << endl;
			node[node[i]->getMAP()]->gab[i - mapNum] = LAB(-1, now);								//該当STAに関するLAB情報の初期化
			node[i]->routing[node[i]->getMAP()]->setHop(-1);										//ルーティング情報の初期化
			node[i]->routing[node[i]->getMAP()]->setNext(-1);
			node[node[i]->getMAP()]->routing[i]->setHop(-1);
			node[node[i]->getMAP()]->routing[i]->setNext(-1);
			flag = true;																						//確認フラグを立てる
		}
		if(flag){																							//確認フラグが立っていたら	
			double minDist = dist(node[i]->getPos(), node[0]->getPos());						//接続MAPとの距離（初期値はMAP0との距離）
			short connectMap = 0;																			//接続MAPID（初期値0）
			for(short j = 1; j < mapNum; j++){															//最も近いMAPを求める
				if(dist(node[i]->getPos(), node[j]->getPos()) < minDist){
					minDist = dist(node[i]->getPos(), node[j]->getPos());
					connectMap = j;
				}
			}
			if(minDist >= RANGE)																				//最も近いMAPとの距離が通信範囲以上なら
				connectMap = -1;																					//接続MAPは無し
			else																									//新接続MAPが存在するなら
				node[connectMap]->sendLab(i, node[i]->getMAP());																	//LAB情報の送信
//			if(i == 255)
//				cout << i << " 接続変更  " << connectMap << endl;
			gab[i] = LAB(connectMap, now);																//理想GABへの登録
			node[i]->setMAP(connectMap);																	//STAに接続MAPを登録
			if(connectMap != -1){
				node[i]->routing[connectMap]->setHop(1);													//ルーティング情報の設定
				node[i]->routing[connectMap]->setNext(connectMap);
				node[connectMap]->routing[i]->setHop(1);
				node[connectMap]->routing[i]->setNext(i);
			}
		}
	}
}

