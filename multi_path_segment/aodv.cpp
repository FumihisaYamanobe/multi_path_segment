//#include "class.h"
//#include "mobileAgent.h"


//
////AODV_RREQのコンストラクタ（新規作成）
////引数（自身のシーケンス番号，宛先のシーケンス番号，現在時刻，送信元ID，現在ノードID，宛先ID，宛先フラグ）
//AODV_RREQ::AODV_RREQ(int sSeq, int dSeq, double tval, int sid, int hid, int did, bool flag){	
//	PACKET* ptr = new PACKET(tval, PACKET::RreqAodv, RREQAODV, sid, hid, did, -1, 0);//リクエストパケット作成
//	pPtr = ptr;																					//リクエストオブジェクトに作成パケットを登録
//	ptr->setAodvRreq(this);																	//パケットにリクエストオブジェクトを登録
//	sourceSeq = sSeq;																			//送信元を設定
//	destSeq = dSeq;																			//宛先を設定
//	destFlag = flag;																			//宛先フラグを設定
//}
//
////AODV_RREQのコンストラクタ（複製）
////引数（複製元オブジェクトポインタ）
//AODV_RREQ::AODV_RREQ(AODV_RREQ* rPtr){
//	PACKET* ptr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//複製用パケットのの作成
//	*ptr =*(rPtr->getPacket());															//複製元リクエストの対応パケット情報をコピー
//	pPtr = ptr;																					//リクエストオブジェクトに作成パケットを登録
//	ptr->setAodvRreq(this);																	//パケットにリクエストオブジェクトを登録
//	sourceSeq = rPtr->getSourceSeq();													//送信元を複製
//	destSeq = rPtr->getDestSeq();															//宛先を複製
//	destFlag = rPtr->getDestFlag();														//宛先フラグを複製
//}
//
////AODV_RREPのコンストラクタ（新規作成）
////引数（自身のシーケンス番号，宛先のシーケンス番号，現在時刻，送信元ID，現在ノードID，宛先ID，宛先フラグ）
//AODV_RREP::AODV_RREP(int sSeq, int dSeq, double tval, int sid, int hid, int did){
//	PACKET* ptr = new PACKET(tval, PACKET::RrepAodv, RREPAODV, hid, hid, did, -1, 0);
//	pPtr = ptr;																					//リプライオブジェクトに作成パケットを登録
//	ptr->setAodvRrep(this);																	//パケットにリプライオブジェクトを登録
//	dest = sid;
//	sourceSeq = sSeq;																			//送信元を設定
//	destSeq = dSeq;																			//宛先を設定
//}
//
////AODV_RREPのコンストラクタ（複製）
////引数（複製元オブジェクトポインタ）
//AODV_RREP::AODV_RREP(AODV_RREP* rPtr){
//	PACKET* ptr = new PACKET(-1, PACKET::Null, -1, -1, -1, -1, -1, -1);			//複製用パケットのの作成
//	*ptr =*(rPtr->getPacket());															//複製元リプライの対応パケット情報をコピー
//	pPtr = ptr;																					//リプライオブジェクトに作成パケットを登録
//	ptr->setAodvRrep(this);																	//パケットにリプライオブジェクトを登録
//	dest = rPtr->getDest();
//	sourceSeq = rPtr->getSourceSeq();													//送信元を複製
//	destSeq = rPtr->getDestSeq();															//宛先を複製
//}
//	
