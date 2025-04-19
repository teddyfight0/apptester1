//apptester的功能文件
#include <iostream>
#include <conio.h>
#include <random>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"
using namespace std;

U8* autoSendBuf;        //用来组织发送数据的缓存，大小为MAX_BUFFER_SIZE,可以在这个基础上扩充设计，形成适合的结构，例程中没有使用，只是提醒一下
int printCount = 0; //打印控制
int spin = 1;  //打印动态信息控制

//------华丽的分割线，一些统计用的全局变量------------
int iSndTotal = 0;  //发送数据总量
int iSndTotalCount = 0; //发送数据总次数
int iSndErrorCount = 0;  //发送错误次数
int iRcvTotal = 0;     //接收数据总量
int iRcvTotalCount = 0; //转发数据总次数
int iRcvUnknownCount = 0;  //收到不明来源数据总次数
//------------------------------------------------
int global_range = 10000;
int output_x; // 这里就是生成的随机数x
int output_y; // 这里是模仿的对于生成的随机数处理之后输出的y
int external_random = 0; // 全局变量，存储接收到的随机数

void generateTrueRandom() {
	std::random_device rd; // 真随机数源
	std::uniform_int_distribution<int> dist(0, global_range); // 均匀分布
	output_x = dist(rd);
}

void print_statistics();
void menu();
//***************重要函数提醒******************************
//名称：InitFunction
//功能：初始化功能面，由main函数在读完配置文件，正式进入驱动机制前调用
//输入：
//输出：
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
		cout << "内存不够" << endl;
		//这个，计算机也太，退出吧
		exit(0);
	}
	for (i = 0; i < MAX_BUFFER_SIZE; i++) {
		autoSendBuf[i] = 'a'; //初始化数据全为字符'a',只是为了测试
	}
	return;
}
//***************重要函数提醒******************************
//名称：EndFunction
//功能：结束功能面，由main函数在收到exit命令，整个程序退出前调用
//输入：
//输出：
void EndFunction()
{
	if (autoSendBuf != NULL)
		free(autoSendBuf);
	return;
}

