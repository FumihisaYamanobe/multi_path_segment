#include "class.h"
#include "mobileAgent.h"

extern unsigned long genrand();																	//メルセンヌツイスタ乱数発生
static int cntTry, cntSuc;


//MACオブジェクトのコンストラクタ 
MAC::MAC(SIMU* ptr, SIMUTIME tval, short idval):EVENT(ptr, tval, EVENT::Mac, idval){
	rtsTime = 16 + (1 + (16 + RTSSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	ctsTime = 16 + (1 + (16 + CTSSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	ackTime = 16 + (1 + (16 + ACKSIZE * 8 + 6 - 1) / (PHYSPEED * 4) + 1) * 4;
	contWindow = 16;
	backoffTime = SIMUTIME(-2, 0);
	retrans = 0;
	framePtr = NULL;																					//対応フレームオブジェクトの初期化（NULL）
	listInsert = false;
	state = Idle;																						//MAC状態の初期化（Idle）
	for(short i = 0; i < 200; i++)																	//受信フレームキャッシュの初期化
		last.push_back(-1);
	orFlag = false;																					//ORフラグの初期化
	orPtr = NULL;																						//ORパケットの初期化
}

////ＭＡＣオブジェクトのデコンストラクタ
//MAC::~MAC(){
//	if(getFrame() != NULL){																			//フレームオブジェクトが存在している場合
//		if(getFrame()->getPacket() != NULL)															//対応パケットオブジェクトが存在している場合
//			delete getFrame()->getPacket();																//パケットオブジェクトを消去
//		delete getFrame();																				//フレームオブジェクトを消去
//	}
//}

void MAC::show(double val = 0){
		int sec = (int)floor(val);
		int musec = (int)((val - sec) * 1000000);
		short lessMuSec = getEventTime().getLessMuSec();
		char* type[20] = {"Idle", "Difs", "Backoff", "Rts", "Cts", 
		"Waitcts", "Data", "BData", "Waitdata", "Ack", "Waitack", "Nav", "NavFin", "WaitOrAck", "WaitOtherAck", "OrAck"};

		if(timeCompare(getEventTime(), SIMUTIME(sec, musec))){
			cout << getEventTime().dtime() << "_";
			if(lessMuSec > 99)
				cout << lessMuSec;
			else if(lessMuSec > 9) 
				cout << "0" << lessMuSec;
			else cout << "00" << lessMuSec;
			cout << " --- " << type[(int)getState()] << "\t";
			if(getSimu()->node[getNid()]->getChannel()->getSignal())
				if(getSimu()->node[getNid()]->getChannel()->getSignal()->getChannel() == getSimu()->node[getNid()]->getChannel())
					cout << "sending" << "\t";
				else
					cout << "receiving" << "\t";
			else
				if(getSimu()->node[getNid()]->getChannel()->signalList.size())
					cout << "carrier sensing" << "\t";
				else
					cout << "idling" << "\t";
			cout << listInsert << endl;
		}
}

//MACイベント処理
//引数（リストオブジェクト，ノードオブジェクト）
//戻り値：（ブール変数）
bool MAC::process(void){
	listInsert = false;																				//リスト登録フラグをおろす
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	NODE* nPtr = sPtr->node[getNid()];															//ノードオブジェクト
	CHANNEL* chPtr = nPtr->getChannel();														//チャネルオブジェクト
	switch(state){																						//MAC状態による場合分け
	case Idle:																							//Idleなら
		cout << now.dtime() << "\t" << getNid() <<  " mac idle error " << endl, exit(1);
		break;
	case Difs:																							//Difsなら
		if(chPtr->signalList.size())																	//受信信号が存在するなら
			cout << "difs error " << endl, exit(1);													//エラー
		makeBackoff(now);																					//バックオフ処理
		break;
	case Backoff:																						//Backoffなら
		if(chPtr->signalList.size())																	//受信信号が存在するなら
			cout << "backoff error " << endl, exit(1);												//エラー
		backoffTime.setTime(-2, 0);																	//バックオフ時間の初期化
		objectId = framePtr->getDest();																//送信対象相手をフレームの宛先に設定
		if(framePtr->getCast() == FRAME::Uni){														//ユニキャストフレームなら
//			cout << now.dtime() << "\t" << getNid() << "\tRTS送信" << endl;
			if(!judgeSendable()){																			//送信不可なら
				nPtr->sendError(framePtr->getPacket());													//ルートエラー送信処理
				reset();																								//送信処理の初期化
				if(!chPtr->signalList.size())																	//受信信号が存在しないなら
					checkFrame();																						//フレームチェック				
				break;
			}
			state = Rts;																						//Rts状態へ遷移
			retrans++;																							//送信機会数をインクリメント
		}
		else{																									//ブロードキャストフレームなら
			state = BData;																						//BData状態へ遷移				
//			cout << now.dtime() << "\t" << getNid() << "\tBDATA送信" << endl;		
		}
		chPtr->sendSignal(now);																			//チャネルの信号送信処理
		chPtr->setTime(now);																				//チャネルの時刻を現在に設定
		sPtr->list.insert(chPtr);																		//チャネルをイベントリストへ登録
		break;
	case Cts: case Data: case Ack: case OrAck:													//Cts，Data，Ack, OrAckなら
		//if(state == Cts)
		//	cout << now.dtime() << "\t" << getNid() << "\tCTS送信" << endl;
		//if(state == Data)
		//	cout << now.dtime() << "\t" << getNid() << "\tDATA送信" << endl;
		//if(state == Ack)
		//	cout << now.dtime() << "\t" << getNid() << "\tACK送信" << endl;
		if(state == OrAck)
			objectId = orSource;
		if(state == Data){																				//Data送信の場合
			if(!judgeSendable()){																			//送信不可なら
				nPtr->sendError(framePtr->getPacket());													//ルートエラー送信処理
				reset();																								//送信処理の初期化
				if(!chPtr->signalList.size())																	//受信信号が存在しないなら
					checkFrame();																						//フレームチェック				
				break;
			}
		}
		chPtr->sendSignal(now);																			//チャネルの信号送信処理
		chPtr->setTime(now);																				//チャネルの時刻を現在に設定
		sPtr->list.insert(chPtr);																		//チャネルをイベントリストへ登録
		break;
	case Nav: case Waitcts: case Waitdata:														//Nav, Waitcts, Waitdataなら
		//if(state == Waitcts)
		//	cout << "waitctsによる再送" << endl;
		//if(state == Waitdata)
		//	cout << "waitdataによる再送" << endl;
		//if(state == Waitack)
		//	cout << "waitackによる再送" << endl;
		state = Idle;																						//Idle状態に変更
		if(!chPtr->signalList.size())																	//受信信号が存在しないなら
			checkFrame();																						//フレームチェック
		break;
	case Waitack:																						//Waitackなら
		if(sPtr->getMac() == SIMU::OR_JOJI_ACK){														//JOJI方式なら
			state = WaitOrAck;																				//OR用ACK受信待機状態に遷移
			addTime(ORACK_RANGE + ackTime + 1);																		//イベント時刻の設定
			sPtr->list.insert(this);																		//自身をイベントリストへ登録
		}
		else{																									//通常の方式なら
			//cout << now.dtime() << "\t" << (double)++cntSuc / cntTry << " rr " << endl;			
			state = Idle;																						//Idle状態に変更
			if(!chPtr->signalList.size())																	//受信信号が存在しないなら
				checkFrame();																						//フレームチェック
		}
		break;
	case WaitOtherAck:
		if(sPtr->getMac() == SIMU::OR_JOJI_ACK){														//ORACK方式の場合	
			state = OrAck;																						//OR用ACK送信待ち状態に遷移
			addTime(genrand() % ORACK_RANGE);															//イベント時刻の設定
			sPtr->list.insert(this);																		//自身をイベントリストへ登録		
			listInsert = true;
		}
		else{																									//ORRTS方式の場合
//**			orFlag = true;																						//OR用フラグを立てる
			//cout << now.dtime() << "\t" << getNid() << "\tOR begin" << orPtr << "\t" << orPtr->getSize() << endl;
			//nPtr->queueShow();
			PACKET* pPtr = nPtr->relayPacket(orPtr, 0);												//パケット中継処理
			//nPtr->queueShow();
			//cout << now.dtime() << "\t" << getNid() << " wait other ack " << orPtr << endl;
			delete orPtr;																						//OR用パケットオブジェクトの廃棄
			orPtr = pPtr;																						//送信バッファに溜めたパケットを新規OR用パケットにする										
			//nPtr->queueShow();
			state = Idle;																						//Idle状態に変更
			if(!chPtr->signalList.size())																	//受信信号が存在しないなら
				checkFrame();																						//フレームチェック
		}
		break;
	case WaitOrAck:
		//cout << now.dtime() << "\t" << (double)++cntSuc / cntTry << endl;			
		state = Idle;																						//Idle状態に変更
		if(!chPtr->signalList.size())																	//受信信号が存在しないなら
			checkFrame();																						//フレームチェック
		break;
	default:
		cout << now.dtime() << " mac process error" << endl, show(0), exit(1);
	}
	return true;
}

//データフレーム送信所要時間の計算
//引数（フレームオブジェクト）
//戻り値（なし）
void MAC::calcDataTime(FRAME* fPtr){
	short size = fPtr->getSize();
	dataTime = 16 + (1 + (size + 1) / (PHYSPEED * 4)) * 4;
}


//信号受信処理
//引数（信号オブジェクト，送信ノードのMACオブジェクト）
//戻り値（なし）
void MAC::receiveSignal(SIGNAL* sigPtr, MAC* omPtr){
	//if(getNid() == 229)
	//cout << getSimu()->getNow().dtime() << "\t" << getNid() << " 信号受信" << sigPtr->getType() 
	//	<< "\t" << sigPtr->getChannel()->getNid() << "\t" << state << endl; 
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	NODE* nPtr = sPtr->node[getNid()];															//ノードオブジェクト 
	CHANNEL* cPtr = nPtr->getChannel();															//チャネルオブジェクト
	switch(sigPtr->getType()){																		//信号タイプによる場合分け
	case SIGNAL::RTS:																					//RTS信号の場合
		if(omPtr->getObject() == getNid()){															//RTSの対象が自分自身なら
			if(state == Nav)																					//NAV状態なら
				break;																								//何もしない
			if(cPtr->signalList.size()){																	//まだ受信信号が存在するなら
				state = Idle;																						//Idle状態へ遷移
				sPtr->list.remove(this);																		//リストに登録されているイベントを解除
				listInsert = false;																				//イベント登録フラグをおろす
				break;
			}
//			cout << now.dtime() << "\t" << getNid() << "\tRTS受信　－＞　CTS送信" << endl;
			cPtr->setDataTime(omPtr->getDatatime());													//受信データ所要時間の設定
			state = Cts;																						//CTS状態へ遷移
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS時間だけ待機
			objectId = omPtr->getNid();																	//送受信対象ノードIDの設定
		}
		else{																									//RTSの対象が自分でなかったら
			if(orFlag && sigPtr->getOrSource() == orSource){										//ORフラグが経っていて同じORRTSを受け取った場合
				//cout << getNid() << " delete by ORRTS " << (int)retrans << endl;
				if(framePtr && framePtr->getPacket() == orPtr){
					reset();																								//送信処理の初期化
					if(!cPtr->signalList.size())																	//他の受信信号がないなら
						checkFrame();																						//送信フレームのチェック			
				}
				else{
					vector<PACKET*>::iterator i = nPtr->getBuffer()->queue.begin();					//バッファからのパケット削除用イテレータ
					while(i != nPtr->getBuffer()->queue.end() && (*i) != orPtr)							//送信バッファから同じORパケットを探す
						i++;
					//if(i == nPtr->getBuffer()->queue.end())													//同じORパケットがなかったらエラー
					//	cout << "OR Packet error " << endl, exit(1);
					nPtr->getBuffer()->queue.erase(i);															//見つけたパケットを削除
					cout << "receive rts " << orPtr << endl;
					delete orPtr;																						//ORパケットを削除
				}
				orPtr = NULL;																						//ORパケットを初期化
				orFlag = false;																					//ORフラグをおろす
			}
			else if(sigPtr->getOrSource() == getNid()){												//ORRTSの対象が自分だったら
				cntSuc--;
				if(framePtr)																						//フレームが存在するなら
					reset();																								//送信処理の初期化
			}
			state = Nav;																						//Nav状態へ遷移
			setTime(now + SIMUTIME(sigPtr->getDuration()));											//Nav時間だけ待機
		}
		sPtr->list.replace(this);																		//イベントリストへ追加
		break;
	case SIGNAL::CTS:																					//CTS信号の場合
		if(omPtr->getObject() == getNid() && state == Waitcts){								//CTSの対象が自分自身なら
			if(cPtr->signalList.size()){																	//まだ受信信号が存在するなら
				state = Idle;																						//Idle状態へ遷移
				if(listInsert){																					//MACイベントがリスト登録されているなら
					sPtr->list.remove(this);																		//リストに登録されているイベントを解除
					listInsert = false;																				//イベント登録フラグをおろす
				}
				break;
			}
//			cout << now.dtime() << "\t" << getNid() << " \tCTS受信　ー＞　DATA送信" << endl;
			state = Data;																						//受信信号が存在しないならDATA状態へ遷移
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS時間だけ待機
		}
		else{																									//CTSの対象が自分でなかったら
//			cout << getNid() << " NAVへ遷移" << endl;
			state = Nav;																						//Nav状態へ遷移
			setTime(now + SIMUTIME(sigPtr->getDuration()));											//Nav時間だけ待機
		}
		sPtr->list.replace(this);																		//イベントリストへ追加
		break;
	case SIGNAL::DATA:																				//DATA信号の場合
		if(omPtr->getObject() == getNid() && state == Waitdata){								//DATAの対象が自分自身なら
//			cout << now.dtime() << "\t" << getNid() << "\tDATA受信　－＞　ACK送信  " << omPtr->getNid() << "\t" << omPtr->getFrame()->getPacket() << "\t" << omPtr->getFrame()->getSeq() << endl;
			FRAME* fPtr = omPtr->getFrame();																//受信フレームオブジェクト
			bool flag = true;																					//同一フレーム再受信チェック用フラグ
			for(short i = 0; i < (short)last.size(); i++){												//直近受信フレームをチェック
				if(last[i] == fPtr->getSeq() + fPtr->getSource() * 1000000){						//同一フレームをすでに受信していたら
					flag = false;																						//フラグを偽にしループ終了
					break;
				}
			}
			if(flag == true){																					//フラグが真（同一フレームの受信なし）の場合
				last.push_back(fPtr->getSeq() + fPtr->getSource() * 1000000);						//直近受信フレームに登録
				last.erase(last.begin());																		//古い受信フレーム情報を削除
				PACKET* pPtr = omPtr->getFrame()->getPacket();											//受信パケットオブジェクト
				if(pPtr->getDest() == getNid() || pPtr->getType() == PACKET::RrepAodv || pPtr->getType() == PACKET::RerrAodv
					|| ((pPtr->getType() == PACKET::InformLoc || pPtr->getType() == PACKET::MrReq) && sPtr->ma[pPtr->getDest() - sPtr->node.size()]->getNid() == getNid())){//パケットの宛先が自分かAODV用制御パケットなら
					RECEIVE* rPtr = new RECEIVE(sPtr, getNid(), now, pPtr, omPtr->getNid());		//受信オブジェクトの作成
					rPtr->setTime(now);																				//イベント時刻の設定
					sPtr->list.insert(rPtr);																		//イベントリストへ追加
				}
				else																									//それ以外のパケットなら
					nPtr->relayPacket(pPtr, 0);																	//パケット中継処理
			}
			if(cPtr->signalList.size()){																	//まだ受信信号が存在するなら
				state = Idle;																						//Idle状態へ遷移
				sPtr->list.remove(this);																		//リストに登録されているイベントを解除
				listInsert = false;																				//イベント登録フラグをおろす
				break;
			}
			state = Ack;																						//受信信号が存在しないならACK状態へ遷移
			setTime(now + SIMUTIME(SIFSTIME));															//SIFS時間だけ待機
			sPtr->list.replace(this);																		//イベントリストへ追加
		}
		else{																									//DATAの対象が自分でなかったら
			if(judgeOpportunistic(omPtr)){																//ORが可能な場合
				state = WaitOtherAck;																			//対象のACK受信待機状態に遷移
				if(orPtr != NULL){																				//ORパケットが存在するなら
					//cout << now.dtime() << "\t" << getNid() << " duplicate opportunistic " << orPtr << endl;
					delete orPtr;																						//ORパケットの削除
					orPtr = NULL;																						//ORパケットの初期化
				}
				orPtr = new PACKET();																			//ORパケットオブジェクト
				//cout << now.dtime() << " opportunistic !!!!"  << getNid() << "\t" << orPtr << endl;
				*orPtr = *(omPtr->getFrame()->getPacket());												//受信パケットをORパケットにコピー
				orPtr->setHere(getNid());
				orSource = omPtr->getNid();																	//OR用フレームの送信元
				orDest = omPtr->getFrame()->getDest();														//OR用フレームの送信宛先
				setTime(now + SIMUTIME(3 * DELAY + SIFSTIME + ackTime));							//ACK待ち時間だけ待機
				sPtr->list.replace(this);																		//イベントリストへ追加	
				listInsert = true;																				//ベント登録フラグを立てる
			}
			else{																									//ORが不可の場合
				if(state == Nav)																					//NAV状態なら
					break;																								//そのまま待機
				state = Idle;																						//NAV状態でないならIdle状態へ遷移
				if(listInsert){																					//イベント登録しているなら
					sPtr->list.remove(this);																		//リストから外す
					listInsert = false;																				//登録フラグをおろす
				}
				if(!cPtr->signalList.size())																	//他の受信信号がないなら
					checkFrame();																						//送信フレーム有無のチェック	
			}
		}
		break;
	case SIGNAL::ACK:																					//ACK信号の場合
		if(omPtr->getObject() == getNid() && state == Waitack){								//ACKの対象が自分自身なら
//			cout << now.dtime() << "\t" << getNid() << "\tACK受信　－＞　フレーム送信成功" << endl;
			reset();																								//送信処理の初期化
			if(!cPtr->signalList.size())																	//他の受信信号がないなら
				checkFrame();																						//送信フレームのチェック
		}
		else{																									//ACKの対象が自分自身でないなら
			//cout << getNid() << "\t" << "OTHER ACK 受信" << endl;
			if(state == WaitOtherAck &&  orSource == omPtr->getObject()){												//				
				//cout << "OTHER ACK 受信" << endl;
				//cout << now.dtime() << "\t" << getNid() << " receive ack " << orPtr << endl;
				delete orPtr;
				orPtr = NULL;
			}
			if(state == Nav)																					//NAV状態なら
				break;																								//そのまま待機
			state = Idle;																						//NAV状態でないならIdle状態へ遷移
			if(listInsert){																					//イベント登録しているなら
				sPtr->list.remove(this);																		//リストから外す
				listInsert = false;																				//登録フラグをおろす
			}
			if(!cPtr->signalList.size())																	//他の受信信号がないなら
				checkFrame();																						//送信フレーム有無のチェック
		}
		break;
	case SIGNAL::BDATA:{																				//BDATA信号の場合
		//cout << getNid() << "\tBDATA受信" << endl;
		PACKET* pPtr = omPtr->getFrame()->getPacket();											//受信パケットオブジェクト		
		RECEIVE* rPtr = new RECEIVE(sPtr, getNid(), now, pPtr, omPtr->getNid());		//受信オブジェクトの作成
		rPtr->setTime(now);																				//イベント時刻の設定
		sPtr->list.insert(rPtr);																		//イベントリストへ追加
		if(state == Nav)																					//NAV状態なら
			break;																								//そのまま待機
		state = Idle;																						//NAV状態でないならIDLE状態へ遷移
		if(listInsert){																					//イベント登録しているなら
			sPtr->list.remove(this);																		//リストから外す
			listInsert = false;																				//登録フラグをおろす
		}
		if(!cPtr->signalList.size())																	//他の受信信号がないなら
			checkFrame();																						//送信フレーム有無のチェック
		break;
							 }
	case SIGNAL::ORACK:
//		cout << now.dtime() << "\t" << omPtr->getNid() << "\t" << omPtr->getOrSource() << "\t", show();
		if(state == WaitOrAck){
			reset();																								//送信処理の初期化
			if(!cPtr->signalList.size())																	//他の受信信号がないなら
				checkFrame();																						//送信フレームのチェック			
		}
		else{
			if(state == Nav)																					//NAV状態なら
				break;																								//そのまま待機
			if(state == OrAck){																				//ORACK送信待ちなら
				cout << "receive orack " << orPtr << endl;
				delete orPtr;																						//ORパケットの削除
				orPtr = NULL;																						//ORパケットの初期化
			}
			state = Idle;																						//NAV状態でないならIdle状態へ遷移
			if(listInsert){																					//イベント登録しているなら
				sPtr->list.remove(this);																		//リストから外す
				listInsert = false;																				//登録フラグをおろす
			}
			if(!cPtr->signalList.size())																	//他の受信信号がないなら
				checkFrame();																						//送信フレーム有無のチェック			
		}
		break;
	}
}

//信号送信完了処理
//引数（信号オブジェクト，送信ノードのMACオブジェクト）
//戻り値(なし)
void MAC::sendFinSignal(SIGNAL* sigPtr, MAC* objectPtr){
	SIMUTIME now = getSimu()->getNow();															//現在時刻	
//	cout << now.dtime() << "\t" << getNid() << "\t送信完了" << sigPtr->getChannel()->getNid() << "\t" << sigPtr->getType() << endl;
	switch(sigPtr->getType()){																		//信号タイプによる場合分け
	case SIGNAL::RTS:																					//RTS信号の場合
		if(orFlag){																							//ORフラグが立っているなら
			sigPtr->setOrSource(orSource);																//OR情報を追加
			orPtr = NULL;																						//ORパケットの初期化
			orFlag = false;																					//ORフラグを下ろす
		}
		state = Waitcts;																					//CTS待ち状態へ遷移
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + ctsTime));								//待機時間の設定
		getSimu()->list.insert(this);																	//イベントリストへ追加
		listInsert = true;																				//イベント登録フラグを立てる
		break;
	case SIGNAL::CTS:																					//CTS信号の場合
		state = Waitdata;																					//DATA待ち状態へ遷移
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + objectPtr->getDatatime()));		//待機時間の設定
		getSimu()->list.insert(this);																	//イベントリストへ追加
		listInsert = true;																				//イベント登録フラグを立てる
		break;
	case SIGNAL::DATA:																				//DATA信号の場合
		cntTry++;
		state = Waitack;																					//ACK待ち状態へ遷移
		setTime(now + SIMUTIME(2 * DELAY + SIFSTIME + ackTime));								//待機時間の設定
		getSimu()->list.insert(this);																	//イベントリストへ追加
		listInsert = true;																				//イベント登録フラグを立てる
		break;
	case SIGNAL::BDATA:																				//BDATA信号の場合
		delete framePtr->getPacket();																	//送信パケットを削除
		delete framePtr;																					//送信フレームを削除
		framePtr = NULL;																					//登録フレームのリセット
	case SIGNAL::ACK:																					//ACK信号の場合
		objectId = -1;																						//送信対象の解除
		if(backoffTime.getLessSec() > 0 || backoffTime.getSec() > 0)						//バックオフ残留期間があるなら
			backoffTime = backoffTime - 1;																//1μ秒だけ減らす
		state = Idle;																						//IDLE状態へ遷移
		if(!getSimu()->node[getNid()]->getChannel()->signalList.size())					//他の受信信号がないなら
			checkFrame();																						//送信フレームがあるかのチェック		
		break;
	case SIGNAL::ORACK:																				//ORACK信号の場合
		objectId = -1;
		getSimu()->node[getNid()]->relayPacket(orPtr, 0);										//パケット中継処理
		cout << "sendfin orack " << orPtr << endl;
		delete orPtr;																						//ORパケットの削除	
		orPtr = NULL;																						//ORパケットの初期化
		break;
	}
}

