#include "class.h"
#include "mobileAgent.h"

//ＴＣＰオブジェクトのコンストラクタ
TCP::TCP(SIMU* ptr, short nid, SIMUTIME tval, int sval):AGENT(ptr, nid, tval, EVENT::Tcp){
	size = sval;																						//ＴＣＰフローのサイズ
	byte = 0;																							//送信完了バイト数（初期値0）
	startTime = tval;																					//送信開始時刻
	rtt = SIMUTIME(0, 100000);																		//ラウンドトリップタイム
	D = 0;																								//タイムアウト設定用偏差
	windowSize = 1;																					//ウィンドウサイズ
	windowThreshold = 10000.0;																		//ウィンドウ制御用閾値
	makeSegNum = 0;																					//次に作成するセグメント番号の初期化（値は0）
	lastSendSeq = -1;																					//直前に送信したセグメントのシーケンス番号（未送信なので値は-1）
	nextSendSeq = 0;																					//次の送信セグメントのシーケンス番号の初期化（値は0）	
	lastReqSeq = 0;																					//直前受信ACKの要求シーケンス番号（値は0）
	buffering = false;																				//バッファリング状態フラグ（初期値は非バッファリング）
	maPtr = NULL;																						//対応モバイルエージェントオブジェクト（初期値なし）
	abort = false;																						//中断フラグ
	finish = false;
//	init = true;
//	finishTime = -1;																					//終了時刻（初期値-1）
//	finishNum = 0;																						//ウィンドウベクタに存在する送信終了セグメント数
//	windowNum = 0;																						//ウィンドウに存在するセグメント数
//	segPacketNum = 0;																					//セグメントパケット数
//	sendSeq = 0;																						//次に送るべきセグメントパケットのシーケンス
//	seq = 0;																								//シーケンス番号
//	ack = 0;																								//ACKによる送信要求シーケンス
//	lastack = -1;																						//直前の送信要求シーケンス
//	ackRepeatNum = 1;																					//同一ACKの連続受信数
//	finish = false;																					//終了フラグ
//	timeOut = false;																					//タイムアウトフラグ
//	measurement = false;	
}

TCP::~TCP(void){
	while(segCash.size() > 0){																		//セグメントキャッシュが存在する限り
		delete segCash[0];																				//キャッシュ先頭のセグメントを消去
		segCash.erase(segCash.begin());																//キャッシュの先頭要素を削除
	}
	if(maPtr)																							//モバイルエージェント対応TCPなら
		delete maPtr;																						//モバイルエージェントも削除
}

//ＴＣＰエージェントの処理
//引数（なし）
//戻り値：（真偽値（ダミー））
bool TCP::process(void){
//	cout << "TCP process in" << endl;
	makeSegment();																						//セグメントの作成
	return true;																						//オブジェクトは破棄しない
}

//SINKとの結合処理
//引数（SINKオブジェクト）
//戻り値（なし）
void TCP::connectSink(TCPSINK* sPtr){
	objectPtr = sPtr;
	sPtr->setObject(this);
}

//セグメント作成
//引数（なし）
//戻り値（なし）
void TCP::makeSegment(void){
//	cout << "test makeSegment --- " << getSimu()->getNow().dtime() << endl;
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	short nid = getNid();																			//ノードID
	short wSize = window.size();
	for(char i = 0; i < (char)(windowSize - wSize); i++){									//ウィンドウサイズの空き分だけ新たにセグメント作成
		if(byte < size){																					//作成済みセグメントサイズがTCPサイズより小さければ
			short segSize = min(size - byte, TCPSIZE);												//送信セグメントサイズ
			SEGMENT* segPtr = new SEGMENT(sPtr, now, this, segSize, makeSegNum++, nid);	//セグメントオブジェクトの作成	
			window.push_back(segPtr);																		//ウィンドウにセグメントを登録
			segCash.push_back(segPtr);																		//キャッシュにもセグメントを登録
			if(segCash.size() > 1000){
				delete segCash[0];
				segCash.erase(segCash.begin());
			}
			byte += segSize;																					//作成セグメントサイズの更新
//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//ＴＣＰセグメントレベルの送信ログが不必要ならコメントアウトする	
			//cout << now.dtime() << "\tS\t" << nid << "\tTCP\t" << this << "\t" << segPtr->getSeq()
			//<< "\t" << windowSize << "\t" << byte << endl;
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
		}
		else																									//作成済みセグメントサイズがTCPサイズ以上なら
			window.push_back(NULL);
	}
//	cout << "last " << lastSendSeq << "\tnext " << nextSendSeq << endl;  
	if(lastSendSeq < nextSendSeq)																	//ウィンドウの先頭セグメントが次の送信セグメントなら
		makePacket();																						//送信パケット作成処理
}

