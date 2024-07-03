#include "duckdb/function/cast/default_casts.hpp"
#include "duckdb/common/operator/cast_operators.hpp"
#include "duckdb/function/cast/vector_cast_helpers.hpp"

namespace duckdb {

constexpr uint8_t VARINT_HEADER_SIZE = 3;

void SetHeader(char *blob, uint64_t number_of_bytes, bool is_negative) {
	uint32_t header = static_cast<uint32_t>(number_of_bytes);
	// Set MSBit of 3rd byte
	header |= 0x00800000;
	if (is_negative) {
		header = ~header;
	}
	// we ignore MSByte  of header.
	// write the 3 bytes to blob.
	blob[0] = static_cast<char>(header >> 16);
	blob[1] = static_cast<char>(header >> 8 & 0xFF);
	blob[2] = static_cast<char>(header & 0xFF);
}

template <class T>
string_t IntToVarInt(Vector &result, T int_value) {
	// Determine if the number is negative
	bool is_negative = int_value < 0;
	// Determine the number of data bytes
	uint64_t abs_value;
	if (is_negative) {
		abs_value = static_cast<uint64_t>(std::abs(static_cast<int64_t>(int_value)));
	} else {
		abs_value = static_cast<uint64_t>(int_value);
	}
	uint32_t data_byte_size;
	if (abs_value != NumericLimits<uint64_t>::Maximum()) {
		data_byte_size = (abs_value == 0) ? 1 : static_cast<uint32_t>(std::ceil(std::log2(abs_value + 1) / 8.0));
	} else {
		data_byte_size = static_cast<uint32_t>(std::ceil(std::log2(abs_value) / 8.0));
	}

	uint32_t blob_size = data_byte_size + VARINT_HEADER_SIZE;
	auto blob = StringVector::EmptyString(result, blob_size);
	auto writable_blob = blob.GetDataWriteable();
	SetHeader(writable_blob, data_byte_size, is_negative);

	// Add data bytes to the blob, starting off after header bytes
	idx_t wb_idx = VARINT_HEADER_SIZE;
	for (int i = static_cast<int>(data_byte_size) - 1; i >= 0; --i) {
		if (is_negative) {
			writable_blob[wb_idx++] = ~static_cast<char>(abs_value >> i * 8 & 0xFF);
		} else {
			writable_blob[wb_idx++] = static_cast<char>(abs_value >> i * 8 & 0xFF);
		}
	}
	blob.Finalize();
	return blob;
}

template <class T>
string_t HugeintToVarInt(Vector &result, T int_value) {
	// Determine if the number is negative
	bool is_negative = int_value.upper < 0;
	// Determine the number of data bytes
	uint64_t abs_value_upper;
	if (is_negative) {
		abs_value_upper = static_cast<uint64_t>(std::abs(static_cast<int64_t>(int_value.upper)));
	} else {
		abs_value_upper = static_cast<uint64_t>(int_value.upper);
	}
	uint32_t data_byte_size;
	if (abs_value_upper != NumericLimits<uint64_t>::Maximum()) {
		data_byte_size =
		    (abs_value_upper == 0) ? 0 : static_cast<uint32_t>(std::ceil(std::log2(abs_value_upper + 1) / 8.0));
	} else {
		data_byte_size = static_cast<uint32_t>(std::ceil(std::log2(abs_value_upper) / 8.0));
	}

	uint32_t upper_byte_size = data_byte_size;
	if (abs_value_upper != NumericLimits<uint64_t>::Maximum()) {
		data_byte_size += static_cast<uint32_t>(std::ceil(std::log2(int_value.lower + 1) / 8.0));
	} else {
		data_byte_size += static_cast<uint32_t>(std::ceil(std::log2(int_value.lower) / 8.0));
	}
	if (data_byte_size == 0) {
		data_byte_size++;
	}
	uint32_t blob_size = data_byte_size + VARINT_HEADER_SIZE;
	auto blob = StringVector::EmptyString(result, blob_size);
	auto writable_blob = blob.GetDataWriteable();
	SetHeader(writable_blob, data_byte_size, is_negative);

	// Add data bytes to the blob, starting off after header bytes
	idx_t wb_idx = VARINT_HEADER_SIZE;
	for (int i = static_cast<int>(upper_byte_size) - 1; i >= 0; --i) {
		if (is_negative) {
			writable_blob[wb_idx++] = ~static_cast<char>(abs_value_upper >> i * 8 & 0xFF);
		} else {
			writable_blob[wb_idx++] = static_cast<char>(abs_value_upper >> i * 8 & 0xFF);
		}
	}
	for (int i = static_cast<int>(data_byte_size - upper_byte_size) - 1; i >= 0; --i) {
		if (is_negative) {
			writable_blob[wb_idx++] = ~static_cast<char>(int_value.lower >> i * 8 & 0xFF);
		} else {
			writable_blob[wb_idx++] = static_cast<char>(int_value.lower >> i * 8 & 0xFF);
		}
	}
	blob.Finalize();
	return blob;
}

int CharToDigit(char c) {
	return c - '0';
}

char DigitToChar(int digit) {
	return static_cast<char>(digit) + '0';
}

// Function to prepare a varchar for conversion
// We trim zero's, check for negative values, and what-not
void VarcharFormatting(string_t &value, idx_t &start_pos, idx_t &end_pos, bool &is_negative, bool &is_zero) {
	// If it's empty we error
	if (value.Empty()) {
		throw ConversionException("Could not convert string '%s' to VARINT", value.GetString());
	}
	start_pos = 0;
	is_zero = false;

	auto int_value_char = value.GetData();
	end_pos = value.GetSize();

	// If first character is -, we have a negative number, if + we have a + number
	is_negative = int_value_char[0] == '-';
	if (is_negative) {
		start_pos++;
	}
	if (int_value_char[0] == '+') {
		start_pos++;
	}
	// Now lets trim 0s
	bool at_least_one_zero = false;
	while (start_pos < end_pos && int_value_char[start_pos] == '0') {
		start_pos++;
		at_least_one_zero = true;
	}
	if (start_pos == end_pos) {
		if (at_least_one_zero) {
			// This is a 0 value
			is_zero = true;
			return;
		}
		// This is either a '+' or '-'. Hence invalid.
		throw ConversionException("Could not convert string '%s' to VARINT", value.GetString());
	}
	idx_t cur_pos = start_pos;
	// Verify all is numeric
	while (cur_pos < end_pos && std::isdigit(int_value_char[cur_pos])) {
		cur_pos++;
	}
	if (cur_pos < end_pos) {
		idx_t possible_end = cur_pos;
		// Oh oh, this is not a digit, if it's a . we might be fine, otherwise, this is invalid.
		if (int_value_char[cur_pos] == '.') {
			cur_pos++;
		} else {
			throw ConversionException("Could not convert string '%s' to VARINT", value.GetString());
		}

		while (cur_pos < end_pos) {
			if (std::isdigit(int_value_char[cur_pos])) {
				cur_pos++;
			} else {
				// By now we can only have numbers, otherwise this is invalid.
				throw ConversionException("Could not convert string '%s' to VARINT", value.GetString());
			}
		}
		// Floor cast this boy
		end_pos = possible_end;
	}
}

string_t DoubleToVarInt(Vector &result, double double_value) {
	vector<char> value;
	while (double_value > 0) {
		value.push_back(static_cast<uint8_t>(std::fmod(double_value, 256)));
		double_value = floor(double_value / 256);
	}

	uint32_t data_byte_size = value.size();

	uint32_t blob_size = data_byte_size + VARINT_HEADER_SIZE;
	auto blob = StringVector::EmptyString(result, blob_size);
	auto writable_blob = blob.GetDataWriteable();
	SetHeader(writable_blob, data_byte_size, false);

	// Add data bytes to the blob, starting off after header bytes
	idx_t wb_idx = VARINT_HEADER_SIZE;
	for (int i = static_cast<int>(data_byte_size) - 1; i >= 0; --i) {
		// if (false) {
		// 	writable_blob[wb_idx++] = ~static_cast<char>(abs_value >> i * 8 & 0xFF);
		// } else {
		writable_blob[wb_idx++] = static_cast<char>(value[i]);
		// }
	}
	blob.Finalize();
	return blob;
}

string_t VarcharToVarInt(Vector &result, string_t int_value) {
	idx_t start_pos, end_pos;
	bool is_negative, is_zero;
	VarcharFormatting(int_value, start_pos, end_pos, is_negative, is_zero);
	if (is_zero) {
		// return zero
		uint32_t blob_size = 1 + VARINT_HEADER_SIZE;
		auto blob = StringVector::EmptyString(result, blob_size);
		auto writable_blob = blob.GetDataWriteable();
		SetHeader(writable_blob, 1, false);
		writable_blob[3] = 0;
		blob.Finalize();
		return blob;
	}

	auto int_value_char = int_value.GetData();
	idx_t actual_size = end_pos - start_pos;
	// convert the string to a byte array
	string abs_str(int_value_char + start_pos, actual_size);
	string blob_string;
	while (!abs_str.empty()) {
		uint8_t remainder = 0;
		std::string quotient;
		// We convert ze string to a big-endian byte array by dividing the number by 256 and storing the remainders
		for (char digit : abs_str) {
			int new_value = remainder * 10 + CharToDigit(digit);
			if (new_value / 256 > 0) {
				quotient += to_string(new_value / 256);
			} else if (!quotient.empty()) {
				quotient += '0';
			}
			remainder = static_cast<uint8_t>(new_value % 256);
		}
		if (is_negative && int_value_char[start_pos] != '0') {
			blob_string.push_back(static_cast<char>(~remainder));
		} else {
			blob_string.push_back(static_cast<char>(remainder));
		}

		// Remove leading zeros from the quotient
		abs_str = quotient;
	}
	uint32_t blob_size = static_cast<uint32_t>(blob_string.size() + VARINT_HEADER_SIZE);
	auto blob = StringVector::EmptyString(result, blob_size);
	auto writable_blob = blob.GetDataWriteable();

	SetHeader(writable_blob, blob_string.size(), is_negative);

	// Write string_blob into blob
	idx_t blob_string_idx = blob_string.size() - 1;
	for (idx_t i = VARINT_HEADER_SIZE; i < blob_size; i++) {
		writable_blob[i] = blob_string[blob_string_idx--];
	}
	blob.Finalize();
	return blob;
}

// Function to convert VARINT blob to a VARCHAR
string VarIntToVarchar(const string_t &blob) {
	if (blob.GetSize() < 4) {
		throw InvalidInputException("Invalid blob size.");
	}
	auto blob_ptr = blob.GetData();
	std::string decimal_string;
	std::vector<uint8_t> temp_array;
	// Determine if the number is negative
	bool is_negative = (blob_ptr[0] & 0x80) == 0;
	for (idx_t i = 3; i < blob.GetSize(); i++) {
		if (is_negative) {
			temp_array.push_back(static_cast<uint8_t>(~blob_ptr[i]));
		} else {
			temp_array.push_back(static_cast<uint8_t>(blob_ptr[i]));
		}
	}
	std::reverse(decimal_string.begin(), decimal_string.end());

	while (!temp_array.empty()) {
		std::string quotient;
		uint8_t remainder = 0;
		for (uint8_t byte : temp_array) {
			int new_value = remainder * 256 + byte;
			quotient += DigitToChar(new_value / 10);
			remainder = static_cast<uint8_t>(new_value % 10);
		}

		decimal_string += DigitToChar(remainder);

		// Remove leading zeros from the quotient
		temp_array.clear();
		for (char digit : quotient) {
			if (digit != '0' || !temp_array.empty()) {
				temp_array.push_back(static_cast<uint8_t>(CharToDigit(digit)));
			}
		}
	}

	if (is_negative) {
		decimal_string += '-';
	}

	// Reverse the string to get the correct decimal representation
	std::reverse(decimal_string.begin(), decimal_string.end());
	return decimal_string;
}

struct IntTryCastToVarInt {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		return IntToVarInt(result, input);
	}
};

