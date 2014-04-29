
//リスト関連クラスの定義


//リスト要素クラス（テンプレートクラス）
template <class T> class LINK{																	//オブジェクトリスト構成要素クラス（テンプレート）
	template <class T> friend class LIST;														//リストクラスはフレンドクラス
	LINK* prevPtr;																						//自分の親要素ポインタ
	LINK* leftPtr;																						//自分の左子要素ポインタ
	LINK* rightPtr;																					//自分の右子要素ポインタ
	T* objectPtr;																						//オブジェクトポインタ
public:
	LINK(T* ptr)		{ prevPtr = leftPtr = rightPtr = NULL, objectPtr = ptr; }	//コンストラクタ
	~LINK()				{ ; }																			//デストラクタ
	LINK* getright()	{ return rightPtr; }														//自分の左子取得関数
	LINK* getleft()	{ return leftPtr; }														//自分の右子取得関数
	LINK* getPrev()	{ return prevPtr; }														//自分の親取得関数
	T* getObject()		{ return objectPtr; }													//オブジェクト取得関数
	LINK<T>* getNext(void);																			//リストの次要素取得関数
	LINK<T>* getBefore(void);																		//リストの前要素取得関数
	void showPtr()    { std::cout << prevPtr << "-" 
		<< leftPtr << "-" << rightPtr << std::endl; }										//親子ポインタ表示関数
};

//リストの次の要素を取得
//引数（対象リスト要素ポインタ）
//戻り値：（取得要素ポインタ）
template <class T> LINK<T>* LINK<T>::getNext(){
	LINK<T>* ptr = this;																				//リストポインタは自分自身を指す
	if(ptr->getright() == NULL){																	//右子がいなければ自分の先祖を左子に持つ初めての親が必ず次の要素
		LINK<T>* nextPtr = ptr->getPrev();															//親へ移動
		if(nextPtr == NULL)																				//親が存在しなければ
			return NULL;																						//次要素は存在しない
		if(nextPtr->getleft() == ptr)																	//親の左子が自分ならば
			return nextPtr;																					//この親が次要素となる
		while(nextPtr != NULL && nextPtr->getright() == ptr){									//親の右子が自分ならば
			ptr = nextPtr;																						//この親を子にして
			nextPtr = nextPtr->getPrev();																	//さらにその親について同様の処理をする
		}
		return nextPtr;																					
	}
	LINK<T>* nextPtr = ptr->getright();															//右子がいるならば
	while(nextPtr->getleft() != NULL)															//右子から左へ辿り続ける
		nextPtr = nextPtr->getleft();																//右子からみて再左子が次の要素
	return nextPtr;
}

//リストの前の要素を取得
//引数（対象リスト要素ポインタ）
//戻り値：（取得要素ポインタ）
template <class T> LINK<T>* LINK<T>::getBefore(){
	LINK<T>* ptr = this;
	if(ptr->getleft() == NULL){																	//左子がいなければ自分の先祖を右子に持つ初めての親が必ず前の要素
		LINK<T>* nextPtr = ptr->getPrev();															//親へ移動
		if(nextPtr == NULL)																				//親が存在しなければ																				//親が存在しなければ
			return NULL;																						//前要素は存在しない
		if(nextPtr->getright() == ptr)																//親の右子が自分ならば
			return nextPtr;																					//この親が前要素となる
		while(nextPtr != NULL && nextPtr->getleft() == ptr){									//親の左子が自分ならば
			ptr = nextPtr;																						//この親を子にして
			nextPtr = nextPtr->getPrev();																	//さらにその親について同様の処理をする
		}
		return nextPtr;
	}
	LINK<T>* nextPtr = ptr->getleft();																//左子がいるならば
	while(nextPtr->getright() != NULL)																//左子から右へ辿り続ける
		nextPtr = nextPtr->getright();																//左子からみて再右子が次の要素
	return nextPtr;
}

