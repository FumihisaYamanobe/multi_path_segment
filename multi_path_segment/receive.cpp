#include "class.h"
#include "mobileAgent.h"

extern double rand_d();																				//[0,1)の実数乱数発生

//受信処理
//引数（リストオブジェクト，ノードオブジェクト）
//戻り値（真偽値（ダミー））
bool RECEIVE::process(void){
	SIMU* sPtr = getSimu();
	SIMUTIME now = sPtr->getNow();
	//if(now.dtime() > 9.000106)
	//	cout << "type=" << getType() << endl;
	short id = getNid();																			//受信ノードID
	short sid = pPtr->getSource();																//送信元ノードID
	short did = pPtr->getDest();																	//送信宛先ノードID
	PACKET::ptype tid = pPtr->getType();
	NODE* nPtr = sPtr->node[id];																//ノードオブジェクト
	PACKET* newPtr;																				//新規作成パケット
	MAC* mPtr = nPtr->getMAC();																//処理対象MACオブジェクト
	if(pPtr->getSpos().getX() >= 0){
		if(timeCompare(pPtr->getSpos().getT(), nPtr->nodePos[sid].getT()))
			nPtr->nodePos[sid] = pPtr->getSpos();
	}
	bool duplicateFlag = false;																//重複パケット受信チェックフラグ
	for(short i = 0; i < (short)nPtr->receivedPacketList.size(); i++){			//受信パケットリストをチェック
		NODE::RECEIVED_PACKET packet = nPtr->receivedPacketList[i];						//リスト内の受信パケットオブジェクト
		if(packet.getType() == tid && packet.getSource() == sid 
			&& packet.getDest() == did && packet.getSeq() == pPtr->getSeq()
			&& packet.getTime().dtime() == pPtr->getSendStart().dtime()){			//リスト内のオブジェクトと同一のパケットなら
				duplicateFlag = true;																	//重複受信フラグを立てる
				break;																						//チェックは終了
		}
	}
	if(duplicateFlag){																			//重複受信フラグが立っていたら
//		static int cnt;
//		cout << now.dtime() << "\t" << pPtr->getType() << " from " << pPtr->getSource()  << " receive duplicate packet " << ++cnt <<endl;
		return false;																					//受信処理はせずに終了
	}	
	else{																								//重複受信フラグが立っていなければ
		NODE::RECEIVED_PACKET rPacket 
			= NODE::RECEIVED_PACKET(tid, sid, did, pPtr->getSeq(), pPtr->getSendStart());						//受信パケットオブジェクト
		nPtr->receivedPacketList.push_back(rPacket);											//受信パケットリストに追加
		nPtr->receivedPacketList.erase(nPtr->receivedPacketList.begin());				//最も古い重複受信パケット情報を消去
	}
	switch(tid){																					//受信パケットタイプによる場合分け
		case PACKET::Routing:																	//ルーティングパケットの場合
			if(sPtr->getArea() == SIMU::MESH && id >= sPtr->getMAP())					//受信ノードがメッシュSTAなら
				break;																						//何もせずに終了
			if(nPtr->getRoute() == NODE::GEDIR)												//GEDIRルーティングなら
				break;
			if(nPtr->routing[sid]->getHop() == 1)												//ルーティングデータの送信元が隣接端末なら 
				nPtr->makeRoutingTable(sid);															//ルーティングテーブル作成
			break;
		case PACKET::RreqDsr:																	//ルートリクエスト（DSR）の場合
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid)						//既に受信したパケットの場合
				break;																						//受信処理をせずに破棄
			nPtr->floodSeq[sid] = pPtr->getSeq();												//受信フラッディングシーケンスの更新
			if(id == did){																				//自分自身が宛先なら
				//cout << now.dtime() << " ルートリクエスト受信 " << id << endl;
				newPtr = new PACKET(sPtr, now, PACKET::RrepDsr, RREPDSR, id, id, sid, -1);//ルートリプライパケット作成
				newPtr->setSendStart(pPtr->getSendStart());										//パケット送信時刻を要求パケットの時間に変更
				nPtr->path[sid].clear();																//過去の経路情報を削除
				nPtr->path[sid].push_back(id);														//ノードの経路情報の初期化
				newPtr->path.push_back(id);															//リプライパケットの経路情報初期化
				for(short i = (short)pPtr->path.size() - 1; i >= 0; i--){
					//cout << pPtr->path[i] << "->";
					nPtr->path[sid].push_back(pPtr->path[i]);											//ノードの経路情報の書き込み	
					newPtr->path.push_back(pPtr->path[i]);												//リプライパケットの経路情報の書き込み
				}
				//cout << endl;
				newPtr->increSize(pPtr->path.size() * 4);											//経路情報分だけパケットサイズを増加
				newPtr->setSeq(nPtr->getSeq());														//シーケンス番号の付与
				nPtr->increSeq();																			//シーケンス番号のインクリメント
				if(!newPtr->queue(sPtr, false))														//パケットをバッファへ挿入
					delete newPtr;																				//挿入に失敗したら削除
			}
			else{																							//自分自身が宛先でない場合
				//cout << now.dtime() << " ルートリクエスト中継 " << id << endl;
				nPtr->receivedPacketList.pop_back();												//中継処理で再度重複受信処理をするためリストから削除
				newPtr = nPtr->relayPacket(pPtr, 4);												//パケット中継処理
				if(newPtr)																					//中継処理が成功した場合
					newPtr->path.push_back(id);															//自身を経路情報に追加
			}
			break;
		case PACKET::RrepDsr:																	//ルートリプライ（DSR）の場合
			//cout << now.dtime() << " ルートリプライ受信 " << pPtr << "\t" << id << "\t" << sid << "\t" << pPtr->getSeq() << "\t" << pPtr->path.size() << endl;
			nPtr->requestTime[pPtr->getSource()] = SIMUTIME(-10,0);					//ルートリクエストタイムメモリの初期化
			nPtr->path[pPtr->getSource()].clear();											//古い経路情報を一旦削除
			for(char i = (char)pPtr->path.size() - 1; i >= 0; i--)
				nPtr->path[pPtr->getSource()].push_back(pPtr->path[i]);
			break;
		case PACKET::RerrDsr:																	//ルートエラー（DSR）の場合
//			cout << now.dtime() << "\t" << getNid() << " ルートエラー受信 from " << pPtr->getSource() << endl;
			if(nPtr->getRoute() == NODE::MAMR){
				switch (pPtr->mpath_check)
				{
				case 0:
					nPtr->path[pPtr->getErrDest()].clear();											//経路情報を消去
					nPtr->path[pPtr->getErrDest()].push_back(id);									//経路情報の初期化
					break;
				case 1:
					nPtr->path1[pPtr->getErrDest()].clear();											//経路情報を消去
					nPtr->path1[pPtr->getErrDest()].push_back(id);									//経路情報の初期化
					break;
				case 2:
					nPtr->path2[pPtr->getErrDest()].clear();											//経路情報を消去
					nPtr->path2[pPtr->getErrDest()].push_back(id);									//経路情報の初期化
					break;
				}
			}
			else{
				nPtr->path[pPtr->getErrDest()].clear();											//経路情報を消去
				nPtr->path[pPtr->getErrDest()].push_back(id);									//経路情報の初期化
			}
			break;
		case PACKET::RreqAodv:																	//ルートリクエスト（AODV）の場合
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid)						//既に受信したパケットの場合
				break;																						//受信処理をせずに破棄
			nPtr->floodSeq[sid] = pPtr->getSeq();												//受信フラッディングシーケンスの更新
			if(nPtr->routing[sid]->getSeq() >= pPtr->getAodvSeqS())						//送信元の情報が自身のルーティングテーブルより古い場合
				break;																						//受信処理をせずに破棄
			//if(nPtr->routing[sid]->getSeq() == pPtr->getAodvSeqS() && nPtr->routing[sid]->getHop() >= pPtr->getHop())
			//	break;
			nPtr->routing[sid]->setNext(object);												//ルーティングテーブルの転送先の更新
			nPtr->routing[sid]->setHop(pPtr->getHop());										//ルーティングテーブルのホップ数の更新
			nPtr->routing[sid]->setSeq(pPtr->getAodvSeqS());								//ルーティングテーブルのシーケンス番号の更新
			if(id == did){																				//自分自身が宛先なら
				//cout << now.dtime() << " AODVルートリクエスト受信 " << id << " from " << sid << endl;
				newPtr = new PACKET(sPtr, now, PACKET::RrepAodv, RREPAODV, id, id, sid, -1);//ルートリプライパケット作成
				newPtr->setReqDest(id);																	//要求宛先ノードIDを設定
				newPtr->setAodvSeqS(nPtr->getAodvSeq());											//自身のAODV用シーケンスを設定
				newPtr->setSendStart(pPtr->getSendStart());										//パケット送信時刻を要求パケットの時間に変更
				newPtr->setSeq(nPtr->getSeq());														//シーケンス番号の付与
				nPtr->increSeq();																			//シーケンス番号のインクリメント
				if(!newPtr->queue(sPtr, false))														//パケットをバッファへ挿入
					delete newPtr;																				//挿入に失敗したら削除
			}
			else{																							//自分自身が宛先でない場合
//				if(id == 0){
				//cout << now.dtime() << "中継ノードAODVルートリクエスト受信 " << id  << " from " << sid 
				//	<< "\t" << mPtr->getNid() << "\t" << object << endl;
				//cout << nPtr->routing[sid]->getNext() << endl;
//				}
				if(nPtr->routing[did]->getSeq() > pPtr->getAodvSeqD()){						//要求パケットより新しい情報を持っている場合
					newPtr = new PACKET(sPtr, now, PACKET::RrepAodv, RREPAODV, id, id, sid, -1);//ルートリプライパケット作成
					newPtr->setReqDest(did);																//要求宛先ノードIDを設定
					newPtr->setAodvSeqS(nPtr->routing[did]->getSeq());								//自身のAODV用シーケンスを設定
					newPtr->setSendStart(pPtr->getSendStart());										//パケット送信時刻を要求パケットの時間に変更
					newPtr->setSeq(nPtr->getSeq());														//シーケンス番号の付与
					nPtr->increSeq();																			//シーケンス番号のインクリメント
					if(!newPtr->queue(sPtr, false))														//パケットをバッファへ挿入
						delete newPtr;																				//挿入に失敗したら削除
				}
				else{																							//要求パケットより新しい情報を持っていない場合
					//cout << now.dtime() << " ルートリクエスト中継 " << id << endl;
					nPtr->receivedPacketList.pop_back();												//中継処理で再度重複受信処理をするためリストから削除
					nPtr->relayPacket(pPtr, 0);															//パケット中継処理
				}
			}
			break;
		case PACKET::RrepAodv:																	//ルートリプライ（AODV）の場合
