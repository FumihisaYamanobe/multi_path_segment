#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//メルセンヌツイスタ乱数発生関数の外部宣言
unsigned long genrand2();																			//メルセンヌツイスタ乱数発生関数の外部宣言
int poison(double);
extern int cntP;
extern int cntS;
extern int cntF;


//extern double rand_d();
////[0,1)の乱数発生
////引数（なし）
////戻り値（発生乱数）
double rand2_d(void){																						
	return genrand2() / 4294967296.0;
}

//
////ノードオブジェクトのコンストラクタ
//NODE::NODE(LIST<EVENT>* list, short idVal, double timeVal, double off, move mid, LOCATION pos, route rid, char sector, 
//			  ANTENNAE::antenna antennaType, double power, bool catMode):EVENT(active, EVENT::Node){	
NODE::NODE(SIMU* ptr, short idval, SIMUTIME tval, SIMUTIME offval, LOCATION pos, move mid, route rid):EVENT(ptr, tval, EVENT::Node, idval){
	bufferPtr = new BUFFER(BUFFERSIZE);	
	mPtr = new MAC(ptr, tval, idval);
	cPtr = new CHANNEL(ptr, mPtr, tval, idval);
	now = ptr->getNow();
	id = idval;																							//ID番号の設定
	activeTime = tval;																				//活動開始時刻の設定
	activeTime.setLessMuSec(id);																	//活動開始時刻（μ未満）の設定
	offTime = offval;																					//終了時刻の設定
	offTime.setLessMuSec(id);																		//終了時刻（μ未満）の設定
	position = pos;																					//位置情報の設定
	destId = -1;																						//目的地IDを-1で初期化
	deriveTime = genrand2() % 10;																	//位置取得タイミング
	moveId = mid;																						//移動タイプの設定
	isRecieveingSignal = false;																	//受信中フラグ（初期値は非受信）
	routeId = rid;																						//ルーティングプロトコルタイプ
	seq = 0;																								//シーケンス番号
	aodvSeq = 0;																						//AODV用シーケンス番号
	overflow = 0;																						//バッファオーバフローによる棄却フレーム数
	neighborLocEnable = false;																		//自動で隣接ノードの位置情報を手に入れられるか
	ptr->gab.push_back(LAB());																		//理想GAB情報
	for(short i = 0; i < 200; i++)
		receivedPacketList.push_back(RECEIVED_PACKET(0, -1, -1, -1, -1));
	pmode = WAIT;																						//電力消費タイプ（初期値は待機）
	usedPower = 0;																						//消費電力
	calcPowerTime = 0;																				//消費電力計算用時刻
	lastInformPos = pos;																				//現在位置を最近の広告位置に設定


//	remainPower = power;																			//残存電力
//	powerDecreTime = 0;																			//電力消費計算時刻
//	relayRate = 0;																					//中継率（セルフィッシュノード用）
//	categoryMode = catMode;																		//カテゴリモードの設定
//	catNum = (categoryMode == false) ? 1 : 4;	
//カテゴリモードによりカテゴリ数を決定
//	macPtr = new MAC(id);																			//MACオブジェクトの作成																				//MACオブジェクトの登録
//	activeCategory = 0;																			//送信処理をしているカテゴリ
//	block = 0;																						//通信切断による棄却フレーム数
//	retransfail = 0;																				//再送上限による棄却フレーム数
}

NODE::~NODE(){
//	cout << "delete node " << this << endl;
	delete bufferPtr;
	delete mPtr;
	delete cPtr;
}

//ノードイベント処理（process関数）
bool NODE::process(void){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	now = sPtr->getNow();																			//現在時刻
	// cout << "node " << now.dtime() << endl;
	// sPtr->list.orderShow();
	// cout << sPtr->node[79]->getAodvSeq() << endl;
	for(short i = 0; i < (short)sPtr->node.size(); i++){									//存在するノード数だけ処理
		NODE* nPtr = sPtr->node[i];																	//処理対象ノードオブジェクト
		nPtr->setNow(now);																				//ノードのイベント時刻を更新
		switch(nPtr->getMove()){																		//移動タイプによる場合分け
			case NO:																							//移動なしの場合
				nPtr->noMove();
				break;																							//位置変更はしない
			case RWP:																						//ランダムウェイポイントの場合
				if(timeCompare(now, nPtr->getPauseRelease()))										//停止解除時刻が未設定か過ぎていた場合
					nPtr->randomWayPoint();																		//ランダムウェイポイント処理				
				break;
			case GRID:																						//グリッド移動の場合
				if(timeCompare(now, nPtr->getPauseRelease()))										//停止解除時刻が未設定か過ぎていた場合
					nPtr->gridWayPoint();																		//グリッド移動処理				
				break;
		}
		if(now.getLessSec() / 100000 == nPtr->getDeriveTime())								//位置情報取得タイミングの場合
			nPtr->posDerive();																				//位置情報の取得
		nPtr->neighborList.clear();																	//隣接ノード情報の初期化
	}
	if(sPtr->getMAloc()){																			//MAへ位置情報送信フラグが立っているなら
		for(short i = 0; i < (short)sPtr->node.size(); i++)
			sPtr->node[i]->sendInfoemLoc();															//位置情報パケット送信
		if(now.dtime() > 30){
			static double maxDif = 0;
			static LOCATION posA, posB;
			static int cnt, cnt2;
			static double totalDif;
			static int totalLinkDif;
			double nowTotalDif = 0;
			for(short i = 0; i < (short)sPtr->node.size(); i++){
				double dif = dist(sPtr->ma[0]->nodePos[i], sPtr->node[i]->getPos());
				totalDif += dif;
				nowTotalDif += dif;
				if(dif > maxDif)
					maxDif = dif;
			}
			for(short i = 0; i < (short)sPtr->node.size() - 1 ; i++){
				for(short j = i + 1; j < (short)sPtr->node.size(); j++){
					if(dist(sPtr->node[i]->getPos(), sPtr->node[j]->getPos()) <= RANGE){
						if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[j]) > RANGE)
							totalLinkDif++;
					}
					else{
						if(dist(sPtr->ma[0]->nodePos[i], sPtr->ma[0]->nodePos[j]) <= RANGE)
							totalLinkDif++;
					}
				}
			}
			cnt2 += sPtr->node.size() * (sPtr->node.size() - 1) / 2;
			cnt += sPtr->node.size();
			//if(now.getSec() % 1 == 0 && now.getLessSec() == 0)
			//	cout << now.dtime() << "," << (double)sPtr->counter2 / sPtr->counter1 << "," << maxDif << "," 
			//	<< totalDif / cnt << "," << (double)totalLinkDif / (cnt2 += sPtr->node.size() * (sPtr->node.size() - 1) / 2 * 100) 
			//	<< "\t" << cntP << "\t" << cntF << "\t" << cntS << endl;
		}
	}
	calcDistance();																					//ノード間距離計算および隣接ノード登録
	if(sPtr->getArea() == SIMU::MESH)
		sPtr->checkConnectMap();
	for(short i = 0; i < (short)sPtr->node.size(); i++)									//存在するノード数だけ処理
		if(sPtr->node[i]->getMAC()->checkIdling() &&  sPtr->node[i]->getMAC()->getState() == MAC::Idle)
			sPtr->node[i]->checkPacket();
	addTime(100000);																					//次のイベント処理時刻へ更新
	sPtr->list.insert(this);																		//イベントリストへ追加
	static bool flag = false;																		//STA要求発生フラグ（降りていれば発生）の初期化
	if(sPtr->getArea() != SIMU::MESH || flag)													//メッシュネットワークでないかフラグがたっていれば
		return true;																						//以降の処理をせずに終了（
	int counter = 0;																					//ルーティング未設定数のカウンタ
	for(short i =0 ; i < (short)sPtr->getMAP(); i++){										//各MAPのルーティング未設定箇所をカウント
		for(short j = 0; j < (short)sPtr->getMAP(); j++){
			if(sPtr->node[i]->routing[j]->getNext() == -1)
				counter++;
		}
	}
	if(!counter){																						//ルーティング未設定箇所がなくなったら
		flag = true;																						//フラグを立てる
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(poison(sPtr->getReqRate())), 
				TIMEOUT::StaReq, genrand2() % sPtr->getSTA() + sPtr->getMAP());			//STA要求の作成
		toPtr->setFlag(false);																			//フラグはおろしておく
		short dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();							//STA要求の宛先
		while(dest == toPtr->getNid())																//宛先が自身と同じである限り
			dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();									//宛先を再設定
		toPtr->setDest(dest);																			//宛先STAの登録
		sPtr->list.insert(toPtr);																		//イベントリストへ登録
		for(char i = 0; i < (char)sPtr->ma.size(); i++){										//MAの数だけ
			toPtr = new TIMEOUT(sPtr, now, TIMEOUT::MeshMa, sPtr->ma[i]->getNid());			//エージェント移動タイムアウトオブジェクトの作成
			toPtr->setMa(sPtr->ma[i]);																		//対応MAの登録
			sPtr->list.insert(toPtr);																		//イベントリストへ登録		
		}
		//short relayCnt[200] = { 0 };
		//for(short i = 0; i < sPtr->getMAP() - 1; i++){
		//	for(short j = i + 1; j < sPtr->getMAP(); j++){
		//		short source = i;
		//		relayCnt[source]++;
		//		while(sPtr->node[source]->routing[j]->getNext() != j){
		//			source = sPtr->node[source]->routing[j]->getNext();
		//			relayCnt[source]++;
		//		}
		//		relayCnt[j]++;
		//	}
		//}
		//for(short i = 0; i < sPtr->getMAP(); i++){
		//	cout << i << "\t" << relayCnt[i] << endl;
		//}
	
	}
	return true;																						//オブジェクトは破棄しない
}