//送信パケット作成
//引数（なし）
//戻り値（なし）
void TCP::makePacket(void){
	if(buffering)																						//既にバッファリングされているパケットがあるなら
		return;																								//何もしない
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	short nid = getNid();																			//ノードID
	for(short i = 0; i < (short)windowSize; i++){											//ウィンドウサイズだけウィンドウの中身をチェック
		if(!window[i])
			break;
		if(window[i]->getSeq() == nextSendSeq){													//次の送信セグメントが存在するなら
			SEGMENT* segPtr = window[i];																	//送信対象セグメント
			PACKET* pPtr																						//パケットオブジェクトの作成
				= new PACKET(sPtr, now, PACKET::Tcp, segPtr->getSize() + 40, nid, nid, objectPtr->getNid(), -1);
//			cout << "make packet " << segPtr->getSeq() << endl;
			pPtr->setSeg(segPtr);																			//パケットの対応セグメント登録
			pPtr->setSeq(segPtr->getSeq());																//パケットのシーケンス番号設定
			if(!pPtr->queue(sPtr, false)){																//バッファへのパケット挿入
				delete pPtr;																						//挿入に失敗したら消去
				setTime(now + 100000);																			//次のパケット発生時間を設定
				sPtr->list.insert(this);																		//イベントリストへ追加
			}
			else{																									//挿入に成功したら
				buffering = true;																					//バッファリングフラグを立てる
				nextSendSeq++;																						//次の送信セグメントシーケンス番号を設定
			}
			break;
		}
	}
}

