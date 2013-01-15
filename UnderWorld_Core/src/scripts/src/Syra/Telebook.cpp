/* Telebook.cpp  -Syra
 *	This is the C++ version of the Telebooks
 * Upon registering to the item set if it is the advanced or not. This is to keep both in one script
 *
 * HORDE == 1 -facepalm-
 */

#include "StdAfx.h"
#include "Setup.h"


#ifdef WIN32
#pragma warning(disable:4305)        // warning C4305: 'argument' : truncation from 'double' to 'float'
#endif

#define LOCATION_COUNT 132

//These match the order found in the switch so the switch matches its intid to the array
static const float Locs[LOCATION_COUNT+1][4] = 
{
	//Alliance
	{389, -385.34317, 142.355087, 7.9},
	//Horde                          
	{389, 3.81, -14.82, -17},
	// Gurubashi Arena               
	{0, -13261.30, 164.45, 35.78},
	// Stormwind                     
	{0, -8913.23, 554.63, 93.79},
	// Ironforge                     
	{0, -4982.16, -880.75, 501.65},
	// Darnassus                     
	{1, 9945.49, 2609.89, 1316.26},
	// Exodar                        
	{530, -4002.67, -11875.54, -0.71},
	// Orgimmar                      
	{1, 1502.71, -4415.41, 21.77},
	// Thunderbluff                  
	{1, -1284.06, 173.41, 130.00},
	// Undercity                     
	{0, 1831.26, 238.52, 60.52},
	// Silvermoon                    
	{530, 9398.75, -7277.41, 14.21},
	// Blackfathom Deeps             
	{1, 4247.34, 744.05, -24.71},
	// Blackrock Depths              
	{0, -7576.74, -1126.68, 262.26},
	// Dire Maul                     
	{1, -3879.52, 1095.26, 154.78},
	// Gnomeregan                    
	{0, -5162.63, 923.21, 257.17},
	// Maraudon                      
	{1, -1412.73, 2816.92, 112.64},
	// Ragefire Chasm                
	{1, 1814.17, -4401.13, -17.67},
	// Razorfen Downs                
	{1, -4378.32, -1949.14, 88.57},
	// Razorfen Kraul                
	{1, -4473.31, -1810.05, 86.11},
	// Scarlet Monastery             
	{0, 2881.84, -816.23, 160.33},
	// Scholomance                   
	{0, 1229.45, -2576.66, 90.43},
	// Shadowfang Keep               
	{0, -243.85, 1517.21, 76.23},
	// Stratholme                    
	{0, 3362.14, -3380.05, 144.78},
	// Sunken Temple                 
	{0, -10452.32, -3817.51, 18.06},
	// The Deadmines                 
	{0, -11084.10, 1556.17, 48.12},
	// The Stockade                  
	{0, -8797.29, 826.67, 97.63},
	// Uldaman                       
	{0, -6072.23, -2955.94, 209.61},
	// Wailing Caverns               
	{1, -735.11, -2214.21, 16.83},
	// Zul'Farrak                    
	{1, -6825.69, -2882.77, 8.91},
	// Blackwing Lair                
	{469, -7666.11, -1101.53, 399.67},
	// Molten Core                   
	{230, 1117.61, -457.36, -102.49},
	// Onyxia's Lair                 
	{1, -4697.81, -3720.44, 50.35},
	// Ruins of Ahn'Qiraj            
	{1, -8380.47, 1480.84, 14.35},
	// Temple of Ahn'Qiraj           
	{1, -8258.27, 1962.73, 129.89},
	// Zul'Gurub                     
	{0, -11916.74, -1203.32, 92.28},
	// Blade's Edge Mountains        
	{530, 2039.24, 6409.27, 134.30},
	// Hellfire Peninsula            
	{530, -247.37, 964.77, 84.33},
	// Nagrand                       
	{530, -605.84, 8442.39, 60.76},
	// Netherstorm                   
	{530, 3055.70, 3671.63, 142.44},
	// Shadowmoon Valley             
	{530, -2859.75, 3184.24, 9.76},
	// Terokkar Forest               
	{530, -1917.17, 4879.45, 2.10},
	// Zangarmarsh                   
	{530, -206.61, 5512.90, 21.58},
	// Auchindoun                    
	{530, -3323.76, 4934.31, -100.21},
	// Caverns of Time               
	{1, -8187.16, -4704.91, 19.33},
	// Coilfang Reservoir            
	{530, 731.04, 6849.35, -66.62},
	// Hellfire Citadel              
	{530, -331.87, 3039.30, -16.66},
	// Magisters' Terrace            
	{530, 12884.92, -7333.78, 65.48},
	// Tempest Keep                  
	{530, 3088.25, 1388.17, 185.09},
	// Black Temple                  
	{530, -3638.16, 316.09, 35.40},
	// Hyjal Summit                  
	{1, -8175.94, -4178.52, -166.74},
	// Serpentshrine Cavern          
	{530, 731.04, 6849.35, -66.62},
	// Gruul's Lair                  
	{530, 3528.99, 5133.50, 1.31},
	// Magtheridon's Lair            
	{530, -337.50, 3131.88, -102.92},
	// Karazhan                      
	{0, -11119.22, -2010.73, 47.09},
	// Sunwell Plateau               
	{530, 12560.79, -6774.58, 15.08},
	// The Eye                       
	{530, 3088.25, 1388.17, 185.09},
	// Zul'Aman                      
	{530, 6850, -7950, 170},
	// Borean Tundra                 
	{571, 2920.15, 4043.40, 1.82},
	// Crystalsong Forest            
	{571, 5371.18, 109.11, 157.65},
	// Dragonblight                  
	{571, 2729.59, 430.70, 66.98},
	// Grizzly Hills                 
	{571, 3587.20, -4545.12, 198.75},
	// Howling Fjord                 
	{571, 154.39, -4896.33, 296.14},
	// Icecrown                      
	{571, 8406.89, 2703.79, 665.17},
	// Sholazar Basin                
	{571, 5569.49, 5762.99, -75.22},
	// The Storm Peaks               
	{571, 6180.66, -1085.65, 415.54},
	// Wintergrasp                   
	{571, 5044.03, 2847.23, 392.64},
	// Zul'Drak                      
	{571, 4700.09, -3306.54, 292.41},
	// Azjol-Nerub                   
	{571, 3738.93, 2164.14, 37.29},
	// Drak'Tharon                   
	{571, 4772.13, -2035.85, 229.38},
	// Gundrak                       
	{571, 6937.12, -4450.80, 450.90},
	// The Culling of Stratholme     
	{1, -8746.94, -4437.69, -199.98},
	// The Halls of Lightning        
	{571, 9171.01, -1375.94, 1099.55},
	// The Halls of Stone            
	{571, 8921.35, -988.56, 1039.37},
	// The Nexus                     
	{571, 3784.76, 6941.97, 104.49},
	// The Violet Hold               
	{571, 5695.19, 505.38, 652.68},
	// Utgarde Keep                  
	{571, 1222.44, -4862.61, 41.24},
	// Utgarde Pinnacle              
	{571, 1251.10, -4856.31, 215.86},
	// Naxxramas                     
	{571, 3669.77, -1275.48, 243.51},
	// The Eye of Eternity           
	{571, 3873.50, 6974.83, 152.04},
	// The Obsidian Sanctum          
	{571, 3547.39, 267.95, -115.96},
	// Vault of Archavon             
	{571, 5410.21, 2842.37, 418.67},
	// SEVEN DEADLY SINS             
	{0, -11068.23, -1817.1, 59},
	// TOWER OF THE DAMNED           
	{530, 6610, -6449.29, 30},
	// PURGATORY                     
	{0, -1231.7655, 418.7186, 3.1},
	// CATACOMBS OF DEATHLORD KAIDON 
	{469, -7657.68, -1097.01, 403},
	// Ulduar                        
	{571, 9330.53, -1115.40, 1245.14},
	// Alterac Mountains             
	{0, 353.79, -607.08, 150.76},
	// Arathi Highlands              
	{0, -2269.78, -2501.06, 79.04},
	// Badlands                      
	{0, -6026.58, -3318.27, 260.64},
	// Blasted Lands                 
	{0, -10797.67, -2994.29, 44.42},
	// Burning Steppes               
	{0, -8357.72, -2537.49, 135.01},
	// Deadwind Pass                 
	{0, -10460.22, -1699.33, 81.85},
	// Dun Morogh                    
	{0, -6234.99, 341.24, 383.22},
	// Duskwood                      
	{0, -10068.30, -1501.07, 28.41},
	// Eastern Plaguelands           
	{0, 1924.70, -2653.54, 59.70},
	// Elwynn Forest                 
	{0, -8939.71, -131.22, 83.62},
	// Eversong Woods                
	{530, 10341.73, -6366.29, 34.31},
	// Ghostlands                    
	{530, 7969.87, -6872.63, 58.66},
	// Hillsbrad Foothills           
	{0, -585.70, 612.18, 83.80},
	// Isle of Quel'Danas            
	{530, 12916.81, -6867.82, 7.69},
	// Loch Modan                    
	{0, -4702.59, -2698.61, 318.75},
	// Redridge Mountains            
	{0, -9600.62, -2123.21, 66.23},
	// Searing Gorge                 
	{0, -6897.73, -1821.58, 241.16},
	// Silverpine Forest             
	{0, 1499.57, 623.98, 47.01},
	// Stranglethorn Vale            
	{0, -11355.90, -383.40, 65.14},
	// Swamp of Sorrows              
	{0, -10552.60, -2355.25, 85.95},
	// The Hinterlands               
	{0, 92.63, -1942.31, 154.11},
	// Tirisfal Glades               
	{0, 1676.13, 1669.37, 137.02},
	// Western Plaguelands           
	{0, 1635.57, -1068.50, 66.57},
	// Westfall                      
	{0, -9827.95, 865.80, 25.80},
	// Wetlands                      
	{0, -4086.32, -2620.72, 43.55},
	// Ashenvale                     
	{1, 3474.41, 853.47, 5.76},
	// Azshara                       
	{1, 2763.93, -3881.34, 92.52},
	// Azuremyst Isle                
	{530, -3972.72, -13914.99, 98.88},
	// Bloodmyst Isle                
	{530, -2721.67, -12208.90, 9.08},
	// Darkshore                     
	{1, 4336.61, 173.83, 46.84},
	// Desolace                      
	{1, 47.28, 1684.64, 93.55},
	// Durotar                       
	{1, -611.61, -4263.16, 38.95},
	// Dustwallow Marsh              
	{1, -3682.58, -2556.93, 58.43},
	// Felwood                       
	{1, 3590.56, -1516.69, 169.98},
	// Feralas                       
	{1, -4300.02, -631.56, -9.35},
	// Moonglade                     
	{1, 7999.68, -2670.19, 512.09},
	// Mulgore                       
	{1, -2931.49, -262.82, 53.25},
	// Silithus                      
	{1, -6814.57, 833.77, 49.74},
	// Stonetalon Mountains          
	{1, -225.34, -765.16, 6.4},
	// Tanaris                       
	{1, -6999.47, -3707.94, 26.44},
	// Teldrassil                    
	{1, 8754.06, 949.62, 25.99},
	// The Barrens                   
	{1, -948.46, -3738.60, 5.98},
	// Thousand Needles              
	{1, -4685.72, -1836.24, -44.04},
	// Un'Goro Crater                
	{1, -6162.47, -1098.74, -208.99},
	// Winterspring                  
	{1, 6896.27, -2302.51, 586.69},
	// dire maul arena
	{1, -3753.69, 1098.76, 131.97},
	//NULL
	{0, 0, 0, 0}
};

