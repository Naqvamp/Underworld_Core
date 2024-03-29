--[[ UNIT - Eastern Plaguelands - Generic.lua

********************************
*                                                            *
* The Moon++ Scripting Project        *
*                                                            *
********************************

This software is provided as free and open source by the
staff of The Moon++ Scripting Project, in accordance with 
the GPL license. This means we provide the software we have 
created freely and it has been thoroughly tested to work for 
the developers, but NO GUARANTEE is made it will work for you 
as well. Please give credit where credit is due, if modifying,
redistributing and/or using this software. Thank you.

Staff of The Moon++ Scripting Project, November 2007
~~End of License Agreement

All credits for the files in this folder that are GameMonkey scripts
go to the GMScripts project, for all their hard work for the Ascent 
community.

-- Moon++ Project, November 2007 ]]

function Darrowshire_Spirit_Spawn (pUnit, Event)
	local x = pUnit:GetX()
	local y = pUnit:GetY()
	local z = pUnit:GetZ()
	local o = pUnit:GetO()
	local  DoSpawn = math.random (2) -- Random number, either 1 or 2
	if (DoSpawn == 2) then
		pUnit:SpawnCreature(11064, x, y, z, o, 35, 300000)
	end
end
RegisterUnitEvent (8530, 4, "Darrowshire_Spirit_Spawn")
RegisterUnitEvent (8531, 4, "Darrowshire_Spirit_Spawn")
RegisterUnitEvent (8532, 4, "Darrowshire_Spirit_Spawn")
