#ifndef _TINKER_LOBBY_SERVER_H
#define _TINKER_LOBBY_SERVER_H

#include <EASTL/vector.h>

#include "lobby.h"

class CGameLobby
{
	friend class CGameLobbyServer;

public:
										CGameLobby();

public:
	void								Initialize(size_t iPort);
	void								Shutdown();

	size_t								GetNumPlayers();
	size_t								GetPlayerIndexByID(size_t iID);
	size_t								GetPlayerIndexByClient(size_t iClient);
	CLobbyPlayer*						GetPlayer(size_t iIndex);
	CLobbyPlayer*						GetPlayerByID(size_t iID);
	CLobbyPlayer*						GetPlayerByClient(size_t iClient);

	void								UpdateInfo(const eastl::string16& sKey, const eastl::string16& sValue);

	void								AddPlayer(size_t iID, size_t iClient);
	void								RemovePlayer(size_t iID);
	void								UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue);

	void								SendFullUpdate(size_t iID);

protected:
	bool								m_bActive;
	eastl::vector<CLobbyPlayer>			m_aClients;
	eastl::map<eastl::string16, eastl::string16> m_asInfo;
};

class CGameLobbyServer
{
public:
	static size_t						CreateLobby(size_t iPort);
	static void							DestroyLobby(size_t iLobby);
	static size_t						AddPlayer(size_t iLobby, size_t iClient);
	static void							RemovePlayer(size_t iID);
	static CGameLobby*					GetLobby(size_t iLobby);

	static size_t						GetActiveLobbies();
	static size_t						GetPlayerLobby(size_t iID);
	static size_t						GetClientPlayerID(size_t iClient);

	static void							UpdateLobby(size_t iLobby, const eastl::string16& sKey, const eastl::string16& sValue);
	static void							UpdatePlayer(size_t iID, const eastl::string16& sKey, const eastl::string16& sValue);

	static void							ClientConnect(class INetworkListener*, class CNetworkParameters*);
	static void							ClientDisconnect(class INetworkListener*, class CNetworkParameters*);

	static size_t						GetNextPlayerID();

protected:
	static eastl::vector<CGameLobby>	s_aLobbies;
	static eastl::map<size_t, size_t>	s_aiPlayerLobbies;
	static eastl::map<size_t, size_t>	s_aiClientPlayerIDs;
};

#endif
