//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"
#include "ObjectMgr.h"
#include "Master.h"
#include "World.h"

/*	TalentIds => TalentRanks
 *	It's a pair o matched arrays of te talentids and rank counts per class
 *	Pulled direct from Talent.dbc
 */
static const uint32 TalentIds[DRUID+1][100] = 
{
	{ 0 },	// NONE
	{ 124, 130, 127, 126, 641, 128, 131, 137, 662, 121, 136, 2232, 132, 133, 125, 123, 134, 129, 1859, 1663, 135, 1862, 2233, 2283, 1824, 1860, 1662, 1661, 1664, 2231, 1863, 142, 1601, 141, 144, 138, 153, 147, 1654, 140, 2247, 151, 146, 150, 152, 149, 702, 1652, 148, 1660, 1653, 1870, 2236, 1666, 1893, 1871, 2246, 1872, 2250, 158, 157, 161, 159, 166, 160, 661, 154, 1581, 1542, 155, 1657, 165, 1543, 1541, 156, 1864, 167, 1655, 1865, 1658, 1868, 1659, 1866, 2234, 1867, 0 },	// WARRIOR
	{ 1403, 1407, 1631, 1464, 1401, 1633, 1411, 1481, 1634, 1632, 1761, 1755, 1410, 1756, 1402, 1757, 2176, 1441, 1758, 1759, 2147, 2148, 1823, 2179, 2149, 2150, 1432, 1463, 1444, 1449, 1628, 1435, 1461, 1443, 1450, 1446, 2198, 1742, 1433, 1465, 1743, 1627, 1745, 1502, 1744, 2190, 1746, 1747, 2199, 2193, 2191, 2192, 1442, 2185, 1748, 1425, 1629, 2280, 1501, 1423, 2281, 1521, 1422, 1431, 1426, 1750, 1429, 2282, 1430, 1751, 1421, 1753, 2195, 1754, 2194, 2204, 2200, 2196, 0 },	// PALADIN
	{ 1382, 1389, 1624, 1381, 1395, 1625, 1384, 2138, 1396, 1385, 1393, 1388, 1387, 1390, 1799, 1397, 1800, 1386, 1801, 2136, 1802, 2140, 1803, 2137, 2227, 2139, 1623, 1820, 1621, 1310, 1304, 1305, 1810, 1622, 1814, 1311, 1309, 2229, 1306, 2228, 1321, 1312, 1303, 1809, 1812, 1325, 1811, 1813, 2141, 2142, 1322, 2143, 2144, 2145, 1341, 2197, 1344, 1806, 1343, 1349, 1818, 1346, 1345, 1819, 1348, 1342, 1351, 1353, 1347, 1804, 1362, 2130, 1361, 1821, 1807, 2131, 2132, 1808, 2133, 2134, 2135, 0 },	// HUNTER
	{ 203, 201, 221, 1827, 187, 181, 204, 301, 182, 206, 222, 186, 1122, 184, 223, 242, 1703, 1706, 1705, 205, 1707, 2072, 1825, 2073, 1709, 2074, 2075, 2076, 276, 272, 270, 273, 2068, 277, 382, 278, 269, 682, 268, 1721, 280, 279, 1762, 283, 274, 2065, 281, 1723, 2069, 1718, 1715, 1719, 2066, 2070, 2071, 2244, 241, 261, 1700, 262, 244, 247, 303, 1123, 246, 245, 263, 1701, 284, 265, 681, 1713, 1702, 1711, 381, 1722, 1712, 2077, 2078, 1714, 2079, 2080, 2081, 0 },	// ROGUE
	{ 342, 1898, 352, 346, 344, 321, 347, 348, 343, 1769, 341, 350, 2268, 1201, 351, 1771, 1772, 1858, 322, 1773, 2235, 1896, 1894, 1895, 1774, 1901, 1202, 1897, 410, 406, 401, 411, 1181, 442, 1636, 361, 1635, 408, 403, 413, 1561, 402, 1766, 404, 1768, 1637, 1765, 2279, 1767, 1904, 1902, 1815, 1903, 1905, 1911, 465, 2027, 462, 466, 482, 463, 542, 481, 501, 483, 881, 461, 541, 484, 1638, 1777, 1781, 2267, 521, 1778, 1906, 1816, 1908, 1779, 1909, 1907, 1910, 0 },	// PRIEST
	{ 1939, 1945, 2017, 1938, 1948, 2217, 1941, 1943, 2086, 1942, 2018, 1953, 2015, 1950, 1949, 1944, 2105, 1960, 1954, 1936, 2259, 1955, 2019, 1959, 1957, 1958, 2034, 1961, 2031, 2020, 1968, 2035, 1973, 2022, 2042, 2215, 2048, 2044, 1981, 1971, 1990, 2030, 1980, 2223, 1993, 1992, 2260, 1999, 2029, 2284, 2210, 1979, 1997, 1975, 2040, 1998, 1989, 2082, 1932, 2218, 1963, 1933, 2025, 1934, 2008, 2047, 1985, 2039, 2004, 2225, 1996, 2005, 2011, 2226, 2009, 2001, 1984, 2285, 2221, 2013, 2085, 1962, 2007, 2003, 2043, 2216, 2036, 2000, 0 },	// DEATH KNIGHT
	{ 564, 563, 561, 1640, 1645, 575, 574, 565, 567, 1642, 1641, 562, 1682, 1685, 721, 573, 2052, 2262, 2049, 1686, 2050, 1687, 2051, 2252, 2053, 586, 595, 589, 1646, 593, 583, 587, 582, 581, 588, 594, 1648, 591, 1695, 592, 1699, 590, 2084, 2060, 1697, 1696, 2061, 1698, 2059, 2063, 2064, 610, 2101, 614, 609, 613, 605, 607, 611, 617, 601, 602, 615, 1647, 616, 2083, 1689, 1643, 2263, 1690, 1692, 901, 2055, 2249, 2054, 1691, 1693, 2056, 2057, 2058, 0 },	// SHAMAN
	{ 27, 1141, 26, 34, 2212, 31, 28, 30, 29, 23, 25, 24, 1639, 1730, 33, 32, 1731, 35, 1733, 36, 1732, 1848, 1734, 1735, 1849, 1850, 1851, 1852, 38, 37, 62, 73, 70, 1649, 65, 61, 69, 63, 741, 66, 67, 72, 64, 1736, 1737, 68, 71, 2214, 1738, 1740, 1853, 1854, 1741, 1855, 1856, 1857, 74, 76, 80, 85, 1650, 75, 82, 81, 1845, 2211, 83, 88, 1142, 2222, 1724, 86, 77, 1726, 421, 1725, 1727, 87, 1844, 1843, 1728, 1729, 2209, 1846, 1826, 1847, 0 },	// MAGE
	{ 944, 943, 982, 1887, 941, 983, 963, 967, 985, 964, 965, 1817, 961, 981, 1679, 966, 968, 1678, 986, 1677, 1889, 1888, 1676, 2045, 1890, 1891, 1284, 1005, 1003, 1006, 1101, 1007, 1004, 2205, 1001, 1061, 1021, 1002, 1764, 1763, 1041, 1081, 1873, 1042, 1878, 1669, 1022, 1668, 1667, 1875, 1670, 2245, 1876, 2041, 1221, 1222, 1223, 1883, 1224, 1225, 1242, 1243, 1282, 1226, 1671, 1262, 1227, 1281, 1261, 1244, 1283, 1680, 1880, 1263, 1673, 2261, 1882, 1672, 1884, 1885, 1886, 0 },	// WARLOCK
	{ 0 },	// NONE
	{ 796, 795, 799, 805, 794, 807, 1162, 798, 802, 803, 801, 1914, 797, 804, 1792, 2242, 808, 1794, 809, 1798, 1793, 2241, 1795, 1919, 1921, 1796, 1920, 1918, 2266, 1927, 821, 823, 822, 824, 841, 826, 829, 827, 1915, 843, 830, 831, 828, 842, 1788, 825, 1797, 844, 1790, 1789, 1922, 1929, 1791, 1930, 2264, 1916, 1917, 762, 2238, 783, 1822, 763, 782, 789, 2240, 764, 792, 784, 1782, 788, 2239, 1784, 790, 1783, 793, 1912, 1785, 1913, 1786, 1924, 1923, 1787, 1925, 1928, 1926, 0 },	// DRUID
};

