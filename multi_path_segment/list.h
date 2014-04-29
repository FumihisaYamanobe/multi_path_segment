
//���X�g�֘A�N���X�̒�`


//���X�g�v�f�N���X�i�e���v���[�g�N���X�j
template <class T> class LINK{																	//�I�u�W�F�N�g���X�g�\���v�f�N���X�i�e���v���[�g�j
	template <class T> friend class LIST;														//���X�g�N���X�̓t�����h�N���X
	LINK* prevPtr;																						//�����̐e�v�f�|�C���^
	LINK* leftPtr;																						//�����̍��q�v�f�|�C���^
	LINK* rightPtr;																					//�����̉E�q�v�f�|�C���^
	T* objectPtr;																						//�I�u�W�F�N�g�|�C���^
public:
	LINK(T* ptr)		{ prevPtr = leftPtr = rightPtr = NULL, objectPtr = ptr; }	//�R���X�g���N�^
	~LINK()				{ ; }																			//�f�X�g���N�^
	LINK* getright()	{ return rightPtr; }														//�����̍��q�擾�֐�
	LINK* getleft()	{ return leftPtr; }														//�����̉E�q�擾�֐�
	LINK* getPrev()	{ return prevPtr; }														//�����̐e�擾�֐�
	T* getObject()		{ return objectPtr; }													//�I�u�W�F�N�g�擾�֐�
	LINK<T>* getNext(void);																			//���X�g�̎��v�f�擾�֐�
	LINK<T>* getBefore(void);																		//���X�g�̑O�v�f�擾�֐�
	void showPtr()    { std::cout << prevPtr << "-" 
		<< leftPtr << "-" << rightPtr << std::endl; }										//�e�q�|�C���^�\���֐�
};

//���X�g�̎��̗v�f���擾
//�����i�Ώۃ��X�g�v�f�|�C���^�j
//�߂�l�F�i�擾�v�f�|�C���^�j
template <class T> LINK<T>* LINK<T>::getNext(){
	LINK<T>* ptr = this;																				//���X�g�|�C���^�͎������g���w��
	if(ptr->getright() == NULL){																	//�E�q�����Ȃ���Ύ����̐�c�����q�Ɏ����߂Ă̐e���K�����̗v�f
		LINK<T>* nextPtr = ptr->getPrev();															//�e�ֈړ�
		if(nextPtr == NULL)																				//�e�����݂��Ȃ����
			return NULL;																						//���v�f�͑��݂��Ȃ�
		if(nextPtr->getleft() == ptr)																	//�e�̍��q�������Ȃ��
			return nextPtr;																					//���̐e�����v�f�ƂȂ�
		while(nextPtr != NULL && nextPtr->getright() == ptr){									//�e�̉E�q�������Ȃ��
			ptr = nextPtr;																						//���̐e���q�ɂ���
			nextPtr = nextPtr->getPrev();																	//����ɂ��̐e�ɂ��ē��l�̏���������
		}
		return nextPtr;																					
	}
	LINK<T>* nextPtr = ptr->getright();															//�E�q������Ȃ��
	while(nextPtr->getleft() != NULL)															//�E�q���獶�֒H�葱����
		nextPtr = nextPtr->getleft();																//�E�q����݂čč��q�����̗v�f
	return nextPtr;
}

//���X�g�̑O�̗v�f���擾
//�����i�Ώۃ��X�g�v�f�|�C���^�j
//�߂�l�F�i�擾�v�f�|�C���^�j
template <class T> LINK<T>* LINK<T>::getBefore(){
	LINK<T>* ptr = this;
	if(ptr->getleft() == NULL){																	//���q�����Ȃ���Ύ����̐�c���E�q�Ɏ����߂Ă̐e���K���O�̗v�f
		LINK<T>* nextPtr = ptr->getPrev();															//�e�ֈړ�
		if(nextPtr == NULL)																				//�e�����݂��Ȃ����																				//�e�����݂��Ȃ����
			return NULL;																						//�O�v�f�͑��݂��Ȃ�
		if(nextPtr->getright() == ptr)																//�e�̉E�q�������Ȃ��
			return nextPtr;																					//���̐e���O�v�f�ƂȂ�
		while(nextPtr != NULL && nextPtr->getleft() == ptr){									//�e�̍��q�������Ȃ��
			ptr = nextPtr;																						//���̐e���q�ɂ���
			nextPtr = nextPtr->getPrev();																	//����ɂ��̐e�ɂ��ē��l�̏���������
		}
		return nextPtr;
	}
	LINK<T>* nextPtr = ptr->getleft();																//���q������Ȃ��
	while(nextPtr->getright() != NULL)																//���q����E�֒H�葱����
		nextPtr = nextPtr->getright();																//���q����݂čĉE�q�����̗v�f
	return nextPtr;
}

