//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parser/expression/function_expression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "parser/parsed_expression.hpp"

namespace duckdb {
//! Represents a function call
class FunctionExpression : public ParsedExpression {
public:
	FunctionExpression(string schema_name, string function_name, vector<unique_ptr<ParsedExpression>> &children);
	FunctionExpression(string function_name, vector<unique_ptr<ParsedExpression>> &children);

	//! Schema of the function
	string schema;
	//! Function name
	string function_name;
	//! List of arguments to the function
	vector<unique_ptr<ParsedExpression>> children;
public:
	string ToString() const override;

	unique_ptr<ParsedExpression> Copy() override;

	bool Equals(const ParsedExpression *other) const override;
	uint64_t Hash() const override;

	//! Serializes a FunctionExpression to a stand-alone binary blob
	void Serialize(Serializer &serializer) override;
	//! Deserializes a blob back into an FunctionExpression
	static unique_ptr<ParsedExpression> Deserialize(ExpressionType type, Deserializer &source);

	size_t ChildCount() const override;
	ParsedExpression *GetChild(size_t index) const override;
	void ReplaceChild(std::function<unique_ptr<ParsedExpression>(unique_ptr<ParsedExpression> expression)> callback,
	                  size_t index) override;
};
} // namespace duckdb