static const uint32 TalentRanks[DRUID+1][100] = 
{
	{ 0 },	// NONE
	{ 3, 5, 2, 2, 3, 3, 2, 1, 2, 3, 3, 3, 5, 1, 5, 5, 2, 3, 2, 2, 1, 2, 2, 1, 3, 2, 3, 1, 2, 5, 1, 2, 5, 3, 3, 5, 1, 2, 2, 5, 2, 2, 3, 2, 1, 2, 5, 2, 1, 3, 3, 2, 1, 1, 3, 3, 2, 1, 3, 2, 5, 5, 5, 3, 1, 3, 5, 5, 2, 5, 3, 1, 2, 2, 5, 3, 1, 2, 2, 5, 1, 1, 3, 5, 1, 0 },	// WARRIOR
	{ 5, 5, 2, 3, 2, 2, 5, 1, 2, 2, 3, 3, 3, 1, 3, 2, 2, 1, 3, 3, 2, 3, 1, 3, 3, 1, 5, 5, 3, 5, 2, 1, 5, 2, 3, 2, 2, 2, 1, 3, 2, 5, 3, 1, 3, 3, 5, 1, 5, 2, 2, 1, 5, 5, 3, 2, 5, 1, 3, 5, 2, 2, 3, 1, 5, 2, 3, 2, 1, 3, 3, 3, 3, 1, 2, 3, 2, 1, 0 },	// PALADIN
	{ 5, 5, 2, 3, 3, 2, 2, 1, 5, 2, 5, 2, 1, 2, 2, 5, 3, 1, 3, 2, 5, 3, 1, 3, 5, 1, 5, 3, 2, 3, 3, 3, 2, 5, 1, 3, 2, 3, 3, 3, 3, 1, 5, 3, 3, 1, 3, 5, 3, 2, 1, 3, 3, 1, 2, 3, 5, 3, 3, 5, 2, 3, 1, 2, 3, 5, 2, 1, 3, 2, 3, 3, 1, 3, 5, 2, 3, 1, 3, 5, 1, 0 },	// HUNTER
	{ 3, 2, 5, 2, 3, 5, 2, 1, 5, 2, 2, 3, 5, 5, 1, 5, 2, 2, 3, 1, 2, 2, 5, 2, 1, 2, 5, 1, 3, 2, 5, 3, 2, 3, 1, 2, 5, 3, 5, 2, 1, 3, 2, 5, 2, 2, 1, 3, 3, 3, 3, 1, 3, 5, 1, 5, 3, 2, 2, 2, 3, 2, 1, 3, 3, 3, 2, 2, 1, 2, 1, 3, 5, 3, 1, 3, 5, 2, 3, 1, 2, 5, 1, 0 },	// ROGUE
	{ 5, 5, 3, 3, 2, 2, 3, 1, 3, 3, 3, 2, 2, 5, 1, 2, 3, 3, 1, 3, 2, 3, 2, 3, 1, 2, 5, 1, 2, 3, 5, 5, 5, 1, 3, 3, 2, 3, 2, 2, 1, 5, 2, 5, 3, 1, 3, 2, 5, 3, 3, 1, 3, 5, 1, 3, 2, 5, 3, 2, 3, 2, 5, 1, 2, 2, 3, 1, 1, 2, 3, 2, 3, 1, 5, 2, 3, 1, 1, 3, 5, 1, 0 },	// PRIEST
	{ 2, 3, 5, 5, 3, 2, 1, 5, 3, 3, 3, 3, 3, 3, 1, 3, 2, 3, 1, 2, 2, 3, 1, 3, 1, 3, 5, 1, 3, 2, 5, 2, 5, 3, 5, 1, 3, 5, 2, 2, 3, 3, 1, 1, 2, 3, 3, 1, 2, 3, 3, 1, 3, 1, 3, 5, 1, 2, 3, 5, 2, 3, 2, 3, 3, 5, 1, 2, 3, 2, 1, 5, 2, 2, 3, 3, 1, 5, 1, 2, 1, 3, 1, 3, 3, 1, 5, 1, 0 },	// DEATH KNIGHT
	{ 5, 5, 3, 3, 3, 5, 1, 5, 2, 3, 2, 1, 3, 3, 5, 1, 3, 2, 2, 3, 3, 1, 3, 5, 1, 5, 5, 2, 3, 5, 3, 3, 1, 3, 3, 5, 3, 1, 3, 5, 5, 1, 1, 2, 2, 3, 3, 1, 2, 5, 1, 3, 2, 5, 2, 5, 2, 3, 3, 1, 3, 5, 5, 2, 1, 3, 3, 3, 2, 1, 3, 1, 3, 1, 2, 3, 1, 2, 5, 1, 0 },	// SHAMAN
	{ 2, 3, 5, 5, 2, 3, 2, 3, 1, 2, 3, 2, 3, 3, 3, 1, 2, 5, 3, 1, 2, 2, 3, 1, 2, 3, 5, 1, 3, 5, 3, 3, 2, 3, 3, 3, 1, 3, 2, 3, 3, 1, 3, 3, 2, 3, 1, 2, 5, 2, 2, 3, 1, 3, 5, 1, 2, 3, 5, 3, 2, 5, 2, 3, 3, 1, 2, 2, 3, 3, 2, 1, 5, 3, 3, 2, 3, 1, 3, 2, 5, 1, 5, 3, 2, 1, 0 },	// MAGE
	{ 5, 5, 2, 3, 3, 2, 1, 5, 2, 2, 3, 3, 3, 1, 3, 5, 1, 3, 3, 5, 2, 3, 1, 3, 5, 1, 2, 3, 5, 2, 2, 2, 2, 2, 3, 1, 2, 2, 3, 5, 1, 1, 2, 5, 3, 5, 1, 2, 3, 3, 1, 1, 5, 1, 2, 3, 3, 2, 2, 3, 3, 3, 1, 1, 3, 5, 2, 1, 2, 5, 3, 3, 1, 3, 5, 2, 3, 1, 3, 5, 1, 0 },	// WARLOCK
	{ 0 },	// NONE
	{ 5, 5, 3, 2, 3, 2, 1, 3, 3, 3, 2, 2, 2, 1, 2, 3, 5, 3, 1, 2, 3, 3, 3, 3, 3, 1, 3, 5, 1, 1, 2, 3, 5, 5, 3, 3, 3, 1, 2, 5, 3, 1, 5, 2, 2, 5, 3, 1, 3, 5, 3, 3, 1, 3, 2, 5, 1, 5, 5, 3, 2, 2, 3, 3, 1, 2, 5, 3, 3, 1, 3, 3, 3, 2, 1, 3, 3, 3, 5, 3, 1, 1, 2, 3, 1, 0 },	// DRUID
};


