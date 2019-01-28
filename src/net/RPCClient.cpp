#include "RPCClient.hpp"
#include "TxManager.hpp"

RPCClient::RPCClient(Configuration *_conf, RdmaSocket *_socket, MemoryManager *_mem, uint64_t _mm)
:conf(_conf), socket(_socket), mem(_mem), mm(_mm) {
	isServer = true;
	taskID  = 1;
}

RPCClient::RPCClient() {
	isServer = false;
	taskID = 1;
	mm = (uint64_t)malloc(sizeof(char) * (1024 * 4 + 1024 * 1024 * 4)); //约4MB的空间
	conf = new Configuration();
	socket = new RdmaSocket(1, mm, (1024 * 4 + 1024 * 1024 * 4), conf, false, 0);
	socket->RdmaConnect();
}

RPCClient::~RPCClient() {
	Debug::notifyInfo("Stop RPCClient.");
	if (!isServer) {
		delete conf;
		delete socket;
		free((void *)mm);
	}
	Debug::notifyInfo("RPCClient is closed successfully.");
}

RdmaSocket* RPCClient::getRdmaSocketInstance() {
	return socket;
}

Configuration* RPCClient::getConfInstance() {
	return conf;
}

bool RPCClient::RdmaCall(uint16_t DesNodeID, char *bufferSend, uint64_t lengthSend, char *bufferReceive, uint64_t lengthReceive) {
	Debug::startTimer("rdma call");
	uint32_t ID = __sync_fetch_and_add( &taskID, 1 ), temp;
	uint64_t sendBuffer, receiveBuffer, remoteRecvBuffer;
	uint16_t offset = 0;
	uint32_t imm = (uint32_t)socket->getNodeID();
	//struct timeval startt, endd;
	//unsigned long diff = 0;
	//unsigned long tempCount = 0;
	GeneralSendBuffer *send = (GeneralSendBuffer*)bufferSend;
	lengthReceive -= ContractSendBuffer(send);
	send->taskID = ID;
	send->sourceNodeID = socket->getNodeID();
	send->sizeReceiveBuffer = lengthReceive;
	if (isServer) {
		offset = mem->getServerSendAddress(DesNodeID, &sendBuffer);
		printf("[RdmaCall] server offset = %d\n", offset);
		receiveBuffer = mem->getServerRecvAddress(socket->getNodeID(), offset);
		remoteRecvBuffer = receiveBuffer - mm;
	} else {
		sendBuffer = mm;
		receiveBuffer = mm;
		remoteRecvBuffer = (socket->getNodeID() - conf->getServerCount() - 1) * CLIENT_MESSAGE_SIZE;
	}
	GeneralReceiveBuffer *recv = (GeneralReceiveBuffer*)receiveBuffer;
	if (isServer)
		recv->message = MESSAGE_INVALID;
	memcpy((void *)sendBuffer, (void *)bufferSend, lengthSend);
	_mm_clflush(recv);
	asm volatile ("sfence\n" : : );
	/**
	SFENCE,LFENCE,MFENCE指令提供了高效的方式来保证读写内存的排序,这种操作发生在产生弱排序数据的程序和读取这个数据的程序之间。
   SFENCE——串行化发生在SFENCE指令之前的写操作但是不影响读操作。
	*/
	/**
	__asm__ __volatile__("hlt"); "__asm__"表示后面的代码为内嵌汇编，"asm"是"__asm__"的别名。"__volatile__"表示编译器不要优化代码，后面的指令 保留原样，"volatile"是它的别名。括号里面是汇编指令。
	*/
	temp = (uint32_t)offset;
	imm = imm + (temp << 16);
	Debug::debugItem("sendBuffer = %lx, receiveBuffer = %lx, remoteRecvBuffer = %lx, ReceiveSize = %d",
		sendBuffer, receiveBuffer, remoteRecvBuffer, lengthReceive);
	if (send->message == MESSAGE_DISCONNECT
		|| send->message == MESSAGE_UPDATEMETA
		|| send->message == MESSAGE_EXTENTREADEND) {
		// socket->_RdmaBatchWrite(DesNodeID, sendBuffer, remoteRecvBuffer, lengthSend, imm, 1);
		// socket->PollCompletion(DesNodeID, 1, &wc);
		Debug::endTimer("rdma call");
		return true;
	}
	socket->_RdmaBatchWrite(DesNodeID, sendBuffer, remoteRecvBuffer, lengthSend, imm, 1);
	if (isServer) {
		while (recv->message == MESSAGE_INVALID || recv->message != MESSAGE_RESPONSE)
			printf("server...\n");
		//server遇到不合法的消息就死循环？
		//应该是死循环

	} else {
		//gettimeofday(&startt,NULL);
		Debug::startTimer("polling result");
		while (recv->message != MESSAGE_RESPONSE) {
			/*
			printf("client...\n");
			//这个相当于在轮询结果
			gettimeofday(&endd,NULL);
			diff = 1000000 * (endd.tv_sec - startt.tv_sec) + endd.tv_usec - startt.tv_usec;
			if (diff > 1000000) {
				Debug::debugItem("Send the Fucking Message Again.");
				ExtentWriteSendBuffer *tempsend = (ExtentWriteSendBuffer *)sendBuffer;
				tempsend->offset = (uint64_t)tempCount;
				tempCount += 1;
				socket->_RdmaBatchWrite(DesNodeID, sendBuffer, remoteRecvBuffer, lengthSend, imm, 1);
				gettimeofday(&startt,NULL);
				diff = 0;
			}
			*/
		}
		Debug::endTimer("polling result");
		/*
		gettimeofday(&endd, NULL);
		diff = 1000000 * (endd.tv_sec - startt.tv_sec) + endd.tv_usec - startt.tv_usec;
		Debug::notifyInfo("polling lasts %ld us", diff);
*/
		/*
		std::string out;

		print_action("");
		*/
	}
	memcpy((void*)bufferReceive, (void *)receiveBuffer, lengthReceive);
	Debug::endTimer("rdma call");
	return true;
}

uint64_t RPCClient::ContractSendBuffer(GeneralSendBuffer *send) {
	uint64_t length = 0;
	switch (send->message) {
		case MESSAGE_MKNODWITHMETA: {
			MakeNodeWithMetaSendBuffer *bufferSend =
	                (MakeNodeWithMetaSendBuffer *)send;
        	    	length = (MAX_FILE_EXTENT_COUNT - bufferSend->metaFile.size) * sizeof(FileMetaTuple);
			length = 0;
			break;
		}
		default: {
			length = 0;
			break;
		}
	}
	// printf("contract length = %d", (int)length);
	return length;
}
