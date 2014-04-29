#include "class.h"
#include "mobileAgent.h"

unsigned long genrand();																			//メルセンヌツイスタ乱数外部宣言（ノード移動以外用）

//セグメントオブジェクトのコンストラクタ
SEGMENT::SEGMENT(SIMU* ptr, SIMUTIME tval, TCP* tPtr, short sizeval, int seqval, short nid):EVENT(ptr, tval, EVENT::Segment, nid){
	tcpPtr = tPtr;
	size = sizeval;																					//セグメントサイズ
	seq = seqval;																						//シーケンス番号
	ackRepeat = 0;																						//重複受信ACK回数は0
	toPtr = NULL;																						//タイムアウトオブジェクトはなし
	NAretrans = false;																				//重複ACK受信による再送済みフラグをおろす
}

SEGMENT::~SEGMENT(){
	//static int cnt;
	//if(cnt++ % 10 == 0)
	//	cout << "delete " << cnt << endl;
}

int cntP = 0;
int cntF = 0;
//パケットオブジェクトのコンストラクタ
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
	type = pval;																						//パケットタイプ
	size = sval;																						//サイズ
	if(size > 1500 || size <= -2 || size == 0)																	//パケットサイズが1～1500でなければ
		cout << "packet size error" << endl, exit(1);											//エラー
	source = sid;																						//送信元ノード
	if(source != -1){
		seq = ptr->node[source]->getSeq();
		ptr->node[source]->increSeq();
	}
	here = hid;																							//現在のノード
	dest = did;																							//宛先ノード
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