//���X�g�N���X�i�e���v���[�g�j
template <class T> class LIST{																	//�I�u�W�F�N�g���X�g�N���X
	LINK<T>* top;																						//���X�g�̃g�b�v
	LINK<T>* first;																					//���X�g�擪�i�ł����̗v�f�j
	LINK<T>* last;																						//���X�g�Ō���i�ł��E�̗v�f�j
public:
	LIST(){ top = first = last = NULL; }														//�R���X�g���N�^
	~LIST(){ deleteList(top); }																	//�f�X�g���N�^
	LINK<T>* insertLeft(LINK<T>*, LINK<T>*);													//���ւ̑}��
	LINK<T>* insertRight(LINK<T>*, LINK<T>*);													//�E�ւ̑}��
	LINK<T>* addFirst(T*);																			//�擪�ւ̒ǉ�
	LINK<T>* addLast(T*);																			//�Ō���ւ̒ǉ�
	LINK<T>* remove(LINK<T>*);																		//�v�f�̃��X�g����̍폜
	LINK<T>* remove(T*);																				//�v�f�̃��X�g����̍폜
	LINK<T>* remove(void);																			//�擪�v�f�̍폜
	LINK<T>* find(LINK<T>*, T*);																	//�w��v�f����̊Y���I�u�W�F�N�g����
	LINK<T>* find(T*);																				//�擪�v�f����̊Y���I�u�W�F�N�g����
	LINK<T>* insert(LINK<T>*);																		//�w��ӏ��ւ̗v�f�}��
	LINK<T>* insert(T*);																				//�擪�ւ̗v�f�}��
	LINK<T>* replace(T*);																			//�v�f�̍Ĕz�u
	void deleteList(LINK<T>*);																		//�w��v�f�ȍ~�̃��X�g�폜
	void deleteList(void);																			//�擪����̃��X�g�폜
	LINK<T>* deleteEvent(T*);																		//�v�f�폜
	LINK<T>* getTop(void)  { return top; }														//�g�b�v�擾�֐�
	LINK<T>* getFirst(void){ return first; }													//�擪�擾�֐�
	LINK<T>* getLast(void) { return last; }													//�Ō���擾�֐�
	LINK<T>* getNEXT(LINK<T>*);																	//���v�f�擾
	void show(LINK<T>*);																				//���X�g�\��
	void show(void);																					//���X�g�\��
	void orderShow(LINK<T>*);																		//�������X�g�\��
	void orderShow(void);																			//�������X�g�\��
};

//���̎q�ւ̑}������
//�����i�}���Ώۃ|�C���^�A�}���|�C���^�j
//�߂�l�F�i�}���|�C���^�j
template <class T> LINK<T>* LIST<T>::insertLeft(LINK<T>* thisPtr, LINK<T>* insertPtr){
	thisPtr->leftPtr = insertPtr;																	//�}���ӏ��͑Ώۃ|�C���^�̍��̎q
	insertPtr->prevPtr = thisPtr;																	//�}���|�C���^�̐e�͑}���Ώ�
	if(first == thisPtr)																				//�}���Ώۂ�first�Ȃ��
		first = insertPtr;																				//���x�͑}���|�C���^��first		
	return insertPtr;																					//�}���|�C���^��Ԃ�
}

//�E�̎q�ւ̑}������
//�����i�}���Ώۃ|�C���^�A�}���|�C���^�j
//�߂�l�F�i�}���|�C���^�j
template <class T> LINK<T>* LIST<T>::insertRight(LINK<T>* thisPtr, LINK<T>* insertPtr){
	thisPtr->rightPtr = insertPtr;																//�}���ӏ��͑Ώۃ|�C���^�̉E�̎q
	insertPtr->prevPtr = thisPtr;																	//�}���|�C���^�̐e�͑}���Ώ�
	if(last == thisPtr)																				//�}���Ώۂ�last�̏ꍇ
		last = insertPtr;																					//���x�͑}���|�C���^��last
	return insertPtr;																					//�}���|�C���^��Ԃ�
}

