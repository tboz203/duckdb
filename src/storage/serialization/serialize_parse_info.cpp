//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/parser/parsed_data/parse_info.hpp"
#include "duckdb/parser/parsed_data/alter_info.hpp"
#include "duckdb/parser/parsed_data/alter_table_info.hpp"
#include "duckdb/parser/parsed_data/comment_on_column_info.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"
#include "duckdb/parser/parsed_data/copy_database_info.hpp"
#include "duckdb/parser/parsed_data/copy_info.hpp"
#include "duckdb/parser/parsed_data/detach_info.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/parser/parsed_data/load_info.hpp"
#include "duckdb/parser/parsed_data/update_extensions_info.hpp"
#include "duckdb/parser/parsed_data/pragma_info.hpp"
#include "duckdb/parser/parsed_data/transaction_info.hpp"
#include "duckdb/parser/parsed_data/vacuum_info.hpp"

namespace duckdb {

void ParseInfo::Serialize(Serializer &serializer) const {
	serializer.WriteProperty<ParseInfoType>(100, "info_type", info_type);
}

unique_ptr<ParseInfo> ParseInfo::Deserialize(Deserializer &deserializer) {
	auto info_type = deserializer.ReadProperty<ParseInfoType>(100, "info_type");
	unique_ptr<ParseInfo> result;
	switch (info_type) {
	case ParseInfoType::ALTER_INFO:
		result = AlterInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::ATTACH_INFO:
		result = AttachInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::COPY_DATABASE_INFO:
		result = CopyDatabaseInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::COPY_INFO:
		result = CopyInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::DETACH_INFO:
		result = DetachInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::DROP_INFO:
		result = DropInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::LOAD_INFO:
		result = LoadInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::PRAGMA_INFO:
		result = PragmaInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::TRANSACTION_INFO:
		result = TransactionInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::UPDATE_EXTENSIONS_INFO:
		result = UpdateExtensionsInfo::Deserialize(deserializer);
		break;
	case ParseInfoType::VACUUM_INFO:
		result = VacuumInfo::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of ParseInfo!");
	}
	return result;
}

void AlterInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WriteProperty<AlterType>(200, "type", type);
	serializer.WritePropertyWithDefault<string>(201, "catalog", catalog);
	serializer.WritePropertyWithDefault<string>(202, "schema", schema);
	serializer.WritePropertyWithDefault<string>(203, "name", name);
	serializer.WriteProperty<OnEntryNotFound>(204, "if_not_found", if_not_found);
	serializer.WritePropertyWithDefault<bool>(205, "allow_internal", allow_internal);
}

unique_ptr<ParseInfo> AlterInfo::Deserialize(Deserializer &deserializer) {
	auto type = deserializer.ReadProperty<AlterType>(200, "type");
	auto catalog = deserializer.ReadPropertyWithDefault<string>(201, "catalog");
	auto schema = deserializer.ReadPropertyWithDefault<string>(202, "schema");
	auto name = deserializer.ReadPropertyWithDefault<string>(203, "name");
	auto if_not_found = deserializer.ReadProperty<OnEntryNotFound>(204, "if_not_found");
	auto allow_internal = deserializer.ReadPropertyWithDefault<bool>(205, "allow_internal");
	unique_ptr<AlterInfo> result;
	switch (type) {
	case AlterType::ALTER_TABLE:
		result = AlterTableInfo::Deserialize(deserializer);
		break;
	case AlterType::ALTER_VIEW:
		result = AlterViewInfo::Deserialize(deserializer);
		break;
	case AlterType::SET_COLUMN_COMMENT:
		result = SetColumnCommentInfo::Deserialize(deserializer);
		break;
	case AlterType::SET_COMMENT:
		result = SetCommentInfo::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of AlterInfo!");
	}
	result->catalog = std::move(catalog);
	result->schema = std::move(schema);
	result->name = std::move(name);
	result->if_not_found = if_not_found;
	result->allow_internal = allow_internal;
	return std::move(result);
}

void AlterTableInfo::Serialize(Serializer &serializer) const {
	AlterInfo::Serialize(serializer);
	serializer.WriteProperty<AlterTableType>(300, "alter_table_type", alter_table_type);
}