//パケット処理
//引数（リストオブジェクト，ノードオブジェクト）
//戻り値（真偽値（ダミー））
bool PACKET::process(void){	
	//if(getSimu()->getNow().dtime() >= 89.635987)
	//	cout << getSimu()->getNow().dtime() << "\t" << getNid() << " パケットプロセス " <<  type 
	//	<< "\t" << path.size() << "\t" << size << "\t" << seq << "\t" << this << "\t" << getSimu()->node[getNid()]->getBuffer()->getLength() << endl;
	//if(getSimu()->getNow().dtime() >= 89.635987){
	//	cout << "dtime--" << getSimu()->getNow().dtime() << "\ttype--" << type << "\tsize--" << size << endl; 
	//}
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	FRAME* fPtr = NULL;																				//フレームオブジェクトポインタ
	NODE* nPtr = sPtr->node[here];																	//ノードオブジェクト
	MAC* mPtr = nPtr->getMAC();																	//MACオブジェクト
	BUFFER* bPtr = nPtr->getBuffer();															//バッファオブジェクト
	//if(type == MrReq)
	//	cout << "test --- " << now.dtime() << "\t" << getSource() << "\t" << getDest() << endl;
	if(!nPtr->getIsActive())																		//ノードが活動停止状態なら
		cout << "off node sends packet!!!" << endl, exit(1);									//エラー
	if(mPtr->getFrame() != NULL)																	//もしもMAC処理対象フレームが存在するなら
		return true;																						//何もせずに終了
	switch(type){																						//パケットタイプによる場合分け
	case Routing: case RreqDsr: case RreqAodv: case LabRa: case MapReqB:	case MigReq:
	case DummyBroadcast:																				//ブロードキャストパケットの場合
		fPtr = new FRAME(sPtr, now, FRAME::Broad, here, -1, seq, this);					//ブロードキャストフレームを作成
		break;

	default:																								//その他の場合
		if(type == Tcp && source == here){															//TCPパケットかつ自身が送信元の場合
			SEGMENT* segPtr = getSeg();																	//対応セグメントオブジェクト
			TCP* tPtr = segPtr->getTcp();																	//対応TCPオブジェクト
			tPtr->resetBuffer();																				//バッファリングフラグをリセット
			if(tPtr->window[0] == NULL || segPtr->getSeq() < tPtr->window[0]->getSeq()){	//既にACK受信済みで送る必要のないパケットの場合
				bPtr->decreLength(size);																		//行列長をパケット長だけ減らす
				bPtr->queue.erase(bPtr->queue.begin());													//バッファの先頭パケットを削除
				tPtr->makePacket();																				//次のパケット作成
				return false;																						//オブジェクトを破棄して終了
			}
			segPtr->setSendStart(now);																		//セグメントの送信時間を現在時刻にする
			tPtr->setLastSendSeq(segPtr->getSeq());
			if(segPtr->getTimeout() == NULL){															//セグメントのタイムアウトが作成されていない場合
				TIMEOUT* ptr = new TIMEOUT(sPtr, now + tPtr->getRtt() + int(tPtr->getD() * 4), 
																	TIMEOUT::Segment, here);					//タイムアウトオブジェクトの作成	
				segPtr->setTimeout(ptr);																		//セグメントにタイムアウトを登録
				ptr->setSegment(segPtr);																		//タイムアウトに対応セグメントを登録
				sPtr->list.insert(ptr);																			//タイムアウトをイベントに追加
			}
			else{																									//セグメントのタイムアウトが既に作成されている場合
				segPtr->getTimeout()->setTime(now + tPtr->getRtt() + int(tPtr->getD() * 4));	//タイムアウトの決定
				sPtr->list.replace(segPtr->getTimeout());													//イベントをリスト内で再配置
			}
		}
		if(nPtr->getRoute() == NODE::DSR){															//ルーティングがDSRの場合
			if(path.size() == 0){
				cout << this << "\t" << type << "\t" << here << "\t" << now.dtime() << endl;
				cout << "DSR route error" << endl, exit(1); 
			}
			for(short i = 0; i < (short)path.size(); i++){											//経路情報の検索
				if(path[i] == here){																				//経路情報に自身が見つかったら
					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);//経路情報よりフレームを作成
					break;
				}
			}
		}
		else if(nPtr->getRoute() == NODE::GEDIR ){													//ルーティングがGEDIRの場合
			short next = -1;																					//転送ノードID（初期値-1）
			double minDist = AREA * 2;																		//あて先までの最小距離（初期値はエリアの2倍）
			short i = 0;																						//カウント用変数
			vector<short> candidate;																		//転送ノード候補登録用ベクタ
			while(nPtr->neighborList[i].getDist() < RANGE)											//自身の隣接ノードをチェック
			{
				short checkId = nPtr->neighborList[i].getId();											//チェック対象ノードID
				if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){	//対象が宛先なら
					next = checkId;																					//必ず転送先にする
					break;																								//チェック終了
				}
				bool flag = true;																					//対象が転送経路に含まれてるかを示すフラグ
				for(char j = 0; j < (char)path.size(); j++){												//パケットの転送経路をチェック
					if(path[j] == checkId){																			//対象が経路に含まれていたら
						flag = false;																						//フラグを下す
						break;																								//チェック終了
					}
				}
				if(flag == true){																					//対象が経路に含まれていないなら
					candidate.push_back(checkId);																	//候補ベクタへ登録
					if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){//あて先までの距離が最短なら
						minDist = dist(dPos, nPtr->nodePos[checkId]);											//最短距離を更新
						next = checkId;																					//転送先を対象ノードにする
					}
				}
				i++;																									//次のチェック対象へ
			}
			if(next == dest || candidate.size() > 0){													//転送先が確定しているか候補ベクタへの登録があるなら
				if(next == -1)																						//転送先が確定していない場合
					next = candidate[genrand() % (int)candidate.size()];									//候補ベクタからランダムに転送先を決定
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//フレームを作成
//				cout << now.dtime() << "  make frame in gedir "<< here << "\t" << next  << endl;
			}
			else{																									//転送先が確定せず候補も存在しない場合
				bPtr->decreLength(size);																		//行列長をパケット長だけ減らす
				bPtr->queue.erase(bPtr->queue.begin());													//バッファの先頭パケットを削除
//				cout << "no next " << endl;
				return false;																						//オブジェクトを破棄して終了
			}
		}
		//************************************************//
		// MAマルチパスルーティングの時
		// 経路情報がない場合はGEDIRと同じ処理をして
		// それ以外だったらDSRと同じ処理する
		//************************************************//
		else if( nPtr->getRoute() == NODE::MAMR ){													//MAMRの場合
			short next = -1;																		//転送ノードID（初期値-1）
			double minDist = AREA * 2;																//あて先までの最小距離（初期値はエリアの2倍）
			short i = 0;																			//カウント用変数
			vector<short> candidate;																//転送ノード候補登録用ベクタ
			//if(type  == 14 )
			//	cout << "test -1 --- " << now.dtime() << endl;
			bool flag = false;
			if(path.size() >= 2 && type != MrReq){																	//pathがある場合
				for(i = 0; i < (short)path.size() - 1; i++){										//経路情報の検索
					if(path[i] == here){																//経路情報に自身が見つかったら
						if(nPtr->routing[path[i + 1]]->getHop() == 1 /*|| destPos.getT().dtime()  + 0.1 <  now.dtime()*/)	//次のノードが隣接端末なら
							flag = true;
						break;
					}
				}
			}
			if(flag == true){
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);	//経路情報よりフレームを作成
			}
			else{
				i = 0;
				while(nPtr->neighborList[i].getDist() < RANGE){										//自身の隣接ノードをチェック
					static int cnt;
 					//if(type == MrReq)
					//	cout << "test 6 " << now.dtime() << endl;
					short checkId = nPtr->neighborList[i].getId();									//チェック対象ノードID
					if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //対象ノードが宛先なら
						next = checkId;																//次に転送するノードを対象ノードとする
						break;
					}
					bool flag = true;																//対象が転送経路に含まれているかを示すフラグ
					for(char j = 0; j < (char)mamrPath.size(); j++){									//転送経路だけ繰り返す
						if(mamrPath[j] == checkId){														//転送経路に対象が含まれている場合
							flag = false;															//フラグを下ろす
							break;
						}
					}
					if(flag == true){																//対象が経路に含まれていない場合
						candidate.push_back(checkId);												//候補ベクタへ登録
						if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //宛先までの経路が最短なら
							minDist = dist(dPos, nPtr->nodePos[checkId]);							//最短経路を更新
							next = checkId;
						}
					}
					i++;
				}
				//if(type == MrReq)
				//	cout << "test 7 " << now.dtime() << endl; 
			
				if(next != -1 || candidate.size() > 0){											//転送先が確定しているか　候補ベクタへの登録があるならば
					if(next == -1)
						next = candidate[genrand() % (int)candidate.size()];						//候補からランダムに転送先を選択
					//if(type == MrReq)
					//	cout << "test 8 " << now.dtime() << endl;
					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//フレームを作成
				}
				else{																				//転送先も確定せず　候補も存在しない場合
					bPtr->decreLength(size);														//行列長をパケット長だけ減らす
					bPtr->queue.erase(bPtr->queue.begin());											//バッファの先頭パケットを削除
					if(type == Tcp || type == Udp)
						nPtr->sendError(this);													//ルートエラー送信処理
					return false;
				}
			}




			//	//cout << "path あり" << path.size() << endl;
			//	for(short i = 0; i < (short)path.size(); i++){										//経路情報の検索
			//		if(path[i] == here){															//経路情報に自身が見つかったら
			//			if(nPtr->routing[path[i + 1]]->getHop() == 1){
			//				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, path[i + 1], nPtr->getSeq(), this);	//経路情報よりフレームを作成
			//				break;
			//			}
			//			else{
			//				while(nPtr->neighborList[i].getDist() < RANGE){										//自身の隣接ノードをチェック
			//					//if(type == MrReq)
			//					//	cout << "test 6 " << now.dtime() << endl;
			//					short checkId = nPtr->neighborList[i].getId();									//チェック対象ノードID
			//					if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //対象ノードが宛先なら
			//						next = checkId;																//次に転送するノードを対象ノードとする
			//						break;
			//					}
			//					bool flag = true;																//対象が転送経路に含まれているかを示すフラグ
			//					for(char j = 0; j < (char)path.size(); j++){									//転送経路だけ繰り返す
			//						if(path[j] == checkId){														//転送経路に対象が含まれている場合
			//							flag = false;															//フラグを下ろす
			//							break;
			//						}
			//					}
			//					if(flag == true){																//対象が経路に含まれていない場合
			//						candidate.push_back(checkId);												//候補ベクタへ登録
			//						if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //宛先までの経路が最短なら
			//							minDist = dist(dPos, nPtr->nodePos[checkId]);							//最短経路を更新
			//							next = checkId;
			//						}
			//					}
			//					i++;
			//				}
			//				//if(type == MrReq)
			//				//	cout << "test 7 " << now.dtime() << endl; 
			//
			//				if(next == dest || candidate.size() > 0){											//転送先が確定しているか　候補ベクタへの登録があるならば
			//					if(next == -1)
			//						next = candidate[genrand() % (int)candidate.size()];						//候補からランダムに転送先を選択
			//					//if(type == MrReq)
			//					//	cout << "test 8 " << now.dtime() << endl;
			//					fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//フレームを作成
			//		
			//				}
			//				else{																				//転送先も確定せず　候補も存在しない場合
			//					bPtr->decreLength(size);														//行列長をパケット長だけ減らす
			//					bPtr->queue.erase(bPtr->queue.begin());											//バッファの先頭パケットを削除
			//					return false;
			//				}
			//			}
			//		}
			//	}
			//}else{																					//pathがない場合　以下GEDIRと同じ方式を使う
			//	while(nPtr->neighborList[i].getDist() < RANGE){										//自身の隣接ノードをチェック
			//		//if(type == MrReq)
			//		//	cout << "test 6 " << now.dtime() << endl;
			//		short checkId = nPtr->neighborList[i].getId();									//チェック対象ノードID
			//		if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //対象ノードが宛先なら
			//			next = checkId;																//次に転送するノードを対象ノードとする
			//			break;
			//		}
			//		bool flag = true;																//対象が転送経路に含まれているかを示すフラグ
			//		for(char j = 0; j < (char)path.size(); j++){									//転送経路だけ繰り返す
			//			if(path[j] == checkId){														//転送経路に対象が含まれている場合
			//				flag = false;															//フラグを下ろす
			//				break;
			//			}
			//		}
			//		if(flag == true){																//対象が経路に含まれていない場合
			//			candidate.push_back(checkId);												//候補ベクタへ登録
			//			if(dist(dPos, nPtr->nodePos[checkId]) < minDist && nPtr->nodePos[checkId].getX() >= 0){ //宛先までの経路が最短なら
			//				minDist = dist(dPos, nPtr->nodePos[checkId]);							//最短経路を更新
			//				next = checkId;
			//			}
			//		}
			//		i++;
			//	}
			//	//if(type == MrReq)
			//	//	cout << "test 7 " << now.dtime() << endl; 
			//
			//	if(next == dest || candidate.size() > 0){											//転送先が確定しているか　候補ベクタへの登録があるならば
			//		if(next == -1)
			//			next = candidate[genrand() % (int)candidate.size()];						//候補からランダムに転送先を選択
			//		//if(type == MrReq)
			//		//	cout << "test 8 " << now.dtime() << endl;
			//		fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//フレームを作成
			//		
			//	}
			//	else{																				//転送先も確定せず　候補も存在しない場合
			//		bPtr->decreLength(size);														//行列長をパケット長だけ減らす
			//		bPtr->queue.erase(bPtr->queue.begin());											//バッファの先頭パケットを削除
			//		return false;
			//	}
			//}
			//else{																					//MA以外の場合
			//	if(path.size() <= 1){																//pathがない場合
			//		if(dPos.getX() >= 0){
			//			while(nPtr->neighborList[i].getDist() < RANGE){										//自身の隣接ノードをチェック
			//				short checkId = nPtr->neighborList[i].getId();									//チェック対象ノードID
			//				if(checkId == dest ||(dest >= (short)sPtr->node.size() && sPtr->ma[dest - sPtr->node.size()]->getNid() == checkId)){ //対象ノードが宛先なら
			//					next = checkId;																//次に転送するノードを対象ノードとする
			//					break;
			//				}
			//				i++;
			//			}
			//			if(next == dest)
			//				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, next, nPtr->getSeq(), this);		//フレームを作成
			//			else
			//				cout << "next error at PACKET process" << endl,exit(1);
			//		}
			//		else
			//			cout << "dPos error at PACKET process" << endl, exit(1);
			//	}
			//	else{																				//pathがある場合
			//		cout << "test --- " << now.dtime() << endl;
			//	}
			//}
			//if(path.size() == 0){																	//経路情報が埋め込まれていない場合
			//	//// ログ表示
			//	//cout << "MAMR経路情報なし" << endl;
			//	//cout << this << "\t" << type << "\t" << here << "\t" << now.dtime() << endl;
			//	short next = -1;																	//転送ノードID（初期値-1）
			//	double minDist = AREA * 2;															//あて先までの最小距離（初期値はエリアの2倍）
			//	short i = 0;																		//カウント用変数
			//	vector<short> candidate;															//転送ノード候補登録用ベクタ
			//else{																						//経路情報を保持している場合
			//	}
			//}
		}
		//*************************************************************//
		// マルチパス処理以上
		//*************************************************************//

		else{																									//その他のルーティングの場合
			//if(source == 50 && dest == 76)
			//cout << here << "->" << dest << "\t" << nPtr->routing[dest]->getNext() << "\t" << (int)nPtr->routing[dest]->getHop() << endl;
			if(nPtr->routing[dest]->getNext() != -1)											//転送先が存在するなら
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, 
								nPtr->routing[dest]->getNext(), nPtr->getSeq(), this);		//ルーティングテーブルよりフレームを作成
			else																							//転送先が存在しないなら
				fPtr = new FRAME(sPtr, now, FRAME::Uni, here, dest, nPtr->getSeq(), this);//あて先よりフレームを作成
		}
		break;
	}
	nPtr->increSeq();
	bPtr->decreLength(size);																		//行列長をパケット長だけ減らす
	bPtr->queue.erase(bPtr->queue.begin());													//バッファの先頭パケットを削除
	//if(here == 248)
	//	nPtr->queueShow();
	if(fPtr != NULL){																					//フレームが作成された場合
		sPtr->increPacket(type, size);
		sPtr->list.insert(fPtr);																		//フレーム処理をイベントリストへ追加
		return true;
//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//ノードレベルのパケット送信ログが不必要ならコメントアウトする	
	//if(type == Udp || type == Tcp || type == Ack){
	//	cout << now.dtime() << "\ts\t" << here << "\t";
	//	switch(type){																					//パケットタイプおよびシーケンス番号
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
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
	}
	else{																									//フレーム作成されなかったなら
//		cout << "packet process error " << endl, exit(1);
		mPtr->setTime(now);																				//MACのイベント時刻を現在に設定
		nPtr->checkPacket();																				//バッファ内のパケットチェック
	}
	return true;
}


