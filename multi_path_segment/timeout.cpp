#include "class.h"
#include "mobileAgent.h"

unsigned long genrand2();																			//メルセンヌツイスタ乱数外部宣言（ノード移動用）
int poison(double);

TIMEOUT::TIMEOUT(SIMU* ptr, SIMUTIME tval, type tid, short idval):EVENT(ptr, tval, EVENT::Timeout, idval){ 
	typeId = tid;
}
//
//TIMEOUT::~TIMEOUT(){ 
//}

//引数（リストオブジェクト，ノードオブジェクト）
//戻り値：（真偽値（ダミー））
bool TIMEOUT::process(void){
	SIMU* sPtr = getSimu();																			//シミュレータオブジェクト
	SIMUTIME now = sPtr->getNow();																//現在時刻
	PACKET* pPtr;																						//パケットオブジェクト
	short id = getNid();																				//ノードID
	NODE* nPtr = sPtr->node[id];																	//ノードオブジェクト
	switch(typeId){																					//タイムアウトの種類による場合分け
		case Tcp:																						//TCPタイムアウトの場合
			delete typePtr.tPtr->getObject();														//TCPシンクを消去
			delete typePtr.tPtr;																			//TCPオブジェクトを消去
			return false;																					//タイムアウトオブジェクト自身も消去
		case Udp:																						//UDPタイムアウトの場合
			//cout << typePtr.uPtr->getSize() << "," << typePtr.uPtr->getObject()->getByte() << ","
			//	<< typePtr.uPtr->getObject()->getByte() / (double)typePtr.uPtr->getSize() << endl;
			sPtr->increSendUdp(typePtr.uPtr->getSize());
			sPtr->increReceiveUdp(typePtr.uPtr->getObject()->getByte());
			delete typePtr.uPtr->getObject();														//UDPシンクを消去
			delete typePtr.uPtr;																			//UDPオブジェクトを消去
			return false;																					//タイムアウトオブジェクト自身も消去
		case Ma:																							//MAタイムアウトの場合
			typePtr.maPtr->getTCP()->setAbort();													//移動用TCPの中断フラグを立てる
			typePtr.maPtr->resetMigration();															//移動フラグをリセット	
			//cout << now.dtime() << " migrate abortion " << typePtr.maPtr << "\t" << typePtr.maPtr->getTCP() << endl;
			return false;																					//タイムアウトオブジェクト自身も消去
		case Segment:{																					//TCPセグメントタイムアウトの場合
			TCP* tcpPtr = typePtr.sPtr->getTcp();													//タイムアウトの対象TCPオブジェクト
////＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
////タイムアウト再送の送信ログが不必要ならコメントアウトする	
//			cout << now.dtime() << "\tTO\t" << getNid() << "\tTCP\t" << tcpPtr << "\t" << typePtr.sPtr->getSeq()
//				<< "\t--------\t--------" << endl;
////＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
			typePtr.sPtr->setNAretrans(false);
			typePtr.sPtr->setTimeout(NULL);
			tcpPtr->retransmission();
			return false;																					//オブジェクトを消去
		}
		case Mareq:																						//MA移動要求の場合
			if(typePtr.maPtr->getCandidate() != -1)												//移動候補が存在するなら
				typePtr.maPtr->migration();																//移動処理
			return true;																					//関数を抜けてもオブジェクトは消去されない
		case StaReq:{																					//STA要求の場合
			if(flag){
				cout << getNid() << " 経路構築失敗" << endl;
				sPtr->increReqFail();
				return false;
			}
			cout << now.dtime() << " sta要求　" << id << "\t" << dest << "\t" << sPtr->gab[dest].getMap() 
				<< "\t" << sPtr->node[dest]->getMAP() << "\t" << nPtr->getMAP() << endl;
			if(nPtr->getMAP() == -1 || sPtr->node[dest]->getMAP() == -1){					//自身か宛先の接続MAPが存在しないなら
				cout << "MAPに非接続" << endl;
				return false;																					//そのまま終了しオブジェクトを削除
			}
			sPtr->increReqNum();
			pPtr = new PACKET(sPtr, now, PACKET::StaReq, 
																REQ, id, id, nPtr->getMAP(), 1);		//パケットオブジェクトの作成
			pPtr->setTimeout(this);
			pPtr->setSTA(id);																				//要求STAの設定
			pPtr->setReqDest(dest);																		//宛先STAの設定
			pPtr->setSeq(nPtr->getSeq());																//パケットのシーケンス番号設定
			nPtr->increSeq();																				//ノードシーケンスのインクリメント
			if(!pPtr->queue(sPtr, false))																//バッファへのパケット挿入
				delete pPtr;																					//挿入に失敗したら消去
			timeVal = now;																					//現在時刻を時刻情報として記憶
			addTime(SIMUTIME(5, 0));																	//要求失敗判定時刻の設定
			flag = true;																					//失敗判定用にフラグをあげる
			sPtr->list.insert(this);																	//イベントリストに登録
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + SIMUTIME(poison(sPtr->getReqRate())), 
				TIMEOUT::StaReq, genrand2() % sPtr->getSTA() + sPtr->getMAP());			//次のSTA要求の作成
			toPtr->setFlag(false);																		//フラグはおろしておく
			short dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();							//STA要求の宛先
			while(dest == toPtr->getNid())																//宛先が自身と同じである限り
				dest = genrand2() % sPtr->getSTA() + sPtr->getMAP();									//宛先を再設定
			toPtr->setDest(dest);																			//宛先STAの登録
			sPtr->list.insert(toPtr);																		//イベントリストへ登録
			return true;																					//タイムアウトオブジェクトは消去しない
						}
		//case MakeUdp:{																					//UDPフロー作成の場合
		//	cout << now.dtime() << "\t UDPフロー発生" << endl;
		//	UDP* udp = new UDP(sPtr, id, now, 200, 500 * 1000);								//UDPオブジェクト
		//	UDPSINK* udpsink = new UDPSINK(sPtr, 0);												//UDPシンク
		//	udp->connectSink(udpsink);																	//エージェントとシンクの結合
		//	sPtr->list.insert(udp);																		//UDPエージェントをイベントリストへ追加

		//	//short source = id;
		//	//cout << source << "->";
		//	//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
		//	//	source = sPtr->node[source]->routing[dest]->getNext();
		//	//	cout << source << "->";
		//	//}
		//	//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
		//	
		//	TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeUdp, genrand2() % (NODENUM - 1) + 1);
		//	toPtr->setRate(rate);
		//	sPtr->list.insert(toPtr);
		//	return false;
		//	//setNid(genrand2() % sPtr->getMAP());
		//	//addTime(poison(rate));																		//次のタイムアウト発生時刻の設定
		//	//sPtr->list.insert(this);																	//タイムアウトをイベントリストへ登録
		//	//return true;
		//				 }
		case MakeUdp:{																					//UDPフロー作成の場合
			//short dest = genrand2() % NODENUM;
			//while(dest == id)
			//	dest = genrand2() % NODENUM;
			dest = 0;
			//cout << now.dtime() << "\t UDPフロー発生 " << id << "\t" << dest << endl;
			UDP* udp = new UDP(sPtr, id, now, 200, 500 * 1000);								//200kbps, 500KBの負荷
			UDPSINK* udpsink = new UDPSINK(sPtr, dest);											//UDPシンク
			udp->connectSink(udpsink);																	//エージェントとシンクの結合
			sPtr->list.insert(udp);																		//UDPエージェントをイベントリストへ追加

			//short source = id;
			//cout << source << "->";
			//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
			//	source = sPtr->node[source]->routing[dest]->getNext();
			//	cout << source << "->";
			//}
			//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
			
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeUdp, genrand2() % (NODENUM - 1) + 1);
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
			return false;
						 }
		case MakeTcp:{																					//UDPフロー作成の場合
			do{
				dest = genrand2() % (sPtr->node.size() - 1); //destはランダム
			}while(dest == id);
//			cout << id << " TCP start " << now.dtime() << "\tdest " << dest << endl;
			TCP* tcp = new TCP(sPtr, id, now , 1000 * 1000);						//TCPエージェント（引数：ID，開始時刻，送信バイト数）
			TCPSINK* tcpsink = new TCPSINK(sPtr, dest);											//TCPシンク（引数：ID）
			tcp->connectSink(tcpsink);																	//エージェントとシンクの結合
			sPtr->list.insert(tcp);																		//UDPエージェントをイベントリストへ追加

			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), MakeTcp, genrand2() % (sPtr->node.size() - 1));
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
//			cout << "next TCP " << toPtr->getEventTime().dtime() << endl;
			return false;

			//short source = id;
			//cout << source << "->";
			//while(sPtr->node[source]->routing[dest]->getNext() != dest && sPtr->node[source]->routing[dest]->getNext() != -1){
			//	source = sPtr->node[source]->routing[dest]->getNext();
			//	cout << source << "->";
			//}
			//cout << sPtr->node[source]->routing[dest]->getNext() << endl;
			
						 }
		case MeshMa:{																					//メッシュネットワークのエージェント巡回の場合
			cout << "エージェント巡回開始 " << getNid() << endl;
			MA* maPtr = typePtr.maPtr;																	//対応エージェント												
			short source = maPtr->meshRoute[maPtr->getRouteOrder()];							//エージェントの滞在ノード
			if(source != getNid())
				cout << "mesh agent error " << endl, exit(1);
			short dest = maPtr->meshRoute[maPtr->increRouteOrder()];							//エージェントの移動先
			if(maPtr->getRouteOrder() == maPtr->meshRoute.size() - 1)						//エージェント経路表の最後まできたら
				maPtr->resetRouteOrder();																	//経路表の最初に戻す
			TCP* tcp = new TCP(sPtr, source, now, maPtr->getSize());							//TCPフローの作成
			TCPSINK* tcpsink = new TCPSINK(sPtr, dest);											//TCPSINKの作成
			tcp->connectSink(tcpsink);																	//コネクションの作成
			tcp->setMA(maPtr);																			//対応MAをTCPに登録
			sPtr->list.insert(tcp);																		//イベントリストへ登録
			return false;																					//オブジェクトは消去
						}
		case BackGround:{																					//UDPフロー作成の場合
			//cout << now.dtime() << "\t UDPフロー発生 " << id << "\t" << dest << endl;
			PACKET* pPtr = new PACKET(sPtr, now, PACKET::DummyBroadcast, 1024, id, id, -1, -1); 
			pPtr->setSeq(nPtr->getSeq());																//パケットのシーケンス番号設定
			nPtr->increSeq();																				//ノードシーケンスのインクリメント			
			if(!pPtr->queue(sPtr, false))																//バッファへのパケット挿入
				delete pPtr;																					//挿入に失敗したら消去			
			TIMEOUT* toPtr = new TIMEOUT(sPtr, now + 1 + poison(rate), BackGround, genrand2() % NODENUM);
			toPtr->setRate(rate);
			sPtr->list.insert(toPtr);
			return false;
						 }
		default:
			return true;
	}
}