unique_ptr<AlterInfo> AlterTableInfo::Deserialize(Deserializer &deserializer) {
	auto alter_table_type = deserializer.ReadProperty<AlterTableType>(300, "alter_table_type");
	unique_ptr<AlterTableInfo> result;
	switch (alter_table_type) {
	case AlterTableType::ADD_COLUMN:
		result = AddColumnInfo::Deserialize(deserializer);
		break;
	case AlterTableType::ALTER_COLUMN_TYPE:
		result = ChangeColumnTypeInfo::Deserialize(deserializer);
		break;
	case AlterTableType::DROP_NOT_NULL:
		result = DropNotNullInfo::Deserialize(deserializer);
		break;
	case AlterTableType::FOREIGN_KEY_CONSTRAINT:
		result = AlterForeignKeyInfo::Deserialize(deserializer);
		break;
	case AlterTableType::REMOVE_COLUMN:
		result = RemoveColumnInfo::Deserialize(deserializer);
		break;
	case AlterTableType::RENAME_COLUMN:
		result = RenameColumnInfo::Deserialize(deserializer);
		break;
	case AlterTableType::RENAME_TABLE:
		result = RenameTableInfo::Deserialize(deserializer);
		break;
	case AlterTableType::SET_DEFAULT:
		result = SetDefaultInfo::Deserialize(deserializer);
		break;
	case AlterTableType::SET_NOT_NULL:
		result = SetNotNullInfo::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of AlterTableInfo!");
	}
	return std::move(result);
}

void AlterViewInfo::Serialize(Serializer &serializer) const {
	AlterInfo::Serialize(serializer);
	serializer.WriteProperty<AlterViewType>(300, "alter_view_type", alter_view_type);
}

unique_ptr<AlterInfo> AlterViewInfo::Deserialize(Deserializer &deserializer) {
	auto alter_view_type = deserializer.ReadProperty<AlterViewType>(300, "alter_view_type");
	unique_ptr<AlterViewInfo> result;
	switch (alter_view_type) {
	case AlterViewType::RENAME_VIEW:
		result = RenameViewInfo::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of AlterViewInfo!");
	}
	return std::move(result);
}

void AddColumnInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WriteProperty<ColumnDefinition>(400, "new_column", new_column);
	serializer.WritePropertyWithDefault<bool>(401, "if_column_not_exists", if_column_not_exists);
}

unique_ptr<AlterTableInfo> AddColumnInfo::Deserialize(Deserializer &deserializer) {
	auto new_column = deserializer.ReadProperty<ColumnDefinition>(400, "new_column");
	auto result = duckdb::unique_ptr<AddColumnInfo>(new AddColumnInfo(std::move(new_column)));
	deserializer.ReadPropertyWithDefault<bool>(401, "if_column_not_exists", result->if_column_not_exists);
	return std::move(result);
}

void AlterForeignKeyInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "fk_table", fk_table);
	serializer.WritePropertyWithDefault<vector<string>>(401, "pk_columns", pk_columns);
	serializer.WritePropertyWithDefault<vector<string>>(402, "fk_columns", fk_columns);
	serializer.WritePropertyWithDefault<vector<PhysicalIndex>>(403, "pk_keys", pk_keys);
	serializer.WritePropertyWithDefault<vector<PhysicalIndex>>(404, "fk_keys", fk_keys);
	serializer.WriteProperty<AlterForeignKeyType>(405, "alter_fk_type", type);
}

unique_ptr<AlterTableInfo> AlterForeignKeyInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<AlterForeignKeyInfo>(new AlterForeignKeyInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "fk_table", result->fk_table);
	deserializer.ReadPropertyWithDefault<vector<string>>(401, "pk_columns", result->pk_columns);
	deserializer.ReadPropertyWithDefault<vector<string>>(402, "fk_columns", result->fk_columns);
	deserializer.ReadPropertyWithDefault<vector<PhysicalIndex>>(403, "pk_keys", result->pk_keys);
	deserializer.ReadPropertyWithDefault<vector<PhysicalIndex>>(404, "fk_keys", result->fk_keys);
	deserializer.ReadProperty<AlterForeignKeyType>(405, "alter_fk_type", result->type);
	return std::move(result);
}

void AttachInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WritePropertyWithDefault<string>(201, "path", path);
	serializer.WritePropertyWithDefault<unordered_map<string, Value>>(202, "options", options);
	serializer.WritePropertyWithDefault<OnCreateConflict>(203, "on_conflict", on_conflict, OnCreateConflict::ERROR_ON_CONFLICT);
}

unique_ptr<ParseInfo> AttachInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<AttachInfo>(new AttachInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadPropertyWithDefault<string>(201, "path", result->path);
	deserializer.ReadPropertyWithDefault<unordered_map<string, Value>>(202, "options", result->options);
	deserializer.ReadPropertyWithDefault<OnCreateConflict>(203, "on_conflict", result->on_conflict, OnCreateConflict::ERROR_ON_CONFLICT);
	return std::move(result);
}

void ChangeColumnTypeInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "column_name", column_name);
	serializer.WriteProperty<LogicalType>(401, "target_type", target_type);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(402, "expression", expression);
}

unique_ptr<AlterTableInfo> ChangeColumnTypeInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<ChangeColumnTypeInfo>(new ChangeColumnTypeInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "column_name", result->column_name);
	deserializer.ReadProperty<LogicalType>(401, "target_type", result->target_type);
	deserializer.ReadPropertyWithDefault<unique_ptr<ParsedExpression>>(402, "expression", result->expression);
	return std::move(result);
}

void CopyDatabaseInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "target_database", target_database);
	serializer.WritePropertyWithDefault<vector<unique_ptr<CreateInfo>>>(201, "entries", entries);
}

unique_ptr<ParseInfo> CopyDatabaseInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CopyDatabaseInfo>(new CopyDatabaseInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "target_database", result->target_database);
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<CreateInfo>>>(201, "entries", result->entries);
	return std::move(result);
}

void CopyInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "catalog", catalog);
	serializer.WritePropertyWithDefault<string>(201, "schema", schema);
	serializer.WritePropertyWithDefault<string>(202, "table", table);
	serializer.WritePropertyWithDefault<vector<string>>(203, "select_list", select_list);
	serializer.WritePropertyWithDefault<bool>(204, "is_from", is_from);
	serializer.WritePropertyWithDefault<string>(205, "format", format);
	serializer.WritePropertyWithDefault<string>(206, "file_path", file_path);
	serializer.WritePropertyWithDefault<case_insensitive_map_t<vector<Value>>>(207, "options", options);
}

unique_ptr<ParseInfo> CopyInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CopyInfo>(new CopyInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "catalog", result->catalog);
	deserializer.ReadPropertyWithDefault<string>(201, "schema", result->schema);
	deserializer.ReadPropertyWithDefault<string>(202, "table", result->table);
	deserializer.ReadPropertyWithDefault<vector<string>>(203, "select_list", result->select_list);
	deserializer.ReadPropertyWithDefault<bool>(204, "is_from", result->is_from);
	deserializer.ReadPropertyWithDefault<string>(205, "format", result->format);
	deserializer.ReadPropertyWithDefault<string>(206, "file_path", result->file_path);
	deserializer.ReadPropertyWithDefault<case_insensitive_map_t<vector<Value>>>(207, "options", result->options);
	return std::move(result);
}

void DetachInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WriteProperty<OnEntryNotFound>(201, "if_not_found", if_not_found);
}

unique_ptr<ParseInfo> DetachInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<DetachInfo>(new DetachInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadProperty<OnEntryNotFound>(201, "if_not_found", result->if_not_found);
	return std::move(result);
}

void DropInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WriteProperty<CatalogType>(200, "type", type);
	serializer.WritePropertyWithDefault<string>(201, "catalog", catalog);
	serializer.WritePropertyWithDefault<string>(202, "schema", schema);
	serializer.WritePropertyWithDefault<string>(203, "name", name);
	serializer.WriteProperty<OnEntryNotFound>(204, "if_not_found", if_not_found);
	serializer.WritePropertyWithDefault<bool>(205, "cascade", cascade);
	serializer.WritePropertyWithDefault<bool>(206, "allow_drop_internal", allow_drop_internal);
	serializer.WritePropertyWithDefault<unique_ptr<ExtraDropInfo>>(207, "extra_drop_info", extra_drop_info);
}