//���X�g�̐擪�ւ̗v�f�ǉ�
//�����i�ǉ��Ώۂ̃I�u�W�F�N�g�|�C���^�j
//�߂�l�i�ǉ��v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::addFirst(T* objectPtr){
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//�}���v�f�̃��X�g�|�C���^�쐬
	if(first == NULL)																					//first���Ȃ����
		insert(insertPtr);																				//�P�Ȃ�C�x���g�̑}�������Ɠ���
	else																									//first�����݂���ꍇ
		insertLeft(first, insertPtr);																	//first�̍��̎q�ւ̑}��
	if(first != insertPtr)																			//�}���v�f�̃|�C���^��first�łȂ���΃G���[����
		std::cout << "addFirst error" << std::endl, exit(1);
	return insertPtr;																					//�}���|�C���^��Ԃ�
}

//���X�g�̍Ō���ւ̗v�f�ǉ�
//�����i�ǉ��Ώۂ̃I�u�W�F�N�g�|�C���^�j
//�߂�l�i�ǉ��v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::addLast(T* objectPtr){
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//�}���v�f�̃��X�g�|�C���^�쐬
	if(last == NULL)																					//last���Ȃ����
		insert(insertPtr);																				//�P�Ȃ�C�x���g�̑}�������Ɠ���
	else																									//last�����݂���ꍇ
		insertRight(last, insertPtr);																	//last�̉E�̎q�ւ̑}��
	if(last != insertPtr)																			//�}���v�f�̃|�C���^��last�łȂ���΃G���[����
		std::cout << "addLast error" << std::endl, exit(1);
	return insertPtr;																					//�}���|�C���^��Ԃ�
}

//���X�g����̗v�f�̍폜�i�v�f�f�[�^���͍̂폜���Ȃ��j
//�����i�폜�Ώۗv�f�̃��X�g�|�C���^�j
//�߂�l�i�폜�Ώۗv�f�̃��X�g�|�C���^�j
template <class T> LINK<T>* LIST<T>::remove(LINK<T>* thisPtr){
	if(thisPtr == NULL)																				//�폜�Ώۂ����݂��Ȃ����
		return NULL;																						//�폜�Ȃ����Ӗ�����NULL��Ԃ�
	LINK<T>* lPtr = thisPtr->leftPtr;															//�폜�Ώۂ̍��q�̃��X�g�|�C���^
	LINK<T>* rPtr = thisPtr->rightPtr;															//�폜�Ώۂ̉E�q�̃��X�g�|�C���^
	LINK<T>* pPtr = thisPtr->prevPtr;															//�폜�Ώۂ̐e�̃��X�g�|�C���^
	LINK<T>* ptr = NULL;																				//�e�폈���Ɏg�p���郊�X�g�|�C���^�ϐ�
	if(lPtr != NULL){																					//�폜�Ώۂɍ��q�����݂���ꍇ
		ptr = lPtr;																							//�e�̐V�����q�͍��q
		if(rPtr != NULL){																					//�E�q�����݂���ꍇ�ɂ͉E�q�̕t���ւ����K�v
			while(lPtr->rightPtr != NULL)																	//���q����o�����E�q�����݂��Ȃ��Ȃ�܂�
				lPtr = lPtr->rightPtr;																		//�E�փ��X�g���ړ�
			lPtr->rightPtr = rPtr;																			//�E�[�֓��B�����炻�̉E�q�ɍ폜�Ώۂ̉E�q��t���ւ���
			rPtr->prevPtr = lPtr;																			//���R�Ȃ���t���ւ���ꂽ�E�q�̐e�͈ړ������E�[�ł���
		}																										//�E�q�����݂��Ȃ���Εt���ւ��̕K�v�͂Ȃ�
	}
	else{																									//�폜�Ώۂɍ��q�����݂��Ȃ��ꍇ
		if(rPtr != NULL)																					//�E�q�͑��݂���Ƃ����ꍇ�ɂ�
			ptr = rPtr;																							//�e�̐V�����q�͉E�q
		else																									//�E�q�����݂��Ȃ��ꍇ�ɂ�
			ptr = NULL;																							//�e�̐V�����q�͖���
	}
	if(pPtr == NULL){																					//�e�����Ȃ��ꍇ
		if(ptr != NULL)																					//�V�����q�����݂���Ȃ�
			ptr->prevPtr = NULL;																				//�������e�ƂȂ�
		top = ptr;																								//���R������top�i�q�����Ȃ����top��NULL�j
	}
	else{																									//�e������ꍇ	
		if(ptr != NULL)																					//�V�����q�����݂���Ȃ�
			ptr->prevPtr = pPtr;																				//���̎q�̐e�͍폜�Ώۂ̐e							
		if(pPtr->leftPtr == thisPtr)																	//�폜�Ώۂ��e�̍��q�Ȃ��
			pPtr->leftPtr = ptr;																				//���̎q���e�̍��q�ɂȂ�
		else																									//�폜�Ώۂ��e�̉E�q�Ȃ��
			pPtr->rightPtr = ptr;																			//���̎q���e�̉E�q�ɂȂ�
	}
	if(top != NULL){																					//�g�b�v�����݂���Ȃ�						
		ptr = top;																					
		while(ptr->leftPtr != NULL)																	//�g�b�v����Ђ����獶�ւ��ǂ�
			ptr = ptr->leftPtr;
		first = ptr;																						//�s�������Ƃ��낪�擪
		ptr = top;
		while(ptr->rightPtr != NULL)																	//�g�b�v����Ђ�����E�ւ��ǂ�
			ptr = ptr->rightPtr;											
		last = ptr;																							//�s�������Ƃ��낪�Ō��
	}
	else																									//�g�b�v�����݂��Ȃ��Ȃ�
		first = last = NULL;																				//first��last�����݂��Ȃ�			
	thisPtr->leftPtr = thisPtr->rightPtr = thisPtr->prevPtr = NULL;					//�폜�Ώۂ̐e�v�f�f�[�^��q�v�f�f�[�^������
	return thisPtr;
}