bool Player::CanStake(bool showmsgs)
{
	if(GetMapId() != 169)
	{
		if(showmsgs)
			sChatHandler.RedSystemMessageToPlr(this,"ERROR: Incorrect Map.");
		return false;
	}
	
	QueryResult * res = WorldDatabase.Query("SELECT x,y,z FROM gm_stakes WHERE map='%u'", GetMapId());
	if(!res)
	{
		//no stakes found at all, we're popping this server's cherry on stakes
		return true; 
	}

	float min_separation = (float)160.0;
	float x,y,z, dist, overlap;

	do
	{		
		x = res->Fetch()[0].GetFloat();
		y = res->Fetch()[1].GetFloat();
		z = res->Fetch()[2].GetFloat();
		dist = CalcDistance(x,y,z);
		if(dist<min_separation)
		{
			overlap = min_separation-dist;
			if(showmsgs)
				sChatHandler.RedSystemMessageToPlr(this,"ERROR: You are too close to another claim.  The claim found had an overlap distance of %f clicks.", overlap );
			delete res;
			return false;
		}
	}while (res->NextRow());
	delete res;

	//If they've reached this point there are no stakes within 160.0 - check passed
	return true;
}
bool Player::StakeClaim()
{
	if(!CanStake(true))
		return false;

	m_stake->mapid = GetMapId();
	m_stake->x = GetPositionX();
	m_stake->y = GetPositionY();
	m_stake->z = GetPositionZ();	
	uint32 credits = ( m_stake->credits ? m_stake->credits:0  );
	
	WorldDatabase.Execute("REPLACE INTO `_claims` (`acct`,`credits`,`map`,`x`,`y`,`z`) VALUES (%u,%u,%u,%f,%f,%f)",pAcctId(),credits,GetMapId(),GetPositionX(),GetPositionY(),GetPositionZ());

	return true;
}
bool Player::IsInClaim(bool showmsgs)
{	
	if(GetMapId() != 169)
	{
		if(showmsgs)
			sChatHandler.RedSystemMessageToPlr(this,"ERROR: Incorrect Map.");
		return false;
	}
	if(m_stake == NULL)
	{
		if(showmsgs)
			sChatHandler.RedSystemMessageToPlr(this,"ERROR: You must stake a claim first before you can spawn or delete.");
		return false;
	}

	float radius = (float)75.0;
	float x,y,z, dist;
	x = m_stake->x;
	y = m_stake->y;
	z = m_stake->z;

	dist = CalcDistance(x,y,z);
	if(dist>radius)
	{
		if(showmsgs)
			sChatHandler.RedSystemMessageToPlr(this,"ERROR: You are outside of your claim by %f clicks.", (dist-radius) );
		
		return false;
	}
	//They're inside their claim, allow spawn
	return true;
}


