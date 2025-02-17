/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include <algorithm>

#include "VM/VM.hpp"

std::vector<var_base_t *> _str_split(const std::string &data, const char delim,
				     const size_t &src_id, const size_t &idx);

static inline void trim(std::string &s);

size_t size_t_pow(size_t base, int exp);
size_t str_to_bin(const std::string &str);

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

var_base_t *str_size(vm_state_t &vm, const fn_data_t &fd)
{
	return make<var_int_t>(STR(fd.args[0])->get().size());
}

var_base_t *str_clear(vm_state_t &vm, const fn_data_t &fd)
{
	STR(fd.args[0])->get().clear();
	return vm.nil;
}

var_base_t *str_empty(vm_state_t &vm, const fn_data_t &fd)
{
	return STR(fd.args[0])->get().size() == 0 ? vm.tru : vm.fals;
}

var_base_t *str_front(vm_state_t &vm, const fn_data_t &fd)
{
	std::string &str = STR(fd.args[0])->get();
	return str.size() == 0 ? vm.nil : make<var_str_t>(std::string(1, str.front()));
}

var_base_t *str_back(vm_state_t &vm, const fn_data_t &fd)
{
	std::string &str = STR(fd.args[0])->get();
	return str.size() == 0 ? vm.nil : make<var_str_t>(std::string(1, str.back()));
}

var_base_t *str_push(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx, "expected string argument for string.push(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	std::string &src  = STR(fd.args[1])->get();
	std::string &dest = STR(fd.args[0])->get();
	if(src.size() > 0) dest += src;
	return fd.args[0];
}

var_base_t *str_pop(vm_state_t &vm, const fn_data_t &fd)
{
	std::string &str = STR(fd.args[0])->get();
	if(str.size() > 0) str.pop_back();
	return fd.args[0];
}

var_base_t *str_ischat(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected first argument to be of type integer for index, found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	if(!fd.args[2]->istype<var_str_t>() && !fd.args[2]->istype<var_int_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected second argument to be of type string or integer for checking, found: %s",
		vm.type_name(fd.args[2]).c_str());
		return nullptr;
	}
	size_t pos	  = mpz_get_ui(INT(fd.args[1])->get());
	std::string &dest = STR(fd.args[0])->get();
	if(pos >= dest.size()) {
		vm.fail(fd.src_id, fd.idx, "position %zu is not within string of length %zu", pos,
			dest.size());
		return nullptr;
	}
	std::string chars;
	if(fd.args[2]->istype<var_int_t>()) {
		chars = mpz_get_si(INT(fd.args[2])->get());
	} else if(fd.args[2]->istype<var_str_t>()) {
		chars = STR(fd.args[2])->get();
	}
	return chars.find(dest[pos]) == std::string::npos ? vm.fals : vm.tru;
}

var_base_t *str_setat(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected first argument to be of type integer for string.set(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	if(!fd.args[2]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected second argument to be of type string for string.set(), found: %s",
			vm.type_name(fd.args[2]).c_str());
		return nullptr;
	}
	size_t pos	  = mpz_get_ui(INT(fd.args[1])->get());
	std::string &dest = STR(fd.args[0])->get();
	if(pos >= dest.size()) {
		vm.fail(fd.src_id, fd.idx, "position %zu is not within string of length %zu", pos,
			dest.size());
		return nullptr;
	}
	std::string &src = STR(fd.args[2])->get();
	if(src.size() == 0) return fd.args[0];
	dest[pos] = src[0];
	return fd.args[0];
}

var_base_t *str_insert(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected first argument to be of type integer for string.insert(), found: %s",
		vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	if(!fd.args[2]->istype<var_str_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected second argument to be of type string for string.insert(), found: %s",
		vm.type_name(fd.args[2]).c_str());
		return nullptr;
	}
	size_t pos	  = mpz_get_ui(INT(fd.args[1])->get());
	std::string &dest = STR(fd.args[0])->get();
	if(pos > dest.size()) {
		vm.fail(fd.src_id, fd.idx, "position %zu is greater than string length %zu", pos,
			dest.size());
		return nullptr;
	}
	std::string &src = STR(fd.args[2])->get();
	dest.insert(dest.begin() + pos, src.begin(), src.end());
	return fd.args[0];
}