//ノード位置の初期化
//引数（なし）
//戻り値（なし）
void NODE::posInit(void){
	SIMU* sPtr = getSimu();
	int x, y;																							//座標用変数
	SIMUTIME t;																							//時刻用変数
	char line = sPtr->getGridLine();																//格子線数
	LOCATION tmpPos, centerPos;																	//位置情報用変数
	switch(sPtr->getArea()){																		//エリアモデルによる場合分け
	case SIMU::CIRCLE:																				//円形エリアの場合
		x = genrand2() % (AREA) - AREA / 2;															//暫定x座標
		y = genrand2() % (AREA) - AREA / 2;															//暫定y座標
		tmpPos.set(x, y, t);																				//暫定位置情報
		centerPos.set(0, 0, t);																			//シミュレーションエリアの中心
		while(dist(tmpPos, centerPos) > sPtr->getAreaSize()){									//暫定位置が円の外ならば
			x = genrand2() % (AREA) - AREA / 2;															//暫定x座標の更新
			y = genrand2() % (AREA) - AREA / 2;															//暫定y座標の更新
			tmpPos.set(x, y, t);																				//暫定位置情報の更新
		}
		position = tmpPos;																				//暫定値を位置情報へ登録
		break;
	case SIMU::SQUARE: case SIMU::MESH:															//正方形エリアかMESHエリアの場合
		x = genrand2() % (AREA);																		//x座標
		y = genrand2() % (AREA);																		//y座標
		position.set(x, y, t);																			//位置情報の登録
		break;
	case SIMU::GRID:																					//格子エリアの場合
		gridId = genrand2() % (line * line);													//格子番号の決定
		position = sPtr->getGridPoint(gridId % line, gridId / line);					//位置情報の登録
		switch(genrand2() % 4){																		//目的格子点は外周の辺のどこか
		case 0:
			destId = genrand2() % line;
			break;
		case 1:
			destId = (genrand2() % line) * line + 10;
			break;
		case 2:
			destId = genrand2() % 10 + 110;
			break;
		case 3:
			destId = (genrand2() % line) * line;
			break;
		}
		while(destId == gridId){																	//目的地が現在地と同じ場合でない場合
			switch(genrand2() % 4){																		//目的地の再設定
			case 0:
				destId = genrand2() % line;
				break;
			case 1:
				destId = (genrand2() % line) * line + 10;
				break;
			case 2:
				destId = genrand2() % 10 + 110;
				break;
			case 3:
				destId = (genrand2() % line) * line;
				break;
			}
		}
		break;
	}
	posDerive();
	lastInformPos = position;
}

//ノード再配置
//引数（近づけたいノードID（なければ-1)）
//戻り値（なし）
void NODE::replace(short id){
	LOCATION pos(genrand2() % (AREA), genrand2() % (AREA));							//再配置となる座標
	if(id != -1){																					//近づけたいノードが存在するなら
		while(dist(getSimu()->node[id]->getPos(), pos) >= RANGE)										//そのノードの通信範囲にない限り
			pos = LOCATION(genrand2() % (AREA), genrand2() % (AREA));						//場所を再決定
	}
	position.setX(pos.getX());
	position.setY(pos.getY());
}


//速度の設定
//引数（なし）
//戻り値（なし）
void NODE::speedSet(void){
	speed = rand2_d() * 9.0 + 1.0;																//移動速度の設定
}

//位置情報の取得
//引数（なし）
//戻り値（なし）
void NODE::posDerive(void){
	int x = position.getX() + (int)((2 * rand2_d() - 1) * LOCERR);						//取得位置座標（ｘ座標）
	int y = position.getY() + (int)((2 * rand2_d() - 1) * LOCERR);						//取得位置座標（ｙ座標
	derivePos[1].set(derivePos[0].getX(), derivePos[0].getY(), derivePos[0].getT());													//直前取得座標の設定（初期値は最新と同じ）
	derivePos[0].set(x, y, position.getT());													//最新取得座標の設定
}