class SCRIPT_DECL Telebook : public GossipScript
{
private:
	uint32 advanced;
	bool Teleport(uint32 id, Player * plr);

public:
	Telebook(bool adv): advanced(adv)	{  }
	~Telebook() {}
	void Destroy() { delete this; }

	void GossipHello(Object* pObject, Player * plr, bool AutoSend);
	void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 intId, const char * code);	
	void GossipEnd(Object* pObject, Player * plr) 
	{
		GossipScript::GossipEnd(pObject, plr); //call base class'
		plr->Gossip_Complete();
	}
};

void Telebook::GossipHello(Object* pObject, Player * plr, bool AutoSend)
{
	if (!plr->IsInWorld())
		return;

	//************************************************************
	//***	Status Checks Here
	//************************************************************
	if (plr->CombatStatus.IsInCombat())
	{
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: You cannot use this in combat.");
		return;
	}

	switch (plr->GetMapId())
	{
	case 44:
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: You cannot use that here. Please direct your attention to the staff member who summoned you.");
		WorldPacket * msg = sChatHandler.FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, "I tried to use my telebook!", plr->GetGUID());
		plr->SendMessageToSet(msg, false); //dont let them see it
		return;
	}

	//end checks

	GossipMenu *Menu;
	objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 50500, plr);
	
	if (advanced)
	{
		Menu->AddItem(6, "Main Cities", 2);
		Menu->AddItem(2, "Azeroth Locations", 3);
		Menu->AddItem(2, "Azeroth Instances", 4);
		Menu->AddItem(2, "Azeroth Raids", 5);
		Menu->AddItem(2, "Outland Locations", 6);
		Menu->AddItem(2, "Outland Instances", 7);
		Menu->AddItem(2, "Outland Raids", 8);
		Menu->AddItem(2, "Northrend Locations", 9);
		Menu->AddItem(2, "Northrend Instances", 10);
		Menu->AddItem(2, "Northrend Raids", 11);
	}
	Menu->AddItem(2, "UNDERWORLD INSTANCES.", 12);
	if (plr->GetTeam() == 0) //ally
		Menu->AddItem(5, "UNDERWORLD MALL", 1000);
	else
		Menu->AddItem(5, "UNDERWORLD MALL", 1001);
	
	if (plr->getLevel() >= 240)
		Menu->AddItem(9, "Gurubashi Arena", 1002);
	else
		Menu->AddItem(9, "Dire Maul Arena", 1131);

	Menu->SendTo(plr);
	
}