//リストクラス（テンプレート）
template <class T> class LIST{																	//オブジェクトリストクラス
	LINK<T>* top;																						//リストのトップ
	LINK<T>* first;																					//リスト先頭（最も左の要素）
	LINK<T>* last;																						//リスト最後尾（最も右の要素）
public:
	LIST(){ top = first = last = NULL; }														//コンストラクタ
	~LIST(){ deleteList(top); }																	//デストラクタ
	LINK<T>* insertLeft(LINK<T>*, LINK<T>*);													//左への挿入
	LINK<T>* insertRight(LINK<T>*, LINK<T>*);													//右への挿入
	LINK<T>* addFirst(T*);																			//先頭への追加
	LINK<T>* addLast(T*);																			//最後尾への追加
	LINK<T>* remove(LINK<T>*);																		//要素のリストからの削除
	LINK<T>* remove(T*);																				//要素のリストからの削除
	LINK<T>* remove(void);																			//先頭要素の削除
	LINK<T>* find(LINK<T>*, T*);																	//指定要素からの該当オブジェクト検索
	LINK<T>* find(T*);																				//先頭要素からの該当オブジェクト検索
	LINK<T>* insert(LINK<T>*);																		//指定箇所への要素挿入
	LINK<T>* insert(T*);																				//先頭への要素挿入
	LINK<T>* replace(T*);																			//要素の再配置
	void deleteList(LINK<T>*);																		//指定要素以降のリスト削除
	void deleteList(void);																			//先頭からのリスト削除
	LINK<T>* deleteEvent(T*);																		//要素削除
	LINK<T>* getTop(void)  { return top; }														//トップ取得関数
	LINK<T>* getFirst(void){ return first; }													//先頭取得関数
	LINK<T>* getLast(void) { return last; }													//最後尾取得関数
	LINK<T>* getNEXT(LINK<T>*);																	//次要素取得
	void show(LINK<T>*);																				//リスト表示
	void show(void);																					//リスト表示
	void orderShow(LINK<T>*);																		//順序リスト表示
	void orderShow(void);																			//順序リスト表示
};

//左の子への挿入処理
//引数（挿入対象ポインタ、挿入ポインタ）
//戻り値：（挿入ポインタ）
template <class T> LINK<T>* LIST<T>::insertLeft(LINK<T>* thisPtr, LINK<T>* insertPtr){
	thisPtr->leftPtr = insertPtr;																	//挿入箇所は対象ポインタの左の子
	insertPtr->prevPtr = thisPtr;																	//挿入ポインタの親は挿入対象
	if(first == thisPtr)																				//挿入対象がfirstならば
		first = insertPtr;																				//今度は挿入ポインタがfirst		
	return insertPtr;																					//挿入ポインタを返す
}

//右の子への挿入処理
//引数（挿入対象ポインタ、挿入ポインタ）
//戻り値：（挿入ポインタ）
template <class T> LINK<T>* LIST<T>::insertRight(LINK<T>* thisPtr, LINK<T>* insertPtr){
	thisPtr->rightPtr = insertPtr;																//挿入箇所は対象ポインタの右の子
	insertPtr->prevPtr = thisPtr;																	//挿入ポインタの親は挿入対象
	if(last == thisPtr)																				//挿入対象がlastの場合
		last = insertPtr;																					//今度は挿入ポインタがlast
	return insertPtr;																					//挿入ポインタを返す
}

//リストの先頭への要素追加
//引数（追加対象のオブジェクトポインタ）
//戻り値（追加要素ポインタ）
template <class T> LINK<T>* LIST<T>::addFirst(T* objectPtr){
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//挿入要素のリストポインタ作成
	if(first == NULL)																					//firstがなければ
		insert(insertPtr);																				//単なるイベントの挿入処理と同じ
	else																									//firstが存在する場合
		insertLeft(first, insertPtr);																	//firstの左の子への挿入
	if(first != insertPtr)																			//挿入要素のポインタがfirstでなければエラー処理
		std::cout << "addFirst error" << std::endl, exit(1);
	return insertPtr;																					//挿入ポインタを返す
}