//目的地の初期化
//引数（なし）
//戻り値（なし）
void NODE::destInit(void){
	SIMU* sPtr = getSimu();
	int x, y;																							//座標用変数
	double angle;																						//円形エリアの偏角用変数
	char newDest;
	char line = sPtr->getGridLine();																//格子線数
	switch(moveId){																					//移動タイプによる場合分け
	case RWP:																							//ランダムウェイポイントの場合
		switch(sPtr->getArea()){																		//エリアタイプによる場合分け
		case SIMU::CIRCLE:																				//円形エリアの場合
			angle = rand2_d() * 2 * 3.14159265;															//偏角
			x = (int)(AREA * cos(angle));																	//極座標系を利用した目的地座標（x）
			y = (int)(AREA * sin(angle));																	//極座標系を利用した目的地座標（y）
			break;
		case SIMU::SQUARE: case SIMU::MESH:															//正方形エリアかMESHエリアの場合
			newDest = genrand2() % 4;																		//目的地辺IDを決定
			while(newDest == destId)																		//今と同じ辺が選ばれる限り
				newDest = genrand2() % 4;																		//目的地辺IDを再決定
			destId = newDest;
			switch(destId){																					//4辺のどこかを目的地とする
			case 0:																								//下辺の場合
				x = genrand2() % (AREA + 1);
				y = 0;
				break;
			case 1:																								//右辺の場合
				x = AREA;
				y = genrand2() % (AREA + 1);
				break;
			case 2:																								//上辺の場合
				x = genrand2() % (AREA + 1);
				y = AREA;
				break;
			case 3:																								//左辺の場合
				x = 0;
				y = genrand2() % (AREA + 1);
				break;
			}
			break;
		default:
			cout << "Contradiction between area model and move model" << endl,  exit(1);
			break;
		}
		moveDestPos.set(x, y);																		//目的地の設定
		pause.setTime(PAUSE_SEC, PAUSE_MSEC * 1000);											//目的地到達時の停止時間の設定
		pauseReleaseTime.setTime(-1, 0);															//停止解除時刻の初期化
		break;
	case GRID:																							//格子移動の場合
		if(sPtr->getArea() != SIMU::GRID)															//格子エリアでなければエラー
			cout << "Contradiction between area model and move model" << endl,  exit(1);
		if(destId % line > gridId % line){															//目的地が現在地より東にある場合
			if(destId / line > gridId / line){															//目的地が現在地より北にある場合
				if(genrand2() % 2)																				//1/2の確率で
					gridId++;																						//東へ進む
				else
					gridId += line;																				//北へ進む
			}
			else if(destId / line == gridId / line)													//目的地が真東にあるなら
				gridId++;																							//東へ進む
			else{																									//目的地が現在地より南にある場合
				if(genrand2() % 2)																				//1/2の確率で
					gridId++;																						//東へ進む
				else
					gridId -= line;																				//南へ進む
			}
		}
		else if(destId % line == gridId % line){													//目的地が現在地より真北か真南にある場合
			if(destId / line > gridId / line)															//目的地が現在地より真北にある場合
				gridId += line;																					//北へ進む
			else																									//目的地が現在地より真北南にある場合
				gridId -= line;																					//南へ進む
		}
		else{																									//目的地が現在地より西にある場合
			if(destId / line > gridId / line){															//目的地が現在地より北にある場合
				if(genrand2() % 2)																				//1/2の確率で
					gridId--;																						//西へ進む
				else
					gridId += line;																				//北へ進む
			}
			else if(destId / line == gridId / line)													//目的地が真西にあるなら
				gridId--;																							//西へ進む
			else{																									//目的地が現在地より南にある場合
				if(genrand2() % 2)																				//1/2の確率で
					gridId--;																						//西へ進む
				else
					gridId -= line;																				//南へ進む
			}
		}
		if(gridId < 0 || gridId >= line * line)
			cout << "gridId error" << endl, exit(1);
		moveDestPos = sPtr->getGridPoint(gridId % line, gridId / line);				//目的地位置情報の登録
		pause.setTime(PAUSE_SEC, PAUSE_MSEC * 1000);											//目的地到達時の停止時間の設定
		pauseReleaseTime.setTime(-1, 0);																//停止解除時刻の初期化
		break;
	}
}

//非移動処理
//引数（なし）
//戻り値（なし）
void NODE::noMove(void){
	position.set(position.getX(), position.getY(), getSimu()->getNow());
}

//ランダムウェイポイント移動処理
//引数（なし）
//戻り値（なし）
void NODE::randomWayPoint(void){
	if(toDestination(moveDestPos)){																//目的地に着いたら
		destInit();
		speedSet();																							//速度の設定
		pauseReleaseTime = now;																			//停止解除時刻の初期化
		pauseReleaseTime.addTime(pause);																//停止解除時刻の設定
	}
	return;
}

//格子移動処理
//引数（なし）
//戻り値（なし）
void NODE::gridWayPoint(void){
	SIMU* sPtr = getSimu();
	if(toDestination(moveDestPos)){																//目的地に着いたら
		if(gridId == destId){																			//最終目的地に着いたら
			char line = sPtr->getGridLine();
			switch(genrand2() % 4){																		//次の最終目的地は外周の辺のどこか
			case 0:
				destId = genrand2() % line;
				break;
			case 1:
				destId = (genrand2() % line) * line + 10;
				break;
			case 2:
				destId = genrand2() % 10 + 110;
				break;
			case 3:
				destId = (genrand2() % line) * line;
				break;
			}
			while(destId % line == gridId % line || destId / line == gridId / line){		//現在地と目的地が同じ辺にあるなら
				switch(genrand2() % 4){																		//次の最終目的地を再決定する
				case 0:
					destId = genrand2() % line;
					break;
				case 1:
					destId = (genrand2() % line) * line + 10;
					break;
				case 2:
					destId = genrand2() % 10 + 110;
					break;
				case 3:
					destId = (genrand2() % line) * line;
					break;
				}
			}
		}
		destInit();
		speedSet();																							//速度の設定
		pauseReleaseTime = now;																			//停止解除時刻の初期化
		pauseReleaseTime.addTime(pause);																//停止解除時刻の設定
	}
}	

//目的地までにの移動処理
//引数（目的地）
//戻り値（目的地に到達したら真，到達しなければ偽）
bool NODE::toDestination(LOCATION dest){
	bool flag;																							//目的地到達フラグ
	int x = position.getX();																		//現在位置（ｘ座標）
	int y = position.getY();																		//現在位置（ｙ座標）
	int dx = dest.getX();																			//目的地（ｘ座標）
	int dy = dest.getY();																			//目的地（ｙ座標）
	int difX = dx - x;																				//目的値との差（ｘ座標）
	int difY = dy - y;																				//目的値との差（ｙ座標）
	int newX, newY;																					//移動後の座標
	double dist = sqrt(pow((double)difX, 2.0) + pow((double)difY, 2.0));				//目的地までの残り距離
	if(dist > speed * 100 * 0.1){																	//移動距離が残り距離より短い場合
		bool isNegative = (difX < 0);																	//ｘ座標の差が負であるかをチェック
		if(isNegative)																						//負の場合
			difX = -difX;																						//正に符号変換
		difX = (int)(difX / dist * speed * 100.0 * 0.1 + 0.5);								//ｘ座標の移動分を計算（端数は四捨五入）
		if(isNegative)																						//先ほど負であったら
			difX = -difX;																						//符号を負に戻す
		isNegative = (difY < 0);																		//ｙ座標の差が負であるかをチェック
		if(isNegative)																						//負の場合
			difY = -difY;																						//正に符号変換
		difY = (int)(difY / dist * speed * 100.0 * 0.1 + 0.5);								//ｙ座標の移動分を計算（端数は四捨五入）
		if(isNegative)																						//先ほど負であったら
			difY = -difY;																						//符号を負に戻す
		newX = x + difX;																					//移動後の座標（ｘ座標）更新
		newY = y + difY;																					//移動後の座標（ｘ座標）更新
		flag = false;																						//目的地到達フラグを偽に
	}
	else{																									//移動距離が残り距離より長い場合（目的地到達）
		newX = dx;																							//移動後の座標は目的地
		newY = dy;																							//移動後の座標は目的地
		flag = true;																						//目的地到達フラグを真に
	}
	position.set(newX, newY, getSimu()->getNow());											//現在位置の更新
	return flag;																						//目的地到達フラグを返す
}