//���X�g����̗v�f�̍폜�i�v�f�f�[�^���͍̂폜���Ȃ��j
//�����i�폜�Ώۗv�f�|�C���^�j
//�߂�l�i�폜�Ώۗv�f�̃��X�g�|�C���^�j
template <class T> LINK<T>* LIST<T>::remove(T* objectPtr){
	return remove(find(objectPtr));																			//�擪�|�C���^���w�肵��remove�֐����Ă�
}

//���X�g����̐擪�v�f�̍폜�i�v�f�f�[�^���͍̂폜���Ȃ��j
//�����i�Ȃ��j
//�߂�l�i�폜�Ώۗv�f�̃��X�g�|�C���^�j
template <class T> LINK<T>* LIST<T>::remove(void){
	return remove(first);																			//�擪�|�C���^���w�肵��remove�֐����Ă�
}
//���X�g�̗v�f����
//�����i�����Ώۗv�f�̃��X�g�|�C���^�C�����p�̃I�u�W�F�N�g�|�C���^�j
//�߂�l�i�폜�Ώۗv�f�̃��X�g�|�C���^�j
template <class T> LINK<T>* LIST<T>::find(LINK<T>* herePtr, T* objectPtr){
	if(first == NULL)																					//�擪�����݂��Ȃ��Ȃ�
		return NULL;																						//�������s���Ӗ�����NULL��Ԃ�
	if(herePtr->getObject() == objectPtr)														//�����Ώۂ��Y������Ȃ�
		return herePtr;																					//���̗v�f�|�C���^��Ԃ�
	if(herePtr->leftPtr != NULL){																	//�����Ώۂ��Y�������E�q�����݂���Ȃ�
		LINK<T>* ptr = find(herePtr->leftPtr, objectPtr);											//�E�q�ɑ΂��Č����v������
		if(ptr != NULL)																					//�E�q�ȍ~�Ńq�b�g������
			return ptr;																							//���̗v�f�|�C���^��Ԃ�
	}	
	if(herePtr->rightPtr == NULL)																	//���q�����݂��Ȃ��ꍇ
		return NULL;																						//�����ȍ~�ɂ̓q�b�g����v�f�͂Ȃ�		
	LINK<T>* ptr = find(herePtr->rightPtr, objectPtr);											//���q�����݂���Ȃ炻��ȍ~�ɑ΂��Č�����
	return ptr;																							//���̌��ʂ�Ԃ�
}