//リストの最後尾への要素追加
//引数（追加対象のオブジェクトポインタ）
//戻り値（追加要素ポインタ）
template <class T> LINK<T>* LIST<T>::addLast(T* objectPtr){
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//挿入要素のリストポインタ作成
	if(last == NULL)																					//lastがなければ
		insert(insertPtr);																				//単なるイベントの挿入処理と同じ
	else																									//lastが存在する場合
		insertRight(last, insertPtr);																	//lastの右の子への挿入
	if(last != insertPtr)																			//挿入要素のポインタがlastでなければエラー処理
		std::cout << "addLast error" << std::endl, exit(1);
	return insertPtr;																					//挿入ポインタを返す
}

//リストからの要素の削除（要素データ自体は削除しない）
//引数（削除対象要素のリストポインタ）
//戻り値（削除対象要素のリストポインタ）
template <class T> LINK<T>* LIST<T>::remove(LINK<T>* thisPtr){
	if(thisPtr == NULL)																				//削除対象が存在しなければ
		return NULL;																						//削除なしを意味するNULLを返す
	LINK<T>* lPtr = thisPtr->leftPtr;															//削除対象の左子のリストポインタ
	LINK<T>* rPtr = thisPtr->rightPtr;															//削除対象の右子のリストポインタ
	LINK<T>* pPtr = thisPtr->prevPtr;															//削除対象の親のリストポインタ
	LINK<T>* ptr = NULL;																				//各種処理に使用するリストポインタ変数
	if(lPtr != NULL){																					//削除対象に左子が存在する場合
		ptr = lPtr;																							//親の新しい子は左子
		if(rPtr != NULL){																					//右子も存在する場合には右子の付け替えが必要
			while(lPtr->rightPtr != NULL)																	//左子から出発し右子が存在しなくなるまで
				lPtr = lPtr->rightPtr;																		//右へリストを移動
			lPtr->rightPtr = rPtr;																			//右端へ到達したらその右子に削除対象の右子を付け替える
			rPtr->prevPtr = lPtr;																			//当然ながら付け替えられた右子の親は移動した右端である
		}																										//右子が存在しなければ付け替えの必要はない
	}
	else{																									//削除対象に左子が存在しない場合
		if(rPtr != NULL)																					//右子は存在するという場合には
			ptr = rPtr;																							//親の新しい子は右子
		else																									//右子も存在しない場合には
			ptr = NULL;																							//親の新しい子は無し
	}
	if(pPtr == NULL){																					//親がいない場合
		if(ptr != NULL)																					//新しい子が存在するなら
			ptr->prevPtr = NULL;																				//自分が親となる
		top = ptr;																								//当然自分がtop（子がいなければtopはNULL）
	}
	else{																									//親がいる場合	
		if(ptr != NULL)																					//新しい子も存在するなら
			ptr->prevPtr = pPtr;																				//その子の親は削除対象の親							
		if(pPtr->leftPtr == thisPtr)																	//削除対象が親の左子ならば
			pPtr->leftPtr = ptr;																				//その子も親の左子になる
		else																									//削除対象が親の右子ならば
			pPtr->rightPtr = ptr;																			//その子も親の右子になる
	}
	if(top != NULL){																					//トップが存在するなら						
		ptr = top;																					
		while(ptr->leftPtr != NULL)																	//トップからひたすら左へたどり
			ptr = ptr->leftPtr;
		first = ptr;																						//行きついたところが先頭
		ptr = top;
		while(ptr->rightPtr != NULL)																	//トップからひたすら右へたどり
			ptr = ptr->rightPtr;											
		last = ptr;																							//行きついたところが最後尾
	}
	else																									//トップが存在しないなら
		first = last = NULL;																				//firstもlastも存在しない			
	thisPtr->leftPtr = thisPtr->rightPtr = thisPtr->prevPtr = NULL;					//削除対象の親要素データや子要素データを消去
	return thisPtr;
}

//リストからの要素の削除（要素データ自体は削除しない）
//引数（削除対象要素ポインタ）
//戻り値（削除対象要素のリストポインタ）
template <class T> LINK<T>* LIST<T>::remove(T* objectPtr){
	return remove(find(objectPtr));																			//先頭ポインタを指定してremove関数を呼ぶ
}