//他ノードとの距離計算
//引数（なし）
//戻り値（なし）
void NODE::calcDistance(void){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	if(routeId == AODV)																				//もしもAODVルーティングなら
		for(short i = 0; i < (short)sPtr->node.size(); i++)								//ノード数だけチェック
			sPtr->node[i]->increAodvSeq();															//AODV用シーケンスのインクリメント
	for(short i = 0; i < (short)sPtr->node.size() - 1; i++){								//ノード数だけチェック
		if(!sPtr->node[i]->getIsActive())															//ノードが非活動状態なら
			continue;																							//これ以降の処理はしない
		for(short j = i + 1; j < (short)sPtr->node.size(); j++){								//計算対象をチェック
			if(!sPtr->node[j]->getIsActive())															//対象が非活動状態なら
				continue;																							//これ以降の処理はしない
			double distance = dist(sPtr->node[i]->getPos(), sPtr->node[j]->getPos());		//対象までの距離
			if(sPtr->getArea() == SIMU::MESH && (i >= sPtr->getMAP() || j >= sPtr->getMAP()));
			else if(distance < RANGE){																			//距離が通信範囲内なら
				sPtr->node[i]->routing[j]->setNext(j);														//対象を自分のの隣接ノードして設定
				sPtr->node[i]->routing[j]->setHop(1);
				sPtr->node[j]->routing[i]->setNext(i);														//対象から見て自分も隣接ノード
				sPtr->node[j]->routing[i]->setHop(1);
				if(routeId == DSR || routeId == MAMR){																				//DSRルーティングの場合
					sPtr->node[i]->path[j].clear();																//対象までの経路情報を初期化
					sPtr->node[i]->path[j].push_back(i);														//対象までの経路を設定
					sPtr->node[i]->path[j].push_back(j);
					sPtr->node[j]->path[i].clear();																//対象から自分までの経路情報を初期化
					sPtr->node[j]->path[i].push_back(j);														//対象から自分までの経路を設定
					sPtr->node[j]->path[i].push_back(i);
				}
			}
			else{																									//距離が通信範囲外なら
				if(sPtr->node[i]->routing[j]->getNext() == j){											//これまでに対象が隣接ノードになっていたら
					sPtr->node[i]->routing[j]->setNext(-1);													//対象を隣接ノードから除外する
					sPtr->node[i]->routing[j]->setHop(-1);
					sPtr->node[j]->routing[i]->setNext(-1);													//対象からみて自分も隣接ノードではなくなる
					sPtr->node[j]->routing[i]->setHop(-1);
					if(routeId == DSR || routeId == MAMR){														//DSRルーティングの場合
						sPtr->node[i]->path[j].clear();															//対象までの経路情報を初期化
						sPtr->node[i]->path[j].push_back(i);													//対象までの経路を設定
						sPtr->node[j]->path[i].clear();															//対象から自分までの経路情報を初期化
						sPtr->node[j]->path[i].push_back(j);													//対象から自分までの経路を設定
					}
				}
				if(routeId == PRO){																				//プロアクティブルーティングの場合
					for(short k = 0; k < (short)sPtr->node.size(); k++){									//全てのノードに対して
						if(sPtr->node[i]->routing[k]->getNext() == j){											//対象を転送ノードに設定している経路の場合
							sPtr->node[i]->routing[k]->setNext(-1);													//その経路を初期化
							sPtr->node[i]->routing[k]->setHop(-1);
						}
						if(sPtr->node[j]->routing[k]->getNext() == i){											//対象からの転送ノードに自身が設定されている場合
							sPtr->node[j]->routing[k]->setNext(-1);													//その経路を初期化
							sPtr->node[j]->routing[k]->setHop(-1);
						}
					}
				}
			}
			if(distance > RANGE * 2)																		//距離が通信範囲の2倍（キャリアセンス可能距離）より大きい場合
				continue;																							//これ以降の処理はしない
			if(distance < RANGE){
				if(sPtr->node[i]->getNeighborLocEnable())
					sPtr->node[i]->nodePos[j] = sPtr->node[j]->getDerivePos(0);
				if(sPtr->node[j]->getNeighborLocEnable())
					sPtr->node[j]->nodePos[i] = sPtr->node[i]->getDerivePos(0);
			}
			NEIGHBOR_LIST listI(i, distance);																//自分から対象への隣接情報オブジェクト（位置情報なし）
			NEIGHBOR_LIST listJ(j, distance);																//自分から対象への隣接情報オブジェクト（位置情報なし）
			short first = 0;																					//これ以降はリストへの挿入処理
			short last = sPtr->node[i]->neighborList.size();										//挿入アルゴリズムについては省略
			short here = (first + last) / 2;
			while(first < last){
				if(sPtr->node[i]->neighborList[here].getDist() > distance)
					last = here;
				else
					first = here + 1;
				here = (first + last) / 2;
			}
			sPtr->node[i]->neighborList.insert(sPtr->node[i]->neighborList.begin() + here, listJ);
			first = 0;
			last = sPtr->node[j]->neighborList.size();
			here = (first + last) / 2;
			while(first < last){
				if(sPtr->node[j]->neighborList[here].getDist() > distance)
					last = here;
				else
					first = here + 1;
				here = (first + last) / 2;
			}
			sPtr->node[j]->neighborList.insert(sPtr->node[j]->neighborList.begin() + here, listI);
		}
	}
}