//ＡＣＫ受信処理
//引数（パケットオブジェクト，リストオブジェクト，ノードオブジェクト）
//戻り値：（なし）
void TCP::getTcpAck(PACKET* pPtr){
	if(abort)
		return;
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	SEGMENT* segPtr = pPtr->getSeg();															//ノードID
//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//送信元ノードのACK受信ログが不必要ならコメントアウトする
//	pPtr->showLog(getNid(), "R", now);
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
	nowRtt = now - segPtr->getSendStart();														//今回のRTTを測定
	int reqSeq = pPtr->getReqSeq();																//ACKが持つ期待シーケンス番号
	int nowRttVal = nowRtt.getSec() * 1000000 + nowRtt.getLessSec();					//直近RTTの整数値
	int rttVal = rtt.getSec() * 1000000 + rtt.getLessSec();								//推測RTTの整数値
	rtt = int(rttVal * RTT_ALPHA + nowRttVal * (1 - RTT_ALPHA));						//更新推測TTの計算 
	int diff = timeCompare(rtt, nowRtt)															//直近RTTと推測RTTの差の整数値の絶対値
		? (rtt - nowRtt).getSec() * 1000000 + (rtt - nowRtt).getLessSec()
		: (nowRtt -rtt).getSec() * 1000000 + (nowRtt - rtt).getLessSec();
	D = D * RTT_ALPHA + diff * (1 - RTT_ALPHA);												//タイムアウト用偏差の計算
//	cout << "receiveSeq " << reqSeq << "\tlastAckSeq " << lastReqSeq << endl;
	if(windowSize <= windowThreshold)															//ウィンドウサイズの更新
		windowSize += 1;																					//通常モード
	else
		windowSize += 1 / windowSize;																	//輻輳回避モード
	if(reqSeq == lastReqSeq && reqSeq < pPtr->getSeq()){									//新しいACK受信で期待シーケンスが前回と同じならば
		if(window[0]->getAckRepeat() == 2 && window[0]->getNAretrans() == false){		//重複ACK受信再送はしてなく既に2回重複ACK受信していた場合
////＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
////ＡＣＫによる再送の送信ログが不必要ならコメントアウトする	
			//cout << now.dtime() << "\tNA\t" << getNid() << "\tTCP\t" << this << "\t" << reqSeq
			//				<< "\t--------\t--------" << endl;
////＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
			window[0]->setNAretrans(true);																//重複ACK受信による再送済みフラグを立てる
			retransmission();																					//再送処理
			return;																								//関数処理終了
		}
		else																									//重複回数が1の場合
			window[0]->increAckRepeat();																	//重複回数のインクリメント
	}
	else if(reqSeq > lastReqSeq){																	//今回受信の期待シーケンスが前回より大きければ
		while(window.size() > 0 && window[0] != NULL){											//ウィンドウサイズが0でない限り
			if(window[0]->getSeq() < reqSeq){															//ウィンドウ先頭のシーケンスが期待値より小さければ
				sPtr->list.remove(window[0]->getTimeout());
				window[0]->setTimeout(NULL);
				window.erase(window.begin());																	//ウィンドウから削除				
			}	
			else{																									//ウィンドウの先頭が期待シーケンスとなったら
				window[0]->increAckRepeat();																	//重複回数のインクリメント
				break;																								//終了
			}
		}
		lastReqSeq = reqSeq;
		nextSendSeq = max(reqSeq, nextSendSeq);
	}
	makeSegment();																						//セグメントの作成					
	if(reqSeq * TCPSIZE >= size && finish == false){																	//もう送信するセグメントがない場合
		finish = true;
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(20,0), TIMEOUT::Tcp, getNid());//TCP消去用タイムアウトオブジェクトを作成
		toPtr->setTcp(this);																				//オブジェクトに対応TCPを登録
		sPtr->list.insert(toPtr);																		//消去タイムアウトオブジェクトをイベントに追加
		if(!maPtr){
			sPtr->increTcpData(size);																		//総送信データサイズのインクリメント
			sPtr->increTcpTime(now - startTime);														//総送信時間のインクリメント
			//cout << maPtr << "\tTCP throughput " << now.dtime() << "\t" << (int)pPtr->getHop() << "\t" << size * 8  / (now - startTime).dtime()  / 1000 / 1000 << endl;
		}
		if(maPtr){																							//オブジェクトが存在する場合
			NODE* nPtr = sPtr->node[getNid()];															//ノードオブジェクト
			if(nPtr->ma.size() > 0){																		//MAを保持しているなら
				vector<MA*>::iterator i = nPtr->ma.begin();	
				while(*i != maPtr && i != nPtr->ma.end())												//今回移動したMAかをチェック
					i++;
				if(i != nPtr->ma.end()){																		//移動したMAが見つかったら
					MA* mPtr = *i;																					//削除対象のMAオブジェクト
					sPtr->list.remove(mPtr->getTimeout());													//移動用タイムアウトオブジェクトをリストから外す
					sPtr->list.remove(mPtr);																	//イベントリストからMAを外す
					nPtr->ma.erase(i);																			//そのMAをリストからはずす
					i = sPtr->ma.begin();																		//シミュレータオブジェクトからMAを探す
					while(*i != maPtr && i != sPtr->ma.end())												//今回移動したMAかをチェック
						i++;							
					sPtr->ma.erase(i);																			//MAをリストから外す
				}
			}
		}
	}
}

//再送処理
//引数（なし）
//戻り値：（なし）
void TCP::retransmission(void){
	//cout << "test1" << endl;
	windowThreshold = windowSize / 2.0;															//閾値は現在のウィンドウサイズの半分
	windowSize = 1;																					//ウィドウサイズは1に初期化
	for(short i = 0; i < (short)window.size(); i++){											//ウィンドウベクタをチェック
		if(!window[i])
			break;
		getSimu()->list.remove(window[i]->getTimeout());										//タイムアウトオブジェクトを削除する
		window[i]->setTimeout(NULL);
	}
	nextSendSeq = window[0]->getSeq();															//次の送信セグメントをウィンドウの先頭に戻す
	lastSendSeq = nextSendSeq - 1;																//直前送信セグメントシーケンスを初期化
	window[0]->resetAckRepeat();																	//重複ACK受信数の初期化
		
	//	cout << "retransmission " << lastSendSeq << "\t" << nextSendSeq << endl;
	makePacket();
}