//リストからの先頭要素の削除（要素データ自体は削除しない）
//引数（なし）
//戻り値（削除対象要素のリストポインタ）
template <class T> LINK<T>* LIST<T>::remove(void){
	return remove(first);																			//先頭ポインタを指定してremove関数を呼ぶ
}
//リストの要素検索
//引数（検索対象要素のリストポインタ，検索用のオブジェクトポインタ）
//戻り値（削除対象要素のリストポインタ）
template <class T> LINK<T>* LIST<T>::find(LINK<T>* herePtr, T* objectPtr){
	if(first == NULL)																					//先頭が存在しないなら
		return NULL;																						//検索失敗を意味するNULLを返す
	if(herePtr->getObject() == objectPtr)														//検索対象が該当するなら
		return herePtr;																					//その要素ポインタを返す
	if(herePtr->leftPtr != NULL){																	//検索対象が該当せず右子が存在するなら
		LINK<T>* ptr = find(herePtr->leftPtr, objectPtr);											//右子に対して検索要求する
		if(ptr != NULL)																					//右子以降でヒットしたら
			return ptr;																							//その要素ポインタを返す
	}	
	if(herePtr->rightPtr == NULL)																	//左子が存在しない場合
		return NULL;																						//自分以降にはヒットする要素はない		
	LINK<T>* ptr = find(herePtr->rightPtr, objectPtr);											//左子が存在するならそれ以降に対して検索し
	return ptr;																							//その結果を返す
}

//リストの要素検索
//引数（検索用のイベントポインタ）
//戻り値（削除対象要素のリストポインタ）
template <class T> LINK<T>* LIST<T>::find(T* objectPtr){
	return find(top, objectPtr);																	//引数がオブジェクトだけならトップから検索
}

//イベントリストへの挿入
//引数（リスト要素ポインタ）
//戻り値：（挿入したリスト要素ポインタ）
template <class T> LINK<T>* LIST<T>::insert(LINK<T>* thisPtr){
	LINK<T>* herePtr = top;																			//挿入判定の比較対象（最初はtopポインタ）
	LINK<T>* prevPtr;
	bool flag;																							//左右のどちらの子に挿入するかを示すフラグ
	while(herePtr != NULL){																			//比較対象がある限り処理を反復
		prevPtr = herePtr;																			//現在の対象は挿入イベントの親になりうる
		if(timeCompare(herePtr->getObject()->getEventTime(), thisPtr->getObject()->getEventTime())){
			herePtr = herePtr->leftPtr;															//挿入イベント時刻が小さければ次に左子と比較
			flag = true;																				//挿入は左子に行うのでフラグは真
		}
		else{
			herePtr = herePtr->rightPtr;															//挿入イベント時刻が大きければ次に右子と比較
			flag = false;																				//挿入は右の子に行うのでフラグは偽
		}
	}
	if(top == NULL){																					//もしtopが存在しなければ挿入イベントがtop
		top = first = last = thisPtr;
		return thisPtr;
	}
	else{																									//topが存在するなら
		if(flag == true)																					//フラグが真なら
			return insertLeft(prevPtr, thisPtr);														//左子で挿入判定
		else																									//フラグが偽なら
			return insertRight(prevPtr, thisPtr);														//右子で挿入判定
	}
}

//イベントリストへの挿入
//引数（オブジェクトポインタ）
//戻り値：（挿入したリスト要素ポインタ）
template <class T> LINK<T>* LIST<T>::insert(T* objectPtr){
	if(objectPtr == NULL)																			//オブジェクトがなければ何もしない
		return NULL;
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//イベントに対応するリスト要素の作成
	return insert(insertPtr);																		//要素ポインタにより挿入処理関数を呼び出す
}

//イベントリストへの再配置
//引数（オブジェクトポインタ）
//戻り値：（挿入したリスト要素ポインタ）
template <class T> LINK<T>* LIST<T>::replace(T* objectPtr){
	if(objectPtr == NULL)																			//オブジェクトがなければ何もしない
		return NULL;
	LINK<T>* findPtr = find(objectPtr);
	if(findPtr){
		LINK<T>* insertPtr = remove(findPtr);
		return insert(insertPtr);																		//要素ポインタにより挿入処理関数を呼び出す
	}
	return insert(objectPtr);
}