//送信バッファのパケット存在チェック
//引数（リストオブジェクト，ノードオブジェクトポインタ）
//戻り値：（なし）
void NODE::checkPacket(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = getSimu()->getNow();
	//if(timeCompare(now, SIMUTIME(14,900000)) && getNid() == 248){
	//if(getNid() == 334){	
	//	cout << now.dtime() << "\t" << getNid() << " チェックパケット " << sPtr->node[334]->path[450].size() << endl;
	//	sPtr->node[334]->queueShow();
	//}
	if(!bufferPtr->queue.size())																	//バッファが空の場合
		return;																								//これ以降の処理はしない
	PACKET* pPtr = bufferPtr->queue[0];																//先頭のパケットオブジェクト																					
	if(sPtr->list.find(pPtr))																		//既にイベントリストに登録されている場合
		return;																								//これ以降の処理はしない
	bool flag = true;																					//パケット消去処理に関するフラグ
	while(timeCompare(now, pPtr->getSendStart() + SIMUTIME(TTLTIME))){				//パケットの生存時間を過ぎてるパケットがある限り
		if(pPtr->getType() == PACKET::Tcp && getNid() == pPtr->getSource()){				//パケットがTCPで自身が送信元なら
			TIMEOUT* ptr = new TIMEOUT(sPtr, now + 100000, TIMEOUT::Segment, getNid());	//タイムアウトオブジェクトの作成	
			pPtr->getSeg()->setTimeout(ptr);																//セグメントにタイムアウトを登録
			ptr->setSegment(pPtr->getSeg());																//タイムアウトに対応セグメントを登録
			sPtr->list.insert(ptr);																			//タイムアウトをイベントに追加
		}
		bufferPtr->decreLength(pPtr->getSize());													//行列長をパケットサイズだけ減らす
//		cout << now.dtime() << "   old packet discarded!! " << endl;
		delete pPtr;																						//パケットオブジェクトを消去
		bufferPtr->queue.erase(bufferPtr->queue.begin());										//パケットを消去
		if(!bufferPtr->queue.size()){																	//バッファが空の場合
			flag = false;																						//フラグをおろす
			break;																								//while文を終了
		}
		pPtr = bufferPtr->queue[0];																	//次の先頭パケットに対して同様の処理を行う
	}
	if(flag == false)																					//バッファが空の場合
		return;																								//これ以降の処理はしない
	PACKET::ptype type = pPtr->getType();														//パケットタイプ
	if(type == PACKET::Udp || type == PACKET::Tcp || type == PACKET::Ack
		|| type == PACKET::MigRep || type == PACKET::RerrAodv || type == PACKET::RerrDsr
		|| type == PACKET::StaReq || type == PACKET::StaRep || type == PACKET::MapReq
		|| type == PACKET::MapRep || type == PACKET::LabCenter || type == PACKET::LabNeighbor
		|| type == PACKET::MigRep || type == PACKET::InformLoc || type == PACKET::MrRep || type == PACKET::MrReq){//ルーティング制御パケット以外のユニキャストパケットの時
		switch(routeId){													//ルーティングタイプによる場合分け

		//****************************************//
		// MAMRの場合の処理
		// マルチパスが無いときは〇〇
		// シングルパスだったら〇〇
		//****************************************//
		case NODE::MAMR:{
			//if()//２つの経路がある場合
			//else if()//1つの経路しかない場合
			//else //どちらの経路もない場合
			//cout << "test checkpacket --- " << now.dtime() << endl;
			if(pPtr->getSource() == id){																	//自身が送信元の場合
				short dest = pPtr->getDest();																//送信宛先
				if(dest >= (short)sPtr->node.size()){														//宛先がMAだったら　MAの中心座標を使って通信を行う．
					pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
					pPtr->setDpos(sPtr->ma[dest - sPtr->node.size()]->getCenter());							//destのposをMAの中人座標にする
					pPtr->setTime(now);
//					cout << "com for ma --- " << now.dtime() << " \t" << id << endl;
				}
				else{																						//宛先がMA以外だったら
					if(path[dest].size() <= 1){																//pathに経路情報が入っていない場合
						if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//対象に関するルートリクエストの最近の送信が無い場合 
							pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
							pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
							pPtr->setDpos(sPtr->ma[0]->getCenter());							//destのposをMAの中人座標にする						
							pPtr->setReqDest(dest);
							pPtr->increSize(4);
							pPtr->queue(sPtr, true);																//パケットをバッファの先頭へ挿入 
							requestTime[dest] = now;													//ルートリクエスト送信時刻の設定
							cout << "TCP transmission start." << endl; 
							setPath1SegSize((short)TCPDEFAULTSIZE);											//path1とpath2のTCP初期セグメントサイズを設定
							setPath2SegSize((short)TCPDEFAULTSIZE);
							cout << getPath1SegSize() << " " << getPath2SegSize() <<endl;
					//		//cout << "make packet --- " << now.dtime() << "\t" << id << "\t" << type << "\t" << dest << endl;
						}
						else
							return;
					}
					else{
						//path1，path2で割り振る処理　小松さんのを参考にする
						if(path[dest].size() == 2 ){
							if(pPtr->path.size() > 0){																	//マルチパスがなくシングルパスがある場合
								pPtr->path.clear();																		//パケットの経路情報を初期化
							}
							for(char i = 0; i < (char)path[dest].size(); i++)										//経路情報の設定
								pPtr->path.push_back(path[dest][i]);
							pPtr->increSize(path[dest].size() * 4);
							bufferPtr->increLength(path[dest].size() * 4);
						}
						else if(path1[dest].size() > 1 && path2[dest].size() > 1){								//マルチパスがどちらもある場合
							//ノードに直前まで使ってた経路を覚えさせる(pathNum)
							if(pathNum[dest] == 2){
								if(pPtr->path.size() > 0){														//マルチパスがなくシングルパスがある場合
									pPtr->path.clear();																				//パケットの経路情報を初期化
								}
								for(char i = 0; i < (char)path1[dest].size(); i++)										//経路情報の設定
									pPtr->path.push_back(path1[dest][i]);
								pPtr->increSize(path1[dest].size() * 4);
								bufferPtr->increLength(path1[dest].size() * 4);
								pathNum[dest] = 1;
								pPtr->mpath_check = 1;
								cout << "path1 Seg Size:" << getPath1SegSize() << endl;
							//	cout << "path 1 利用！--" << now.dtime() <<  endl;
							}
							else if(pathNum[dest] == 1){
								if(pPtr->path.size() > 0){														//マルチパスがなくシングルパスがある場合
									pPtr->path.clear();																				//パケットの経路情報を初期化
								}
								for(char i = 0; i < (char)path2[dest].size(); i++)										//経路情報の設定
									pPtr->path.push_back(path2[dest][i]);
								pPtr->increSize(path2[dest].size() * 4);
								bufferPtr->increLength(path2[dest].size() * 4);
								pathNum[dest] = 2;
								pPtr->mpath_check = 2;
								cout << "path2 Seg Size:" << getPath2SegSize() << endl;
							//	cout << "path 2 利用！--" << now.dtime() << "\tdest:" << dest << "\ttype:" << getType() << endl;
							}
							else{
								cout << "pathNum error!!!" << endl, exit(1);
							}
						}
						else if(path[dest].size() > 1){																		//無いときは　シングルパスをみる
							if(timeCompare(now, requestTime[dest] + REQUEST_INT)){
								pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
								pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
								pPtr->setDpos(sPtr->ma[0]->getCenter());							//destのposをMAの中人座標にする						
								pPtr->setReqDest(dest);
								pPtr->increSize(4);
								pPtr->queue(sPtr, true);																//パケットをバッファの先頭へ挿入 
								requestTime[dest] = now;													//ルートリクエスト送信時刻の設定
								sPtr->list.insert(pPtr);
								return;
							}
							else{
								if(pPtr->path.size() > 0){																	//マルチパスがなくシングルパスがある場合
									pPtr->path.clear();																		//パケットの経路情報を初期化
								}
								for(char i = 0; i < (char)path[dest].size(); i++)										//経路情報の設定
									pPtr->path.push_back(path[dest][i]);
								pPtr->increSize(path[dest].size() * 4);
								bufferPtr->increLength(path[dest].size() * 4);
								pPtr->mpath_check = 0;
							}
							//cout << "シングルパス！--" << now.dtime() << endl;
						}
						else
							cout << "path error!!!!" << endl, exit(1);

						pPtr->setTime(now);																				//送信時刻の更新
						pPtr->setDpos(nodePos[dest]);
					}
				}
			}
			else{																//自身が送信元でない場合
//				cout << "not make packet --- " << now.dtime() << endl;
				pPtr->setTime(now);
			}
			break;
			//if(pPtr->getSource() == id){											//自身が送信元の場合
			//	short dest = pPtr->getDest();
			//	if(path1[dest].size() > 1 && path2[dest].size() > 1){			//２つの経路がある場合
			//		if(pathNum[dest] == 1){
			//			for(char i = 0; i < (char)path1[dest].size(); i++)		//経路情報の設定
			//				pPtr->path.push_back(path[dest][i]);					//ノードの経路情報をパケットにコピー
			//			pPtr->setTime(now);										//経路構築時間の更新
			//			pPtr->pathNum = 1;
			//			pathNum[dest] = 2;										//次の経路を２に設定
			//		}
			//		else{
			//			for(char i = 0; i < (char)path2[dest].size(); i++)		//経路情報の設定
			//				pPtr->path.push_back(path[dest][i]);					//ノードの経路情報をパケットにコピー
			//			pPtr->setTime(now);										//経路構築時間の更新
			//			pPtr->pathNum = 2;
			//			pathNum[dest] = 1;										//次の経路を１に設定
			//		}
			//	}
			//	else if(path[dest].size() == 2){								//destが隣接ノードの場合
			//		for(char i = 0; i < (char)path[dest].size(); i++)			//経路情報の設定
			//			pPtr->path.push_back(path[dest][i]);
			//		pPtr->setTime(now);
			//	}
			//	else{															//どちらかの経路がない場合
			//		if(path[dest].size() * path1[dest].size() * path2[dest].size() == 1){ //全ての経路がない場合
			//			//パケットのサイズはRREQDSRを使用していいのか
			//			//パケットの作り方はあっているかBOSSに確認 あっている
			//			pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
			//			pPtr->path.push_back(id);																	//パケットの経路情報の作成
			//			pPtr->increSize(4);
			//			pPtr->queue(sPtr, true);																		//パケットをバッファの先頭へ挿入 
			//			requestTime[sPtr->node.size()] = now;														//ルートリクエスト送信時刻の設定
			//		}
			//		else{														//どこかの経路があるなら
			//			if(path[dest].size() > 1){								//pathがある場合（マルチパスじゃない経路）
			//				for(char i = 0; i < (char)path[dest].size(); i++)			//経路情報の設定
			//					pPtr->path.push_back(path[dest][i]);
			//				pPtr->setTime(now);
			//			}
			//			//シングルパスが存在せず，マルチパスの片方が存在する場合は経路構築要求をする
			//			else if(path1[dest].size() > 1 || path2[dest].size() > 1){
			//				pPtr = new PACKET(sPtr, now, PACKET::MrReq, RREQDSR, id, id , sPtr->node.size(), -1);
			//				pPtr->path.push_back(id);																	//パケットの経路情報の作成
			//				pPtr->increSize(4);
			//				pPtr->queue(sPtr, true);																	//パケットをバッファの先頭へ挿入 
			//				requestTime[sPtr->node.size()] = now;														//ルートリクエスト送信時刻の設定
			//			}
			//			else{
			//				cout << "???CheckPacket" << endl;
			//			}
			//		}
			//	}
			//}
			//else																//自身が送信元でない場合
			//	pPtr->setTime(now);
			//break;
		//****************************************//
		// MAMRの場合　以上
		//****************************************//
		}
		case NODE::DSR:																					//DSRの場合
			if(pPtr->getSource() == id){																	//自身が送信元の場合
				short dest = pPtr->getDest();																	//送信宛先
				if(path[dest].size() == 1){																	//経路無しの場合
					if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//対象に関するルートリクエストの最近の送信が無い場合 
						pPtr = new PACKET(sPtr, now, PACKET::RreqDsr, RREQDSR, id, id, dest, -1);		//ルートリクエストパケットの作成
//						cout << now.dtime() << " route request " << id << endl;
						pPtr->path.push_back(id);																		//パケットの経路情報の作成
						pPtr->increSize(4);
						pPtr->queue(sPtr, true);																		//パケットをバッファの先頭へ挿入 
						requestTime[dest] = now;																		//ルートリクエスト送信時刻の設定
					}
					else																									//同一宛先で最近経路探索をしている場合
						return;																								//何もせず処理を終了
				}
				else{																									//経路有りの場合
					//if(path[dest].size() == 0)
					//	cout << id << "\t" << dest << endl;
					if(pPtr->path.size() > 0)
						pPtr->path.clear();																				//パケットの経路情報を初期化
					for(char i = 0; i < (char)path[dest].size(); i++)										//経路情報の設定
						pPtr->path.push_back(path[dest][i]);														//ノードの経路情報をパケットにコピー
					pPtr->increSize(path[dest].size() * 4);
					bufferPtr->increLength(path[dest].size() * 4);
					pPtr->setTime(now);																				//送信時刻の更新
				}
			}
			else																									//自身が送信元でない場合
				pPtr->setTime(now);																				//送信時刻の更新
			break;
		case NODE::AODV:																					//AODVの場合
			if(pPtr->getSource() == id){																	//自身が送信元の場合
				short dest = pPtr->getDest();																	//送信宛先 
				if(routing[dest]->getNext() == -1){															//経路無しの場合
					if(timeCompare(now, requestTime[dest] + REQUEST_INT)){								//対象に関するルートリクエストの最近の送信が無い場合 
						//cout << id << " ルートリクエスト送信" << aodvSeq << endl;
						pPtr = new PACKET(sPtr, now, PACKET::RreqAodv, RREQAODV, id, id, dest, -1);	//ルートリクエストパケットの作成
						pPtr->setAodvSeqS(aodvSeq);																	//AODV用シーケンス（送信元）の設定
						pPtr->setAodvSeqD(routing[dest]->getSeq());												//AODV用シーケンス（送信宛先）の設定
						pPtr->queue(sPtr, true);																		//パケットをバッファの先頭へ挿入 
						requestTime[dest] = now;																		//ルートリクエスト送信時刻の設定
					}
					else																									//同一宛先で最近経路探索をしている場合								
						return;																								//何もせず処理を終了
				}
				else																									//経路有りの場合
					pPtr->setTime(now);																		//送信時刻の更新
			}
			else																									//自身が送信元でない場合
				pPtr->setTime(now);																		//送信時刻の更新
			break;
		case NODE::PRO:																					//プロアクティブルーティングの場合
//			cout << "check packet  " << getNid() << endl;
			pPtr->setTime(now);																				//送信時刻の更新
			break;
		case NODE::GEDIR:
			//cout << "GIDER --- " << sPtr->getNow().dtime() << endl;
			if(pPtr->getSource() == id){																	//自身が送信元の場合
				short dest = pPtr->getDest();																	//送信宛先
				if(dest >= (short)sPtr->node.size()){
					pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
					pPtr->setDpos(sPtr->ma[dest - sPtr->node.size()]->getCenter());
					pPtr->setTime(now);
				}
				else{

					if(sPtr->node[id]->nodePos[dest].getX() < 0)
						cout << now.dtime() << "\t" << dest << "\t" << (int)sPtr->node[id]->routing[dest]->getHop() << "\t" << pPtr->getType() << " dest position error" << endl, exit(1);
					else{
						pPtr->setSpos(sPtr->node[id]->getDerivePos(0));
						pPtr->setDpos(sPtr->node[id]->nodePos[dest]);
						pPtr->setTime(now);																				//送信時刻の更新
					}
				}
			}
			else																									//自身が送信元でない場合
				pPtr->setTime(now);																				//送信時刻の更新
			break;
		}
	}
	else																									//その他の場合
		pPtr->setTime(now);																				//送信時刻の更新
	sPtr->list.insert(pPtr);																		//パケット処理をイベントリストへ追加
}

