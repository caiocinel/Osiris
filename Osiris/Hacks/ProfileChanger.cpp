#include "../Memory.h"
#include "../Config.h"
#include "../SDK/MemAlloc.h"
#include "ProfileChanger.h"
#include "Protobuffs/pbwrap.h"
#include "Protobuffs/protoparser.h"
#include "Protobuffs/protomessages.h"


#define CAST(cast, address, add) (cast)((uint32_t)address + (uint32_t)add)

std::string change_main(void* pubDest, uint32_t* pcubMsgSize) {
	matchmaking_client_hello_t msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	matchmaking_client_hello_t::player_commendation_info_t commendations;
	commendations.cmd_friendly().set(config->profilechanger.friendly);
	commendations.cmd_teaching().set(config->profilechanger.teach);
	commendations.cmd_leader().set(config->profilechanger.leader);
	msg.commendation().set(commendations);

	matchmaking_client_hello_t::player_ranking_info_t ranking;
	ranking.rank_type_id().set(rank_type::mm);
	ranking.account_id().set(memory->SteamUser->GetSteamID().GetAccountID());
	ranking.rank_id().set(config->profilechanger.rankMatchmaking);
	ranking.wins().set(config->profilechanger.winsMatchmaking);
	msg.ranking().set(ranking);

	msg.player_level().set(config->profilechanger.level);
	msg.player_level_xp().set(config->profilechanger.exp);

	if (config->profilechanger.ban_type != 0 && config->profilechanger.ban_time != 0) {
		msg.penalty_reason().set(config->profilechanger.ban_type);
		static int num;
		switch (config->profilechanger.ban_type) {
		case 0: num = config->profilechanger.ban_time; break;
		case 1: num = config->profilechanger.ban_time * 60; break;
		case 2: num = config->profilechanger.ban_time * 3600; break;
		case 3: num = config->profilechanger.ban_time * 86400; break;
		}
		msg.penalty_seconds().set(num);
	}

	return msg.serialize();
}

std::string change_other(void* pubDest, uint32_t* pcubMsgSize) {

	msg_client_rank_update_t msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);

	auto ranking = msg.ranking().get_all();

	for (int i = 0; i < ranking.size(); i++) {
		auto _ranking = ranking[i];

		if (_ranking.rank_type_id().get() == rank_type::wingmans) {
			_ranking.rank_id().set(config->profilechanger.rankWingman);
			_ranking.wins().set(config->profilechanger.winsWingman);

			msg.ranking().set(_ranking, i);
		}

		if (_ranking.rank_type_id().get() == rank_type::danger_zone) {
			_ranking.rank_id().set(config->profilechanger.rankDangerzone);
			_ranking.wins().set(config->profilechanger.winsDangerzone);

			msg.ranking().set(_ranking, i);
		}
	}

	return msg.serialize();
}

namespace profile_changer {
	void write_packet(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize) {
		if ((uint32_t)packet.size() <= cubDest - 8)
		{
			memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
			*pcubMsgSize = packet.size() + 8;
		} else if (memory->memAlloc) {
			auto memPtr = *CAST(void**, thisPtr, 0x18);
			auto memPtrSize = *CAST(uint32_t*, thisPtr, 0x1C);
			auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

			auto packetMemory = memory->memAlloc->Realloc(memPtr, newSize + 4);

			*CAST(void**, thisPtr, 0x18) = packetMemory;
			*CAST(uint32_t*, thisPtr, 0x1C) = newSize;
			*CAST(void**, oldEBP, -0x14) = packetMemory;

			memcpy(CAST(void*, packetMemory, 0x1C), (void*)packet.data(), packet.size());

			*pcubMsgSize = packet.size() + 8;
		}
	}

	void receive_message(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize) {
		static bool standart_upd;
		static bool other_upd;

		if (config->profilechanger.level <2)
			return;

		switch (messageType)
		{
		case msg_matchmaking_gc_client_hello:
			write_packet(change_main(pubDest, pcubMsgSize), thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
			standart_upd = true;
			break;
		case msg_client_rank_update:
			write_packet(change_other(pubDest, pcubMsgSize), thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
			other_upd = true;
			break;
		}

		if (other_upd && standart_upd) {
			other_upd = false;
			standart_upd = false;
		}
	}

	void send_update_messages() {
		send_client_hello();
		send_client_rank_update();
		send_matchmaking_client_hello();
	}

	bool send_client_hello() {
		msg_client_hello_t msg;
		msg.client_session_need().set(1);
		auto packet = msg.serialize();

		void* ptr = malloc(packet.size() + 8);
		if (!ptr) return false;

		((uint32_t*)ptr)[0] = msg_client_hello | ((DWORD)1 << 31);
		((uint32_t*)ptr)[1] = 0;

		memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
		bool result = memory->SteamGameCoordinator->GCSendMessage(msg_client_hello | ((DWORD)1 << 31), ptr, packet.size() + 8) == 0;
		free(ptr);

		return result;
	}

	bool send_matchmaking_client_hello() {
		proto_writer msg(0);
		auto packet = msg.serialize();
		void* ptr = malloc(packet.size() + 8);
		if (!ptr) return false;

		((uint32_t*)ptr)[0] = msg_matchmaking_client_hello | ((DWORD)1 << 31);
		((uint32_t*)ptr)[1] = 0;

		memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
		bool result = memory->SteamGameCoordinator->GCSendMessage(msg_matchmaking_client_hello | ((DWORD)1 << 31), ptr, packet.size() + 8) == 0;
		free(ptr);

		return result;
	}

	bool send_client_rank_update() {
		matchmaking_client_hello_t::player_ranking_info_t ranking_wingman;
		matchmaking_client_hello_t::player_ranking_info_t ranking_dangerzone;
		ranking_wingman.rank_type_id().set(rank_type::wingmans);
		ranking_dangerzone.rank_type_id().set(rank_type::danger_zone);

		msg_client_rank_update_t msg;
		msg.ranking().add(ranking_wingman);
		msg.ranking().add(ranking_dangerzone);

		auto packet = msg.serialize();

		void* ptr = malloc(packet.size() + 8);
		if (!ptr) return false;

		((uint32_t*)ptr)[0] = msg_client_rank_update | ((DWORD)1 << 31);
		((uint32_t*)ptr)[1] = 0;

		memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
		bool result = memory->SteamGameCoordinator->GCSendMessage(msg_client_rank_update | ((DWORD)1 << 31), ptr, packet.size() + 8) == 0;
		free(ptr);

		return result;
	}
}