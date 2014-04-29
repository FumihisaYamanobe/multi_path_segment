
#define TIMELIMIT 100																				//シミュレーション終了時刻（秒）
//#define TIMELIMIT 2000																			//シミュレーション終了時刻
#define AREA     1000 * 100																		//シミュレーションエリアの半径（円形）もしくは1辺（正方形）（ｃｍ）
#define RANGE    100 * 100																			//通信可能距離（ｃｍ）
#define GRIDNUM 10																					//格子エリアモデルの場合の格子数
#define PAUSE_SEC 1																					//目的地到着時の停止時間（秒単位の値）
#define PAUSE_MSEC 0																					//目的地到着時の停止時間（ミリ秒単位の値）

#define NODENUM 500

//#define RX_THRESHOLD 1.585e-9																	//受信可能電力(-88dBm)
#define RX_THRESHOLD 6.310e-8																		//受信電力レベル（g, 54MB, -72dBm)
//#define CS_THRESHOLD 5.012e-11																	//キャリアセンス可能受信電力(-103dBm)
#define CS_THRESHOLD 3.981e-9																		//キャリアセンス可能受信電力(-84dBm)
#define CP_THRESHOLD 10.0																			//受信識別電力比の閾値（10倍）


//#define TX_POWER  0.28183815																		//送信電力

//#define TX_POWER  1.246e-0																		//送信電力

//#define TX_POWER  3.162e-2																		//送信電力
	
#define FREQUENCY 2.472e9																				//周波数
//#define FREQUENCY 914e6

#define TCPSIZE 1024																					//TCPパケットサイズ
#define TCPDEFAULTSIZE 1024																				//TCPセグメント初期サイズ
#define TCPRESETSIZE 50																					//TCPセグメントの再設定時の引いたり足したりするサイズ
#define UDPSIZE 1024																					//UDPパケットサイズ
#define TCPACKSIZE 40																				//ACKパケットサイズ

#define MAXHOP 100

#define RREQDSR 24
#define RREPDSR 20
#define RERRDSR 24

#define RREQAODV 24
#define RREPAODV 20
#define RERRAODV 24

#define LABPACKET 24																					//LABパケットのデフォルトサイズ
#define REQ 24																							//各種リクエストパケットのデフォルトサイズ
#define REP 24																							//各種リプライパケットのデフォルトサイズ

#define INFORM 28																						//デフォルト20バイトに位置情報4バイトと時間情報4バイト

#define REQUEST_INT 1000 * 1000																	//ルートリクエスト・ルートエラーパケットの最短送信インターバル（μｓ）
#define TTLTIME 20 * 1000000																		//パケットの生存時間上限（μｓ）

#define RTT_ALPHA 0.875																				//RTT推測用係数(TCP標準）

#define SEND_POWER 530.0																			//送信電力（mW）DC2J1DZ150
#define RECEIVE_POWER 326.0																		//受信電力（mW）DC2J1DZ150
#define WAIT_POWER 0.4																				//待機電力（mW）
#define SLEEP_POWER 0.02																			//スリープ時消費電力（mW）

#define LOCERR 0 * 100																				//測定位置誤差（±ｃｍ）	

#define INITTIME 30 * 60																			//データ測定開始時刻

#define BUFFERSIZE 1000 * 1000																	//端末のバッファサイズ（単位：バイト数）
#define DATASIZE 1024																				//データサイズ（バイト）
#define RETRANS_MAX 7																				//再送上限

#define PHYSPEED 54																					//物理層の伝送速度（Mbps）
#define DELAY 1																						//伝搬遅延（μｓ）
#define SLOTTIME 20																					//スロット時間（μｓ）
#define DIFSTIME 56
#define SIFSTIME 16																					//ＳＩＦＳ時間（μｓ）
#define RTSSIZE 20																					//ＲＴＳサイズ（バイト）
#define CTSSIZE 14																					//ＣＴＳサイズ（バイト）
#define ACKSIZE 14																					//ＡＣＫサイズ（バイト）
#define BEACONINT 1000000																			//ビーコン周期（μ秒）
//#define BROADINTERVAL 1000

#define ORACK_RANGE 100																				//ORACKを返すための待機時間範囲

#define ROUTEREPMASIZE 48