//パケット中継処理
//引数（元のパケットオブジェクト）
//戻り値（中継パケットオブジェクト）
PACKET* NODE::relayPacket(PACKET* pPtr, short increSize){
	//if(pPtr->getType() == PACKET::RerrDsr)
		//cout << getSimu()->getNow().dtime() << " relay at " << getNid() << "\t" << pPtr->getType() 
		//	<< "--" << pPtr->getSeq() << " size " << pPtr->getSize()  << endl; 
//	cout << now.dtime() << " relay inform " << endl;
	bool duplicateFlag = false;
	RECEIVED_PACKET rPacket = RECEIVED_PACKET(pPtr->getType(), pPtr->getSource(), pPtr->getDest(), pPtr->getSeq(), pPtr->getSendStart());
	for(short i = 0; i < (short)receivedPacketList.size(); i++){
		RECEIVED_PACKET packet = receivedPacketList[i];
		if(packet.getType() == rPacket.getType() && packet.getSource() == rPacket.getSource() 
			&& packet.getDest() == rPacket.getDest() && packet.getSeq() == rPacket.getSeq()
			&& packet.getTime().dtime() == rPacket.getTime().dtime()){
			duplicateFlag = true;
			break;
		}
	}
	if(duplicateFlag){
		//static int cnt;
		//cout << "relay　duplicate receive " << ++cnt << endl;
		return false;
	}
	else{
		receivedPacketList.push_back(rPacket);
		receivedPacketList.erase(receivedPacketList.begin());
	}
	PACKET* newPtr = new PACKET();																//複製用パケット
	*newPtr = *pPtr;																					//元のパケット情報を複製パケットへコピー
	newPtr->setHere(getNid());																		//パケットの現在位置は自分に変更
	newPtr->increSize(increSize);
	//if(newPtr->getType() == PACKET::DummyBroadcast)
	//	cout << getNid() << " が " << newPtr->getDest() << " 行きのパケットを中継 " << endl; 
	if(!newPtr->queue(getSimu(), false)){														//パケットをバッファへ挿入
		delete newPtr;																						//挿入に失敗したら削除
		newPtr = NULL;
	}
	return newPtr;
}

