//apptester�Ĺ����ļ�
#include <iostream>
#include <conio.h>
#include <random>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"
using namespace std;

U8* autoSendBuf;        //������֯�������ݵĻ��棬��СΪMAX_BUFFER_SIZE,���������������������ƣ��γ��ʺϵĽṹ��������û��ʹ�ã�ֻ������һ��
int printCount = 0; //��ӡ����
int spin = 1;  //��ӡ��̬��Ϣ����

//------�����ķָ��ߣ�һЩͳ���õ�ȫ�ֱ���------------
int iSndTotal = 0;  //������������
int iSndTotalCount = 0; //���������ܴ���
int iSndErrorCount = 0;  //���ʹ������
int iRcvTotal = 0;     //������������
int iRcvTotalCount = 0; //ת�������ܴ���
int iRcvUnknownCount = 0;  //�յ�������Դ�����ܴ���
//------------------------------------------------
int global_range = 10000;
int output_x; // ����������ɵ������x
int output_y; // ������ģ�µĶ������ɵ����������֮�������y
int external_random = 0; // ȫ�ֱ������洢���յ��������

void generateTrueRandom() {
	std::random_device rd; // �������Դ
	std::uniform_int_distribution<int> dist(0, global_range); // ���ȷֲ�
	output_x = dist(rd);
}

void print_statistics();
void menu();
//***************��Ҫ��������******************************
//���ƣ�InitFunction
//���ܣ���ʼ�������棬��main�����ڶ��������ļ�����ʽ������������ǰ����
//���룺
//�����
void InitFunction(CCfgFileParms& cfgParms)
{
	int i;
	int retval;
	
	retval = cfgParms.getValueInt(autoSendTime, (char*)"autoSendTime");
	if (retval == -1 || autoSendTime == 0) {
		autoSendTime = DEFAULT_AUTO_SEND_TIME;
	}
	retval = cfgParms.getValueInt(autoSendSize, (char*)"autoSendSize");
	if (retval == -1 || autoSendSize == 0) {
		autoSendSize = DEFAULT_AUTO_SEND_SIZE;
	}

	autoSendBuf = (char*)malloc(MAX_BUFFER_SIZE);
	if (autoSendBuf == NULL) {
		cout << "�ڴ治��" << endl;
		//����������Ҳ̫���˳���
		exit(0);
	}
	for (i = 0; i < MAX_BUFFER_SIZE; i++) {
		autoSendBuf[i] = 'a'; //��ʼ������ȫΪ�ַ�'a',ֻ��Ϊ�˲���
	}
	return;
}
//***************��Ҫ��������******************************
//���ƣ�EndFunction
//���ܣ����������棬��main�������յ�exit������������˳�ǰ����
//���룺
//�����
void EndFunction()
{
	if (autoSendBuf != NULL)
		free(autoSendBuf);
	return;
}

