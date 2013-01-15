/* Hyrdas.h
 *	This is a Quest script to handle phasing for the Lernaen Hyrdas
 *	Quest ID: 50058
 *	Phases Used:
 *		16 = 1 Hydra
 *		32 = 2 Hydras
 *		64 = 3 Hydras
 */

class SCRIPT_DECL LernaenHydra : public QuestScript
{
	void OnQuestStart(Player* plr, QuestLogEntry * qst) 
	{
		plr->Phase(PHASE_ADD, 16);
	}

	void OnPlayerItemPickup(uint32 itemId, uint32 totalCount, Player * plr, QuestLogEntry * qle)
	{
		switch (totalCount)
		{
		case 1:
			plr->Phase(PHASE_DEL, 16);
			plr->Phase(PHASE_ADD, 32);
			break;

		case 3:
			plr->Phase(PHASE_DEL, 32);
			plr->Phase(PHASE_ADD, 64);
			break;

		case 6:
			plr->Phase(PHASE_DEL, 64);
			break;
		}
	}
};