//ルートエラーパケット送信処理
//引数（イベントリスト，ノードオブジェクト，パケットオブジェクト）
//戻り値：なし
void NODE::sendError(PACKET* pPtr){
	//pPtr->showPath();
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	//cout << now.dtime() << "\t" << getNid() << "  ルートエラー処理 " << pPtr->getType() << "\t" << pPtr->getSeq() << endl;
	//queueShow();
	//cout << "エラーパケットは必要？" << endl;
	//mPtr->show(0);
	if(routeId == NODE::PRO){																		//プロアクティブルーティングの場合
		checkPacket();																						//パケットチェック処理
		return;																								//何もしないで終了
	}
	PACKET::ptype type = pPtr->getType();														//パケットタイプ
	if(type != PACKET::Tcp && type != PACKET::Ack && type != PACKET::Udp){			//パケットがデータパケット以外の場合
		checkPacket();																						//パケットチェック処理
		return;																								//何もしないで終了
	}
	short id = getNid();																				//自身のノードID
	short sid = pPtr->getSource();																//送信元ノードID
	switch(routeId){																					//ルーティングタイプによる場合分け
	case NODE::DSR:	case NODE::MAMR:																	//DSRの場合
		if(id == sid){																						//自身がパケットの送信元の場合
			path[pPtr->path.back()].clear();																//宛先までの経路情報をクリア
			path[pPtr->path.back()].push_back(id);														//経路情報の初期化
		}
		else{																									//自身がパケットの送信元でない場合
			if(timeCompare(requestTime[sid] + REQUEST_INT, now)){									//対象へルートエラーもしくはリクエストパケットの最近の送信がある場合
				checkPacket();																						//パケットチェック処理
				return;																								//何もしないで終了
			}
			requestTime[sid] = now;																			//ルートエラー送信時刻の設定
			PACKET* newPtr = new PACKET(sPtr, now, PACKET::RerrDsr, RERRDSR, id, id, sid);//ルートエラーパケットの作成
			newPtr->setErrDest(pPtr->getDest());														//エラー対象ノードの設定
			path[pPtr->path[0]].clear();
			char cnt;
			if(routeId == NODE::DSR){
				for(cnt = 0; pPtr->path[cnt] != id; cnt++);												//パケットの経路情報から自信を探す
				for(char i = cnt; i >= 0; i--)																//ルートエラーパケットへ経路情報の登録
					path[pPtr->path[0]].push_back(pPtr->path[i]);	
			}
			else{
				cout << pPtr->getType() << "\tpath" << pPtr->mamrPath.size() << endl; 
				newPtr->mpath_check = pPtr->mpath_check;
				for(cnt = 0; pPtr->mamrPath[cnt] != id; cnt++);												//パケットの経路情報から自信を探す
				for(char i = cnt; i >= 0; i--)																//ルートエラーパケットへ経路情報の登録
					path[pPtr->mamrPath[0]].push_back(pPtr->mamrPath[i]);	
			}
			//			cout << "エラーパケット送信" << endl;
			if(!newPtr->queue(sPtr, false))																//バッファへのパケット挿入
				delete newPtr;																						//挿入に失敗したら消去
		}
		break;
		case NODE::AODV:																				//AODVの場合
			if(id == sid){																					//自身がパケットの送信元の場合
				routing[pPtr->getDest()]->setNext(-1);													//転送先の初期化											
				routing[pPtr->getDest()]->setHop(-1);													//ホップ数の初期化
			}
			else{																								//自身が送信元でない場合
				if(timeCompare(requestTime[sid] + REQUEST_INT, now)){								//対象へルートエラーもしくはリクエストパケットの最近の送信がある場合
				checkPacket();																						//パケットチェック処理
				return;																								//何もしないで終了
			}
			requestTime[sid] = now;																			//ルートエラー送信時刻の設定
			PACKET* newPtr = new PACKET(sPtr, now, PACKET::RerrAodv, RERRAODV, id, id, sid);//ルートエラーパケットの作成
			newPtr->setErrDest(pPtr->getDest());														//エラー対象ノードの設定
//			cout << "エラーパケット送信" << endl;
			if(!newPtr->queue(sPtr, false))																//バッファへのパケット挿入
				delete newPtr;																						//挿入に失敗したら消去
		}
		break;
	}
}

//バッファ表示
//引数（なし）
//戻り値（なし）
void NODE::queueShow(void){
	cout << "node" << getNid() << " size= " << bufferPtr->getLength() << " ";
	PACKET* pPtr;
	if(getMAC()->getFrame()){
		pPtr = getMAC()->getFrame()->getPacket();
		cout << "f" << pPtr << "-T" << pPtr->getType() << "-Se" << pPtr->getSeq() << "-So" << pPtr->getSource() << "-D" << pPtr->getDest();
		if(pPtr->getType() == PACKET::Ack)
			cout << "-" << pPtr->getSeg()->getTcp() << " ";
		else
			cout << " ";
		cout << "\t";
	}
	if(bufferPtr->queue.size()){
		if(bufferPtr->getLength() <= 0)
			cout << "queue show error " << bufferPtr->queue.size() << "\t" << bufferPtr->getLength() << endl, exit(1);
		for(short i = 0; i < (short)bufferPtr->queue.size(); i++){	
			pPtr = bufferPtr->queue[i];
			cout << pPtr << "-T" << pPtr->getType() << "-Se" << pPtr->getSeq() << "-So" << pPtr->getSource() << "-D" << pPtr->getDest()
				<< "-Si" << pPtr->getSize();
			if(pPtr->getType() == PACKET::Ack)
				cout << "-" << pPtr->getSeg()->getTcp() << " ";
			else
				cout << " ";
			cout << "\t";
		}
	}
	cout << endl;
}

//LAB情報の送信
//引数（なし）
//戻り値（なし）
void NODE::sendLab(short staId, short mapId){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	short id = getNid();																				//ノードID
	LAB lab = LAB(id, now);																			//LAB情報
	gab[staId - sPtr->getMAP()] = lab;															//自信のGAB情報に該当LAB情報を登録
	PACKET* pPtr;																						//LABパケット用オブジェクト
	switch(sPtr->getMesh()){																		//管理方式による場合分け
	case SIMU::CENTRAL:																				//集中管理方式の場合
		pPtr = new PACKET(sPtr, now, PACKET::LabCenter, LABPACKET, id, id, sPtr->getCenter(), -1);//LABパケットの作成
		pPtr->setSTA(staId);																				//STAIDの登録
		pPtr->setLAB(lab);																				//LAB情報の登録
		pPtr->setSeq(seq++);																				//シーケンス番号の設定
		if(!pPtr->queue(sPtr, false))																	//バッファへのパケット挿入
			delete pPtr;																						//挿入に失敗したら消去
		break;
	case SIMU::RAOLSR:																				//RA-OLSRの場合
		pPtr = new PACKET(sPtr, now, PACKET::LabRa, LABPACKET, id, id, -1, -1);			//LABパケットの作成
		pPtr->setSTA(staId);																				//STAIDの登録
		pPtr->setLAB(lab);																				//LAB情報の登録
		pPtr->setSeq(seq++);																				//シーケンス番号の設定
		if(!pPtr->queue(sPtr, false))																	//バッファへのパケット挿入
			delete pPtr;																						//挿入に失敗したら消去
		break;
	case SIMU::NEIGHBOR:	case SIMU::AGENT:														//隣接通達か巡回エージェントの場合
		if(mapId == -1)
			break;
		pPtr = new PACKET(sPtr, now, PACKET::LabNeighbor, LABPACKET, id, id, mapId, -1);	//LABパケットの作成
		pPtr->setSTA(staId);																				//STAIDの登録
		pPtr->setLAB(lab);																				//LAB情報の登録
		pPtr->setSeq(seq++);																				//シーケンス番号の設定
		if(!pPtr->queue(sPtr, false))																	//バッファへのパケット挿入
			delete pPtr;																						//挿入に失敗したら消去
		break;
	}
}