var_base_t *str_erase(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected argument to be of type integer for string.erase(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	size_t pos	 = mpz_get_ui(INT(fd.args[1])->get());
	std::string &str = STR(fd.args[0])->get();
	if(pos < str.size()) str.erase(str.begin() + pos);
	return fd.args[0];
}

var_base_t *str_find(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected argument to be of type str for string.find(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	std::string &str  = STR(fd.args[0])->get();
	std::string &what = STR(fd.args[1])->get();
	size_t pos	  = str.find(what);
	if(pos == std::string::npos) {
		return make<var_int_t>(-1);
	}
	return make<var_int_t>(pos);
}

var_base_t *str_rfind(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected argument to be of type str for string.find(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	std::string &str  = STR(fd.args[0])->get();
	std::string &what = STR(fd.args[1])->get();
	size_t pos	  = str.rfind(what);
	if(pos == std::string::npos) {
		return make<var_int_t>(-1);
	}
	return make<var_int_t>(pos);
}

var_base_t *str_substr(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_int_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected begin argument to be of type integer for string.erase(), found: %s",
		vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	if(!fd.args[2]->istype<var_int_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected length argument to be of type integer for string.erase(), found: %s",
		vm.type_name(fd.args[2]).c_str());
		return nullptr;
	}
	size_t pos	 = mpz_get_ui(INT(fd.args[1])->get());
	size_t len	 = mpz_get_ui(INT(fd.args[2])->get());
	std::string &str = STR(fd.args[0])->get();
	return make<var_str_t>(str.substr(pos, len));
}

var_base_t *str_last(vm_state_t &vm, const fn_data_t &fd)
{
	return make<var_int_t>(STR(fd.args[0])->get().size() - 1);
}

var_base_t *str_trim(vm_state_t &vm, const fn_data_t &fd)
{
	std::string &str = STR(fd.args[0])->get();
	trim(str);
	return fd.args[0];
}

var_base_t *str_upper(vm_state_t &vm, const fn_data_t &fd)
{
	std::string str = STR(fd.args[0])->get();
	size_t len	= str.size();
	for(size_t i = 0; i < len; ++i) {
		str[i] = str[i] >= 'a' && str[i] <= 'z' ? str[i] ^ 0x20 : str[i];
	}
	return make<var_str_t>(str);
}

var_base_t *str_split(vm_state_t &vm, const fn_data_t &fd)
{
	var_str_t *str = STR(fd.args[0]);
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx, "expected string argument for delimiter, found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	if(STR(fd.args[1])->get().size() == 0) {
		vm.fail(fd.src_id, fd.idx, "found empty delimiter for string split");
		return nullptr;
	}
	char delim			  = STR(fd.args[1])->get()[0];
	std::vector<var_base_t *> res_vec = _str_split(str->get(), delim, fd.src_id, fd.src_id);
	return make<var_vec_t>(res_vec, false);
}

var_base_t *str_starts_with(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(
		fd.src_id, fd.idx,
		"expected argument to be of type string for string.starts_with(), found: %s",
		vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	const std::string &str = STR(fd.args[0])->get();
	std::string &with      = STR(fd.args[1])->get();
	return make<var_bool_t>(str.rfind(with, 0) == 0);
}

var_base_t *str_ends_with(vm_state_t &vm, const fn_data_t &fd)
{
	if(!fd.args[1]->istype<var_str_t>()) {
		vm.fail(fd.src_id, fd.idx,
			"expected argument to be of type string for string.ends_with(), found: %s",
			vm.type_name(fd.args[1]).c_str());
		return nullptr;
	}
	const std::string &str = STR(fd.args[0])->get();
	std::string &with      = STR(fd.args[1])->get();
	size_t pos	       = str.rfind(with);
	return make<var_bool_t>(pos != std::string::npos && pos + with.size() == str.size());
}

var_base_t *hex_str_to_bin_str(vm_state_t &vm, const fn_data_t &fd)
{
	static std::unordered_map<char, const char *> hextobin = {
	{'0', "0000"}, {'1', "0001"}, {'2', "0010"}, {'3', "0011"}, {'4', "0100"}, {'5', "0101"},
	{'6', "0110"}, {'7', "0111"}, {'8', "1000"}, {'9', "1001"}, {'a', "1010"}, {'b', "1011"},
	{'c', "1100"}, {'d', "1101"}, {'e', "1110"}, {'f', "1111"},
	};

	const std::string &str = STR(fd.args[0])->get();
	std::string bin;
	for(auto &ch : str) {
		char c = tolower(ch);
		if((c < '0' || c > '9') && (c < 'a' || c > 'f')) {
			vm.fail(fd.src_id, fd.idx, "expected hex string, found character: %c", c);
			return nullptr;
		}
		bin += hextobin[c];
	}
	while(!bin.empty() && bin.front() == '0') bin.erase(bin.begin());
	return make<var_str_t>(bin);
}

var_base_t *utf8_char_from_bin_str(vm_state_t &vm, const fn_data_t &fd)
{
	std::string str = STR(fd.args[0])->get();
	if(str.empty()) return make<var_str_t>("");

	// reference: https://en.wikipedia.org/wiki/UTF-8#Encoding
	if(str.size() > 21) {
		vm.fail(fd.src_id, fd.idx, "UTF-8 cannot be more than 21 bytes, found bytes: %zu",
			str.size());
		return nullptr;
	}

	for(auto &c : str) {
		if(c == '0' || c == '1') continue;
		vm.fail(fd.src_id, fd.idx, "expected bin string, found character: %c", c);
		return nullptr;
	}

	var_str_t *res = make<var_str_t>("");
	std::string &r = STR(res)->get();
	if(str.size() <= 7) {
		while(str.size() < 7) {
			str.insert(str.begin(), '0');
		}
		r = str_to_bin("0" + str);
	} else if(str.size() <= 11) {
		while(str.size() < 11) {
			str.insert(str.begin(), '0');
		}
		r = str_to_bin("110" + str.substr(0, 5));
		r += str_to_bin("10" + str.substr(5));
	} else if(str.size() <= 16) {
		while(str.size() < 16) {
			str.insert(str.begin(), '0');
		}
		r = str_to_bin("1110" + str.substr(0, 4));
		r += str_to_bin("10" + str.substr(4, 6));
		r += str_to_bin("10" + str.substr(10));
	} else { // str.size() <= 21
		while(str.size() < 21) {
			str.insert(str.begin(), '0');
		}
		r = str_to_bin("11110" + str.substr(0, 3));
		r += str_to_bin("10" + str.substr(3, 6));
		r += str_to_bin("10" + str.substr(9, 6));
		r += str_to_bin("10" + str.substr(15));
	}

	return res;
}

// character (str[0]) to its ASCII (int)
var_base_t *byt(vm_state_t &vm, const fn_data_t &fd)
{
	const std::string &str = STR(fd.args[0])->get();
	if(str.empty()) return make<var_int_t>(0);
	return make<var_int_t>((unsigned char)str[0]);
}

// ASCII (int) to character (str)
var_base_t *chr(vm_state_t &vm, const fn_data_t &fd)
{
	return make<var_str_t>(std::string(1, (unsigned char)mpz_get_si(INT(fd.args[0])->get())));
}

INIT_MODULE(str)
{
	var_src_t *src = vm.current_source();

	vm.add_native_typefn<var_str_t>("len", str_size, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("clear", str_clear, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("empty", str_empty, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("front", str_front, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("back", str_back, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("push", str_push, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("pop", str_pop, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("ischat", str_ischat, 2, src_id, idx);
	vm.add_native_typefn<var_str_t>("set", str_setat, 2, src_id, idx);
	vm.add_native_typefn<var_str_t>("insert", str_insert, 2, src_id, idx);
	vm.add_native_typefn<var_str_t>("erase", str_erase, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("find", str_find, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("rfind", str_rfind, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("substr_native", str_substr, 2, src_id, idx);
	vm.add_native_typefn<var_str_t>("lastidx", str_last, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("trim", str_trim, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("upper", str_upper, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("split_native", str_split, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("starts_with", str_starts_with, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("ends_with", str_ends_with, 1, src_id, idx);
	vm.add_native_typefn<var_str_t>("getBinStrFromHexStr", hex_str_to_bin_str, 0, src_id, idx);
	vm.add_native_typefn<var_str_t>("getUTF8CharFromBinStr", utf8_char_from_bin_str, 0, src_id,
					idx);

	vm.add_native_typefn<var_str_t>("byt", byt, 0, src_id, idx);
	vm.add_native_typefn<var_int_t>("chr", chr, 0, src_id, idx);

	return true;
}

std::vector<var_base_t *> _str_split(const std::string &data, const char delim,
				     const size_t &src_id, const size_t &idx)
{
	std::string temp;
	std::vector<var_base_t *> vec;

	for(auto c : data) {
		if(c == delim) {
			if(temp.empty()) continue;
			vec.push_back(new var_str_t(temp, src_id, idx));
			temp.clear();
			continue;
		}

		temp += c;
	}

	if(!temp.empty()) vec.push_back(new var_str_t(temp, src_id, idx));
	return vec;
}

// trim from start (in place)
static inline void ltrim(std::string &s)
{
	s.erase(s.begin(),
		std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(),
		s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
	ltrim(s);
	rtrim(s);
}

size_t size_t_pow(size_t base, int exp)
{
	size_t result = 1;
	while(exp) {
		if(exp % 2) result *= base;
		exp /= 2;
		base *= base;
	}
	return result;
}

size_t str_to_bin(const std::string &str)
{
	size_t exp = size_t_pow(2, str.size() - 1);
	size_t bin = 0;
	for(auto &c : str) {
		if(c == '1') {
			bin += exp;
		}
		exp /= 2;
	}
	return bin;
}