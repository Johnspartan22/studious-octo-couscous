/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2016  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "otpch.h"

#include "actions.h"
#include "bed.h"
#include "configmanager.h"
#include "container.h"
#include "game.h"
#include "pugicast.h"
#include "spells.h"
#include "rewardchest.h"

extern Game g_game;
extern Spells* g_spells;
extern Actions* g_actions;
extern ConfigManager g_config;

Actions::Actions() :
	scriptInterface("Action Interface")
{
	scriptInterface.initState();
}

Actions::~Actions()
{
	clear();
}

inline void Actions::clearMap(ActionUseMap& map)
{
	// Filter out duplicates to avoid double-free
	std::unordered_set<Action*> set;
	for (const auto& it : map) {
		set.insert(it.second);
	}
	map.clear();

	for (Action* action : set) {
		delete action;
	}
}

void Actions::clear()
{
	clearMap(useItemMap);
	clearMap(uniqueItemMap);
	clearMap(actionItemMap);

	scriptInterface.reInitState();
}

LuaScriptInterface& Actions::getScriptInterface()
{
	return scriptInterface;
}

std::string Actions::getScriptBaseName() const
{
	return "actions";
}

Event* Actions::getEvent(const std::string& nodeName)
{
	if (strcasecmp(nodeName.c_str(), "action") != 0) {
		return nullptr;
	}
	return new Action(&scriptInterface);
}