//ルーティングテーブル更新処理
//引数（対象ノードID）
//戻り値：（なし）
void NODE::makeRoutingTable(short you){
	SIMU* sPtr = getSimu();
	for(int i = 0; i < (int)sPtr->node.size(); i++){										//全てのノードに対して更新是非をチェック
		ROUTING_DATA* routeMe = routing[i];															//自信が持つ対象のルーティング情報
		ROUTING_DATA* routeYou = sPtr->node[you]->routing[i];									//相手が持つ対象のルーティング情報
		if(routeYou->getNext() == -1){																//相手が情報を持ってなければ
			if(routeMe->getNext() == you){															//相手が転送先に設定されていたら
				routeMe->setNext(-1);																		//転送情報を消去
				routeMe->setHop(-1);
			}
			continue;																						//相手が設定されていなければ無視
		}
		if(routeYou->getNext() == getNid()){														//相手の転送先が自分の場合
			if(routeMe->getNext() == you){															//自分の転送先が相手なら
				routeMe->setNext(-1);																		//転送情報を消去
				routeMe->setHop(-1);
			}
			continue;																						//自分の転送先が相手でなければ無視
		}
		//＊＊＊相手の転送先が自分でない場合＊＊＊
		if(routeMe->getNext() == -1){																//自分が転送情報を持ってなければ
			routeMe->setNext(you);																		//転送情報を更新
			routeMe->setHop(routeYou->getHop() + 1);
			continue;
		}
		if(routeYou->getHop() + 1 < routeMe->getHop()){										//相手を経由したほうが最短の場合
			routeMe->setNext(you);																		//転送情報を更新
			routeMe->setHop(routeYou->getHop() + 1);
			continue;
		}
		if(routeMe->getNext() == you)																//相手が転送先の場合
			routeMe->setHop(routeYou->getHop() + 1);												//転送情報のホップ数を更新		
	}
}


//消費電力の計算
//引数（なし）
//戻り値（なし）
void NODE::calcPower(void){
	SIMUTIME now = getSimu()->getNow();															//現在時刻
	if(timeCompare(calcPowerTime, now))
		return;
	double consume;																				//単位時間当たりの消費電力
	switch(pmode){																					//ノードの状態により消費電力を決定
	case SLEEP:
		consume = SLEEP_POWER;
		break;
	case WAIT:
		consume = WAIT_POWER;
		break;
	case SEND:
		consume = SEND_POWER;
		break;
	case RECEIVE:
		consume = RECEIVE_POWER;
		break;
	}
	usedPower += consume * (now - calcPowerTime).dtime();								//現在時刻と計算開始時刻より残存電力を更新
	calcPowerTime = now;
}


//位置情報パケットの送信
//引数（なし）
//戻り値（なし）
void NODE::sendInfoemLoc(void){
	SIMU* sPtr = getSimu();
//	if(dist(derivePos[0] , lastInformPos) > sPtr->getRate()){
	if(dist(derivePos[0] , lastInformPos) >= 2000){							//位置情報更新のしきい値（cm)
		sPtr->counter1++;
//		cout << now.dtime() << "\t" << "send inform " << cntP << "\t" << (double)sPtr->counter2 / sPtr->counter1 << endl;
		lastInformPos = derivePos[0];
		PACKET* pPtr = new PACKET(sPtr, now, PACKET::InformLoc, INFORM, getNid(), getNid(), sPtr->node.size() + 0, -1);//InformLocパケットオブジェクトの作成
//		pPtr->setSeq(seq++);
		if(!pPtr->queue(sPtr, false))														//バッファへのパケット挿入
			delete pPtr;																				//挿入に失敗したら消去
	}
}

////AODVRREQ受信時の経路情報チェック
////引数（パケットオブジェクト，転送先）
////戻り値：（なし）
//void NODE::aodvRreqCheck(PACKET* pPtr, int next){
//	bool flag = false;																				//経路情報更新フラグ（初期値非更新）
//	AODV_RREQ* rPtr = pPtr->getAodvRreq();														//パケットが持つRREQオブジェクト
//	int sid = pPtr->getSource();																	//送信元ノード
//	if(sid == id)																						//送信元が自分ならば何もしない
//		return;
//	if(routing[sid]->getSeq() < rPtr->getSourceSeq())										//宛先シーケンスが古い場合
//		flag = true;																						//更新フラグを立てる
//	else if(routing[sid]->getSeq() == rPtr->getSourceSeq())								//宛先シーケンスが同じ場合
//		if(routing[sid]->getHop() > pPtr->getHop())												//パケットの持つホップ数以上ならば
//			flag = true;																						//更新フラグを立てる
//	if(flag == true){																					//更新フラグが立っている場合
//		routing[sid]->setNext(next);																	//ネクストホップの更新
//		routing[sid]->setHop(pPtr->getHop());														//ホップ数の更新
//		routing[sid]->setSeq(rPtr->getSourceSeq());												//シーケンスの更新
//	}
//}
//
////AODVRREP受信時の経路情報チェック
////引数（パケットオブジェクト，転送先）
////戻り値：（なし）
//void NODE::aodvRrepCheck(PACKET* pPtr, int next){
//	bool flag = false;																				//経路情報更新フラグ（初期値非更新）
//	AODV_RREP* rPtr = pPtr->getAodvRrep();														//パケットが持つRREPオブジェクト
//	int sid = rPtr->getDest();																		//リプライが指定する送信元ノード
//	if(sid == id)																						//送信元が自分ならば何もしない
//		return;
//	if(routing[sid]->getSeq() < rPtr->getDestSeq())											//宛先シーケンスが古い場合
//		flag = true;																						//更新フラグを立てる
//	else if(routing[sid]->getSeq() == rPtr->getDestSeq())									//宛先シーケンスが同じ場合
//		if(routing[sid]->getHop() > pPtr->getHop())												//パケットの持つホップ数以上ならば
//			flag = true;																						//更新フラグを立てる
//	if(flag == true){																					//更新フラグが立っている場合
//		routing[sid]->setNext(next);																	//ネクストホップの更新
//		routing[sid]->setHop(pPtr->getHop());														//ホップ数の更新
//		routing[sid]->setSeq(rPtr->getDestSeq());													//シーケンスの更新
//	}
//	requestTime[sid] = -2 * REQUEST_INT;														//ルートリクエストタイムメモリの初期化
//}
//
////ノード間距離チェック
////引数（ノードオブジェクト，チェック距離）
////戻り値（チェック距離以内なら真，以上なら偽）
//bool NODE::distCheck(NODE* nPtr, double range){
//	if(dist(position, nPtr->getPos()) <= range)
//		return true;
//	return false;
//}
//

////経路表示
////引数（宛先ノード）
////戻り値（なし）
//void NODE::pathShow(_NODE node, int dest){
//	int cnt = 1;
//	int next = routing[dest]->getNext();
//	cout << id << "-";
//	while(next != -1 && next != dest && cnt < 20){
//		cout << next << "-";
//		next = node[next]->routing[dest]->getNext();
//		cnt++;
//	}
//	cout << next << endl;
//}
//