struct HugeintTryCastToVarInt {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		return HugeintToVarInt(result, input);
	}
};

struct DoubleTryCastToVarInt {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		return DoubleToVarInt(result, input);
	}
};

struct VarcharTryCastToVarInt {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		return VarcharToVarInt(result, input);
	}
};

struct VarIntTryCastToVarchar {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		return StringVector::AddStringOrBlob(result, VarIntToVarchar(input));
	}
};

BoundCastInfo DefaultCasts::ToVarintCastSwitch(BindCastInput &input, const LogicalType &source,
                                               const LogicalType &target) {
	D_ASSERT(target.id() == LogicalTypeId::VARINT);
	// now switch on the result type
	switch (source.id()) {
	case LogicalTypeId::TINYINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<int8_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::UTINYINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<uint8_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::SMALLINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<int16_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::USMALLINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<uint16_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::INTEGER:
		return BoundCastInfo(&VectorCastHelpers::StringCast<int32_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::UINTEGER:
		return BoundCastInfo(&VectorCastHelpers::StringCast<uint32_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::BIGINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<int64_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::UBIGINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<uint64_t, duckdb::IntTryCastToVarInt>);
	case LogicalTypeId::VARCHAR:
		return BoundCastInfo(&VectorCastHelpers::StringCast<string_t, duckdb::VarcharTryCastToVarInt>);
	case LogicalTypeId::UHUGEINT:
		return BoundCastInfo(&VectorCastHelpers::StringCast<uhugeint_t, duckdb::HugeintTryCastToVarInt>);
	case LogicalTypeId::DOUBLE:
		return BoundCastInfo(&VectorCastHelpers::StringCast<double, duckdb::DoubleTryCastToVarInt>);
	case LogicalTypeId::HUGEINT:
	case LogicalTypeId::DECIMAL:

	case LogicalTypeId::FLOAT:
	default:
		return TryVectorNullCast;
	}
}

BoundCastInfo DefaultCasts::VarintCastSwitch(BindCastInput &input, const LogicalType &source,
                                             const LogicalType &target) {
	D_ASSERT(source.id() == LogicalTypeId::VARINT);
	// now switch on the result type
	switch (target.id()) {
	case LogicalTypeId::VARCHAR:
		return BoundCastInfo(&VectorCastHelpers::StringCast<string_t, duckdb::VarIntTryCastToVarchar>);
	default:
		return TryVectorNullCast;
	}
}

} // namespace duckdb
