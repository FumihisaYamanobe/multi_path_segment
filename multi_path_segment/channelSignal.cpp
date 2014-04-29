#include "class.h"
#include "mobileAgent.h"

int cntS = 0;

SIGNAL::SIGNAL(CHANNEL* cPtr){																	//コンストラクタ
//	cout << "signal make " << ++cntS << endl;
	chPtr = cPtr;
	MAC* mPtr = chPtr->getSimu()->node[chPtr->getNid()]->getMAC();
	orSource = -1;
	switch(mPtr->getState()){
	case MAC::Rts:
		typeId = RTS;
		if(mPtr->getOrFlag())
			orSource = mPtr->getOrSource(), cout << cPtr->getSimu()->getNow().dtime() << "  fheroirgei "  << cPtr->getNid() << endl;
		length = mPtr->getRtsTime() + DELAY;
		if(chPtr->getSimu()->getMac() != SIMU::OR_JOJI_ACK)
			duration = 3 * (SIFSTIME + DELAY) + mPtr->getCtsTime() + mPtr->getDatatime() + mPtr->getAckTime();
		else
			duration = 3 * (SIFSTIME + DELAY) + mPtr->getCtsTime() + mPtr->getDatatime() + mPtr->getAckTime() + ORACK_RANGE;
		break;
	case MAC::Cts:
		typeId = CTS;
		length = mPtr->getCtsTime() + DELAY;
		if(chPtr->getSimu()->getMac() != SIMU::OR_JOJI_ACK)
			duration = 2 * (SIFSTIME + DELAY) + cPtr->getDataTime() + mPtr->getAckTime();
		else
			duration = 2 * (SIFSTIME + DELAY) + cPtr->getDataTime() + mPtr->getAckTime() + ORACK_RANGE;
		break;
	case MAC::Data:
		typeId = DATA;
		length = mPtr->getDatatime() + DELAY;
		break;
	case MAC::Ack:
		typeId = ACK;
		length = mPtr->getAckTime() + DELAY;
		break;
	case MAC::BData:
		typeId = BDATA;
		length = mPtr->getDatatime() + DELAY;
		break;
	case MAC::OrAck:
		typeId = ORACK;
		length = mPtr->getAckTime() + DELAY;
		break;
	default:
		cout << "WHAT" << endl;
		length = 100;
		break;
	}
}

SIGNAL::~SIGNAL(){
//	cout << "signal delete " << --cntS << endl;
}

CHANNEL::CHANNEL(SIMU* ptr, MAC* mac, SIMUTIME tval, short idval):EVENT(ptr, tval, EVENT::Channel, idval){	//コンストラクタ{
	mPtr = mac;
	sigPtr = NULL;																						//受信信号オブジェクト（初期値はNULL）
	sendFlag = false;
}