//***************��Ҫ��������******************************
//���ƣ�TimeOut
//���ܣ�������������ʱ����ζ��sBasicTimer�����õĳ�ʱʱ�䵽�ˣ�
//      �������ݿ���ȫ���滻Ϊ������Լ����뷨
//      ������ʵ���˼���ͬʱ���й��ܣ����ο�
//      1)����iWorkMode����ģʽ���ж��Ƿ񽫼�����������ݷ��ͣ������Զ����͡������������ʵ��Ӧ�ò��
//        ��Ϊscanf�����������¼�ʱ���ڵȴ����̵�ʱ����ȫʧЧ������ʹ��_kbhit()������������ϵ��ڼ�ʱ�Ŀ������жϼ���״̬�������Get��û��
//      2)����ˢ�´�ӡ����ͳ��ֵ��ͨ����ӡ���Ʒ��Ŀ��ƣ�����ʼ�ձ�����ͬһ�д�ӡ��Get��
//      3)�����iWorkMode����Ϊ�Զ����ͣ���ÿ����autoSendTime * DEFAULT_TIMER_INTERVAL ms����ӿ�0����һ��
//���룺ʱ�䵽�˾ʹ�����ֻ��ͨ��ȫ�ֱ�����������
//���������Ǹ�����Ŭ���ɻ����ʵ����
void TimeOut()
{
	int iSndRetval;
	int len;
	U8* bufSend;
	int i;

	printCount++;
	if (_kbhit()) {
		//�����ж���������˵�ģʽ
		menu();
	}
	switch (iWorkMode / 10) {
	case 0:
		break;
	case 1:
		//��ʱ����, ÿ���autoSendTime * DEFAULT_TIMER_INTERVAL ms ����һ��
		if (printCount % autoSendTime == 0) {
			for (i = 0; i < min(autoSendSize, 8); i++) {
				//��ͷ�����ֽ���26����ĸ�м����������ڹ۲�
				autoSendBuf[i] = 'a' + printCount % 26;
			}

			len = autoSendSize; //ÿ�η�������
			if (lowerMode[0] == 0) {
				//�Զ�����ģʽ�£�ֻ��ӿ�0����
				bufSend = (U8*)malloc(len * 8);
				//�²�ӿ��Ǳ���������
				iSndRetval = ByteArrayToBitArray(bufSend, len * 8, autoSendBuf, len);
				iSndRetval = SendtoLower(bufSend, iSndRetval, 0);

				free(bufSend);
			}
			else {
				//�²�ӿ����ֽ����飬ֱ�ӷ���
				for (i = 0; i < min(autoSendSize, 8); i++) {
					//��ͷ�����ֽ���26����ĸ�м����������ڹ۲�
					autoSendBuf[i] = 'a' + printCount % 26;
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; //�����λ
			}
			//����ͳ��
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}
			//��Ҫ��Ҫ��ӡ����
			switch (iWorkMode % 10) {
			case 1:
				print_data_bit(autoSendBuf, len, 1);
				break;
			case 2:
				print_data_byte(autoSendBuf, len, 1);
				break;
			case 0:
				break;
			}
		}
		break;
	case 2:
		// ��ʱ�������������λ����ʽ����ÿ�� autoSendTime * DEFAULT_TIMER_INTERVAL ms ����һ��
		if (printCount % autoSendTime == 0) {
			// �����µ������
			generateTrueRandom();
			printf("���ɵ������x��%d\n", output_x);
			if (lowerMode[0] == 0) {
				// λ��ģʽ���������ת��Ϊ������λ��
				const int bit_len = 32; // ���� int �� 32 λ
				bufSend = (U8*)malloc(bit_len); // ���� 32 λ�ռ�
				if (bufSend == NULL) {
					iSndErrorCount++; // ����ʧ�ܣ���¼����
					break;
				}

				// ֱ�ӽ� output_x ת��Ϊλ�����Ӹ�λ����λ��
				for (i = 0; i < bit_len; i++) {
					bufSend[i] = (output_x >> (bit_len - 1 - i)) & 1; // ��ȡÿһλ
				}

				// ����λ�����ӿ�0
				iSndRetval = SendtoLower(bufSend, bit_len, 0);

				free(bufSend); // �ͷ���ʱ������
			}
			else {
				// �ֽ���ģʽ������������ֽڷ��ͣ�����ԭʼ�߼���
				len = 4; // �̶�Ϊ 4 �ֽڣ�32 λ��
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (output_x >> (i * 8)) & 0xFF; // ���ֽ���ȡ
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; // �����λ
			}

			// ����ͳ��
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}

			// ���� iWorkMode % 10 �����Ƿ��ӡ
			switch (iWorkMode % 10) {
			case 1:
				// ��ӡλ����ʹ�� autoSendBuf ģ��λ����ӡ��
				len = 4; // ��ʱ�����ֽ������Դ�ӡ
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (output_x >> (i * 8)) & 0xFF;
				}
				print_data_bit(autoSendBuf, len, 1);
				break;
			case 2:
				// ��ӡ�ֽ���
				len = 4;
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (output_x >> (i * 8)) & 0xFF;
				}
				print_data_byte(autoSendBuf, len, 1);
				break;
			case 0:
				break;
			}
		}
		break;
	case 3:
		// ��ʱ���ͣ����������������յ��������ӣ���ӡ�����������������ӽ������λ����ʽ��
		if (printCount % autoSendTime == 0) {
			// ���ɱ��������
			generateTrueRandom();
			// ʹ�ô� RecvfromLower ���յ� external_random
			int sum_result = output_y + external_random; // ���



			// ��ӡ���������
			printf("Generated local random number: %d\n", output_y);

			if (lowerMode[0] == 0) {
				// λ��ģʽ������ӽ��ת��Ϊ������λ��
				const int bit_len = 32; // ���� int �� 32 λ
				bufSend = (U8*)malloc(bit_len); // ���� 32 λ�ռ�
				if (bufSend == NULL) {
					iSndErrorCount++; // ����ʧ�ܣ���¼����
					break;
				}

				// ֱ�ӽ� sum_result ת��Ϊλ�����Ӹ�λ����λ��
				for (i = 0; i < bit_len; i++) {
					bufSend[i] = (sum_result >> (bit_len - 1 - i)) & 1; // ��ȡÿһλ
				}

				// ����λ�����ӿ�0
				iSndRetval = SendtoLower(bufSend, bit_len, 0);

				free(bufSend); // �ͷ���ʱ������
			}
			else {
				// �ֽ���ģʽ������ӽ�����ֽڷ���
				len = 4; // �̶�Ϊ 4 �ֽڣ�32 λ��
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (sum_result >> (i * 8)) & 0xFF; // ���ֽ���ȡ
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; // �����λ
			}

			// ����ͳ��
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}

			// ���� iWorkMode % 10 �����Ƿ��ӡ
			switch (iWorkMode % 10) {
			case 1:
				// ��ӡλ����ʹ�� autoSendBuf ģ��λ����ӡ��
				len = 4; // ��ʱ�����ֽ������Դ�ӡ
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (sum_result >> (i * 8)) & 0xFF;
				}
				print_data_bit(autoSendBuf, len, 1);
				break;
			case 2:
				// ��ӡ�ֽ���
				len = 4;
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (sum_result >> (i * 8)) & 0xFF;
				}
				print_data_byte(autoSendBuf, len, 1);
				break;
			case 0:
				break;
			}
		}
		break;
	}
	//���ڴ�ӡͳ������
	print_statistics();

}
//------------�����ķָ��ߣ����������ݵ��շ�,--------------------------------------------