bool Player::IsGMFrozen()
{
	if(bFROZEN)
		return true;
	if(HasAura(9454) || HasAura(17691) || HasAura(50224)) 
	{
		DoGMFreeze();
		return true;
	}
	return false;
}
void Player::DoGMFreeze()
{
	/*if(!bFROZEN)return true;*/

	if(HasAura(9454)) //freeze
	{	
		Neutralize();
		CastSpellOnSelf(50224);
		CastSpellOnSelf(9454);
		bFROZEN=true;
	}
	if(HasAura(17691)) //time out
	{
		Neutralize();
		CastSpellOnSelf(50224);
		CastSpellOnSelf(17691);
		bFROZEN=true;
	}
}




void Player::LearnMissingSpells()
{
	//if you increase max 2d -- up the limit
	static uint32 spellarray[DRUID+1][8] = {
/*N/A*/	{ 0 },
/*WAR*/ { 71, 7386, 355, 2458, 0 },
/*PAL*/ { 7328, 0 },
/*HUN*/ { 883, 2641, 6991, 982, 1515, 0 },
/*ROG*/ { 0 },
/*PRI*/ { 0 },
/*DK */ { 48778, 50977, 53428, 0 },
/*SHA*/ { 8071, 3599, 5394, 0 },
/*MAGE*/{ 53140, 0 },
/*LOCK*/{ 688, 697, 712, 5784, 691, 1122, 18540, 0 },
/*N/A*/	{ 0 },
/*DRU*/	{ 5487, 6795, 6807, 1066, 8946, 40120, 62078, 0 },
	};
	
	uint32 c = getClass();
	for(uint32 i = 0; spellarray[c][i] != 0; ++i)
	{
		addSpell(spellarray[c][i]);
	}
}
void Player::UnstuckAccount()
{
	if(GetMapId() != 0 && GetMapId() != 1 && GetMapId() != 530 && GetMapId() != 571)
	{
		BroadcastMessage("This function may only be used on one of the 4 major continents.");
		return;
	}
	uint32 canuse = m_session->m_LastAcctUnstuck;
	if((uint32)UNIXTIME >= canuse)
	{
		CharacterDatabase.Execute("UPDATE characters SET `mapId`='%u', `positionX`='%f', `positionY`='%f', `positionZ`='%f' WHERE `acct`='%u' AND `level`>'1' AND `mapId` <> 44", GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), m_session->GetAccountId() );
		m_session->m_LastAcctUnstuck = ((uint32)UNIXTIME+300);
		BroadcastMessage("Your offline characters have been successfully teleported to your location.");
	}
	else
	{
		BroadcastMessage("You must wait %u more seconds before being able to use this function.", (canuse-(uint32)UNIXTIME) );
	}
}