//			cout << id << "\t" << nPtr->routing[pPtr->getReqDest()]->getSeq() << "\t" << pPtr->getAodvSeqS() << endl;
			if(nPtr->routing[pPtr->getReqDest()]->getSeq() >= pPtr->getAodvSeqS())	//送信元の情報が自身のルーティングテーブルより古い場合
				break;																						//受信処理をせずに破棄
			nPtr->routing[pPtr->getReqDest()]->setNext(object);
			nPtr->routing[sid]->setHop(pPtr->getHop());										//ルーティングテーブルのホップ数の更新
			nPtr->routing[pPtr->getReqDest()]->setSeq(pPtr->getAodvSeqS());
			if(id != did){
//				cout << "中継ノードAODVルートリプライ受信\t" << id << endl;
				nPtr->receivedPacketList.pop_back();												//中継処理で再度重複受信処理をするためリストから削除
				nPtr->relayPacket(pPtr, 0);															//パケット中継処理
			}
			//else
			//	cout << "送信元AODVルートリプライ受信\t" << id << endl;
			break;
		case PACKET::RerrAodv:																	//ルートリプライ（AODV）の場合
			nPtr->routing[pPtr->getErrDest()]->setNext(-1);													//転送先の初期化											
			nPtr->routing[pPtr->getErrDest()]->setHop(-1);													//ホップ数の初期化
			if(id != did){
				nPtr->receivedPacketList.pop_back();												//中継処理で再度重複受信処理をするためリストから削除
				nPtr->relayPacket(pPtr, 0);															//パケット中継処理
			}
			//else
			//	cout << "送信元AODVルートエラー受信\t" << id << endl;
			break;
		case PACKET::Udp:																			//UDPデータの場合
			pPtr->getUdp()->getObject()->increByte(pPtr->getSize() - 28);				//データグラム受信バイトをインクリメント
			sPtr->increTransDelay(now - pPtr->getSendStart());
			sPtr->increTransDelayCnt();