//バックオフ時間の設定
//引数（なし）
//戻り値（無し）
void MAC::makeBackoff(SIMUTIME now){
	if(timeCompare(backoffTime, now + (1023 * SLOTTIME)))									//バックオフタイムの上限を超えていたら
		cout << "backoff error " << endl, backoffTime.show(), exit(1);						//エラー
	if(backoffTime.getSec() == -2)																//バックオフタイマが起動していなければ
		backoffTime = now + (genrand() % contWindow) * SLOTTIME;								//ウィンドウサイズからバックオフ時間を作成
	else																									//バックオフタイマが起動している場合
		backoffTime = now + backoffTime;																//タイマ残り時間からバックオフ時間を作成
	state = Backoff;
	setTime(backoffTime);																			//バックオフ時間の設定
	getSimu()->list.insert(this);
	listInsert = true;																				//リスト登録フラグを立てる
}

//送信可能かどうかの判定
//引数（自身のノードオブジェクトポインタ）
//戻り値（送信可なら真、送信不可なら偽）
bool MAC::judgeSendable(void){
	NODE* nPtr = getSimu()->node[getNid()];													//ノードオブジェクト
	if(nPtr->routing[objectId]->getNext() != objectId){
//		cout << getNid() << "cannot send to " << objectId << endl; 
		return false;
	}
	return true;
}