bool Actions::registerEvent(Event* event, const pugi::xml_node& node)
{
	Action* action = static_cast<Action*>(event); //event is guaranteed to be an Action

	pugi::xml_attribute attr;
	if ((attr = node.attribute("itemid"))) {
		uint16_t id = pugi::cast<uint16_t>(attr.value());

		auto result = useItemMap.emplace(id, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << id << std::endl;
		}
		return result.second;
	} else if ((attr = node.attribute("fromid"))) {
		pugi::xml_attribute toIdAttribute = node.attribute("toid");
		if (!toIdAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing toid in fromid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromId = pugi::cast<uint16_t>(attr.value());
		uint16_t iterId = fromId;
		uint16_t toId = pugi::cast<uint16_t>(toIdAttribute.value());

		auto result = useItemMap.emplace(iterId, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << iterId << " in fromid: " << fromId << ", toid: " << toId << std::endl;
		}

		bool success = result.second;
		while (++iterId <= toId) {
			result = useItemMap.emplace(iterId, action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << iterId << " in fromid: " << fromId << ", toid: " << toId << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	} else if ((attr = node.attribute("uniqueid"))) {
		uint16_t uid = pugi::cast<uint16_t>(attr.value());

		auto result = uniqueItemMap.emplace(uid, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with uniqueid: " << uid << std::endl;
		}
		return result.second;
	} else if ((attr = node.attribute("fromuid"))) {
		pugi::xml_attribute toUidAttribute = node.attribute("touid");
		if (!toUidAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing touid in fromuid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromUid = pugi::cast<uint16_t>(attr.value());
		uint16_t iterUid = fromUid;
		uint16_t toUid = pugi::cast<uint16_t>(toUidAttribute.value());

		auto result = uniqueItemMap.emplace(iterUid, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with unique id: " << iterUid << " in fromuid: " << fromUid << ", touid: " << toUid << std::endl;
		}

		bool success = result.second;
		while (++iterUid <= toUid) {
			result = uniqueItemMap.emplace(iterUid, action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with unique id: " << iterUid << " in fromuid: " << fromUid << ", touid: " << toUid << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	} else if ((attr = node.attribute("actionid"))) {
		uint16_t aid = pugi::cast<uint16_t>(attr.value());

		auto result = actionItemMap.emplace(aid, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with actionid: " << aid << std::endl;
		}
		return result.second;
	} else if ((attr = node.attribute("fromaid"))) {
		pugi::xml_attribute toAidAttribute = node.attribute("toaid");
		if (!toAidAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing toaid in fromaid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromAid = pugi::cast<uint16_t>(attr.value());
		uint16_t iterAid = fromAid;
		uint16_t toAid = pugi::cast<uint16_t>(toAidAttribute.value());

		auto result = actionItemMap.emplace(iterAid, action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with action id: " << iterAid << " in fromaid: " << fromAid << ", toaid: " << toAid << std::endl;
		}

		bool success = result.second;
		while (++iterAid <= toAid) {
			result = actionItemMap.emplace(iterAid, action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with action id: " << iterAid << " in fromaid: " << fromAid << ", toaid: " << toAid << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	}
	return false;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos)
{
	if (pos.x != 0xFFFF) {
		const Position& playerPos = player->getPosition();
		if (playerPos.z != pos.z) {
			return playerPos.z > pos.z ? RETURNVALUE_FIRSTGOUPSTAIRS : RETURNVALUE_FIRSTGODOWNSTAIRS;
		}

		if (!Position::areInRange<1, 1>(playerPos, pos)) {
			return RETURNVALUE_TOOFARAWAY;
		}
	}
	return RETURNVALUE_NOERROR;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos, const Item* item)
{
	Action* action = getAction(item);
	if (action) {
		return action->canExecuteAction(player, pos);
	}
	return RETURNVALUE_NOERROR;
}

ReturnValue Actions::canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight, bool checkFloor)
{
	if (toPos.x == 0xFFFF) {
		return RETURNVALUE_NOERROR;
	}

	const Position& creaturePos = creature->getPosition();
	if (checkFloor && creaturePos.z != toPos.z) {
		return creaturePos.z > toPos.z ? RETURNVALUE_FIRSTGOUPSTAIRS : RETURNVALUE_FIRSTGODOWNSTAIRS;
	}

	if (!Position::areInRange<7, 5>(toPos, creaturePos)) {
		return RETURNVALUE_TOOFARAWAY;
	}

	if (checkLineOfSight && !g_game.canThrowObjectTo(creaturePos, toPos)) {
		return RETURNVALUE_CANNOTTHROW;
	}

	return RETURNVALUE_NOERROR;
}

Action* Actions::getAction(const Item* item)
{
	if (item->hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
		auto it = uniqueItemMap.find(item->getUniqueId());
		if (it != uniqueItemMap.end()) {
			return it->second;
		}
	}

	if (item->hasAttribute(ITEM_ATTRIBUTE_ACTIONID)) {
		auto it = actionItemMap.find(item->getActionId());
		if (it != actionItemMap.end()) {
			return it->second;
		}
	}

	auto it = useItemMap.find(item->getID());
	if (it != useItemMap.end()) {
		return it->second;
	}

	//rune items
	return g_spells->getRuneSpell(item->getID());
}

bool Actions::hasAction(const Item* item)
{
	return getAction(item) != nullptr;
}

ReturnValue Actions::internalUseItem(Player* player, const Position& pos, uint8_t index, Item* item)
{
	if (Door* door = item->getDoor()) {
		if (!door->canUse(player)) {
			return RETURNVALUE_CANNOTUSETHISOBJECT;
		}
	}

	Action* action = getAction(item);
	if (action) {
		if (action->isScripted()) {
			if (action->executeUse(player, item, pos, nullptr, pos)) {
				return RETURNVALUE_NOERROR;
			}

			if (item->isRemoved()) {
				return RETURNVALUE_CANNOTUSETHISOBJECT;
			}
		} else if (action->function) {
			if (action->function(player, item, pos, nullptr, pos)) {
				return RETURNVALUE_NOERROR;
			}
		}
	}

	if (BedItem* bed = item->getBed()) {
		if (!bed->canUse(player)) {
			return RETURNVALUE_CANNOTUSETHISOBJECT;
		}

		bed->sleep(player);
		return RETURNVALUE_NOERROR;
	}

	if (Container* container = item->getContainer()) {
		Container* openContainer;

		//depot container
		if (DepotLocker* depot = container->getDepotLocker()) {
			DepotLocker* myDepotLocker = player->getDepotLocker(depot->getDepotId());
			myDepotLocker->setParent(depot->getParent());
			openContainer = myDepotLocker;
			player->setLastDepotId(depot->getDepotId());
		} else {
			openContainer = container;
		}

		//reward chest
		if (container->getRewardChest()) {
			RewardChest* myRewardChest = player->getRewardChest();
			if (myRewardChest->size() == 0) {
				return RETURNVALUE_REWARDCHESTISEMPTY;
			}

			myRewardChest->setParent(container->getParent()->getTile());
			for (auto& it : player->rewardMap) {
				it.second->setParent(myRewardChest);
			}

			openContainer = myRewardChest;
		}

		//reward container proxy created when the boss dies
		if (container->getID() == ITEM_REWARD_CONTAINER && !container->getReward()) {
			if (Reward* reward = player->getReward(container->getIntAttr(ITEM_ATTRIBUTE_DATE), false)) {
				reward->setParent(container->getRealParent());
				openContainer = reward;
			} else {
				return RETURNVALUE_THISISIMPOSSIBLE;
			}
		}

		uint32_t corpseOwner = container->getCorpseOwner();
		if (container->isRewardCorpse()) {
			//only players who participated in the fight can open the corpse
			if (!player->getReward(container->getIntAttr(ITEM_ATTRIBUTE_DATE), false)) {
				return RETURNVALUE_YOUARENOTTHEOWNER;
			}
		}

		//open/close container
		int32_t oldContainerId = player->getContainerID(openContainer);
		if (oldContainerId != -1) {
			player->onCloseContainer(openContainer);
			player->closeContainer(oldContainerId);
		} else {
			player->addContainer(index, openContainer);
			player->onSendContainer(openContainer);
		}

		return RETURNVALUE_NOERROR;
	}

	const ItemType& it = Item::items[item->getID()];
	if (it.canReadText) {
		if (it.canWriteText) {
			player->setWriteItem(item, it.maxTextLen);
			player->sendTextWindow(item, it.maxTextLen, true);
		} else {
			player->setWriteItem(nullptr);
			player->sendTextWindow(item, 0, false);
		}

		return RETURNVALUE_NOERROR;
	}

	return RETURNVALUE_CANNOTUSETHISOBJECT;
}

bool Actions::useItem(Player* player, const Position& pos, uint8_t index, Item* item)
{
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::ACTIONS_DELAY_INTERVAL));
	player->stopWalk();

	ReturnValue ret = internalUseItem(player, pos, index, item);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		return false;
	}
	return true;
}

bool Actions::useItemEx(Player* player, const Position& fromPos, const Position& toPos,
                        uint8_t toStackPos, Item* item, Creature* creature/* = nullptr*/)
{
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::EX_ACTIONS_DELAY_INTERVAL));
	player->stopWalk();

	Action* action = getAction(item);
	if (!action) {
		player->sendCancelMessage(RETURNVALUE_CANNOTUSETHISOBJECT);
		return false;
	}

	ReturnValue ret = action->canExecuteAction(player, toPos);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		return false;
	}

	if (!action->executeUse(player, item, fromPos, action->getTarget(player, creature, toPos, toStackPos), toPos)) {
		if (!action->hasOwnErrorHandler()) {
			player->sendCancelMessage(RETURNVALUE_CANNOTUSETHISOBJECT);
		}
		return false;
	}
	return true;
}

Action::Action(LuaScriptInterface* interface) :
	Event(interface), function(nullptr), allowFarUse(false), checkFloor(true), checkLineOfSight(true) {}

Action::Action(const Action* copy) :
	Event(copy), function(copy->function), allowFarUse(copy->allowFarUse), checkFloor(copy->checkFloor), checkLineOfSight(copy->checkLineOfSight) {}

bool Action::configureEvent(const pugi::xml_node& node)
{
	pugi::xml_attribute allowFarUseAttr = node.attribute("allowfaruse");
	if (allowFarUseAttr) {
		setAllowFarUse(allowFarUseAttr.as_bool());
	}

	pugi::xml_attribute blockWallsAttr = node.attribute("blockwalls");
	if (blockWallsAttr) {
		setCheckLineOfSight(blockWallsAttr.as_bool());
	}

	pugi::xml_attribute checkFloorAttr = node.attribute("checkfloor");
	if (checkFloorAttr) {
		setCheckFloor(checkFloorAttr.as_bool());
	}

	return true;
}

bool Action::loadFunction(const pugi::xml_attribute& attr)
{
	const char* functionName = attr.as_string();
	if (strcasecmp(functionName, "increaseitemid") == 0) {
		function = increaseItemId;
	} else if (strcasecmp(functionName, "decreaseitemid") == 0) {
		function = decreaseItemId;
	} else {
		std::cout << "[Warning - Action::loadFunction] Function \"" << functionName << "\" does not exist." << std::endl;
		return false;
	}

	scripted = false;
	return true;
}

bool Action::increaseItemId(Player*, Item* item, const Position&, Thing*, const Position&)
{
	g_game.startDecay(g_game.transformItem(item, item->getID() + 1));
	return true;
}

bool Action::decreaseItemId(Player*, Item* item, const Position&, Thing*, const Position&)
{
	g_game.startDecay(g_game.transformItem(item, item->getID() - 1));
	return true;
}

std::string Action::getScriptEventName() const
{
	return "onUse";
}

ReturnValue Action::canExecuteAction(const Player* player, const Position& toPos)
{
	if (!getAllowFarUse()) {
		return g_actions->canUse(player, toPos);
	} else {
		return g_actions->canUseFar(player, toPos, getCheckLineOfSight(), getCheckFloor());
	}
}

Thing* Action::getTarget(Player* player, Creature* targetCreature, const Position& toPosition, uint8_t toStackPos) const
{
	if (targetCreature) {
		return targetCreature;
	}
	return g_game.internalGetThing(player, toPosition, toStackPos, 0, STACKPOS_USETARGET);
}

bool Action::executeUse(Player* player, Item* item, const Position& fromPos, Thing* target, const Position& toPos)
{
	//onUse(player, item, fromPosition, target, toPosition)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - Action::executeUse] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");

	LuaScriptInterface::pushThing(L, item);
	LuaScriptInterface::pushPosition(L, fromPos);

	LuaScriptInterface::pushThing(L, target);
	LuaScriptInterface::pushPosition(L, toPos);

	return scriptInterface->callFunction(5);
}
