//===----------------------------------------------------------------------===//
//                         DuckDB
//
// common/types.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "common/assert.hpp"
#include "common/constants.hpp"
#include "common/enums.hpp"

#include <type_traits>

namespace duckdb {

class Serializer;
class Deserializer;

struct blob_t {
	char *data;
	uint64_t size;
};

//===--------------------------------------------------------------------===//
// Internal Types
//===--------------------------------------------------------------------===//
enum class TypeId : uint8_t {
	INVALID = 0,
	BOOLEAN = 1,    /* bool */
	TINYINT = 2,    /* int8_t */
	SMALLINT = 3,   /* int16_t */
	INTEGER = 4,    /* int32_t */
	BIGINT = 5,     /* int64_t */
	POINTER = 6,    /* uint64_t */
	FLOAT = 7,      /* float32_t */
	DOUBLE = 8,     /* float64_t */
	VARCHAR = 9,    /* char*, representing a null-terminated UTF-8 string */
	VARBINARY = 10  /* blob_t, representing arbitrary bytes */
};

//===--------------------------------------------------------------------===//
// SQL Types
//===--------------------------------------------------------------------===//
enum class SQLTypeId : uint8_t {
	INVALID = 0,
	BOOLEAN = 1,
	TINYINT = 2,
	SMALLINT = 3,
	INTEGER = 4,
	BIGINT = 5,
	DATE = 6,
	TIMESTAMP = 7,
	REAL = 8,
	DOUBLE = 9,
	DECIMAL = 10,
	VARCHAR = 11,
	CHAR = 12,
	VARBINARY = 13
};

struct SQLType {
	SQLTypeId id;
	uint16_t width;
	uint8_t scale;

	SQLType(SQLTypeId id, uint16_t width = 0, uint8_t scale = 0) : id(id), width(width), scale(scale) { }

	//! Serializes a SQLType to a stand-alone binary blob
	void Serialize(Serializer &serializer);
	//! Deserializes a blob back into an SQLType
	static SQLType Deserialize(Deserializer &source);
};

string SQLTypeIdToString(SQLTypeId type);
string SQLTypeToString(SQLType type);

TypeId GetInternalType(SQLType type);

//! Returns the TypeId for the given type
template <class T> TypeId GetTypeId() {
	if (std::is_same<T, bool>()) {
		return TypeId::BOOLEAN;
	} else if (std::is_same<T, int8_t>()) {
		return TypeId::TINYINT;
	} else if (std::is_same<T, int16_t>()) {
		return TypeId::SMALLINT;
	} else if (std::is_same<T, int32_t>()) {
		return TypeId::INTEGER;
	} else if (std::is_same<T, int64_t>()) {
		return TypeId::BIGINT;
	} else if (std::is_same<T, uint64_t>()) {
		return TypeId::POINTER;
	} else if (std::is_same<T, float>()) {
		return TypeId::FLOAT;
	} else if (std::is_same<T, double>()) {
		return TypeId::DOUBLE;
	} else if (std::is_same<T, const char *>() || std::is_same<T, char *>()) {
		return TypeId::VARCHAR;
	} else {
		return TypeId::INVALID;
	}
}

template <class T> bool IsValidType() {
	return GetTypeId<T>() != TypeId::INVALID;
}

string TypeIdToString(TypeId type);
string TypeIdToString(TypeId type);
size_t GetTypeIdSize(TypeId type);
bool TypeIsConstantSize(TypeId type);
bool TypeIsIntegral(TypeId type);
bool TypeIsNumeric(TypeId type);
bool TypeIsInteger(TypeId type);

template <class T> bool IsIntegerType() {
	return TypeIsIntegral(GetTypeId<T>());
}

}; // namespace duckdb