const char * Player::GetNameClick(const char * color)
{
	char NAME[256]= "";
	snprintf(NAME,256,"|Hplayer:%s|h[%s]|h%s",GetName(), GetName(), color);
	char * name = NAME;
	return name;
}


void Player::RemoveAurasUW()
{
	for(uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
	{
		if(m_auras[i] != 0)
		{
			if(m_auras[i]->GetSpellProto())
			{				
				switch(m_auras[i]->GetSpellId())
				{
				case 17691: //timeout
					continue;
				case 9454:  //freeze
					continue;
				case 50224: //GM incapacitate visual aura
					continue;
				}
			}
			m_auras[i]->Remove();
		}
			
	}
}


bool Player::CastSpellOnSelf(uint32 spellid)
{
	//Unit *target = this;
	
	SpellEntry *spellentry = dbcSpell.LookupEntry(spellid);
	if(!spellentry)
	{
		return false;
	}

	Spell * sp = new Spell(this, spellentry, true, 0);
	if (!sp)
		return true;
	SpellCastTargets targets(this->GetGUID());
	sp->prepare(&targets);

	return true;
}




bool Player::RingCheck()
{
	uint32 bronze = 70003;
	uint32 silver = 70004;
	uint32 gold	  = 70005;
	uint32 plat   = 70006;

	if(GetItemInterface()->GetItemCount(bronze, true) > 0)
		return true;
	if(GetItemInterface()->GetItemCount(silver, true) > 0)
		return true;
	if(GetItemInterface()->GetItemCount(gold, true) > 0)
		return true;
	if(GetItemInterface()->GetItemCount(plat, true) > 0)
		return true;

	//Didn't find any rings
	return false;

}

uint32 Player::AddLootBars()
{
	if (m_ItemInterface->AddItemById(66668, 4, 0))
		return 2;
	else
		return 1;
}

bool Player::AllowedPortTo(uint32 mapid, bool include_msg)
{
	//thse maps are crashing maps and no one is allowed on
	switch(mapid)
	{
	case 209: //zul'farak
		{
			if(include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: This map can cause the server to crash. Access denied.");
			return false;
		}break;
	}

	if(m_session->m_gmData->rank > RANK_PLAT)
		return true;

	switch(mapid)
	{
	case 576: //the nexus
		{
			if(include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: This map is off limits.");
			return false;
		}
		break;
	case 309: //ZG (mt. olympus)
		{
			if (!sWorld.dark_light_enabled)
				return true;

			if(bLight)
				return true;
			if(bGMTagOn)
			{
				if(include_msg)
					sChatHandler.BlueSystemMessageToPlr(this,"NOTICE: As long as your GM tag and you are \"on-duty\" you are permitted to be in this area.  Please note using your GM tag to subvert the separation of light and dark characters or your own gain will result in a suspension or ban.");
				return true;
			}
			return false;
		}break;
	case 540: //dark instances
	case 542:
	case 543:
	case 544:
		{
			if (sWorld.dark_light_enabled)
			{
				if(bDark)
					return true;
				if(bGMTagOn)
				{
					if(include_msg)
						sChatHandler.BlueSystemMessageToPlr(this,"NOTICE: As long as your GM tag and you are \"on-duty\" you are permitted to be in this area.  Please note using your GM tag to subvert the separation of light and dark characters or your own gain will result in a suspension or ban.");
					return true;
				}
				return false;
			} //else fall through to other OL maps
		}

	//outlands for 240 bosses stuff
	case 530:
		if (sWorld.realmID & REALM_ALPHA_X)
			if (getLevel() >= 70 && getLevel() < 150)
				return true; //allow these levels on to do the totd level area -_-
	case 533:
	case 545:
	case 546:
	case 547:
	case 548:
	case 550:
	case 552:
	case 553:
	case 554:
	case 555:
	case 556:
	case 557:
	case 558:
	case 564:
	case 565:
		{
			if (!sWorld.blockOutlands)
				return true;

			if (m_ItemInterface->GetItemCount(81552, true) > 0) //pomegranate seed
				return true;

			if(bGMTagOn)
			{
				if(include_msg)
					sChatHandler.BlueSystemMessageToPlr(this,"NOTICE: As long as your GM tag and you are \"on-duty\" you are permitted to be in this area.  Please note using your GM tag to subvert the restrictions for your own gain will result in a suspension or ban.");
				return true;
			}

			if (include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: This map is restricted to players who have completed the quest Enter the Portal.");
			return false;
		}
		break;

	case 169: //Emerald Dream
		{
			if(include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: Emerald Dream is an Admin-Only area.");
			return false;
		}break;
	case 451: //Dev Isle
		{
			if(include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: Developer/Programmer Isle is an Admin-Only area. Your client likely doesn't have the map anyways.");
			return false;
		}break;
	
	case 559: //Nagrand Arena
		{
			if (sWorld.arenaEventInProgress)
			{
				if(include_msg)
				{
					int seconds = ((sWorld.arenaEventTimeout - (uint32)UNIXTIME) / 1000);
					int mins=0;
					int hours=0;
					if(seconds >= 60)
					{
						mins = seconds / 60;
						if(mins)
						{
							seconds -= mins*60;
							if(mins >= 60)
							{
								hours = mins / 60;
								if(hours)
								{
									mins -= hours*60;
								}
							}
						}
					}

					
					sChatHandler.RedSystemMessageToPlr(this, "ERROR: There is or was recently a PvP event and the arena will expire in %d hours, %d minutes, and %d seconds. Please come back later.", hours, mins, seconds);
				}
				return false;
			}
		}
			/* Commented until this is implemented
			else
			{
				if (this->m_killsLifetime < sWorld.arena3MinKills)
				{
					if (include_msg)
						sChatHandler.RedSystemMessageToPlr(this, "ERROR: You do not meet the minumum required kills to get in here. You need %u more.", sWorld.arena3MinKills - this->m_killsLifetime);
					return false;
				}
			}
		}break;
	case 562: //Blade's Edge Arena
		{
			if (this->m_killsLifetime < sWorld.arena2MinKills)
			{
				if (include_msg)
					sChatHandler.RedSystemMessageToPlr(this, "ERROR: You do not meet the minumum required kills to get in here. You need %u more.", sWorld.arena2MinKills - this->m_killsLifetime);
				return false;
			}

		}break;
	case 572: //Ruins of Lordaeron
		{
			if (this->m_killsLifetime < sWorld.arena1MinKills)
			{
				if (include_msg)
					sChatHandler.RedSystemMessageToPlr(this, "ERROR: You do not meet the minumum required kills to get in here. You need %u more.", sWorld.arena1MinKills - this->m_killsLifetime);
				return false;
			}
		}break;*/
	case 568: //GM Mall (ZA)
		{
			if(m_session->m_gmData->rank == RANK_NO_RANK)
			{
				if(include_msg)
					sChatHandler.RedSystemMessageToPlr(this, "ERROR: Only GMs may enter the GM Mall.");
				return false;
			}
		}break;
	case 534: //hyjal
	case 489: //Warsong Gulch
	case 529: //Arathi Basin
	case 566: //Eye of the Storm
	case 30:  //Alterac Valley
		{
			if(include_msg)
				sChatHandler.RedSystemMessageToPlr(this, "ERROR: Battlegrounds are restricted access.");
			return false;
		}break;

	case 230: //capital bar leveling road
		{
			if (getLevel() > 70)
			{
				if(bGMTagOn)
				{
					if(include_msg)
						sChatHandler.BlueSystemMessageToPlr(this,"NOTICE: As long as your GM tag and you are \"on-duty\" you are permitted to be in this area.  Please note using your GM tag to subvert the restrictions for your own gain will result in a suspension or ban.");
					return true;
				}
				
				if(include_msg)
					sChatHandler.RedSystemMessageToPlr(this, "ERROR: This map has a level restriction of 70.");
				return false;
			}
		}

	}
	return true;
}

//-----------------------
//Check if player is in a restricted map
bool Player::InRestrictedArea()
{
	return !(AllowedPortTo(m_mapId, false));
}


void Player::LearnAllTalents()
{
	Reset_Talents();

	if (sWorld.realmID & REALM_ALPHA_X)
		m_specs[SPEC_PRIMARY].m_customTalentPointOverride = 300;

	uint8 c = getClass();
	for (uint32 ii = 0; TalentIds[c][ii] != 0 && TalentRanks[c][ii] != 0; ii++)
	{
		LearnTalent(TalentIds[c][ii], TalentRanks[c][ii]-1, true);
	}
}

void Player::LearnItAllUW()
{
	vector<uint32> spells;
	spells.reserve(25);
	static uint32 voters[60] = 
	{
		750, 822, 5227, 7164, 7744, 8737, 9077, 10059, 11416, 11417, 11418, 11419, 11420, 11789, 13159, 20549, 20550, 20573, 20589, 20591, 20598, 20600, 20798, 25392, 25431, 25898, 26990, 26992, 27052, 27053, 27054, 27055, 27056, 27124, 28148, 28880, 32266, 32267, 33691, 35236, 35717, 35853, 36839, 39055, 40497, 43689, 48074, 58984, 0
	};

	//preserve the list of voter spells they have
	for (uint32 ii = 0; voters[ii] != 0; ii++)
	{
		if (HasSpell(voters[ii]))
			spells.push_back(voters[ii]);
	}

	Reset_Spells();
	LearnAllTalents();

	for (uint32 ii = 0; ii < spells.size(); ii++)
		addSpell(spells.at(ii));

	if (ResetSpells)
	{
		ResetSpells = false;
		sChatHandler.GreenSystemMessageToPlr(this, "This is a one time thing. You will not be reset again upon login.");
	}

	sChatHandler.BlueSystemMessageToPlr(this, "Your spells and talents have been reset to the standard list. All voters have been retained");
	
}