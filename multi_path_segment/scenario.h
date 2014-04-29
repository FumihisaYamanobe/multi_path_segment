
#define TIMELIMIT 100																				//�V�~�����[�V�����I�������i�b�j
//#define TIMELIMIT 2000																			//�V�~�����[�V�����I������
#define AREA     1000 * 100																		//�V�~�����[�V�����G���A�̔��a�i�~�`�j��������1�Ӂi�����`�j�i�����j
#define RANGE    100 * 100																			//�ʐM�\�����i�����j
#define GRIDNUM 10																					//�i�q�G���A���f���̏ꍇ�̊i�q��
#define PAUSE_SEC 1																					//�ړI�n�������̒�~���ԁi�b�P�ʂ̒l�j
#define PAUSE_MSEC 0																					//�ړI�n�������̒�~���ԁi�~���b�P�ʂ̒l�j

#define NODENUM 500

//#define RX_THRESHOLD 1.585e-9																	//��M�\�d��(-88dBm)
#define RX_THRESHOLD 6.310e-8																		//��M�d�̓��x���ig, 54MB, -72dBm)
//#define CS_THRESHOLD 5.012e-11																	//�L�����A�Z���X�\��M�d��(-103dBm)
#define CS_THRESHOLD 3.981e-9																		//�L�����A�Z���X�\��M�d��(-84dBm)
#define CP_THRESHOLD 10.0																			//��M���ʓd�͔��臒l�i10�{�j


//#define TX_POWER  0.28183815																		//���M�d��

//#define TX_POWER  1.246e-0																		//���M�d��

//#define TX_POWER  3.162e-2																		//���M�d��
	
#define FREQUENCY 2.472e9																				//���g��
//#define FREQUENCY 914e6

#define TCPSIZE 1024																					//TCP�p�P�b�g�T�C�Y
#define TCPDEFAULTSIZE 1024																				//TCP�Z�O�����g�����T�C�Y
#define TCPRESETSIZE 50																					//TCP�Z�O�����g�̍Đݒ莞�̈������葫�����肷��T�C�Y
#define UDPSIZE 1024																					//UDP�p�P�b�g�T�C�Y
#define TCPACKSIZE 40																				//ACK�p�P�b�g�T�C�Y

#define MAXHOP 100

#define RREQDSR 24
#define RREPDSR 20
#define RERRDSR 24

#define RREQAODV 24
#define RREPAODV 20
#define RERRAODV 24

#define LABPACKET 24																					//LAB�p�P�b�g�̃f�t�H���g�T�C�Y
#define REQ 24																							//�e�탊�N�G�X�g�p�P�b�g�̃f�t�H���g�T�C�Y
#define REP 24																							//�e�탊�v���C�p�P�b�g�̃f�t�H���g�T�C�Y

#define INFORM 28																						//�f�t�H���g20�o�C�g�Ɉʒu���4�o�C�g�Ǝ��ԏ��4�o�C�g

#define REQUEST_INT 1000 * 1000																	//���[�g���N�G�X�g�E���[�g�G���[�p�P�b�g�̍ŒZ���M�C���^�[�o���i�ʂ��j
#define TTLTIME 20 * 1000000																		//�p�P�b�g�̐������ԏ���i�ʂ��j

#define RTT_ALPHA 0.875																				//RTT�����p�W��(TCP�W���j

#define SEND_POWER 530.0																			//���M�d�́imW�jDC2J1DZ150
#define RECEIVE_POWER 326.0																		//��M�d�́imW�jDC2J1DZ150
#define WAIT_POWER 0.4																				//�ҋ@�d�́imW�j
#define SLEEP_POWER 0.02																			//�X���[�v������d�́imW�j

#define LOCERR 0 * 100																				//����ʒu�덷�i�}�����j	

#define INITTIME 30 * 60																			//�f�[�^����J�n����

#define BUFFERSIZE 1000 * 1000																	//�[���̃o�b�t�@�T�C�Y�i�P�ʁF�o�C�g���j
#define DATASIZE 1024																				//�f�[�^�T�C�Y�i�o�C�g�j
#define RETRANS_MAX 7																				//�đ����

#define PHYSPEED 54																					//�����w�̓`�����x�iMbps�j
#define DELAY 1																						//�`���x���i�ʂ��j
#define SLOTTIME 20																					//�X���b�g���ԁi�ʂ��j
#define DIFSTIME 56
#define SIFSTIME 16																					//�r�h�e�r���ԁi�ʂ��j
#define RTSSIZE 20																					//�q�s�r�T�C�Y�i�o�C�g�j
#define CTSSIZE 14																					//�b�s�r�T�C�Y�i�o�C�g�j
#define ACKSIZE 14																					//�`�b�j�T�C�Y�i�o�C�g�j
#define BEACONINT 1000000																			//�r�[�R�������i�ʕb�j
//#define BROADINTERVAL 1000

#define ORACK_RANGE 100																				//ORACK��Ԃ����߂̑ҋ@���Ԕ͈�

#define ROUTEREPMASIZE 48