void Telebook::GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 intId, const char * code)
{
	GossipMenu *Menu;	

	switch(intId)
	{
	case 999:     // Return to start
		GossipHello(pObject, plr, true);
		return;

	case 1:		//close
		GossipEnd(pObject, plr);
		return;

	case 2: //home cities
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			if (plr->GetTeam() == 0)
			{
				Menu->AddItem(2, "Stormwind", 1003);
				Menu->AddItem(2, "Ironforge", 1004);
				Menu->AddItem(2, "Darnassus", 1005);
				Menu->AddItem(2, "Exodar",1006);
			}
			else
			{			
				Menu->AddItem(2, "Orgimmar", 1007);
				Menu->AddItem(2, "Thunderbluff", 1008);
				Menu->AddItem(2, "Undercity", 1009);
				Menu->AddItem(2, "Silvermoon", 1010);
			}

			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;


	case 3: //Azeroth Locations
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Eastern Kingdoms", 13);
			Menu->AddItem(2, "Kalimdor", 14);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;
		
	case 4: // Azeroth Instances
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Blackfathom Deeps", 1011);
			Menu->AddItem(2, "Blackrock Depths", 1012);
			Menu->AddItem(2, "Dire Maul", 1013);
			Menu->AddItem(2, "Gnomeregan", 1014);
			Menu->AddItem(2, "Maraudon", 1015);
			if (plr->GetTeam() == 1)
				Menu->AddItem(2, "Ragefire Chasm", 1016);
			Menu->AddItem(2, "Razorfen Downs", 1017);
			Menu->AddItem(2, "Razorfen Kraul", 1018);
			Menu->AddItem(2, "Scarlet Monastery", 1019);

			Menu->AddItem(0, "Next Page", 994);

			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 994: // Azeroth Instances Cont.
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Scholomance", 1020);
			Menu->AddItem(2, "Shadowfang Keep", 1021);
			Menu->AddItem(2, "Stratholme", 1022);
			Menu->AddItem(2, "Sunken Temple", 1023);
			Menu->AddItem(2, "The Deadmines", 1024);
			if (plr->GetTeam() == 0)
				Menu->AddItem(2, "The Stockade", 1025);
			Menu->AddItem(2, "Uldaman", 1026);
			Menu->AddItem(2, "Wailing Caverns", 1027);
			Menu->AddItem(2, "Zul'Farrak", 1028);

			Menu->AddItem(0, "Previous Page", 4);

			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;
		

	case 5: // Azeroth Raids
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Blackwing Lair", 1029);
			Menu->AddItem(2, "Molten Core", 1030);
			Menu->AddItem(2, "Onyxia's Lair", 1031);
			Menu->AddItem(2, "Ruins of Ahn'Qiraj", 1032);
			Menu->AddItem(2, "Temple of Ahn'Qiraj", 1033);
			Menu->AddItem(2, "Zul'Gurub", 1034);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;
		

	case 6: // Outland Locations
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Blade's Edge Mountains", 1035);
			Menu->AddItem(2, "Hellfire Peninsula", 1036);
			Menu->AddItem(2, "Nagrand", 1037);
			Menu->AddItem(2, "Netherstorm", 1038);
			Menu->AddItem(2, "Shadowmoon Valley", 1039);
			Menu->AddItem(2, "Terokkar Forest", 1040);
			Menu->AddItem(2, "Zangarmarsh", 1041);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 7: // Outland Instances		
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Auchindoun", 1042);
			Menu->AddItem(2, "Caverns of Time", 1043);
			Menu->AddItem(2, "Coilfang Reservoir", 1044);
			Menu->AddItem(2, "Hellfire Citadel", 1045);
			Menu->AddItem(2, "Magisters' Terrace", 1046);
			Menu->AddItem(2, "Tempest Keep", 1047);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 8: // Outland Raids		
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Black Temple", 1048);
			Menu->AddItem(2, "Hyjal Summit", 1049);
			Menu->AddItem(2, "Serpentshrine Cavern", 1050);
			Menu->AddItem(2, "Gruul's Lair", 1051);
			Menu->AddItem(2, "Magtheridon's Lair", 1052);
			Menu->AddItem(2, "Karazhan", 1053);
			Menu->AddItem(2, "Sunwell Plateau", 1054);
			Menu->AddItem(2, "The Eye", 1055);
			Menu->AddItem(2, "Zul'Aman", 1056);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 9: // Northrend Locations	
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Borean Tundra", 1057);
			Menu->AddItem(2, "Crystalsong Forest", 1058);
			Menu->AddItem(2, "DragonBlight", 1059);
			Menu->AddItem(2, "Grizzly Hills", 1060);
			Menu->AddItem(2, "Howling Fjord", 1061);
			Menu->AddItem(2, "Icecrown", 1062);
			Menu->AddItem(2, "Sholazar Basin", 1063);
			Menu->AddItem(2, "The Storm Peaks", 1064);
			Menu->AddItem(2, "WinterGrasp", 1065);
			Menu->AddItem(2, "Zul'Drak", 1066);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 10: // Northrend Instances		
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Azjol-Nerub", 1067);
			Menu->AddItem(2, "Drak'Tharon Keep", 1068);
			Menu->AddItem(2, "Gundrak", 1069);
			Menu->AddItem(2, "The Culling of Stratholme", 1070);
			Menu->AddItem(2, "The Halls of Lightning", 1071);
			Menu->AddItem(2, "The Halls of Stone", 1072);
			Menu->AddItem(2, "The Nexus", 1073);
			Menu->AddItem(2, "The Violet Hold", 1074);
			Menu->AddItem(2, "Utgarde Keep", 1075);
			Menu->AddItem(2, "Utgarde Pinnacle", 1076);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 11: // Northrend Raids
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Naxxramas", 1077);
			Menu->AddItem(2, "The Eye of Eternity", 1078);
			Menu->AddItem(2, "The Obsidian Sanctum", 1079);
			Menu->AddItem(2, "Vault of Archavon", 1080);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 12: // UNDERWORLD INSTANCES		
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Seven Deadly Sins", 1081);
			Menu->AddItem(2, "Tower of the Damned", 1082);
			Menu->AddItem(2, "Purgatory", 1083);
			Menu->AddItem(2, "Catacombs of Deathlord Kaidon", 1084);
			if (advanced)
				Menu->AddItem(2, "Struggle For Ulduar", 1085);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 13: // Eastern Kingdoms
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Alterac Mountains", 1086);
			Menu->AddItem(2, "Arathi Highlands", 1087);
			Menu->AddItem(2, "Badlands", 1088);
			Menu->AddItem(2, "Blasted Lands", 1089);
			Menu->AddItem(2, "Burning Steppes", 1090);
			Menu->AddItem(2, "Deadwind Pass", 1091);
			Menu->AddItem(2, "Dun Morogh", 1092);
			Menu->AddItem(2, "Duskwood", 1093);
			Menu->AddItem(2, "Eastern Plaguelands", 1094);
			Menu->AddItem(2, "Elwynn Forest", 1095);
			Menu->AddItem(2, "Eversong Woods", 1096);
			Menu->AddItem(2, "Ghostlands", 1097);
			Menu->AddItem(2, "Hillsbrad Foothills", 1098);

			Menu->AddItem(0, "Next Page", 996);
			
			Menu->AddItem(0, "[HOME]", 999);
		}
		Menu->SendTo(plr);
		break;

	case 996: // Eastern Kingdoms Cont.
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Isle of Quel'Danas", 1099);
			Menu->AddItem(2, "Loch Modan", 1100);
			Menu->AddItem(2, "Redridge Mountains", 1101);
			Menu->AddItem(2, "Searing Gorge", 1102);
			Menu->AddItem(2, "Silverpine Forest", 1103);
			Menu->AddItem(2, "Stranglethorn Vale", 1104);
			Menu->AddItem(2, "Swamp of Sorrows", 1105);
			Menu->AddItem(2, "The Hinterlands", 1106);
			Menu->AddItem(2, "Tirisfal Glades", 1107);
			Menu->AddItem(2, "Western Plaguelands", 1108);
			Menu->AddItem(2, "Westfall", 1109);
			Menu->AddItem(2, "Wetlands", 1110);

			Menu->AddItem(0, "Previous Page", 13);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 14: // Kalimdor
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Ashenvale", 1111);
			Menu->AddItem(2, "Azshara", 1112);
			Menu->AddItem(2, "Azuremyst Isle", 1113);
			Menu->AddItem(2, "Bloodmyst Isle", 1114);
			Menu->AddItem(2, "Darkshore", 1115);
			Menu->AddItem(2, "Desolace", 1116);
			Menu->AddItem(2, "Durotar", 1117);
			Menu->AddItem(2, "Dustwallow Marsh", 1118);
			Menu->AddItem(2, "Felwood", 1119);
			Menu->AddItem(2, "Feralas", 1120);

			Menu->AddItem(0, "Next Page", 995);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	case 995: // Kalimdor Cont.	
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
			Menu->AddItem(2, "Moonglade", 1121);
			Menu->AddItem(2, "Mulgore", 1122);
			Menu->AddItem(2, "Silithus", 1123);
			Menu->AddItem(2, "Stonetalon Mountains", 1124);
			Menu->AddItem(2, "Tanaris", 1125);
			Menu->AddItem(2, "Teldrassil", 1126);
			Menu->AddItem(2, "The Barrens", 1127);
			Menu->AddItem(2, "Thousand Needles", 1128);
			Menu->AddItem(2, "Un'Goro Crater", 1129);
			Menu->AddItem(2, "Winterspring", 1130);

			Menu->AddItem(0, "Previous Page", 14);
			
			Menu->AddItem(0, "[HOME]", 999);
			Menu->SendTo(plr);
		}
		break;

	default:
		if (intId >= 1000)
		{
			GossipEnd(pObject, plr);
			if (!Teleport(intId-1000, plr))
			{
				//message here?
			}
		}

	}
}

bool Telebook::Teleport(uint32 id, Player * plr)
{
	//Second combat. Can hold the book then join combat -_-
	if (plr->CombatStatus.IsInCombat())
	{
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: You cannot use this in combat.");
		return false;
	}

	if (id >= LOCATION_COUNT)	
	{
		sChatHandler.BlueSystemMessageToPlr(plr, "Location id [%u] does not exist in the telebook. Report to Syrathia.", id);
		return false;
	}

	//locs[id]->[map][x][y][z]
	if (!plr->AllowedPortTo(uint32(Locs[id][0]), true)) 
		return false;
	
	plr->EventTeleport(uint32(Locs[id][0]), Locs[id][1], Locs[id][2], Locs[id][3]);
	return true;
}


void SetupTelebooks(ScriptMgr * mgr)
{
	GossipScript * basic = (GossipScript*) new Telebook(false);
	GossipScript * advance = (GossipScript*) new Telebook(true);
	mgr->register_item_gossip_script(190004, basic);
	mgr->register_item_gossip_script(190003, advance);
}