//ＡＣＫ送信処理
//引数（パケットオブジェクト，リストオブジェクト，ノードオブジェクト）
//戻り値（なし）
void TCPSINK::sendAck(PACKET* pPtr){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	SEGMENT* segPtr = pPtr->getSeg();															//セグメントオブジェクトポインタ
	int id = getNid();																				//ノードID
	bool newSegFlag = true;																			//新しいセグメント受信かを示すフラグ（真で初期化）
	int receiveSeq = segPtr->getSeq();															//受信セグメントのシーケンス番号
	int requestSeq = lastSeq + 1;																	//受信期待シーケンス番号
//	cout << "last " << lastSeq << "\trequest " << requestSeq << "\treceive " << receiveSeq << endl;
	if(receiveSeq > lastSeq){																		//重複受信セグメントでない場合
		if(receiveSeq > requestSeq){																	//受信セグメントのシーケンスが期待値より大きい場合
			bool flag = true;																					//パケットのキャッシュ存在を示すフラグ（非存在で初期化）
			for(short i = 0; i < (short)cashSeq.size(); i++){										//受信不連続セグメントのキャッシュシーケンスをチェック
				if(cashSeq[i] == receiveSeq){																	//キャッシュ内に受信セグメントがある場合
					flag = false;																						//キャッシュ存在フラグを下ろす
					newSegFlag = false;																				//新規受信セグメントフラグは偽
					break;
				}
			}
			if(flag)																								//キャッシュに受信セグメントが無い場合
				cashSeq.push_back(receiveSeq);																//受信セグメントの追加登録
		}
		else{																									//受信セグメントが期待シーケンスを持つ場合
			requestSeq++;																						//受信期待シーケンスのインクリメント
			bool flag = true;																					//連続受信パケットチェック用フラグ
			while(cashSeq.size() != 0 && flag == true){												//連続受信パケットチェックの対象がある限り
				flag = false;																						//キャッシュに期待セグメントが存在するかを示すフラグ	
				for(vector<int>::iterator i = cashSeq.begin(); i != cashSeq.end();){				//キャッシュ内をチェック
					if(*i == requestSeq){																			//期待セグメントが存在したら
						requestSeq++;																						//期待シーケンス番号をインクリメント
						cashSeq.erase(i);																					//対象セグメントを削除
						flag = true;																						//次の対象をチェックするためのフラグ設定
						break;																								//チェックは終了
					}
					else
						i++;
				}
			}
		}
	}
	else																									//受信セグメントシーケンスが期待値以下の場合
		newSegFlag = false;																				//新規受信フラグを下す
	if(newSegFlag == true)																			//新規受信フラグが真なら
		byte += segPtr->getSize();																		//セグメントの総受信バイトをインクリメント
			
//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//宛先ノードのTCP受信ログが不必要ならコメントアウトする
//	pPtr->showLog(id, "r", now);
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
	if(byte == objectPtr->getSize()){															//TCPの最後のパケットを受信したら
		MA* maPtr = objectPtr->getMA();																//MAオブジェクト
		NODE* nPtr = sPtr->node[id];																	//ノードオブジェクト
		if(maPtr){																							//MA用のTCPであったなら
			bool flag = true;																					//すでにMAが移動しているかを表すフラグ（いなければ真）
			for(char i = 0; i < (char)nPtr->ma.size(); i++){											//自身が保持しているMAをチェック
				if(nPtr->ma[i]->getId() == maPtr->getId()){												//TCPに対応したMAとIDが同じならば
					flag = false;																						//フラグをおろす
					break;																								//チェックは終了
				}
			}
			if(flag){																							//MAがまだ移動していなければ
				MA* newMaPtr = new MA(sPtr);																	//コピー用オブジェクトの作成
				*newMaPtr = *maPtr;																				//MAオブジェクトのコピー
				newMaPtr->setNid(id);																			//MAの滞在ノードIDを自身に変更
				newMaPtr->setTime(now + MACHECKINTERVAL);													//時刻の初期化
				newMaPtr->resetMigration();																	//移動中フラグを下す
				nPtr->ma.push_back(newMaPtr);																	//ノードの保持MAリストに追加
				sPtr->ma.push_back(newMaPtr);																	//シミュレータの保持MAリストに追加
				newMaPtr->increMigNum();																		//移動回数のインクリメント
				if(sPtr->getArea() == SIMU::MESH){
					TIMEOUT* toPtr = new TIMEOUT(sPtr, now + newMaPtr->getStayTime(), TIMEOUT::MeshMa, id);	//エージェント移動タイムアウトオブジェクトの作成
					toPtr->setMa(newMaPtr);																			//対応MAの登録
					sPtr->list.insert(toPtr);																		//イベントリストへ登録
				}
				else
					sPtr->list.insert(newMaPtr);
//				cout << "node " << id << " receive MA" << maPtr->getId() << " at " << now.dtime() << " ---- " << endl; 
			}
		}
	}
	PACKET* packetPtr																					//ACKオブジェクトの作成
		= new PACKET(sPtr, now, PACKET::Ack, TCPACKSIZE, id, id, objectPtr->getNid(), -1);
	packetPtr->setSeg(segPtr);																		//パケットの対応セグメント登録
	packetPtr->setReqSeq(requestSeq);															//受信期待セグメントのシーケンス番号登録
	packetPtr->setSeq(receiveSeq);																//ACK対象セグメントのシーケンス番号設定
	if(!packetPtr->queue(sPtr, false))															//バッファへのパケット挿入
		delete packetPtr;																					//挿入に失敗したら消去
	lastSeq = requestSeq - 1;																		//直近受信セグメントのシーケンス番号設定

//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//宛先ノードのACK送信ログが不必要ならコメントアウトする
//	packetPtr->showLog(id, "S", now);
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
}
extern int cntP;