//���X�g�̗v�f����
//�����i�����p�̃C�x���g�|�C���^�j
//�߂�l�i�폜�Ώۗv�f�̃��X�g�|�C���^�j
template <class T> LINK<T>* LIST<T>::find(T* objectPtr){
	return find(top, objectPtr);																	//�������I�u�W�F�N�g�����Ȃ�g�b�v���猟��
}

//�C�x���g���X�g�ւ̑}��
//�����i���X�g�v�f�|�C���^�j
//�߂�l�F�i�}���������X�g�v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::insert(LINK<T>* thisPtr){
	LINK<T>* herePtr = top;																			//�}������̔�r�Ώہi�ŏ���top�|�C���^�j
	LINK<T>* prevPtr;
	bool flag;																							//���E�̂ǂ���̎q�ɑ}�����邩�������t���O
	while(herePtr != NULL){																			//��r�Ώۂ�������菈���𔽕�
		prevPtr = herePtr;																			//���݂̑Ώۂ͑}���C�x���g�̐e�ɂȂ肤��
		if(timeCompare(herePtr->getObject()->getEventTime(), thisPtr->getObject()->getEventTime())){
			herePtr = herePtr->leftPtr;															//�}���C�x���g��������������Ύ��ɍ��q�Ɣ�r
			flag = true;																				//�}���͍��q�ɍs���̂Ńt���O�͐^
		}
		else{
			herePtr = herePtr->rightPtr;															//�}���C�x���g�������傫����Ύ��ɉE�q�Ɣ�r
			flag = false;																				//�}���͉E�̎q�ɍs���̂Ńt���O�͋U
		}
	}
	if(top == NULL){																					//����top�����݂��Ȃ���Α}���C�x���g��top
		top = first = last = thisPtr;
		return thisPtr;
	}
	else{																									//top�����݂���Ȃ�
		if(flag == true)																					//�t���O���^�Ȃ�
			return insertLeft(prevPtr, thisPtr);														//���q�ő}������
		else																									//�t���O���U�Ȃ�
			return insertRight(prevPtr, thisPtr);														//�E�q�ő}������
	}
}

//�C�x���g���X�g�ւ̑}��
//�����i�I�u�W�F�N�g�|�C���^�j
//�߂�l�F�i�}���������X�g�v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::insert(T* objectPtr){
	if(objectPtr == NULL)																			//�I�u�W�F�N�g���Ȃ���Ή������Ȃ�
		return NULL;
	LINK<T>* insertPtr = new LINK<T>(objectPtr);												//�C�x���g�ɑΉ����郊�X�g�v�f�̍쐬
	return insert(insertPtr);																		//�v�f�|�C���^�ɂ��}�������֐����Ăяo��
}

//�C�x���g���X�g�ւ̍Ĕz�u
//�����i�I�u�W�F�N�g�|�C���^�j
//�߂�l�F�i�}���������X�g�v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::replace(T* objectPtr){
	if(objectPtr == NULL)																			//�I�u�W�F�N�g���Ȃ���Ή������Ȃ�
		return NULL;
	LINK<T>* findPtr = find(objectPtr);
	if(findPtr){
		LINK<T>* insertPtr = remove(findPtr);
		return insert(insertPtr);																		//�v�f�|�C���^�ɂ��}�������֐����Ăяo��
	}
	return insert(objectPtr);
}

//�v�f�̏����i�C�x���g���̂͏�������Ȃ��j
//�����i�폜�Ώۂ̃C�x���g�|�C���^�j
//�߂�l�F�i�Ȃ��j
template <class T> LINK<T>* LIST<T>::deleteEvent(T* objectPtr){
	LINK<T>* listPtr = find(objectPtr);															//�Y���I�u�W�F�N�g�����v�f������
	remove(listPtr);																					//�q�b�g�����v�f�����X�g����폜
	delete listPtr;																					//�v�f�f�[�^������
	return listPtr;
}