//			if(sid == 19 && id == 77)
////＊＊＊＊＊＊＊＊ログ表示＊＊＊＊＊＊＊＊
////宛先ノードのUDP受信ログが不必要ならコメントアウトする
//			pPtr->showLog(id, "R", now);
////＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊
			break;
		case PACKET::Tcp:																			//TCPパケットの場合
			pPtr->getSeg()->getTcp()->getObject()->sendAck(pPtr);							//ACK送信処理
			break;
		case PACKET::Ack:																			//TCPACKパケットの場合
			pPtr->getSeg()->getTcp()->getTcpAck(pPtr);										//ACK受信処理
			break;
		case PACKET::LabCenter: case PACKET::LabRa: case PACKET::LabNeighbor: 	//Labパケットの場合
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid 
																  || id >= sPtr->getMAP())			//既に受信したパケットかSTAが受信した場合
				break;																						//受信処理をせずに破棄
			nPtr->floodSeq[sid] = pPtr->getSeq();												//受信フラッディングシーケンスの更新
//			cout << now.dtime() << " LabRa受信 " << id << "\t" << sid << endl;
			if(timeCompare(pPtr->getLAB().getTime(), nPtr->gab[pPtr->getSTA() - sPtr->getMAP()].getTime()))//新しいLAB情報であれば
				nPtr->gab[pPtr->getSTA() - sPtr->getMAP()] = pPtr->getLAB();				//LABの更新
			if(pPtr->getType() == PACKET::LabRa)												//RA-OLSRの場合
				nPtr->relayPacket(pPtr, 0);															//パケット中継処理
			break;
		case PACKET::StaReq:{																	//StaReqパケットの場合
			cout << now.dtime() << "\t" << id << " StaReq 受信" << endl;
			short sSta = pPtr->getSTA();															//要求STA
			short dSta = pPtr->getReqDest();														//宛先STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//宛先STAの接続MAP情報
			if(sPtr->getMesh() == SIMU::IDEAL)
				dMap = sPtr->gab[dSta].getMap();
			cout << sSta << "\t" << dSta << "\t" << dMap << endl;
			if(dMap == id)																				//宛先STAの接続先が自分なら
				newPtr = new PACKET(sPtr, now, PACKET::StaRep, REP, id, id, sSta, -1);//StaRepパケットオブジェクトの作成
			else{																							//宛先STAの接続先が自分でないなら
				if(sPtr->getMesh() == SIMU::NONE){													//非管理方式なら
					newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReqパケットオブジェクトの作成					
				}
				else if(sPtr->getMesh() == SIMU::CENTRAL)											//集中管理方式なら												
					newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, sPtr->getCenter(), -1);	//MapReqパケットオブジェクトの作成
				else{																							//その他の方式なら
					if(dMap != -1)																				//接続MAP情報が存在するなら
						newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, dMap, -1);	//MapReqパケットオブジェクトの作成
					else																							//接続MAP情報が存在しないなら
						newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReqBパケットオブジェクトの作成
				}
			}
			newPtr->setTimeout(pPtr->getTimeout());											//要求タイムアウトオブジェクトの設定
			newPtr->setSTA(sSta);																	//要求STAの登録
			newPtr->setReqSource(id);
			newPtr->setReqDest(dSta);																//宛先STAの登録
			newPtr->setSeq(nPtr->getSeq());														//パケットのシーケンス番号設定
			nPtr->increSeq();																			//ノードシーケンスのインクリメント
			if(!newPtr->queue(sPtr, false))														//バッファへのパケット挿入
				delete newPtr;																				//挿入に失敗したら消去
			break;
								  }
		case PACKET::MapReq:{																	//MapReqパケットの場合
			cout << id << " MapReq 受信  " << pPtr->getReqDest() << "\t" << nPtr->gab[pPtr->getReqDest() - sPtr->getMAP()].getMap() <<  endl;
			short sSta = pPtr->getSTA();															//要求STA
			short dSta = pPtr->getReqDest();														//宛先STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//宛先STAの接続MAP情報
			if(dMap == id){																				//宛先STAの接続先が自分なら
				//cout << id << " が " << sid << " へMapRep 返信  " <<  endl;
				newPtr = new PACKET(sPtr, now, PACKET::MapRep, REP, id, id, pPtr->getReqSource(), -1);	//MapRepパケットオブジェクトの作成
				newPtr->setLAB(nPtr->gab[dSta - sPtr->getMAP()]);								//LAB情報の登録
			}
			else if(dMap != -1)																		//他のMAPに接続しているという情報があるなら
				newPtr = new PACKET(sPtr, now, PACKET::MapReq, REQ, id, id, dMap, -1);	//MapReqパケットオブジェクトの作成
			else{																							//接続情報を持っていないなら
				newPtr = new PACKET(sPtr, now, PACKET::MapReqB, REQ, id, id, -1, -1);	//MapReqBパケットオブジェクトの作成
				//cout << sid << " 不明のためブロードキャスト" << endl;
			}
			newPtr->setTimeout(pPtr->getTimeout());											//要求タイムアウトオブジェクトの設定
			newPtr->setSTA(sSta);																	//要求STAの登録
			newPtr->setReqSource(pPtr->getReqSource());
			newPtr->setReqDest(dSta);																//宛先STAの登録
			newPtr->setSeq(nPtr->getSeq());														//パケットのシーケンス番号設定
			nPtr->increSeq();																			//ノードシーケンスのインクリメント
			if(!newPtr->queue(sPtr, false))														//バッファへのパケット挿入
				delete newPtr;																				//挿入に失敗したら消去
			break;
								  }
		case PACKET::MapReqB:{
			if(nPtr->floodSeq[sid] >= pPtr->getSeq() || id == sid 
																  || id >= sPtr->getMAP())			//既に受信したパケットかSTAが受信した場合
				break;																						//受信処理をせずに破棄
			//if(sid == 112)
			//	cout << id << " MapReqB 受信 " << pPtr->getReqDest() << "\t" << (int)pPtr->getHop() << endl;
			nPtr->floodSeq[sid] = pPtr->getSeq();												//受信フラッディングシーケンスの更新
			short sSta = pPtr->getSTA();															//要求STA
			short dSta = pPtr->getReqDest();														//宛先STA
			short dMap = nPtr->gab[dSta - sPtr->getMAP()].getMap();						//宛先STAの接続MAP情報
			if(dMap == id){																				//宛先STAの接続先が自分なら
			//	cout << "MapRep 送信" << endl;
				newPtr = new PACKET(sPtr, now, PACKET::MapRep, REP, id, id, pPtr->getReqSource(), -1);	//MapRepパケットオブジェクトの作成
				newPtr->setLAB(nPtr->gab[dSta - sPtr->getMAP()]);								//LAB情報の登録
				newPtr->setTimeout(pPtr->getTimeout());											//要求タイムアウトオブジェクトの設定
				newPtr->setSTA(sSta);																	//要求STAの登録
				newPtr->setReqDest(dSta);																//宛先STAの登録
				newPtr->setSeq(nPtr->getSeq());														//パケットのシーケンス番号設定
				nPtr->increSeq();																			//ノードシーケンスのインクリメント
				if(!newPtr->queue(sPtr, false))														//バッファへのパケット挿入
					delete newPtr;																				//挿入に失敗したら消去
			}
			else																								//宛先STAの接続先が自分でないなら		
				nPtr->relayPacket(pPtr, 0);																//パケット中継処理
			break;
									}
		case PACKET::MapRep:{																	//MapRepパケットの場合
//			cout << id << " MapRep 受信 from " << sid << endl;
			short sSta = pPtr->getSTA();															//要求STA
			short dSta = pPtr->getReqDest();														//宛先STA
			nPtr->gab[dSta - sPtr->getMAP()] = pPtr->getLAB();								//LAB情報の設定
			newPtr = new PACKET(sPtr, now, PACKET::StaRep, REP, id, id, sSta, -1);	//StaRepパケットオブジェクトの作成
			newPtr->setTimeout(pPtr->getTimeout());											//要求タイムアウトオブジェクトの設定
			newPtr->setSTA(sSta);																	//要求STAの登録
			newPtr->setReqDest(dSta);																//宛先STAの登録
			newPtr->setSeq(nPtr->getSeq());														//パケットのシーケンス番号設定
			nPtr->increSeq();																			//ノードシーケンスのインクリメント
			if(!newPtr->queue(sPtr, false))														//バッファへのパケット挿入
				delete newPtr;																				//挿入に失敗したら消去
			break;
								  }
		case PACKET::StaRep:
			cout << now.dtime() << "\t" << id << " StaRep受信 " << endl;
			cout << "遅延" << (now - pPtr->getTimeout()->getTime()).dtime() << endl;
			if(timeCompare(now - pPtr->getTimeout()->getTime(), SIMUTIME(5, 0)))
				break;
			if(sPtr->list.deleteEvent(pPtr->getTimeout()) != NULL){
				sPtr->increDelay((now - pPtr->getTimeout()->getTime()).dtime());
				delete pPtr->getTimeout();
			}
			break;
		case PACKET::MigReq:{
			//cout << "get migreq at " << id << " from " << sid << endl;
			LOCATION nowLoc = nPtr->getDerivePos(0);
			LOCATION lastLoc = nPtr->getDerivePos(1);
			MAREP* repPtr = new MAREP(nowLoc, nowLoc.getX() - lastLoc.getX(),
				nowLoc.getY() - lastLoc.getY(), SIMUTIME(1,0), nPtr->getRemainPower(), pPtr->getMa());
			newPtr = new PACKET(sPtr, now, PACKET::MigRep, MIGREP, id, id, sid, -1);//MA移動応答パケットの作成
			newPtr->setMigRep(repPtr);
			newPtr->setDpos(nPtr->nodePos[sid]);
			if(!newPtr->queue(sPtr, false))															//パケットをバッファへ挿入
				delete newPtr;																				//挿入に失敗したら削除
			break;								  								  }
		case PACKET::MigRep:{
			//cout << now.dtime() << " get migrep from " << sid << "\t" << dist(sPtr->node[sid]->getDerivePos(0), sPtr->ma[0]->getCenter()) << endl;
			MA* mPtr = pPtr->getMigRep()->getMa();
			if(mPtr->getTimeout() && !mPtr->getMigration())
				mPtr->decideMigratingNode(sid, pPtr->getMigRep());
			break;
								  }
		case PACKET::InformLoc:								//位置情報をMAに送っている
			//cout << "get informlog --- " << now.dtime() << "\t" << id << "\t" << sid << "\t" << did << endl;
			//cout << now.dtime() << "\t" << id << "   informloc " << endl;
			sPtr->counter2++;
			//cout << now.dtime() << "," << sPtr->counter1 << "," << sPtr->counter2 << "," << (double)sPtr->counter2 / sPtr->counter1 << endl;
			sPtr->ma[0]->lastPos[sid] = sPtr->ma[0]->nodePos[sid];									//現在の位置情報を直近の位置情報へ変更
			sPtr->ma[did - sPtr->node.size()]->nodePos[sid] = pPtr->getSpos();						//MAが持っているノードの位置情報にパケットの送信元の位置情報を入れる	
			//pPtr->getSpos().show();
			break;
		case PACKET::DummyBroadcast:
			//cout << now.dtime() << "\t" << id << "  receive flooding " << endl;
			nPtr->receivedPacketList.pop_back();												//中継処理で再度重複受信処理をするためリストから削除
			nPtr->relayPacket(pPtr, 0);															//パケット中継処理
			break;


		//*****************************************************************//
		// マルチパスリプライパケット
		//*****************************************************************//
		case PACKET::MrRep:{														//マルチパスリプライパケット処理
			//cout << " get MrRep --- " << now.dtime() << " from " << sid << endl;
			int dest = pPtr->reqPath[0];
			nPtr->requestTime[dest] = -2 * REQUEST_INT;										//ルートリクエストタイムメモリの登録
			nPtr->path[dest].clear();														//古い経路情報を一旦削除
			nPtr->path1[dest].clear();
			nPtr->path2[dest].clear();
			nPtr->nodePos[dest] = pPtr->getReqDPos();
			//cout << "reqpath size " << pPtr->reqPath.size() << endl;
			if(pPtr->reqPath.size() > 1){
				for(int i = pPtr->reqPath.size() -1; i >= 0; i--){
					nPtr->path[dest].push_back(pPtr->reqPath[i]);
				}
			}
			if(pPtr->reqPath1.size() > 1){
				for(int i = pPtr->reqPath1.size() -1; i >= 0; i--){
					nPtr->path1[dest].push_back(pPtr->reqPath1[i]);
				}
			}
			if(pPtr->reqPath2.size() > 1){
				for(int i = pPtr->reqPath2.size() -1; i >= 0; i--){
					nPtr->path2[dest].push_back(pPtr->reqPath2[i]);
				}
			}
			//cout << "path[" << dest << "] size after input:" << nPtr->path[dest].size() << endl;
			//cout << "path ::: ";
			//for(int i = 0; i < (int)nPtr->path[dest].size(); i++ ){
			//	cout << "->" << nPtr->path[dest][i];
			//}cout << endl;
			//cout << "path1 ::: ";
			//for(int i = 0; i < (int)nPtr->path1[dest].size(); i++ ){
			//	cout << "->" << nPtr->path1[dest][i];
			//}cout << endl;
			//cout << "path2 ::: ";
			//for(int i = 0; i < (int)nPtr->path2[dest].size(); i++ ){
			//	cout << "->" << nPtr->path2[dest][i];
			//}cout << endl;
			//cout << "path pos ::: " << endl;
			//for(int i = 0; i < nPtr->path[dest].size(); i++ ){
			//	cout << nPtr->path[dest][i] << "\t";
			//	sPtr->node[nPtr->path[dest][i]]->getPos().show();
			//}
			//for(int i = 0; i < nPtr->path[dest].size()-1; i++ ){
			//	cout << nPtr->path[dest][i] << "-" << nPtr->path[dest][i+1] << "\t" << dist(sPtr->node[nPtr->path[dest][i]]->getPos(),sPtr->node[nPtr->path[dest][i+1]]->getPos()) << endl;;
			//}
			break;
						   }
		//*****************************************************************//
		// マルチパスリクエストパケット
		//*****************************************************************//
		case PACKET::MrReq:														//マルチパスリクエストパケット処理
			//cout << " get MrReq --- " << now.dtime() << "\tsid:" << sid << "\tdid:" << did << "\tid:" << id <<  "\treqdest\t" << pPtr->getReqDest() << endl;

			//MAの関数を呼び出してパケットに中身を代入
			newPtr = nPtr->ma[0]->makeMultiRoute(sPtr, sid, did, now, pPtr, id);
			newPtr->setReqDPos(nPtr->ma[0]->nodePos[pPtr->getReqDest()]);
			//cout << "test out " << endl;
			//cout << "path size " << newPtr->reqPath.size() << endl;
			//cout << "path1 size " << newPtr->reqPath1.size() << endl;
			//cout << "path2 size " << newPtr->reqPath2.size() << endl;
			for(int i = (int)newPtr->reqPath.size() - 1; i >= 0; i--){						//要求パケットの到達経路を逆向きに応答パケット返信経路に設定
				sPtr->node[id]->path[sid].push_back(newPtr->reqPath[i]);
				//cout << "test out  2 " << endl;
				//cout << newPtr->reqPath[i] << endl;
			}
			newPtr->setSize(newPtr->getSize() + 4 * (int)newPtr->reqPath.size() + 4 * (int)newPtr->reqPath1.size() + 4 * (int)newPtr->reqPath2.size());
			if(!newPtr->queue(sPtr, false)){														//パケットをバッファへ挿入
				cout << "packet delete at makeMultiRoute" << endl;
				delete newPtr;																				//挿入に失敗したら削除
			}
			break;


