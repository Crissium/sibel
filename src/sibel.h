#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class substitution_table
{
private:
	std::unordered_map<std::string, std::vector<std::string>> table_letters; // letter by letter substitution
	std::unordered_map<std::string, std::vector<std::string>> table_ligatures; // 2-letter substitution
	static void generate_substitutions(const std::unordered_map<std::string, std::vector<std::string>> &map, const std::string &input, std::vector<std::string> &result, std::string::size_type index, std::string current, std::string::size_type len_of_key);

public:
	substitution_table(const std::unordered_map<std::string, std::vector<std::string>> &&table_letters, const std::unordered_map<std::string, std::vector<std::string>> &&table_ligatures);
	static bool is_substitutable(const std::string &s);
	std::vector<std::string> substitute(const std::string &original) const;
};

extern const std::unordered_map<std::string, substitution_table> SUBSTITUTION_TABLES;

std::string simplify(const std::string &s);
bool is_without_banned_chars(const std::string &s);