//ＵＤＰエージェントの処理
//引数（なし）
//戻り値：(エージェントが継続するなら真，終了するなら偽）
bool UDP::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	short here = getNid();
	int dSize = min(size - byte, UDPSIZE);											//データグラムサイズ
	PACKET* pPtr																				//パケットオブジェクトの作成
		= new PACKET(sPtr, now, PACKET::Udp, dSize + 28, here, here, objectPtr->getNid(), -1);
	pPtr->setUdp(this);																	//パケットの対応ＵＤＰエージェント登録
	pPtr->setSeq(seq++);																	//パケットのシーケンス番号設定とインクリメント
	if(!pPtr->queue(sPtr, false))															//バッファへのパケット挿入
		delete pPtr;																				//挿入に失敗したら消去

//＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
//ＵＤＰレベルの送信ログが不必要ならコメントアウトする	
	//cout << now.dtime() << "\tS\t" << here << "\tUDP\t" << this << "\t" << getSeq() - 1
	//<< "\t--------\t" << byte + dSize << "\t" << cntP << endl;
//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊

	if(increByte(dSize) < size){														//送信バイト数が総サイズに達していなければ
		setTime(now + int(8 * dSize / (1000 * rate) * 1000000.0));						//レートから次のパケット発生時間を設定
		sPtr->list.insert(this);																			//イベントリストへ追加
	}
	else{																								//送信バイト数が総サイズに達していたら
		TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(20,0), TIMEOUT::Udp, getNid());//UDP消去用タイムアウトオブジェクトを作成
		toPtr->setUdp(this);																			//オブジェクトに対応UDPを登録
		sPtr->list.insert(toPtr);																			//イベントに追加
	}
	return true;
}

//SINKとの結合処理
//引数（SINKオブジェクト）
//戻り値：（なし）
void UDP::connectSink(UDPSINK* sPtr){
	objectPtr = sPtr;
	sPtr->setObject(this);
}