//			newPtr = new PACKET(sPtr, now, PACKET::MrRep, RREQDSR, id, id , sid, -1);
//			nPtr->path[sid].clear();
//			nPtr->path[sid].push_back(id);
//			newPtr->path.push_back(id);
//			for(short i = (short)pPtr->path.size() - 1; i >= 0; i--){
//				//cout << pPtr->path[i] << "->";
//				nPtr->path[sid].push_back(pPtr->path[i]);											//ノードの経路情報の書き込み	
//				newPtr->path.push_back(pPtr->path[i]);												//リプライパケットの経路情報の書き込み
//			}
//			newPtr->increSize(pPtr->path.size() * 4);											//経路情報分だけパケットサイズを増加
//			newPtr->setSeq(nPtr->getSeq());														//シーケンス番号の付与
//			nPtr->increSeq();																			//シーケンス番号のインクリメント
//			if(!newPtr->queue(sPtr, false))														//パケットをバッファへ挿入
//				delete newPtr;																				//挿入に失敗したら削除
//			break;



			//	//MAの関数を呼び出して経路構築をする

			//	//関数呼び出し!!!

			//}
			//else{																//自分自身が宛先でない場合
			//	nPtr->receivedPacketList.pop_back();								//中継処理で再度重複受信処理をするためリストから削除
			//	newPtr = nPtr->relayPacket(pPtr, 4);								//パケット中継処理
			//	if(newPtr)														//中継処理が成功した場合
			//		newPtr->path.push_back(id);									//自身を経路情報に追加
			//}
			//break;
		//******************************************************************//
		// マルチパスリクエストパケット　以上
		//******************************************************************//


