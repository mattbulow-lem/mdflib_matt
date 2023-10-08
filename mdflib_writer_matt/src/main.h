#pragma once

enum class CanChName : int {
	Timestamp	= 0, 
	BusChannel	= 1, 
	ID			= 2,			
	IDE			= 3,		
	DLC			= 4,       
	DataLength	= 5,
	DataBytes	= 6,  
	Dir			= 7,
	EDL			= 8,
	ESI			= 9,
	BRS			= 10,
};

constexpr int toInt(CanChName value) {
	return static_cast<int>(value);
}