//バッファへのパケット挿入
//引数（リストオブジェクト，ノードオブジェクトポインタ）
//戻り値（なし）
bool PACKET::queue(SIMU* ptr, bool front){
	SIMUTIME now = ptr->getNow();																	//現在時刻
	//if(type != Tcp && type != Ack)
	//cout << now.dtime() << "\t" << here << " パケットキュー " << endl;
	NODE* nPtr = ptr->node[here];																	//ノードオブジェクト
	setNid(here);																						//パケットイベント所持ノードを自身に変更
	bool flag = true;																					//バッファ挿入が成功したかを示すフラグ（初期値は成功）
	if(ttl == 0){																						//TTLが0（ホップ数の上限オーバ）の場合
		flag = false;																					//挿入成功フラグを下ろす
//		cout << now.dtime() << "\tHD\t" << here << "\t";					//ログ表示
//		switch(getType()){																			//パケットタイプおよびシーケンス番号
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
	else{																								//TTLが0でない場合
		if(nPtr->getRoute() == NODE::GEDIR || nPtr->getRoute() == NODE::MAMR){														//GEDIRの場合
			mamrPath.push_back(here);																		//経路情報として自身を挿入する
			size += 4;
		}
		ttl--;																							//TTLのデクリメント
		hop++;																							//ホップ数のインクリメント
		BUFFER* bPtr = nPtr->getBuffer();														//バッファオブジェクトポインタ
		if(front){																						//先頭挿入フラグが立っていたら
			bPtr->queue.insert(bPtr->queue.begin(), this);										//バッファの先頭に追加
			bPtr->increLength(size);
			if(bPtr->getLength() > bPtr->getSize()){
//				cout << getNid() << "  bPtrGetSize " << bPtr->getSize() << endl;
				bPtr->decreLength(bPtr->queue[bPtr->queue.size() - 1]->getSize());
				delete bPtr->queue[bPtr->queue.size() - 1];
				bPtr->queue.pop_back();
			}
		}
		else{																								//先頭挿入フラグが立っていないなら
			if(bPtr->getLength() + size < bPtr->getSize()){										//パケットを挿入しても行列長がサイズより小さいなら
				bPtr->queue.push_back(this);																//行列の最後にパケットを追加
				bPtr->increLength(size);																	//行列長をパケットサイズだけ増やす
			}
			else{																									//パケット挿入の余裕がない場合
				nPtr->increOverflow();																			//バッファオーバフローカウンタをインクリメント
				flag = false;																						//挿入成功フラグをおろす
				//static int cnt;
				//cout << now.dtime() << "  overflow at  " << getNid() << "\t" << seq << "\t" << type << endl;
//				nPtr->queueShow();
//				cout << now.dtime() << "\tOD\t" << getHere() << "\t";
//			switch(type){																							//パケットタイプおよびシーケンス番号
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
	if(front == false && mPtr->checkIdling() && (mPtr->getState() == MAC::Idle || mPtr->getState() == MAC::NavFin)){								//バッファ後方への挿入でかつノードのMAC状態がIdleなら
		mPtr->setTime(now);																		//MACのイベント時刻を現在に設定
		nPtr->checkPacket();																		//バッファ内のパケットチェック
	}
	return flag;																					//挿入成功フラグを返す
}


//ログ表示関数
//引数（パケットオブジェクト）
//戻り値：（なし）
void PACKET::showLog(short nid, char* id, SIMUTIME now){
	cout << now.dtime() << "\t";																//現在時刻
	cout << id << "\t";																			//状態（受信，送信など）
	cout << nid << "\t";																			//ノード番号
	switch(type){																					//パケットタイプおよびシーケンス番号
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
	if(dest == nid && type == PACKET::Tcp)													//受信バイト数（宛先ノードのみ表示）
		cout << typePtr.segPtr->getTcp()->getObject()->getByte() << endl;
	else if(dest == nid && type == PACKET::Udp)
		cout << typePtr.udpPtr->getObject()->getByte() << endl;
	else
		cout << "--------" << endl;
}

//経路表示
//引数（なし）
//戻り値（なし）
void PACKET::showPath(void){
	for(char i = 0; i < (char)path.size() - 1; i++)
		cout << path[i] << "->";
	if(path.size() > 1)
		cout << path[path.size() - 1] << endl;
}

//フレームオブジェクトのコンストラクタ
FRAME::FRAME(SIMU* ptr, SIMUTIME tval, castType cid, short sid, short did, int seqid, PACKET* pPtr):EVENT(ptr, tval, EVENT::Frame, sid){ 
	//cout << "create frame " << this << endl;
	cast = cid;																						//キャストタイプ
	source = sid;																					//転送元ノード
	dest = did;																						//転送先ノード
	seq = seqid;																					//シーケンス番号
	packetPtr = pPtr;																				//対応パケットオブジェクト
	size = 16 + (pPtr->getSize() + 36) * 8 + 6;											//フレームサイズ
	//cout << "frame " << cnt++ << endl;
	cntF++;
}

FRAME::~FRAME(){
	//cout << "delete frame " << this << endl;
	//cout << "frame " << cnt-- << endl;
	cntF--;
}

//フレーム処理
//引数（リストオブジェクト，ノードオブジェクト）
//戻り値：（真偽値（ダミー））
bool FRAME::process(void){
//	cout << getNid() << " フレームプロセス" << size << "\t" << dest << "\t" << packetPtr->getSeq() << endl;
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();															//現在時刻
	NODE* nPtr = sPtr->node[getNid()];														//フレーム発生ノードオブジェクト
	MAC* mPtr = nPtr->getMAC();																//フレーム発生ノードのMACオブジェクトポインタ
	mPtr->setFrame(this);																		//MACオブジェクトにこのフレームオブジェクトを登録
	mPtr->calcDataTime(this);																	//データ送信時間の設定
	if(nPtr->getChannel()->signalList.size() == 0 && mPtr->getState() != MAC::Nav){	//受信信号がなくNAVでもないなら
		mPtr->setState(MAC::Difs);																	//状態をDifs待機に設定
		mPtr->setTime(now + DIFSTIME);															//DIFS時間だけ待機
		sPtr->list.insert(mPtr);																	//イベントリストへ登録
		mPtr->setListInsert(true);																	//リスト登録フラグを立てる
	}
	if(packetPtr->getType() == PACKET::Tcp && packetPtr->getSource() == packetPtr->getHere())
		packetPtr->getSeg()->getTcp()->makePacket();
	return true;																						//関数を抜けてもオブジェクトは消去しない
}

//ビーコン処理
//引数（リストオブジェクト，ノードオブジェクト）
//戻り値（オブジェクト破棄の真偽値）
bool BEACON::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	NODE* nPtr = sPtr->node[getNid()];
	PACKET* packetPtr																					//Routing用パケットの作成
		= new PACKET(sPtr, now, PACKET::Routing, UDPSIZE, getNid(), getNid(), -1, 1);
	packetPtr->setSpos(sPtr->node[getNid()]->getDerivePos(0));
	if(!packetPtr->queue(sPtr, false))															//バッファへ挿入
		delete packetPtr;																					//挿入に失敗したら消去
	addTime(genrand() % (BEACONINT / 100) + BEACONINT - BEACONINT / 200);//次のビーコン発生時刻設定
	sPtr->list.insert(this);																				//ビーコン発生処理をイベントリストへ追加
	return true;
}