//		case PACKET::Dissemination:{
//			bool flag = false;
////			cout << "receive dissmination " << id << endl;
//			if(dist(node[id]->getPos(), pPtr->getMa()->getCenter()) < MARANGE)
//				flag = true;
//			else{
//				int lid = pPtr->getLast();
//				if(dist(node[id]->getDerivePos(0), pPtr->getMa()->getCenter())
//				< dist(node[lid]->getDerivePos(0), pPtr->getMa()->getCenter()))
//					flag = true;
//			}
//			if(flag){
//				PACKET* newPtr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//複製用パケット
//				*newPtr = *pPtr;																				//元のパケット情報を複製パケットへコピー
////				cout << "relay " << id << endl;
//				if(newPtr->queue(list, node[id], false, true, getTime()) == false)							//バッファへ挿入
//					delete newPtr;																					//挿入に失敗したら消去
//				newPtr->setHere(id);																				//パケットの現在位置を自分に設定
//				newPtr->setLast(id);
//			}
//			break;
//											}
	}
	return false;																					//受信オブジェクトは消去
}

//void result(SIMU* sPtr){
//	double sumPow = 0;
//	double outTime = 0;
//	int migNum = 0;
//
//	for(int i = 0; i < (int)sPtr->getNode().size(); i++)
//		sumPow += 100000000 - sPtr->getNode()[i]->getRemainPower();
//	for(int i = 0; i < (int)sPtr->ma.size(); i++){
//		outTime += sPtr->ma[i]->getOutTime();
//		migNum += sPtr->ma[i]->getMigNum();
//	}
//	cout << sumPow << "," << outTime << "," << migNum << endl;
//}
//
//void result(const char* fname, SIMU* sPtr){
//	fstream fstr (fname, ios::app);	
//	double sumPow = 0;
//	double outTime = 0;
//	int migNum = 0;
//	double max = 0;
//	for(int i = 0; i < (int)sPtr->getNode().size(); i++){
//		if(max < 100000000 - sPtr->getNode()[i]->getRemainPower())
//			max = 100000000 - sPtr->getNode()[i]->getRemainPower();
//		sumPow += 100000000 - sPtr->getNode()[i]->getRemainPower();
//	}
//	for(int i = 0; i < (int)sPtr->ma.size(); i++){
//		outTime += sPtr->ma[i]->getOutTime() / 1000000.0;
//		migNum += sPtr->ma[i]->getMigNum();
//	}
////	fstr << load << "," << sumPow / 1000.0 / (int)sPtr->node.size() << "," << max / 1000.0 << "," << outTime / (TIMELIMIT) << "," << migNum << endl;
//}