unique_ptr<ParseInfo> DropInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<DropInfo>(new DropInfo());
	deserializer.ReadProperty<CatalogType>(200, "type", result->type);
	deserializer.ReadPropertyWithDefault<string>(201, "catalog", result->catalog);
	deserializer.ReadPropertyWithDefault<string>(202, "schema", result->schema);
	deserializer.ReadPropertyWithDefault<string>(203, "name", result->name);
	deserializer.ReadProperty<OnEntryNotFound>(204, "if_not_found", result->if_not_found);
	deserializer.ReadPropertyWithDefault<bool>(205, "cascade", result->cascade);
	deserializer.ReadPropertyWithDefault<bool>(206, "allow_drop_internal", result->allow_drop_internal);
	deserializer.ReadPropertyWithDefault<unique_ptr<ExtraDropInfo>>(207, "extra_drop_info", result->extra_drop_info);
	return std::move(result);
}

void DropNotNullInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "column_name", column_name);
}

unique_ptr<AlterTableInfo> DropNotNullInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<DropNotNullInfo>(new DropNotNullInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "column_name", result->column_name);
	return std::move(result);
}

void LoadInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "filename", filename);
	serializer.WriteProperty<LoadType>(201, "load_type", load_type);
	serializer.WritePropertyWithDefault<string>(202, "repository", repository);
	serializer.WritePropertyWithDefault<string>(203, "version", version);
}

unique_ptr<ParseInfo> LoadInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<LoadInfo>(new LoadInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "filename", result->filename);
	deserializer.ReadProperty<LoadType>(201, "load_type", result->load_type);
	deserializer.ReadPropertyWithDefault<string>(202, "repository", result->repository);
	deserializer.ReadPropertyWithDefault<string>(203, "version", result->version);
	return std::move(result);
}

void PragmaInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(201, "parameters", parameters);
	serializer.WritePropertyWithDefault<case_insensitive_map_t<unique_ptr<ParsedExpression>>>(202, "named_parameters", named_parameters);
}

unique_ptr<ParseInfo> PragmaInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<PragmaInfo>(new PragmaInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(201, "parameters", result->parameters);
	deserializer.ReadPropertyWithDefault<case_insensitive_map_t<unique_ptr<ParsedExpression>>>(202, "named_parameters", result->named_parameters);
	return std::move(result);
}

void RemoveColumnInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "removed_column", removed_column);
	serializer.WritePropertyWithDefault<bool>(401, "if_column_exists", if_column_exists);
	serializer.WritePropertyWithDefault<bool>(402, "cascade", cascade);
}

unique_ptr<AlterTableInfo> RemoveColumnInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<RemoveColumnInfo>(new RemoveColumnInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "removed_column", result->removed_column);
	deserializer.ReadPropertyWithDefault<bool>(401, "if_column_exists", result->if_column_exists);
	deserializer.ReadPropertyWithDefault<bool>(402, "cascade", result->cascade);
	return std::move(result);
}

void RenameColumnInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "old_name", old_name);
	serializer.WritePropertyWithDefault<string>(401, "new_name", new_name);
}

unique_ptr<AlterTableInfo> RenameColumnInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<RenameColumnInfo>(new RenameColumnInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "old_name", result->old_name);
	deserializer.ReadPropertyWithDefault<string>(401, "new_name", result->new_name);
	return std::move(result);
}

void RenameTableInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "new_table_name", new_table_name);
}

unique_ptr<AlterTableInfo> RenameTableInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<RenameTableInfo>(new RenameTableInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "new_table_name", result->new_table_name);
	return std::move(result);
}

void RenameViewInfo::Serialize(Serializer &serializer) const {
	AlterViewInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "new_view_name", new_view_name);
}