//***************��Ҫ��������******************************
//���ƣ�RecvfromUpper
//���ܣ�������������ʱ����ζ���յ�һ�ݸ߲��·�������
//      ��������ȫ�������滻��������Լ���
//      ���̹��ܽ���
//         1)ͨ���Ͳ�����ݸ�ʽ����lowerMode���ж�Ҫ��Ҫ������ת����bit�����鷢�ͣ�����ֻ�����Ͳ�ӿ�0��
//           ��Ϊû���κοɹ��ο��Ĳ��ԣ���������Ӧ�ø���Ŀ�ĵ�ַ�ڶ���ӿ���ѡ��ת���ġ�
//         2)�ж�iWorkMode�������ǲ�����Ҫ�����͵��������ݶ���ӡ������ʱ���ԣ���ʽ����ʱ�����齫����ȫ����ӡ��
//���룺U8 * buf,�߲㴫���������ݣ� int len�����ݳ��ȣ���λ�ֽ�
//�����
void RecvfromUpper(U8* buf, int len)
{
	//Ӧ�ò㲻���յ����߲㡱�����ݣ������Լ�����
}
//***************��Ҫ��������******************************
//���ƣ�RecvfromLower
//���ܣ�������������ʱ����ζ�ŵõ�һ�ݴӵͲ�ʵ��ݽ�����������
//      ��������ȫ�������滻���������Ҫ������
//      ���̹��ܽ��ܣ�
//          1)����ʵ����һ���򵥴ֱ���������Ĳ��ԣ����дӽӿ�0�����������ݶ�ֱ��ת�����ӿ�1�����ӿ�1�������Ͻ����߲㣬������ô����
//          2)ת�����Ͻ�ǰ���ж��ս����ĸ�ʽ��Ҫ���ͳ�ȥ�ĸ�ʽ�Ƿ���ͬ��������bite��������ֽ�������֮��ʵ��ת��
//            ע����Щ�жϲ������������ݱ�����������������������ļ������������ļ��Ĳ���д���ˣ��ж�Ҳ�ͻ�ʧ��
//          3)����iWorkMode���ж��Ƿ���Ҫ���������ݴ�ӡ
//���룺U8 * buf,�Ͳ�ݽ����������ݣ� int len�����ݳ��ȣ���λ�ֽڣ�int ifNo ���Ͳ�ʵ����룬�����������ĸ��Ͳ�
//�����
// ���˼�����£�
//1.�ж�����һ���������ݣ����ܵ������������㣬������·�㣬����㣩
//2.1 ��������������ݣ���Ӧ���ж���bit�����黹��byte��������Ȼ����з�װ��ȷ���Ƿ������������ݣ����ת�����ϲ�����
//2.2 �����������·���������ݣ���Ҫʹ�ò���csma/cd Э�����ж� ��Ȼ���ǻ᲻��̫���ˣ�Ӱ������ת�������û�о�����ת������
//2.3 �����������������ݣ����ж��Ƿ�ӵ����Ȼ����·��Э����������������ɣ�����������
// �����Ǵ��µ��ϣ����Ӧ�ľ����ϵ��£�˼·����������ʹ����ͬ���ֶν�����ݾͺ��� easy


