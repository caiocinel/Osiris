//https://developers.google.com/protocol-buffers/docs/encoding
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#define MAKE_TAG(FIELD_NUMBER, TYPE) (uint32_t)(((FIELD_NUMBER) << kTagTypeBits) | (TYPE))

#define make_struct(_name_, _size_) \
	struct _name_ : proto_writer { \
		constexpr static size_t MAX_FIELD = _size_; \
		_name_()						: proto_writer(MAX_FIELD) {} \
		_name_(void* data, size_t size) : proto_writer(data, size, MAX_FIELD) {} \
		_name_(std::string data)        : proto_writer(data, MAX_FIELD) {}

#define make_field(_name_, _id_, _type_) \
	constexpr static tag _name_ = { _id_, _type_ }; \
	void clear_##_name_() { this->clear(_name_); } \
	bool has_##_name_() { return this->has(_name_); } \
	field get_##_name_() { return this->get(_name_); } \
	std::vector<field> getAll_##_name_() { return this->getAll(_name_); } \
	\
	void add_##_name_(std::string v) { this->add(_name_, v); } \
	template<typename T> void add_##_name_(T v) { this->add(_name_, v); } \
	void replace_##_name_(std::string v) { this->replace(_name_, v); } \
	void replace_##_name_(std::string v, uint32_t index) { this->replace(_name_, v, index); } \
	template<typename T> void replace_##_name_(T v) { this->replace(_name_, v); } \
	template<typename T> void replace_##_name_(T v, uint32_t index) { this->replace(_name_, v, index); } \
	template<class T> T get_##_name_() { return std::move( T(this->get(_name_)._string()) ); }

struct tag {
	unsigned field;
	unsigned type;
};

enum field_type {
	TYPE_DOUBLE = 1,
	TYPE_FLOAT = 2,
	TYPE_INT64 = 3,
	TYPE_UINT64 = 4,
	TYPE_INT32 = 5,
	TYPE_FIXED64 = 6,
	TYPE_FIXED32 = 7,
	TYPE_BOOL = 8,
	TYPE_STRING = 9,
	TYPE_GROUP = 10,
	TYPE_MESSAGE = 11,
	TYPE_BYTES = 12,
	TYPE_UINT32 = 13,
	TYPE_ENUM = 14,
	TYPE_SFIXED32 = 15,
	TYPE_SFIXED64 = 16,
	TYPE_SINT32 = 17,
	TYPE_SINT64 = 18,
	MAX_FIELD_TYPE = 18,
};

struct field {
	friend class proto_writer;
public:
	field& operator=(const field& f);

	field() : _tag({ 0,0 }), value(""), full("") { }
	field(unsigned field, unsigned type, std::string value, std::string full);

	field(tag _tag, std::string value);
	field(unsigned field, unsigned type, std::string value);

	template<typename T>
	field(tag _tag, T value);

	template<typename T>
	field(unsigned field, unsigned type, T value);

public:
	static field read_field(void* data, size_t& bytesRead);

public:
	float _float();
	double _double();
	int32_t get_int32();
	int64_t get_int64();
	uint32_t _uint32();
	uint64_t _uint64();
	uint32_t _fixed32();
	uint64_t _fixed64();
	int32_t _sfixed32();
	int64_t _sfixed64();
	bool _bool();
	std::string _string();

private:
	tag _tag;
	std::string value;
	std::string full;

	static std::string get_bytes_varint32(uint32_t value);
	static std::string get_bytes_varint64(uint64_t value);
	static uint32_t read_var_uint32(void* data, size_t& bytesRead);
	static uint64_t read_var_uint64(void* data, size_t& bytesRead);

	enum wire_type {
		WIRETYPE_VARINT = 0,
		WIRETYPE_FIXED64 = 1,
		WIRETYPE_LENGTH_DELIMITED = 2,
		WIRETYPE_START_GROUP = 3,
		WIRETYPE_END_GROUP = 4,
		WIRETYPE_FIXED32 = 5,
	};

	constexpr static wire_type kWireTypeForFieldType[19] = {
		(wire_type)(-1),  // invalid
		WIRETYPE_FIXED64,           // TYPE_DOUBLE
		WIRETYPE_FIXED32,           // TYPE_FLOAT
		WIRETYPE_VARINT,            // TYPE_INT64
		WIRETYPE_VARINT,            // TYPE_UINT64
		WIRETYPE_VARINT,            // TYPE_INT32
		WIRETYPE_FIXED64,           // TYPE_FIXED64
		WIRETYPE_FIXED32,           // TYPE_FIXED32
		WIRETYPE_VARINT,            // TYPE_BOOL
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_STRING
		WIRETYPE_START_GROUP,       // TYPE_GROUP
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_MESSAGE
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_BYTES
		WIRETYPE_VARINT,            // TYPE_UINT32
		WIRETYPE_VARINT,            // TYPE_ENUM
		WIRETYPE_FIXED32,           // TYPE_SFIXED32
		WIRETYPE_FIXED64,           // TYPE_SFIXED64
		WIRETYPE_VARINT,            // TYPE_SINT32
		WIRETYPE_VARINT,            // TYPE_SINT64
	};

	constexpr static int kTagTypeBits = 3;
	constexpr static uint32_t kTagTypeMask = (1 << kTagTypeBits) - 1;
	constexpr static int kMaxVarintBytes = 10;
	constexpr static int kMaxVarint32Bytes = 5;
};

class proto_writer {
public:
	proto_writer();
	proto_writer(size_t maxFields);
	proto_writer(void* data, size_t size, size_t maxFields);
	proto_writer(std::string dataStr, size_t maxFields);

public:
	/* Old method */
	void add(field field);
	void replace(field field);
	void replace(field field, uint32_t index);
	void clear(unsigned fieldId);
	bool has(unsigned fieldId);
	field get(unsigned fieldId);
	std::vector<field> getAll(unsigned fieldId);

public:
	/* New method */
	void add(tag _tag, std::string value);
	void replace(tag _tag, std::string value);
	void replace(tag _tag, std::string value, uint32_t index);
	void clear(tag _tag);
	bool has(tag _tag);
	field get(tag _tag);
	std::vector<field> getAll(tag _tag);

	template<typename T>
	void add(tag _tag, T value);

	template<typename T>
	void replace(tag _tag, T value);

	template<typename T>
	void replace(tag _tag, T value, uint32_t index);


	std::string serialize();
	void print();

private:
	std::vector<std::vector<field>> fields;
};

#pragma region Helper Functions
std::string field::get_bytes_varint32(uint32_t value) {
	uint8_t bytes[kMaxVarint32Bytes];
	int size = 0;
	while (value > 0x7F) {
		bytes[size++] = ((uint8_t)(value) & 0x7F) | 0x80;
		value >>= 7;
	}
	bytes[size++] = (uint8_t)(value) & 0x7F;
	return std::string{ (const char*)(&bytes[0]), (size_t)size };
}

std::string field::get_bytes_varint64(uint64_t value) {
	uint8_t bytes[kMaxVarintBytes];
	int size = 0;
	while (value > 0x7F) {
		bytes[size++] = ((uint8_t)(value) & 0x7F) | 0x80;
		value >>= 7;
	}
	bytes[size++] = (uint8_t)(value) & 0x7F;
	return std::string{ (const char*)(&bytes[0]), (size_t)size };
}

uint32_t field::read_var_uint32(void* data, size_t& bytesRead) {
	auto ptr = (const uint8_t*)(data);
	auto value = 0u;
	auto bytes = 0u;

	do {
		value |= (uint32_t)(*ptr & 0x7f) << (7 * bytes);
		bytes++;
	} while (*(ptr++) & 0x80 && bytes <= 5);

	bytesRead = bytes;
	return value;
}

uint64_t field::read_var_uint64(void* data, size_t& bytesRead) {
	auto ptr = (const uint8_t*)(data);
	auto value = 0ull;
	auto bytes = 0u;

	do
	{
		value |= (uint64_t)(*ptr & 0x7f) << (7 * bytes);
		bytes++;
	} while (*(ptr++) & 0x80 && bytes <= 10);

	bytesRead = bytes;

	return value;
}

field field::read_field(void* data, size_t& bytesRead) {
	unsigned _field = *(uint16_t*)(data);
	unsigned type = _field & kTagTypeMask;

	if (_field == 0xffff) {
		bytesRead = 0;
		return field();
	}

	if (_field & 0x80) {
		_field = ((_field & 0x7f) | ((_field & 0xff00) >> 1)) >> kTagTypeBits;
		bytesRead = 2;
	} else {
		_field = (_field & 0xff) >> kTagTypeBits;
		bytesRead = 1;
	}

	size_t length, sizeDelimited;
	std::string value, full;
	switch (type)
	{
	case WIRETYPE_VARINT:
		read_var_uint64((void*)((ptrdiff_t)data + bytesRead), length);
		value = std::string{ (const char*)((void*)((ptrdiff_t)data + bytesRead)), length };
		full = std::string{ (const char*)(data), bytesRead + length };
		bytesRead += length;
		break;
	case WIRETYPE_FIXED64:
		value = std::string{ (const char*)((void*)((ptrdiff_t)data + bytesRead)), 8 };
		full = std::string{ (const char*)(data), bytesRead + 8 };
		bytesRead += 8;
		break;
	case WIRETYPE_LENGTH_DELIMITED:
		sizeDelimited = read_var_uint32((void*)((ptrdiff_t)data + bytesRead), length);
		value = std::string{ (const char*)((void*)((ptrdiff_t)data + bytesRead)), length + sizeDelimited };
		full = std::string{ (const char*)(data), bytesRead + length + sizeDelimited };
		bytesRead += length + sizeDelimited;
		break;
	case WIRETYPE_START_GROUP:
		throw("WIRETYPE_START_GROUP unrealised");
		break;
	case WIRETYPE_END_GROUP:
		throw("WIRETYPE_END_GROUP unrealised");
		break;
	case WIRETYPE_FIXED32:
		value = std::string{ (const char*)((void*)((ptrdiff_t)data + bytesRead)), 4 };
		full = std::string{ (const char*)(data), bytesRead + 4 };
		bytesRead += 4;
		break;
	default:
		throw("Unknown type %i", type);
		break;
	}


	return field(_field, type, value, full);
}

#pragma endregion 

#pragma region field Definition
field& field::operator=(const field& f) {
	this->_tag = f._tag;
	this->value = f.value;
	this->full = f.full;
	return *this;
}

field::field(unsigned field, unsigned type, std::string value, std::string full) {
	this->_tag = { field, type };
	this->value = value;
	this->full = full;
}

template<typename T>
field::field(tag _tag, T value) {
	auto wire_type = kWireTypeForFieldType[_tag.type];
	full = get_bytes_varint32(MAKE_TAG(_tag.field, wire_type));

	switch (wire_type) {
	case WIRETYPE_VARINT:
		full += getBytesVarint64((uint64_t)(value));
		break;
	case WIRETYPE_FIXED32:
		full += std::string{ (const char*)(&value), 4 };
		break;
	case WIRETYPE_FIXED64:
		full += std::string{ (const char*)(&value), 8 };
		break;
	}
}

template<typename T>
field::field(unsigned field, unsigned type, T value) {
	auto wire_type = kWireTypeForFieldType[type];
	tag = { field, (unsigned)wire_type };
	full = get_bytes_varint32(MAKE_TAG(field, wire_type));

	switch (wire_type) {
	case WIRETYPE_VARINT:
		full += getBytesVarint64((uint64_t)(value));
		break;
	case WIRETYPE_FIXED32:
		full += std::string{ (const char*)(&value), 4 };
		break;
	case WIRETYPE_FIXED64:
		full += std::string{ (const char*)(&value), 8 };
		break;
	}
}

field::field(tag _tag, std::string value) {
	auto wire_type = kWireTypeForFieldType[_tag.type];
	full = get_bytes_varint32(MAKE_TAG(_tag.field, wire_type));
	full += get_bytes_varint32(value.size());
	full += value;
}

field::field(unsigned field, unsigned type, std::string value) {
	auto wire_type = kWireTypeForFieldType[type];
	_tag = { field, (unsigned)wire_type };
	full = get_bytes_varint32(MAKE_TAG(field, wire_type));
	full += get_bytes_varint32(value.size());
	full += value;
}


float field::_float() {
	return *(float*)((void*)value.data());
}
double field::_double() {
	return *(double*)((void*)value.data());
}
int32_t field::get_int32() {
	size_t bytesRead;
	return (int32_t)(read_var_uint64((void*)value.data(), bytesRead));
}
int64_t field::get_int64() {
	size_t bytesRead;
	return read_var_uint64((void*)value.data(), bytesRead);
}
uint32_t field::_uint32() {
	size_t bytesRead;
	return read_var_uint32((void*)value.data(), bytesRead);
}
uint64_t field::_uint64() {
	size_t bytesRead;
	return read_var_uint64((void*)value.data(), bytesRead);
}
uint32_t field::_fixed32() {
	return *(uint32_t*)((void*)value.data());
}
uint64_t field::_fixed64() {
	return *(uint64_t*)((void*)value.data());
}
int32_t field::_sfixed32() {
	return *(int32_t*)((void*)value.data());
}
int64_t field::_sfixed64() {
	return *(int64_t*)((void*)value.data());
}
bool field::_bool() {
	size_t bytesRead;
	return !!read_var_uint32((void*)value.data(), bytesRead);
}

std::string field::_string() {
	size_t bytesRead;
	void* data = (void*)value.data();
	auto length = read_var_uint32((void*)value.data(), bytesRead);
	auto value = std::string{ (const char*)((void*)((ptrdiff_t)data + bytesRead)), length };
	return value;
}

#pragma endregion

#pragma region proto_writer Definition
proto_writer::proto_writer() {

} 

proto_writer::proto_writer(size_t maxFields) {
	size_t vector_size = maxFields + 1;
	fields.resize(vector_size);
	fields.reserve(vector_size);
}

proto_writer::proto_writer(void* data, size_t size, size_t maxFields) : proto_writer(maxFields) {
	size_t vector_size = maxFields + 1, pos = 0, bytesRead;

	if (data == nullptr) return;
	// parse packet
	while (pos < size) {
		auto field = field::read_field((void*)((ptrdiff_t)data + pos), bytesRead);
		if (!bytesRead) break;

		auto index = field._tag.field;
		if (index >= vector_size) throw("fields range error: field[%i]", index);
		fields[index].push_back(field);
		pos += bytesRead;
	}
}

proto_writer::proto_writer(std::string dataStr, size_t maxFields) : proto_writer((void*)dataStr.data(), dataStr.size(), maxFields) {

}

std::string proto_writer::serialize() {
	std::string result;
	for (auto& f0 : fields) {
		for (auto& f1 : f0) {
			result += f1.full;
		}
	}
	return result;
}

void proto_writer::print() {
	auto data = serialize();
	void* mem = (void*)data.data();
	size_t size = data.size();
	int j = 0;
	for (size_t i = 0; i <= size; ++i) {
		printf("%.2X ", *(unsigned char*)((uintptr_t)mem + i));
		j++;
		if (j == 16)
		{
			j = 0;
			printf("\n");
		}
	}
	printf("\n");
}

void proto_writer::add(field field) {
	fields[field._tag.field].push_back(field);
}

void proto_writer::replace(field field) {
	fields[field._tag.field].clear();
	fields[field._tag.field].push_back(field);
}

void proto_writer::replace(field field, uint32_t index) {
	fields[field._tag.field][index] = field;
}

void proto_writer::clear(unsigned fieldId) {
	return fields[fieldId].clear();
}

bool proto_writer::has(unsigned fieldId) {
	return fields[fieldId].size() > 0;
}

field proto_writer::get(unsigned fieldId) {
	if (fields[fieldId].empty()) return field();
	return fields[fieldId][0];
}

std::vector<field> proto_writer::getAll(unsigned fieldId) {
	return fields[fieldId];
}

template<typename T>
void proto_writer::add(tag _tag, T value) {
	fields[_tag.field].push_back(field(tag, value));
}

template<typename T>
void proto_writer::replace(tag _tag, T value) {
	fields[_tag.field].clear();
	fields[_tag.field].push_back(field(_tag, value));
}

template<typename T>
void proto_writer::replace(tag _tag, T value, uint32_t index) {
	fields[_tag.field][index] = field(_tag, value);
}

void proto_writer::add(tag _tag, std::string value) {
	fields[_tag.field].push_back(field(_tag, value));
}

void proto_writer::replace(tag _tag, std::string value) {
	fields[_tag.field].clear();
	fields[_tag.field].push_back(field(_tag, value));
}

void proto_writer::replace(tag _tag, std::string value, uint32_t index) {
	fields[_tag.field][index] = field(_tag, value);
}

void proto_writer::clear(tag _tag) {
	return fields[_tag.field].clear();
}

bool proto_writer::has(tag _tag) {
	return fields[_tag.field].size() > 0;
}

field proto_writer::get(tag _tag) {
	if (fields[_tag.field].empty()) return field();
	return fields[_tag.field][0];
}

std::vector<field> proto_writer::getAll(tag _tag) {
	return fields[_tag.field];
}

#pragma endregion