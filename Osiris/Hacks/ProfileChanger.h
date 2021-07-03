#pragma once
#include <windows.h>
#include <string>

namespace profile_changer {
	void write_packet(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
	void receive_message(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
	void send_update_messages();
	bool send_client_hello();
	bool send_matchmaking_client_hello();
	bool send_client_rank_update();
}