bool CHANNEL::process(void){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	NODE* nPtr = sPtr->node[getNid()];															//ノードオブジェクト
	if(sendFlag){																						//信号送信処理の場合						
		sendFlag = false;																					//送信フラグを下す
		double txPower = pow((RANGE / 100) / 1.5, 4) * RX_THRESHOLD;						//送信電力
		int interval = sigPtr->getLength() + DELAY;												//信号送信時間の設定
		for(short i = 0; i < (short)nPtr->neighborList.size(); i++){						//隣接ノードへの送信
			short nid = nPtr->neighborList[i].getId();												//対象のノードID
			CHANNEL* chPtr = sPtr->node[nid]->getChannel();											//チャネルオブジェクト
			double distance = nPtr->neighborList[i].getDist() / 100.0;							//対象までの距離（メートル換算）
			double rxPower = txPower * pow(1.5 / distance, 4.0);									//Two-ray groundの減衰モデルにより電力を算出
			if(rxPower > CS_THRESHOLD){																	//キャリアセンス可能なら
				RX_SIGNAL_LIST signal(getNid(), rxPower);													//信号オブジェクト
				signal.setRxFinTime(now + SIMUTIME(interval));											//信号受信完了時刻の設定
				if(chPtr->signalList.size() == 0){															//受信信号が存在しないなら
					sPtr->node[nid]->calcPower();
					sPtr->node[nid]->setPowerMode(NODE::RECEIVE);
					MAC* omPtr = chPtr->getMAC();																	//受信ノードのMACオブジェクト
					if(omPtr->getState() == MAC::Backoff || omPtr->getState() == MAC::Difs){		//バックオフかDifs待機中なら
						omPtr->setState(MAC::Idle);																	//Idle状態に変更
						sPtr->list.remove(omPtr);																		//バックオフイベントをリストから外す
						omPtr->setListInsert(false);																	//リスト登録フラグをおろす
						SIMUTIME bTime = omPtr->getBackoff();														//バックオフ時間
						if(timeCompare(bTime, now) || (bTime.getSec() == now.getSec() && 
																	bTime.getLessSec() == now.getLessSec())		)//バックオフ待機中なら
							omPtr->setBackoff(bTime - now);																//残りバックオフタイムの記憶
					}
					chPtr->signalList.push_back(signal);														//信号情報設定
					chPtr->setTime(now + SIMUTIME(interval));													//信号受信完了時刻の設定
					sPtr->list.insert(chPtr);																		//イベントリストへ追加
					if(rxPower > RX_THRESHOLD)																		//信号識別可能電力なら
						chPtr->setSignal(sigPtr);																		//信号オブジェクトの設定
				}
				else{																									//受信信号が存在するなら
					vector<RX_SIGNAL_LIST>::iterator j = chPtr->signalList.begin();					//信号情報リストのイテレータ（初期値はリストの先頭）
					while(j != chPtr->signalList.end() && (*j).getRxPower() > rxPower)				//リストの電力が信号電力より小さくなるまで後方チェック
						j++;
					if(j == chPtr->signalList.begin()){															//自分の電力が最大の場合
						chPtr->setTime(now + SIMUTIME(interval));													//信号受信完了時刻の設定
						sPtr->list.replace(chPtr);																		//イベントリストの再配置
						if(rxPower > RX_THRESHOLD && rxPower > (*j).getRxPower() * CP_THRESHOLD)		//信号識別可能電力なら
							chPtr->setSignal(sigPtr);																		//信号オブジェクトの設定
						else if(chPtr->getSignal())																	//信号識別不可電力ですでに識別信号受信中なら
							chPtr->setSignal(NULL);																			//受信信号を識別不可にする
					}
					else																									//自分の電力が最大でない場合
						if(chPtr->getSignal() != NULL && chPtr->getSignal()->getChannel() != chPtr
							&& rxPower * CP_THRESHOLD > chPtr->signalList[0].getRxPower())					//識別可能信号が存在しこれに干渉する場合
							chPtr->setSignal(NULL);																			//受信信号を識別不可にする
					chPtr->signalList.insert(j, signal);														//受信信号リストへの挿入
				}
			}
		}
		addTime(interval + 1);																			//送信完了時刻の設定
		sPtr->list.insert(this);																		//イベントリストの設定
	}
	else{																									//信号受信処理の場合
		if(signalList.size() == 0)																		//受信信号が存在しないなら														
			cout << now.dtime() << "\t" << getNid() << "  signal receive error " << endl, exit(1);										//受信信号エラー
		vector<RX_SIGNAL_LIST>::iterator i = signalList.begin();								//信号情報リストのイテレータ（初期値はリストの先頭）
		i = signalList.erase(i);																				//リストの先頭を消去
		while(i != signalList.end()){																	//リストを最後尾までチェック
			if(timeCompare(now, (*i).getRxFnTime()))													//既に受信完了している信号がある場合
				i = signalList.erase(i);																				//リストから消去
			else
				i++;
		}
		if(sigPtr){																							//信号オブジェクトが設定されていたら
			if(sigPtr->getChannel() != this){															//他ノードの送信信号であったら
				mPtr->receiveSignal(sigPtr, sigPtr->getChannel()->getMAC());						//信号受信処理
				sigPtr = NULL;																						//信号登録をリセット
			}
			else{																									//自ノードの送信信号であったら
				if(sigPtr->getType() != SIGNAL::BDATA )													//ブロードキャスト信号でないなら											
					mPtr->sendFinSignal(sigPtr, sPtr->node[mPtr->getObject()]->getMAC());			//ユニキャスト信号送信完了処理
				else																									//ブロードキャスト信号なら	
					mPtr->sendFinSignal(sigPtr, NULL);															//ブロードキャスト信号送信完了処理
				delete sigPtr;																						//信号オブジェクトの消去
				sigPtr = NULL;																						//信号登録をリセット
			}
			if(signalList.size()){																			//受信信号がある場合
				setTime(signalList[0].getRxFnTime());														//イベント発生時刻を受信完了時刻に設定
				sPtr->list.insert(this);																		//イベントリストへ登録
			}
			else{
				getSimu()->node[getNid()]->calcPower();
				getSimu()->node[getNid()]->setPowerMode(NODE::WAIT);
			}
		}
		else{																									//信号オブジェクトが設定されていなかったら
			//cout << getNid() << " 識別不可信号受信" << endl;
			if(!signalList.size()){																			//受信信号がない場合
				getSimu()->node[getNid()]->calcPower();
				getSimu()->node[getNid()]->setPowerMode(NODE::WAIT);
				if(mPtr->getState() == MAC::Idle)															//Idle状態の場合
					mPtr->checkFrame();																				//フレームチェック
			}
			else{																									//受信信号がある場合
				setTime(signalList[0].getRxFnTime());														//次の受信完了時刻の設定
				sPtr->list.insert(this);																		//イベントリストへ登録
			}
		}
	}
	return true;																						//関数を終了（オブジェクトは消去しない）
}

//信号送信処理
//引数（なし）
//戻り値（なし）
void CHANNEL::sendSignal(SIMUTIME now){
	getSimu()->node[getNid()]->calcPower();
	getSimu()->node[getNid()]->setPowerMode(NODE::SEND);
	SIGNAL* siPtr = new SIGNAL(this);															//信号オブジェクトの作成
	int interval = siPtr->getLength() + 1;														//信号送信時間の設定
	sigPtr = siPtr;																					//信号オブジェクトの設定
	RX_SIGNAL_LIST signal(getNid(), 10000000000000);										//信号オブジェクト
	signal.setRxFinTime(now + SIMUTIME(interval));											//信号受信完了時刻の設定
	if(signalList.size() > 0)																		//受信信号が存在するなら
		getSimu()->list.remove(this);																	//イベントリストから受信処理を外す
	signalList.insert(signalList.begin(), signal);											//自分の信号を受信信号リストへ挿入
	sendFlag = true;
}