void RecvfromLower(U8* buf, int len, int ifNo)
{
	int retval;
	U8* bufRecv = NULL;

	if (lowerMode[ifNo] == 0) {
		// �Ͳ��� bit �������ʽ����Ҫת��
		bufRecv = (U8*)malloc(len / 8 + 1);
		if (bufRecv == NULL) {
			printf("û�гɹ����µ���ʱ���ݷ����ڴ�!");
			return;
		}

		// ת��Ϊ�ֽ�����
		retval = BitArrayToByteArray(buf, len, bufRecv, len / 8 + 1);

		// ���������������ǰ 4 �ֽ��� 32 λ������
		if (len >= 32) { // ȷ��λ���㹻����32 λ = 4 �ֽڣ�
			external_random = (bufRecv[0] << 24) | (bufRecv[1] << 16) | (bufRecv[2] << 8) | bufRecv[3]; // �����
		}
		else {
			external_random = 0; // ���ݲ��㣬��Ϊ 0
		}
	}
	else {
		// �ֽ���ģʽ
		retval = len * 8; // �����λ������ͳ��

		// ���������������ǰ 4 �ֽ��� 32 λ������
		if (len >= 4) { // ȷ���ֽ����㹻��
			external_random = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]; // �����
		}
		else {
			external_random = 0; // ���ݲ��㣬��Ϊ 0
		}
	}
	iRcvTotal += retval;
	iRcvTotalCount++;

	if (bufRecv != NULL) {
		free(bufRecv);
	}

	// ��ӡ
	switch (iWorkMode % 10) {
	case 1:
		cout << endl << "���սӿ� " << ifNo << " ���ݣ�" << endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "���սӿ� " << ifNo << " ���ݣ�" << endl;
		print_data_byte(buf, len, lowerMode[ifNo]);
		break;
	case 0:
		break;
	}
}
/*
void RecvfromLower(U8* buf, int len, int ifNo)
{
	int retval;
	U8* bufRecv = NULL;

	if (lowerMode[ifNo] == 0) {
		//�Ͳ���bit�������ʽ����Ҫת�����ŷ����ӡ
		bufRecv = (U8*)malloc(len / 8 + 1);
		if (bufRecv == NULL) {
			printf("û�гɹ����µ���ʱ���ݷ����ڴ�!");
			return;
		}

		//����ӿ�0�Ǳ��������ʽ����ת�����ֽ����飬�����ϵݽ�
		retval = BitArrayToByteArray(buf, len, bufRecv, len / 8 + 1);
	}
	else {
		retval = len * 8;//�����λ,����ͳ��
	}
	iRcvTotal += retval;
	iRcvTotalCount++;


	if (bufRecv != NULL) {
		free(bufRecv);
	}

	//��ӡ
	switch (iWorkMode % 10) {
	case 1:
		cout <<endl<< "���սӿ� " <<ifNo <<" ���ݣ�"<<endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "���սӿ� " << ifNo << " ���ݣ�" << endl;
		print_data_byte(buf, len, lowerMode[ifNo]);
		break;
	case 0:
		break;
	}
}
*/
//��ӡͳ����Ϣ
void print_statistics()
{
	if (printCount % 10 == 0) {
		switch (spin) {
		case 1:
			printf("\r-");
			break;
		case 2:
			printf("\r\\");
			break;
		case 3:
			printf("\r|");
			break;
		case 4:
			printf("\r/");
			spin = 0;
			break;
		}
		cout << "������ " << iSndTotal << " λ," << iSndTotalCount << " ��," << "���� " << iSndErrorCount << " �δ���;";
		cout << " ������ " << iRcvTotal << " λ," << iRcvTotalCount << " ��" ;
		spin++;
	}
}
//PrintParms ��ӡ����������ע�ⲻ��cfgFilms�������ģ�����Ŀǰ��Ч�Ĳ���
void PrintParms()
{
	size_t i;
	cout << "�豸��: " << strDevID << " ���: " << strLayer << "ʵ��: " << strEntity << endl;
	cout << "�ϲ�ʵ���ַ: " << inet_ntoa(upper_addr.sin_addr) << "  UDP�˿ں�: " << ntohs(upper_addr.sin_port) << endl;

	cout << "����ʵ���ַ: " << inet_ntoa(local_addr.sin_addr) << "  UDP�˿ں�: " << ntohs(local_addr.sin_port) << endl;
	if (strLayer.compare("PHY") == 0) {
		if (lowerNumber <= 1) {
			cout << "�²�㵽���ŵ�" << endl;
			cout << "��·�Զ˵�ַ: ";
		}
		else {
			cout << "�²�㲥ʽ�ŵ�" << endl;
			cout << "�����ŵ�վ�㣺";
		}
	}
	else {
		cout << "�²�ʵ��";
	}
	if (lowerNumber == 1) {
		cout << "��ַ��" << inet_ntoa(lower_addr[0].sin_addr) << "  UDP�˿ں�: " << ntohs(lower_addr[0].sin_port) << endl;
	}
	else {
		if (strLayer.compare("PHY") == 0) {
			cout << endl;
			for (i = 0; i < lowerNumber; i++) {
				cout << "        ��ַ��" << inet_ntoa(lower_addr[i].sin_addr) << "  UDP�˿ں�: " << ntohs(lower_addr[i].sin_port) << endl;
			}
		}
		else {
			cout << endl;
			for (i = 0; i < lowerNumber; i++) {
				cout << "        �ӿ�: [" << i << "] ��ַ" << inet_ntoa(lower_addr[i].sin_addr) << "  UDP�˿ں�: " << ntohs(lower_addr[i].sin_port) << endl;
			}
		}
	}
	string strTmp;
	//strTmp = getValueStr("cmdIpAddr");
	cout << "ͳһ����ƽ̨��ַ: " << inet_ntoa(cmd_addr.sin_addr);
	//strTmp = getValueStr("cmdPort");
	cout << "  UDP�˿ں�: " << ntohs(cmd_addr.sin_port) << endl;
	//strTmp = getValueStr("oneTouchAddr");
	cout << "oneTouchһ��������ַ: " << inet_ntoa(oneTouch_addr.sin_addr);
	//strTmp = getValueStr("oneTouchPort");
	cout << "  UDP�˿ں�; " << ntohs(oneTouch_addr.sin_port) << endl;
	cout << "##################" << endl;
	//printArray();
	cout << "--------------------------------------------------------------------" << endl;
	cout << endl;

}
void menu()
{
	int selection;
	unsigned short port;
	int iSndRetval;
	char kbBuf[100]; 
	int len;
	U8* bufSend;
	//����|��ӡ��[���Ϳ��ƣ�0���ȴ��������룻1���Զ���][��ӡ���ƣ�0�������ڴ�ӡͳ����Ϣ��1����bit����ӡ���ݣ�2���ֽ�����ӡ����]
	cout << endl << endl << "�豸��:" << strDevID << ",    ���:" << strLayer << ",    ʵ���:" << strEntity;
	cout << endl << "1-�����Զ�����;" << endl << "2-ֹͣ�Զ�����; " << endl << "3-�Ӽ������뷢��; ";
	cout << endl << "4-����ӡͳ����Ϣ; " << endl << "5-����������ӡ��������;" << endl << "6-���ֽ�����ӡ��������;";
	cout << endl << "7-��ӡ����������; ";
	cout << endl << "8-����x; ";
	cout << endl << "9-��x��ȥ�����y��Ȼ���Զ���ӡ��� ";
	cout << endl << "0-ȡ��" << endl << "����������ѡ�����";
	cin >> selection;
	switch (selection) {
	case 0:
		
		break;
	case 1:
		iWorkMode = 10 + iWorkMode % 10; 
		break;
	case 2:
		iWorkMode = iWorkMode % 10;
		break;
	case 3:
		cout << "�����ַ���(������100�ַ�)��";
		cin >> kbBuf;
		cout << "����Ͳ�ӿںţ�";
		cin >> port;

		len = (int)strlen(kbBuf) + 1; //�ַ�������и�������
		if (port >= lowerNumber) {
			cout << "û������ӿ�" << endl;
			return;
		}
		if (lowerMode[port] == 0) {
			//�²�ӿ��Ǳ���������,��ҪһƬ�µĻ�����ת����ʽ
			bufSend = (U8*)malloc(len * 8);

			iSndRetval = ByteArrayToBitArray(bufSend, len * 8, kbBuf, len);
			iSndRetval = SendtoLower(bufSend, iSndRetval, port);
			free(bufSend);
		}
		else {
			//�²�ӿ����ֽ����飬ֱ�ӷ���
			iSndRetval = SendtoLower(kbBuf, len, port);
			iSndRetval = iSndRetval * 8; //�����λ
		}
		//����ͳ��
		if (iSndRetval > 0) {
			iSndTotalCount++;
			iSndTotal += iSndRetval;
		}
		else {
			iSndErrorCount++;
		}
		//��Ҫ��Ҫ��ӡ����
		cout << endl << "��ӿ� " << port << " �������ݣ�" << endl;
		switch (iWorkMode % 10) {
		case 1:
			print_data_bit(kbBuf, len, 1);
			break;
		case 2:
			print_data_byte(kbBuf, len, 1);
			break;
		case 0:
			break;
		}
		break;
	case 4:
		iWorkMode = (iWorkMode / 10) * 10 + 0;
		break;
	case 5:
		iWorkMode = (iWorkMode / 10) * 10 + 1;
		break;
	case 6:
		iWorkMode = (iWorkMode / 10) * 10 + 2;
		break;
	case 7:
		PrintParms();
		break;
	case 8:
		// ��������������������͸�����һ���豸
		iWorkMode = 20 + iWorkMode % 10;
		break;
	case 9:
		iWorkMode = 30 + iWorkMode % 10;
		break;
	}

}