//送信処理のリセット
//引数（なし）
//戻り値（なし）
void MAC::reset(void){
	contWindow = 16;																					//コンテンションウィンドウを初期化
	retrans = 0;																						//再送回数を初期化
	//cout << "reset " << framePtr << "\t" << framePtr->getPacket() << endl;
	delete framePtr->getPacket();																	//パケットオブジェクトを消去
	delete framePtr;																					//送信フレームオブジェクトを消去
	framePtr = NULL;																					//フレームオブジェクト登録を初期化
	objectId = -1;																						//送受信対象を初期化
	state = Idle;																						//IDLE状態へ遷移
	getSimu()->list.remove(this);																	//MACイベントをリストから外す
	listInsert = false;																				//リスト登録フラグをおろす
}

//フレームのチェック処理
//引数（リストオブジェクト、ノードオブジェクトポインタ）
//戻り値（なし）
void MAC::checkFrame(void){

//	cout << getNid() << "\tチェックフレーム" << endl;
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	NODE* nPtr = sPtr->node[getNid()];															//ノードオブジェクト
	CHANNEL* cPtr = nPtr->getChannel();															//チャネルオブジェクト
	if(nPtr->getChannel()->signalList.size() != 0)											//他の信号を受信中の場合
		cout << "check frame error " << endl, exit(1);											//エラー
	if(framePtr != NULL){																			//送信待ちのフレームが存在する場合
		if(retrans == RETRANS_MAX){																	//再送回数が上限に達していたら
			//cout << "フレーム再送失敗処理 " << framePtr->getPacket()->getSeq() << "\t" << endl;	
			reset();																								//送信処理の初期化
			nPtr->checkPacket();																				//送信パケットの有無のチェック
			return;																								//他は何もせずに終了
		}
//		cout << "新フレーム" << endl;
		if(retrans > 0){																					//再送フレームの場合
//			cout << getNid() << "  フレーム再送処理 " << endl;
			contWindow = min(1024, contWindow * 2);													//コンテンションウィンドウの更新
		}
		state = Difs;																						//状態をDifs待機に設定
		setTime(now + DIFSTIME);																		//DIFS時間だけ待機
		sPtr->list.insert(this);																		//イベントリストへ登録
	}
	else																									//送信待ちのフレームが存在しない場合
		nPtr->checkPacket();																				//送信バッファのパケット有無をチェック
}