//���X�g�̏����i�C�x���g�������j
//�����i�����Ώۂ̃��X�g�v�f�|�C���^�j
//�߂�l�F�i�Ȃ��j
template <class T> void LIST<T>::deleteList(LINK<T>* ptr){
	if(ptr == NULL)																					//�����Ώۂ����݂��Ȃ���Ή������Ȃ�
		return;	
	deleteList(ptr->leftPtr);																		//�����̍��q�ȍ~������
	deleteList(ptr->rightPtr);																		//�����̉E�q�ȍ~������
	delete ptr;																							//�v�f���g������
}

//���X�g�̏����i�C�x���g�������j
//�����i�Ȃ��j
//�߂�l�F�i�Ȃ��j
template <class T> void LIST<T>::deleteList(void){
	deleteList(top);																					//�������Ȃ��ꍇ�ɂ�top��������J�n
}

//���X�g�̎��̗v�f���擾
//�����i�Ώۃ��X�g�v�f�|�C���^�j
//�߂�l�F�i�擾�v�f�|�C���^�j
template <class T> LINK<T>* LIST<T>::getNEXT(LINK<T>* ptr){
	if(ptr == last)																					//�Ώۂ�last�ł���Ύ��̗v�f�͑��݂��Ȃ�
		return NULL;
	if(ptr->getright() == NULL){																		//�E�q�����Ȃ���Ύ��������q�Ɏ��e���K�����̗v�f
		LINK<T>* nextPtr = ptr->getPrev();
		if(nextPtr->getleft() == ptr)
			return nextPtr;
		while(nextPtr->getright() == ptr){
			ptr = nextPtr;
			nextPtr = nextPtr->getPrev();
		}
		return nextPtr;
	}
	LINK<T>* nextPtr = ptr->getright();																//�E�q������Ȃ��
	while(nextPtr->getleft() != NULL)																//�E�q���獶�֒H�葱����
		nextPtr = nextPtr->getleft();																	//�E�q����݂čč��q�����̗v�f
	return nextPtr;
}

//���X�g�\��
//�����i�\���Ώۂ̗v�f�|�C���^�j
//�߂�l�F�i�擾�v�f�|�C���^�j
template <class T> void LIST<T>::show(LINK<T>* ptr){
	if(ptr == NULL)																					//�Ώۂ��Ȃ���Ή������Ȃ�
		return;
	cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
		<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
		<< "-->";
	ptr->getObject()->getEventTime().showMuSec();
	cout << "\t";
	ptr->getObject()->show();																		//�Ώۗv�f�̃C�x���g�\��
	if(ptr->leftPtr != NULL)																		//���q������ꍇ
		show(ptr->leftPtr);																				//���q�̃��X�g�\��
	if(ptr->rightPtr != NULL)																		//�E�q������ꍇ
	show(ptr->rightPtr);																					//�E�q�̃��X�g�\��
}

//���X�g�\��
//�����i�\���Ώۂ̗v�f�|�C���^�j
//�߂�l�F�i�擾�v�f�|�C���^�j
template <class T> void LIST<T>::show(void){
	cout << "first " << first << endl;
	show(top);																							//�Ώۂ̎w�肪�Ȃ����top��\��
	cout << endl;																						//���s
}

template <class T> void LIST<T>::orderShow(LINK<T>* ptr){
	if(ptr->leftPtr == NULL){
		cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
			<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
			<< "-->";
		ptr->getObject()->getEventTime().showMuSec();
		cout << "\t";
		ptr->getObject()->show();																	//�Ώۗv�f�̃C�x���g�\��
		if(ptr->rightPtr != NULL)
			orderShow(ptr->rightPtr);																//�E�q�̃��X�g�\��
		else
			return;
	}
	else{
		orderShow(ptr->leftPtr);																	//�E�q�̃��X�g�\��
		cout << "p" << ptr << "-" << ptr->prevPtr << "-" 
			<< ptr->leftPtr << "-" << ptr->rightPtr << "--> " << ptr->getObject() 
			<< "-->";
		ptr->getObject()->getEventTime().showMuSec();
		cout << "\t";
		ptr->getObject()->show();																	//�Ώۗv�f�̃C�x���g�\��
		if(ptr->rightPtr != NULL)
			orderShow(ptr->rightPtr);																//�E�q�̃��X�g�\��			
	}
}

template <class T> void LIST<T>::orderShow(void){
	orderShow(top);																					//�Ώۂ̎w�肪�Ȃ����top��\��
	cout << endl;																						//���s
}
