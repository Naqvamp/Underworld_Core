//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"

bool ItemInterface::DeleteItemById(uint32 itemid)
{
	int16 i = 0;

	for(i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if (item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
	}

	for(i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if (item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
	}

	for(i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if (item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
	}

	for(i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if (item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
	}

	for(i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if(item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
		else
		{
			if(item && item->IsContainer() && item->GetProto())
			{
				for (uint32 j = 0; j < item->GetProto()->ContainerSlots; j++)
				{
					Item *item2 = ((Container*)item)->GetItem(static_cast<int16>( j ));
					if (item2 && item2->GetProto() && item2->GetProto()->ItemId == itemid)
					{
						return ((Container*)item)->SafeFullRemoveItemFromSlot(static_cast<int16>( j ));
					}
				}
			}
		}
	}

	for(i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
	{
		Item *item = GetInventoryItem(i);

		if (item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
	}

	for(i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
	{
		Item *item = GetInventoryItem(i);
		if(item && item->GetProto() && item->GetProto()->ItemId == itemid)
		{
			return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
		}
		else
		{
			if(item && item->IsContainer() && item->GetProto())
			{
				for (uint32 j = 0; j < item->GetProto()->ContainerSlots; j++)
				{
					Item *item2 = ((Container*)item)->GetItem(static_cast<int16>( j ));
					if (item2 && item2->GetProto() && item2->GetProto()->ItemId == itemid)
					{
						return ((Container*)item)->SafeFullRemoveItemFromSlot(static_cast<int16>( j ));
					}
				}
			}
		}
	}
	return false;
}