//信号レベルのアイドリングチェック
//引数（なし）
//戻り値（真偽値，アイドリングなら真）
bool MAC::checkIdling(void){
	if(getSimu()->node[getNid()]->getChannel()->getSignal() == NULL)
		return true;
	return false;
}

//ORをするかの判断
//引数（送信元ノードのMACオブジェクト）
//戻り値（真偽値，ORするなら真）
bool MAC::judgeOpportunistic(MAC* omPtr){
	if(getSimu()->getMac() == SIMU::NORMAL)													//通常のMACなら
		return false;																						//ORはしない
	if(orPtr)																							//OR中なら
		return false;																						//あらたなORはしない
	FRAME* fPtr = omPtr->getFrame();																//フレームオブジェクト
	if(fPtr->getDest() == fPtr->getPacket()->getDest())									//フレームと対応パケットの宛先が同じなら				
		return false;																						//ORはしない
	NODE* nPtr = getSimu()->node[getNid()];													//ノードオブジェクト
	if(dist(nPtr->getPos(), getSimu()->node[fPtr->getDest()]->getPos()) > RANGE / 2.0)	//本来の送信宛先ノードとの距離が通信範囲の半分以上なら
		return false;																						//ORはしない
	return true;																						//上記のいずれにも当てはまらないならORをする
}