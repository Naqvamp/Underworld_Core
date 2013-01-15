/*
 * GuildWars System
 *	Guild will fight over multiple zones of control
 *	Each Zone will be controlled by capturing a GO. Capturing will be a 10 second timer
 *
 *	Assaulting notes: When a guild assaults a node they will get the guild id in the m_assaulters and the owning guild will still technically 
 *	own the node.  When a guild assaults a node that is already being assaulted it will remove the old timer and add a new one. If the
 *	guild owns that node they are assaulting the will 'defend' the node and it will just drop the timer.
 */
#ifndef _GUILD_WAR_H
#define _GUILD_WAR_H

namespace GuildWars
{
	static const uint32 ASSAULT_TIMER = 300*1000; //5 minutes

	enum GWN_FLAGS
	{
		//these three can be checked w/ the owner ids...
		ASSAULTING = 0x1,
		OWNED = 0x2, 
		NEUTRAL = 0x4,
	};

	//For action logging and announcing both. Log every announcable action and maybe some more
	enum ACTION_ANNOUNCE_TYPES
	{
		ASSAULT_NODE = 1,
		DEFEND_NODE,
		CAPTURE_NODE,
		GUILD_REGISTER,
		NODE_ERROR,
		REGISTER_ERROR,
	};
}


class GuildWarNode : public EventableObject
{
public:
	bool AssaultNode(Player * plr); //call when opening the slow lock on the GO
	void CaptureNode(uint32 guildId); //Callback for after assaulting a node
	void SaveNode(); //saves the node in the database

private:
	//node id
	uint32 m_id;

	//guild IDs 
	uint32 m_assaulter;
	uint32 m_owner;
	uint32 m_prevOwner;

	//<3 bitmasks
	uint32 m_flags;

	//One object per each state
	GameObject * m_flagNeutral;
	GameObject * m_flagCaptured;
	GameObject * m_flagProgressing;
};

typedef HM_NAMESPACE::hash_map<uint32,GuildWarNode*> GWNodeMap;

class GuildWarManager : public Singleton < GuildWarManager >
{
public:
	GuildWarManager();
	~GuildWarManager();

	bool AssaultNode(uint32 nodeId, Player * plr);
	void Update();

	//Call this to make various announces. Will automaticaly log the action
	void Announcer(uint32 type, Player * plr, GuildWarNode * node);
	void LogAction(uint32 type, WorldSession * m_session, GuildWarNode * node, char * notes);

private:
	GWNodeMap m_nodes;
	Mutex m_lock;
	uint32 m_lastUpdate;
	//fill this 
	vector<uint32> m_guilds;

	//Database functions
	void _loadEvent();
	void _saveEvent();
	void _saveNode(uint32 nodeId);
	void _loadNode(uint32 nodeId);
	

};

#define sGWarMgr GuildWarManager::getSingleton()

#endif