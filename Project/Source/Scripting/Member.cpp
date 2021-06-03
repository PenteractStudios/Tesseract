#include "Member.h"

#include "Utils/Logging.h"

#include "Math/myassert.h"

#include "Utils/Leaks.h"

const char* GetMemberTypeName(MemberType type) {
	switch (type) {
	case MemberType::BOOL:
		return "Bool";
	case MemberType::INT:
		return "Int";
	case MemberType::UINT:
		return "UInt";
	case MemberType::LONGLONG:
		return "LongLong";
	case MemberType::ULONGLONG:
		return "ULongLong";
	case MemberType::FLOAT:
		return "Float";
	case MemberType::FLOAT2:
		return "Float2";
	case MemberType::FLOAT3:
		return "Float3";
	case MemberType::DOUBLE:
		return "Double";
	case MemberType::STRING:
		return "String";
	case MemberType::GAME_OBJECT_UID:
		return "GameObjectUID";
	case MemberType::PREFAB_RESOURCE_UID:
		return "PrefabResourceUID";
	default:
		LOG("Member of type %i hasn't been registered in GetMemberTypeName.", (unsigned) type);
		assert(false); // ERROR: Member type not registered
		return nullptr;
	}
}

MemberType GetMemberTypeFromName(const char* name) {
	if (strcmp(name, "Bool") == 0) {
		return MemberType::BOOL;
	} else if (strcmp(name, "Int") == 0) {
		return MemberType::INT;
	} else if (strcmp(name, "UInt") == 0) {
		return MemberType::UINT;
	} else if (strcmp(name, "LongLong") == 0) {
		return MemberType::LONGLONG;
	} else if (strcmp(name, "ULongLong") == 0) {
		return MemberType::ULONGLONG;
	} else if (strcmp(name, "Float") == 0) {
		return MemberType::FLOAT;
	} else if (strcmp(name, "Double") == 0) {
		return MemberType::DOUBLE;
	} else if (strcmp(name, "String") == 0) {
		return MemberType::STRING;
	} else if (strcmp(name, "GameObjectUID") == 0) {
		return MemberType::GAME_OBJECT_UID;
	} else if (strcmp(name, "PrefabResourceUID") == 0) {
		return MemberType::PREFAB_RESOURCE_UID;
	} else if (strcmp(name, "Float2") == 0) {
		return MemberType::FLOAT2;
	} else if (strcmp(name, "Float3") == 0) {
		return MemberType::FLOAT3;
	} else {
		LOG("No member of name %s exists.", (unsigned) name);
		assert(false); // ERROR: Invalid name
		return MemberType::UNKNOWN;
	}
}