//要素の消去（イベント自体は消去されない）
//引数（削除対象のイベントポインタ）
//戻り値：（なし）
template <class T> LINK<T>* LIST<T>::deleteEvent(T* objectPtr){
	LINK<T>* listPtr = find(objectPtr);															//該当オブジェクトをもつ要素を検索
	remove(listPtr);																					//ヒットした要素をリストから削除
	delete listPtr;																					//要素データを消去
	return listPtr;
}

//リストの消去（イベントも消去）
//引数（消去対象のリスト要素ポインタ）
//戻り値：（なし）
template <class T> void LIST<T>::deleteList(LINK<T>* ptr){
	if(ptr == NULL)																					//消去対象が存在しなければ何もしない
		return;	
	deleteList(ptr->leftPtr);																		//自分の左子以降を消去
	deleteList(ptr->rightPtr);																		//自分の右子以降を消去
	delete ptr;																							//要素自身を消去
}

//リストの消去（イベントも消去）
//引数（なし）
//戻り値：（なし）
template <class T> void LIST<T>::deleteList(void){
	deleteList(top);																					//引数がない場合にはtopから消去開始
}

//リストの次の要素を取得
//引数（対象リスト要素ポインタ）
//戻り値：（取得要素ポインタ）
template <class T> LINK<T>* LIST<T>::getNEXT(LINK<T>* ptr){
	if(ptr == last)																					//対象がlastであれば次の要素は存在しない
		return NULL;
	if(ptr->getright() == NULL){																		//右子がいなければ自分を左子に持つ親が必ず次の要素
		LINK<T>* nextPtr = ptr->getPrev();
		if(nextPtr->getleft() == ptr)
			return nextPtr;
		while(nextPtr->getright() == ptr){
			ptr = nextPtr;
			nextPtr = nextPtr->getPrev();
		}
		return nextPtr;
	}
	LINK<T>* nextPtr = ptr->getright();																//右子がいるならば
	while(nextPtr->getleft() != NULL)																//右子から左へ辿り続ける
		nextPtr = nextPtr->getleft();																	//右子からみて再左子が次の要素
	return nextPtr;
}

//リスト表示
//引数（表示対象の要素ポインタ）
//戻り値：（取得要素ポインタ）
template <class T> void LIST<T>::show(LINK<T>* ptr){
	if(ptr == NULL)																					//対象がなければ何もしない
		return;
	cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
		<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
		<< "-->";
	ptr->getObject()->getEventTime().showMuSec();
	cout << "\t";
	ptr->getObject()->show();																		//対象要素のイベント表示
	if(ptr->leftPtr != NULL)																		//左子がいる場合
		show(ptr->leftPtr);																				//左子のリスト表示
	if(ptr->rightPtr != NULL)																		//右子がいる場合
	show(ptr->rightPtr);																					//右子のリスト表示
}

//リスト表示
//引数（表示対象の要素ポインタ）
//戻り値：（取得要素ポインタ）
template <class T> void LIST<T>::show(void){
	cout << "first " << first << endl;
	show(top);																							//対象の指定がなければtopを表示
	cout << endl;																						//改行
}

template <class T> void LIST<T>::orderShow(LINK<T>* ptr){
	if(ptr->leftPtr == NULL){
		cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
			<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
			<< "-->";
		ptr->getObject()->getEventTime().showMuSec();
		cout << "\t";
		ptr->getObject()->show();																	//対象要素のイベント表示
		if(ptr->rightPtr != NULL)
			orderShow(ptr->rightPtr);																//右子のリスト表示
		else
			return;
	}
	else{
		orderShow(ptr->leftPtr);																	//右子のリスト表示
		cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
			<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
			<< "-->";
		ptr->getObject()->getEventTime().showMuSec();
		cout << "\t";
		ptr->getObject()->show();																	//対象要素のイベント表示
		if(ptr->rightPtr != NULL)
			orderShow(ptr->rightPtr);																//右子のリスト表示			
	}
}

template <class T> void LIST<T>::orderShow(void){
	orderShow(top);																					//対象の指定がなければtopを表示
	cout << endl;																						//改行
}