unique_ptr<AlterViewInfo> RenameViewInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<RenameViewInfo>(new RenameViewInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "new_view_name", result->new_view_name);
	return std::move(result);
}

void SetColumnCommentInfo::Serialize(Serializer &serializer) const {
	AlterInfo::Serialize(serializer);
	serializer.WriteProperty<CatalogType>(300, "catalog_entry_type", catalog_entry_type);
	serializer.WriteProperty<Value>(301, "comment_value", comment_value);
	serializer.WritePropertyWithDefault<string>(302, "column_name", column_name);
}

unique_ptr<AlterInfo> SetColumnCommentInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SetColumnCommentInfo>(new SetColumnCommentInfo());
	deserializer.ReadProperty<CatalogType>(300, "catalog_entry_type", result->catalog_entry_type);
	deserializer.ReadProperty<Value>(301, "comment_value", result->comment_value);
	deserializer.ReadPropertyWithDefault<string>(302, "column_name", result->column_name);
	return std::move(result);
}

void SetCommentInfo::Serialize(Serializer &serializer) const {
	AlterInfo::Serialize(serializer);
	serializer.WriteProperty<CatalogType>(300, "entry_catalog_type", entry_catalog_type);
	serializer.WriteProperty<Value>(301, "comment_value", comment_value);
}

unique_ptr<AlterInfo> SetCommentInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SetCommentInfo>(new SetCommentInfo());
	deserializer.ReadProperty<CatalogType>(300, "entry_catalog_type", result->entry_catalog_type);
	deserializer.ReadProperty<Value>(301, "comment_value", result->comment_value);
	return std::move(result);
}

void SetDefaultInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "column_name", column_name);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(401, "expression", expression);
}

unique_ptr<AlterTableInfo> SetDefaultInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SetDefaultInfo>(new SetDefaultInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "column_name", result->column_name);
	deserializer.ReadPropertyWithDefault<unique_ptr<ParsedExpression>>(401, "expression", result->expression);
	return std::move(result);
}

void SetNotNullInfo::Serialize(Serializer &serializer) const {
	AlterTableInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(400, "column_name", column_name);
}

unique_ptr<AlterTableInfo> SetNotNullInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SetNotNullInfo>(new SetNotNullInfo());
	deserializer.ReadPropertyWithDefault<string>(400, "column_name", result->column_name);
	return std::move(result);
}

void TransactionInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WriteProperty<TransactionType>(200, "type", type);
}

unique_ptr<ParseInfo> TransactionInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<TransactionInfo>(new TransactionInfo());
	deserializer.ReadProperty<TransactionType>(200, "type", result->type);
	return std::move(result);
}

void UpdateExtensionsInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<string>>(200, "extensions_to_update", extensions_to_update);
}

unique_ptr<ParseInfo> UpdateExtensionsInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<UpdateExtensionsInfo>(new UpdateExtensionsInfo());
	deserializer.ReadPropertyWithDefault<vector<string>>(200, "extensions_to_update", result->extensions_to_update);
	return std::move(result);
}

void VacuumInfo::Serialize(Serializer &serializer) const {
	ParseInfo::Serialize(serializer);
	serializer.WriteProperty<VacuumOptions>(200, "options", options);
	serializer.WritePropertyWithDefault<bool>(201, "has_table", has_table);
	serializer.WritePropertyWithDefault<unique_ptr<TableRef>>(202, "ref", ref);
	serializer.WritePropertyWithDefault<vector<string>>(203, "columns", columns);
}

unique_ptr<ParseInfo> VacuumInfo::Deserialize(Deserializer &deserializer) {
	auto options = deserializer.ReadProperty<VacuumOptions>(200, "options");
	auto result = duckdb::unique_ptr<VacuumInfo>(new VacuumInfo(options));
	deserializer.ReadPropertyWithDefault<bool>(201, "has_table", result->has_table);
	deserializer.ReadPropertyWithDefault<unique_ptr<TableRef>>(202, "ref", result->ref);
	deserializer.ReadPropertyWithDefault<vector<string>>(203, "columns", result->columns);
	return std::move(result);
}

} // namespace duckdb
