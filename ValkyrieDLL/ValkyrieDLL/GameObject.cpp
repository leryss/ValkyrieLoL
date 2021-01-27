#include "GameObject.h"
#include <string>
#include "Strings.h"
#include "Offsets.h"
#include "Memory.h"
#include "Logger.h"

#include <windows.h>

GameObject::GameObject(std::string name)
{
	this->name = name;
}

void GameObject::ReadFromBaseAddress(int baseAddr)
{
	address   = baseAddr;
	networkId = ReadInt(baseAddr + Offsets::ObjNetworkID);
}