//***************重要函数提醒******************************
//名称：TimeOut
//功能：本函数被调用时，意味着sBasicTimer中设置的超时时间到了，
//      函数内容可以全部替换为设计者自己的想法
//      例程中实现了几个同时进行功能，供参考
//      1)根据iWorkMode工作模式，判断是否将键盘输入的数据发送，还是自动发送——这个功能其实是应用层的
//        因为scanf会阻塞，导致计时器在等待键盘的时候完全失效，所以使用_kbhit()无阻塞、不间断地在计时的控制下判断键盘状态，这个点Get到没？
//      2)不断刷新打印各种统计值，通过打印控制符的控制，可以始终保持在同一行打印，Get？
//      3)如果把iWorkMode设置为自动发送，就每经过autoSendTime * DEFAULT_TIMER_INTERVAL ms，向接口0发送一次
//输入：时间到了就触发，只能通过全局变量供给输入
//输出：这就是个不断努力干活的老实孩子
void TimeOut()
{
	int iSndRetval;
	int len;
	U8* bufSend;
	int i;

	printCount++;
	if (_kbhit()) {
		//键盘有动作，进入菜单模式
		menu();
	}
	switch (iWorkMode / 10) {
	case 0:
		break;
	case 1:
		//定时发送, 每间隔autoSendTime * DEFAULT_TIMER_INTERVAL ms 发送一次
		if (printCount % autoSendTime == 0) {
			for (i = 0; i < min(autoSendSize, 8); i++) {
				//开头几个字节在26个字母中间轮流，便于观察
				autoSendBuf[i] = 'a' + printCount % 26;
			}

			len = autoSendSize; //每次发送数量
			if (lowerMode[0] == 0) {
				//自动发送模式下，只向接口0发送
				bufSend = (U8*)malloc(len * 8);
				//下层接口是比特流数组
				iSndRetval = ByteArrayToBitArray(bufSend, len * 8, autoSendBuf, len);
				iSndRetval = SendtoLower(bufSend, iSndRetval, 0);

				free(bufSend);
			}
			else {
				//下层接口是字节数组，直接发送
				for (i = 0; i < min(autoSendSize, 8); i++) {
					//开头几个字节在26个字母中间轮流，便于观察
					autoSendBuf[i] = 'a' + printCount % 26;
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; //换算成位
			}
			//发送统计
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}
			//看要不要打印数据
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
		// 定时发送随机数（以位流格式），每隔 autoSendTime * DEFAULT_TIMER_INTERVAL ms 发送一次
		if (printCount % autoSendTime == 0) {
			// 生成新的随机数
			generateTrueRandom();
			printf("生成的随机数x：%d\n", output_x);
			if (lowerMode[0] == 0) {
				// 位流模式：将随机数转换为二进制位流
				const int bit_len = 32; // 假设 int 是 32 位
				bufSend = (U8*)malloc(bit_len); // 分配 32 位空间
				if (bufSend == NULL) {
					iSndErrorCount++; // 分配失败，记录错误
					break;
				}

				// 直接将 output_x 转换为位流（从高位到低位）
				for (i = 0; i < bit_len; i++) {
					bufSend[i] = (output_x >> (bit_len - 1 - i)) & 1; // 提取每一位
				}

				// 发送位流到接口0
				iSndRetval = SendtoLower(bufSend, bit_len, 0);

				free(bufSend); // 释放临时缓冲区
			}
			else {
				// 字节流模式：将随机数按字节发送（沿用原始逻辑）
				len = 4; // 固定为 4 字节（32 位）
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (output_x >> (i * 8)) & 0xFF; // 按字节提取
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; // 换算成位
			}

			// 发送统计
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}

			// 根据 iWorkMode % 10 决定是否打印
			switch (iWorkMode % 10) {
			case 1:
				// 打印位流（使用 autoSendBuf 模拟位流打印）
				len = 4; // 临时构造字节数组以打印
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (output_x >> (i * 8)) & 0xFF;
				}
				print_data_bit(autoSendBuf, len, 1);
				break;
			case 2:
				// 打印字节流
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
		// 定时发送：将本地随机数与接收的随机数相加，打印本地随机数，发送相加结果（以位流格式）
		if (printCount % autoSendTime == 0) {
			// 生成本地随机数
			generateTrueRandom();
			// 使用从 RecvfromLower 接收的 external_random
			int sum_result = output_y + external_random; // 相加



			// 打印本地随机数
			printf("Generated local random number: %d\n", output_y);

			if (lowerMode[0] == 0) {
				// 位流模式：将相加结果转换为二进制位流
				const int bit_len = 32; // 假设 int 是 32 位
				bufSend = (U8*)malloc(bit_len); // 分配 32 位空间
				if (bufSend == NULL) {
					iSndErrorCount++; // 分配失败，记录错误
					break;
				}

				// 直接将 sum_result 转换为位流（从高位到低位）
				for (i = 0; i < bit_len; i++) {
					bufSend[i] = (sum_result >> (bit_len - 1 - i)) & 1; // 提取每一位
				}

				// 发送位流到接口0
				iSndRetval = SendtoLower(bufSend, bit_len, 0);

				free(bufSend); // 释放临时缓冲区
			}
			else {
				// 字节流模式：将相加结果按字节发送
				len = 4; // 固定为 4 字节（32 位）
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (sum_result >> (i * 8)) & 0xFF; // 按字节提取
				}
				iSndRetval = SendtoLower(autoSendBuf, len, 0);
				iSndRetval = iSndRetval * 8; // 换算成位
			}

			// 发送统计
			if (iSndRetval > 0) {
				iSndTotalCount++;
				iSndTotal += iSndRetval;
			}
			else {
				iSndErrorCount++;
			}

			// 根据 iWorkMode % 10 决定是否打印
			switch (iWorkMode % 10) {
			case 1:
				// 打印位流（使用 autoSendBuf 模拟位流打印）
				len = 4; // 临时构造字节数组以打印
				for (i = 0; i < len; i++) {
					autoSendBuf[i] = (sum_result >> (i * 8)) & 0xFF;
				}
				print_data_bit(autoSendBuf, len, 1);
				break;
			case 2:
				// 打印字节流
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
	//定期打印统计数据
	print_statistics();

}
//------------华丽的分割线，以下是数据的收发,--------------------------------------------

//***************重要函数提醒******************************
//名称：RecvfromUpper
//功能：本函数被调用时，意味着收到一份高层下发的数据
//      函数内容全部可以替换成设计者自己的
//      例程功能介绍
//         1)通过低层的数据格式参数lowerMode，判断要不要将数据转换成bit流数组发送，发送只发给低层接口0，
//           因为没有任何可供参考的策略，讲道理是应该根据目的地址在多个接口中选择转发的。
//         2)判断iWorkMode，看看是不是需要将发送的数据内容都打印，调试时可以，正式运行时不建议将内容全部打印。
//输入：U8 * buf,高层传进来的数据， int len，数据长度，单位字节
//输出：
void RecvfromUpper(U8* buf, int len)
{
	//应用层不会收到“高层”的数据，都是自己产生
}
//***************重要函数提醒******************************
//名称：RecvfromLower
//功能：本函数被调用时，意味着得到一份从低层实体递交上来的数据
//      函数内容全部可以替换成设计者想要的样子
//      例程功能介绍：
//          1)例程实现了一个简单粗暴不讲道理的策略，所有从接口0送上来的数据都直接转发到接口1，而接口1的数据上交给高层，就是这么任性
//          2)转发和上交前，判断收进来的格式和要发送出去的格式是否相同，否则，在bite流数组和字节流数组之间实现转换
//            注意这些判断并不是来自数据本身的特征，而是来自配置文件，所以配置文件的参数写错了，判断也就会失误
//          3)根据iWorkMode，判断是否需要把数据内容打印
//输入：U8 * buf,低层递交上来的数据， int len，数据长度，单位字节，int ifNo ，低层实体号码，用来区分是哪个低层
//输出：
// 设计思想如下：
//1.判断是哪一层来的数据（可能的情况包括物理层，数据链路层，网络层）
//2.1 如果是物理层的数据，就应该判断是bit流数组还是byte流数组吗，然后进行封装，确定是否是正常的数据，最后转发给上层数据
//2.2 如果是数据链路层来的数据，就要使用采用csma/cd 协议来判断 ，然后考虑会不会太大了，影响正常转发，如果没有就正常转发即可
//2.3 如果是网络层来的数据，就判断是否拥塞，然后用路由协议分析，最后输出即可（容易嘻嘻）
// 这里是从下到上，相对应的就是上到下，思路清晰，不断使用相同的手段解封数据就好了 easy


void RecvfromLower(U8* buf, int len, int ifNo)
{
	int retval;
	U8* bufRecv = NULL;

	if (lowerMode[ifNo] == 0) {
		// 低层是 bit 流数组格式，需要转换
		bufRecv = (U8*)malloc(len / 8 + 1);
		if (bufRecv == NULL) {
			printf("没有成功给新的临时数据分配内存!");
			return;
		}

		// 转换为字节数组
		retval = BitArrayToByteArray(buf, len, bufRecv, len / 8 + 1);

		// 解析随机数（假设前 4 字节是 32 位整数）
		if (len >= 32) { // 确保位流足够长（32 位 = 4 字节）
			external_random = (bufRecv[0] << 24) | (bufRecv[1] << 16) | (bufRecv[2] << 8) | bufRecv[3]; // 大端序
		}
		else {
			external_random = 0; // 数据不足，设为 0
		}
	}
	else {
		// 字节流模式
		retval = len * 8; // 换算成位，进行统计

		// 解析随机数（假设前 4 字节是 32 位整数）
		if (len >= 4) { // 确保字节流足够长
			external_random = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]; // 大端序
		}
		else {
			external_random = 0; // 数据不足，设为 0
		}
	}
	iRcvTotal += retval;
	iRcvTotalCount++;

	if (bufRecv != NULL) {
		free(bufRecv);
	}

	// 打印
	switch (iWorkMode % 10) {
	case 1:
		cout << endl << "接收接口 " << ifNo << " 数据：" << endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "接收接口 " << ifNo << " 数据：" << endl;
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
		//低层是bit流数组格式，需要转换，才方便打印
		bufRecv = (U8*)malloc(len / 8 + 1);
		if (bufRecv == NULL) {
			printf("没有成功给新的临时数据分配内存!");
			return;
		}

		//如果接口0是比特数组格式，先转换成字节数组，再向上递交
		retval = BitArrayToByteArray(buf, len, bufRecv, len / 8 + 1);
	}
	else {
		retval = len * 8;//换算成位,进行统计
	}
	iRcvTotal += retval;
	iRcvTotalCount++;


	if (bufRecv != NULL) {
		free(bufRecv);
	}

	//打印
	switch (iWorkMode % 10) {
	case 1:
		cout <<endl<< "接收接口 " <<ifNo <<" 数据："<<endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "接收接口 " << ifNo << " 数据：" << endl;
		print_data_byte(buf, len, lowerMode[ifNo]);
		break;
	case 0:
		break;
	}
}
*/
//打印统计信息
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
		cout << "共发送 " << iSndTotal << " 位," << iSndTotalCount << " 次," << "发生 " << iSndErrorCount << " 次错误;";
		cout << " 共接收 " << iRcvTotal << " 位," << iRcvTotalCount << " 次" ;
		spin++;
	}
}
//PrintParms 打印工作参数，注意不是cfgFilms读出来的，而是目前生效的参数
void PrintParms()
{
	size_t i;
	cout << "设备号: " << strDevID << " 层次: " << strLayer << "实体: " << strEntity << endl;
	cout << "上层实体地址: " << inet_ntoa(upper_addr.sin_addr) << "  UDP端口号: " << ntohs(upper_addr.sin_port) << endl;

	cout << "本层实体地址: " << inet_ntoa(local_addr.sin_addr) << "  UDP端口号: " << ntohs(local_addr.sin_port) << endl;
	if (strLayer.compare("PHY") == 0) {
		if (lowerNumber <= 1) {
			cout << "下层点到点信道" << endl;
			cout << "链路对端地址: ";
		}
		else {
			cout << "下层广播式信道" << endl;
			cout << "共享信道站点：";
		}
	}
	else {
		cout << "下层实体";
	}
	if (lowerNumber == 1) {
		cout << "地址：" << inet_ntoa(lower_addr[0].sin_addr) << "  UDP端口号: " << ntohs(lower_addr[0].sin_port) << endl;
	}
	else {
		if (strLayer.compare("PHY") == 0) {
			cout << endl;
			for (i = 0; i < lowerNumber; i++) {
				cout << "        地址：" << inet_ntoa(lower_addr[i].sin_addr) << "  UDP端口号: " << ntohs(lower_addr[i].sin_port) << endl;
			}
		}
		else {
			cout << endl;
			for (i = 0; i < lowerNumber; i++) {
				cout << "        接口: [" << i << "] 地址" << inet_ntoa(lower_addr[i].sin_addr) << "  UDP端口号: " << ntohs(lower_addr[i].sin_port) << endl;
			}
		}
	}
	string strTmp;
	//strTmp = getValueStr("cmdIpAddr");
	cout << "统一管理平台地址: " << inet_ntoa(cmd_addr.sin_addr);
	//strTmp = getValueStr("cmdPort");
	cout << "  UDP端口号: " << ntohs(cmd_addr.sin_port) << endl;
	//strTmp = getValueStr("oneTouchAddr");
	cout << "oneTouch一键启动地址: " << inet_ntoa(oneTouch_addr.sin_addr);
	//strTmp = getValueStr("oneTouchPort");
	cout << "  UDP端口号; " << ntohs(oneTouch_addr.sin_port) << endl;
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
	//发送|打印：[发送控制（0，等待键盘输入；1，自动）][打印控制（0，仅定期打印统计信息；1，按bit流打印数据，2按字节流打印数据]
	cout << endl << endl << "设备号:" << strDevID << ",    层次:" << strLayer << ",    实体号:" << strEntity;
	cout << endl << "1-启动自动发送;" << endl << "2-停止自动发送; " << endl << "3-从键盘输入发送; ";
	cout << endl << "4-仅打印统计信息; " << endl << "5-按比特流打印数据内容;" << endl << "6-按字节流打印数据内容;";
	cout << endl << "7-打印工作参数表; ";
	cout << endl << "8-发送x; ";
	cout << endl << "9-将x减去计算出y，然后自动打印输出 ";
	cout << endl << "0-取消" << endl << "请输入数字选择命令：";
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
		cout << "输入字符串(不超过100字符)：";
		cin >> kbBuf;
		cout << "输入低层接口号：";
		cin >> port;

		len = (int)strlen(kbBuf) + 1; //字符串最后有个结束符
		if (port >= lowerNumber) {
			cout << "没有这个接口" << endl;
			return;
		}
		if (lowerMode[port] == 0) {
			//下层接口是比特流数组,需要一片新的缓冲来转换格式
			bufSend = (U8*)malloc(len * 8);

			iSndRetval = ByteArrayToBitArray(bufSend, len * 8, kbBuf, len);
			iSndRetval = SendtoLower(bufSend, iSndRetval, port);
			free(bufSend);
		}
		else {
			//下层接口是字节数组，直接发送
			iSndRetval = SendtoLower(kbBuf, len, port);
			iSndRetval = iSndRetval * 8; //换算成位
		}
		//发送统计
		if (iSndRetval > 0) {
			iSndTotalCount++;
			iSndTotal += iSndRetval;
		}
		else {
			iSndErrorCount++;
		}
		//看要不要打印数据
		cout << endl << "向接口 " << port << " 发送数据：" << endl;
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
		// 这里生成随机数，并发送给另外一个设备
		iWorkMode = 20 + iWorkMode % 10;
		break;
	case 9:
		iWorkMode = 30 + iWorkMode % 10;
		break;
	}

}