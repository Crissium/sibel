#include "sibel.h"

#include <algorithm>
#include <cctype>

substitution_table::substitution_table(const std::unordered_map<std::string, std::vector<std::string>> &&table_letters, const std::unordered_map<std::string, std::vector<std::string>> &&table_ligatures) : table_letters(table_letters), table_ligatures(table_ligatures) {}

const std::unordered_map<std::string, substitution_table> SUBSTITUTION_TABLES({
	// https://en.wikipedia.org/wiki/Afrikaans#Orthography
	{"af", substitution_table({
		{"A", {"\u00C1", "\u00C4"}},
		{"a", {"\u00E1", "\u00E4"}},
		{"E", {"\u00C8", "\u00C9", "\u00CA", "\u00CB"}},
		{"e", {"\u00E8", "\u00E9", "\u00EA", "\u00EB"}},
		{"I", {"\u00CD", "\u00CE", "\u00CF"}},
		{"i", {"\u00ED", "\u00EE", "\u00EF"}},
		{"O", {"\u00D3", "\u00D4", "\u00D6"}},
		{"o", {"\u00F3", "\u00F4", "\u00F6"}},
		{"U", {"\u00DA", "\u00DB", "\u00DC"}},
		{"u", {"\u00FA", "\u00FB", "\u00FC"}},
		{"Y", {"\u00DD"}},
		{"y", {"\u00FD"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Czech_orthography#Alphabet
	// One thing about Czech is unclear at the moment: could U+010F be expanded to d'?

	// https://en.wikipedia.org/wiki/Danish_orthography
	// Danish has the letter 'slashed o,' and I don't know how it is usually represented with ASCII

	// https://en.wikipedia.org/wiki/German_orthography#Alphabet
	{"de", substitution_table({
		{"A", {"\u00C4"}},
		{"a", {"\u00E4"}},
		{"O", {"\u00D6"}},
		{"o", {"\u00F6"}},
		{"U", {"\u00DC"}},
		{"u", {"\u00FC"}}
	}, {
		{"AE", {"\u00C4"}},
		{"ae", {"\u00E4"}},
		{"OE", {"\u00D6"}},
		{"oe", {"\u00F6"}},
		{"ss", {"\u00DF"}}, // The capital sharp s is a recent introduction
		{"UE", {"\u00DC"}},
		{"ue", {"\u00FC"}}
	})},

	// https://www.tandem.net/blog/spanish-accents
	// Spanish could be further optimised because the acute accent only appears once in a word
	{"es", substitution_table({
		{"A", {"\u00C1"}},
		{"a", {"\u00E1"}},
		{"E", {"\u00C9"}},
		{"e", {"\u00E9"}},
		{"I", {"\u00CD"}},
		{"i", {"\u00ED"}},
		{"N", {"\u00D1"}},
		{"n", {"\u00F1"}},
		{"O", {"\u00D3"}},
		{"o", {"\u00F3"}},
		{"U", {"\u00DA", "\u00DC"}},
		{"u", {"\u00FA", "\u00FC"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Estonian_orthography
	{"et", substitution_table({
		{"A", {"\u00C4"}},
		{"a", {"\u00E4"}},
		{"O", {"\u00D5", "\u00D6"}},
		{"o", {"\u00F5", "\u00F6"}},
		{"S", {"\u0160"}},
		{"s", {"\u0161"}},
		{"U", {"\u00DC"}},
		{"u", {"\u00FC"}},
		{"Z", {"\u017D"}},
		{"z", {"\u017E"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Finnish_orthography
	{"fi", substitution_table({
		{"A", {"\u00C4", "\u00C5"}},
		{"a", {"\u00E4", "\u00E5"}},
		{"O", {"\u00D6"}},
		{"o", {"\u00F6"}},
		{"S", {"\u0160"}},
		{"s", {"\u0161"}},
		{"Z", {"\u017D"}},
		{"z", {"\u017E"}}
	}, {})},

	// https://fr.wikipedia.org/wiki/Diacritiques_utilis%C3%A9s_en_fran%C3%A7ais#Combinaisons
	{"fr", substitution_table({
		{"A", {"\u00C0", "\u00C2"}},
		{"a", {"\u00E0", "\u00E2"}},
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"E", {"\u00C8", "\u00C9", "\u00CA", "\u00CB"}},
		{"e", {"\u00E8", "\u00E9", "\u00EA", "\u00EB"}},
		{"I", {"\u00CE", "\u00CF"}},
		{"i", {"\u00EE", "\u00EF"}},
		{"O", {"\u00D4"}},
		{"o", {"\u00F4"}},
		{"U", {"\u00D9", "\u00DB", "\u00DC"}},
		{"u", {"\u00F9", "\u00FB", "\u00FC"}}
	}, {
		{"AE", {"\u00C6"}},
		{"ae", {"\u00E6"}},
		{"OE", {"\u0152"}},
		{"oe", {"\u0153"}}
	})},

	// https://en.wikipedia.org/wiki/Irish_orthography#Diacritics
	{"ga", substitution_table({
		{"A", {"\u00C1"}},
		{"a", {"\u00E1"}},
		{"E", {"\u00C9"}},
		{"e", {"\u00E9"}},
		{"I", {"\u00CD"}},
		{"i", {"\u00ED"}},
		{"O", {"\u00D3"}},
		{"o", {"\u00F3"}},
		{"U", {"\u00DA"}},
		{"u", {"\u00FA"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Scottish_Gaelic_orthography#Alphabet
	{"gd", substitution_table({
		{"A", {"\u00C0"}},
		{"a", {"\u00E0"}},
		{"E", {"\u00C8"}},
		{"e", {"\u00E8"}},
		{"I", {"\u00CC"}},
		{"i", {"\u00EC"}},
		{"O", {"\u00D2"}},
		{"o", {"\u00F2"}},
		{"U", {"\u00D9"}},
		{"u", {"\u00F9"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Manx_language#Diacritics
	{"gv", substitution_table({}, {
		{"Ch", {"\u00C7h"}},
		{"ch", {"\u00E7h"}}
	})},

	// https://en.wikipedia.org/wiki/Haitian_Creole#Orthography
	{"ht", substitution_table({
		{"E", {"\u00C8"}},
		{"e", {"\u00E8"}},
		{"O", {"\u00D2"}},
		{"o", {"\u00F2"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Hungarian_alphabet
	{"hu", substitution_table({
		{"A", {"\u00C1"}},
		{"a", {"\u00E1"}},
		{"E", {"\u00C9"}},
		{"e", {"\u00E9"}},
		{"I", {"\u00CD"}},
		{"i", {"\u00ED"}},
		{"O", {"\u00D3", "\u00D6", "\u0150"}},
		{"o", {"\u00F3", "\u00F6", "\u0151"}},
		{"U", {"\u00DA", "\u00DC", "\u0170"}},
		{"u", {"\u00FA", "\u00FC", "\u0171"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Icelandic_language#Writing_system
	// I don't know how to handle thorns

	// https://www.italianpod101.com/blog/2021/01/18/italian-written-accents/
	// The Wikipedia article is utterly confusing
	{"it", substitution_table({
		{"A", {"\u00C0"}},
		{"a", {"\u00E0"}},
		{"E", {"\u00C8", "\u00C9"}},
		{"e", {"\u00E8", "\u00E9"}},
		{"I", {"\u00CC"}},
		{"i", {"\u00EC"}},
		{"O", {"\u00D2"}},
		{"o", {"\u00F2"}},
		{"U", {"\u00D9"}},
		{"u", {"\u00F9"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Lithuanian_language#Script
	{"lt", substitution_table({
		{"A", {"\u0104"}},
		{"a", {"\u0105"}},
		{"C", {"\u010C"}},
		{"c", {"\u010D"}},
		{"E", {"\u0116", "\u0118"}},
		{"e", {"\u0117", "\u0119"}},
		{"I", {"\u012E"}},
		{"i", {"\u012F"}},
		{"S", {"\u0160"}},
		{"s", {"\u0161"}},
		{"U", {"\u016A", "\u0172"}},
		{"u", {"\u016B", "\u0173"}},
		{"Z", {"\u017D"}},
		{"z", {"\u017E"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Latvian_language#Orthography
	{"lv", substitution_table({
		{"A", {"\u0100"}},
		{"a", {"\u0101"}},
		{"C", {"\u010C"}},
		{"c", {"\u010D"}},
		{"E", {"\u0112"}},
		{"e", {"\u0113"}},
		{"G", {"\u0122"}},
		{"g", {"\u0123"}},
		{"I", {"\u012A"}},
		{"i", {"\u012B"}},
		{"K", {"\u0136"}},
		{"k", {"\u0137"}},
		{"L", {"\u013B"}},
		{"l", {"\u013C"}},
		{"N", {"\u0145"}},
		{"n", {"\u0146"}},
		{"S", {"\u0160"}},
		{"s", {"\u0161"}},
		{"U", {"\u016A"}},
		{"u", {"\u016B"}},
		{"Z", {"\u017D"}},
		{"z", {"\u017E"}}
	}, {})},

	// https://en.wikipedia.org/wiki/M%C4%81ori_language#Orthography
	{"mi", substitution_table({
		{"A", {"\u0100"}},
		{"a", {"\u0101"}},
		{"E", {"\u0112"}},
		{"e", {"\u0113"}},
		{"I", {"\u012A"}},
		{"i", {"\u012B"}},
		{"O", {"\u014C"}},
		{"o", {"\u014D"}},
		{"U", {"\u016A"}},
		{"u", {"\u016B"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Maltese_language#Orthography
	// Only final vowels may take the grave accent. This could be optimised
	{"mt", substitution_table({
		{"a", {"\u00E0"}},
		{"C", {"\u010A"}},
		{"c", {"\u010B"}},
		{"e", {"\u00E8"}},
		{"G", {"\u0120"}},
		{"g", {"\u0121"}},
		{"H", {"\u0126"}},
		{"h", {"\u0127"}},
		{"i", {"\u00EC"}},
		{"o", {"\u00F2"}},
		{"u", {"\u00F9"}},
		{"Z", {"\u017B"}},
		{"z", {"\u017C"}}
	}, {})},

	// https://www.ucl.ac.uk/libnet/library-procedures/collections/cataloguing/dutch-guide-cataloguers
	{"nl", substitution_table({
		{"A", {"\u00C1", "\u00C4"}},
		{"a", {"\u00E1", "\u00E4"}},
		{"E", {"\u00C9", "\u00CB"}},
		{"e", {"\u00E9", "\u00EB"}},
		{"I", {"\u00CD", "\u00CF"}},
		{"i", {"\u00ED", "\u00EF"}},
		{"O", {"\u00D3", "\u00D6"}},
		{"o", {"\u00F3", "\u00F6"}},
		{"U", {"\u00DA", "\u00DC"}},
		{"u", {"\u00FA", "\u00FC"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Norwegian_language#Written_language
	// The slashed o in Norwegian is again a problem

	// https://en.wikipedia.org/wiki/Polish_alphabet#Letters
	{"pl", substitution_table({
		{"A", {"\u0104"}},
		{"a", {"\u0105"}},
		{"C", {"\u0106"}},
		{"c", {"\u0107"}},
		{"E", {"\u0118"}},
		{"e", {"\u0119"}},
		{"L", {"\u0141"}},
		{"l", {"\u0142"}},
		{"N", {"\u0143"}},
		{"n", {"\u0144"}},
		{"O", {"\u00D3"}},
		{"o", {"\u00F3"}},
		{"S", {"\u015A"}},
		{"s", {"\u015B"}},
		{"Z", {"\u0179", "\u017B"}},
		{"z", {"\u017A", "\u017C"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Portuguese_orthography#Diacritics
	// I won't include A with a grave accent because no user in the right frame of mind would look up such words
	{"pt", substitution_table({
		{"A", {"\u00C1", "\u00C2", "\u00C3"}},
		{"a", {"\u00E1", "\u00E2", "\u00E3"}},
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"E", {"\u00C9", "\u00CA"}},
		{"e", {"\u00E9", "\u00EA"}},
		{"I", {"\u00CD"}},
		{"i", {"\u00ED"}},
		{"O", {"\u00D3", "\u00D4", "\u00D5"}},
		{"o", {"\u00F3", "\u00F4", "\u00F5"}},
		{"U", {"\u00DA"}},
		{"u", {"\u00FA"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Romanian_language#Romanian_alphabet
	{"ro", substitution_table({
		{"A", {"\u00C2", "\u0102"}},
		{"a", {"\u00E2", "\u0103"}},
		{"I", {"\u00CE"}},
		{"i", {"\u00EE"}},
		{"S", {"\u0218"}},
		{"s", {"\u0219"}},
		{"T", {"\u021A"}},
		{"t", {"\u021B"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Slovak_orthography#Alphabet
	// Slovak has again the problem of "d'"

	// https://en.wikipedia.org/wiki/Slovene_alphabet#Diacritics
	// Are diacritics used or not by Hunspell?

	// https://en.wikipedia.org/wiki/Samoan_language#Alphabet
	// How do you handle the apostrophe?

	// https://en.wikipedia.org/wiki/Albanian_alphabet
	{"sq", substitution_table({
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"E", {"\u00CB"}},
		{"e", {"\u00EB"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Swedish_alphabet#Letters
	{"sv", substitution_table({
		{"A", {"\u00C4", "\u00C5"}},
		{"a", {"\u00E4", "\u00E5"}},
		{"O", {"\u00D6"}},
		{"o", {"\u00F6"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Turkmen_alphabet
	{"tk", substitution_table({
		{"A", {"\u00C4"}},
		{"a", {"\u00E4"}},
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"N", {"\u0147"}},
		{"n", {"\u0148"}},
		{"O", {"\u00D6"}},
		{"o", {"\u00F6"}},
		{"S", {"\u015E"}},
		{"s", {"\u015F"}},
		{"U", {"\u00DC"}},
		{"u", {"\u00FC"}},
		{"Y", {"\u00DD"}},
		{"y", {"\u00FD"}},
		{"Z", {"\u017D"}},
		{"z", {"\u017E"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Filipino_alphabet
	{"tl", substitution_table({
		{"N", {"\u00D1"}},
		{"n", {"\u00F1"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Turkish_alphabet#Letters
	{"tr", substitution_table({
		{"a", {"\u00E2"}},
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"G", {"\u011E"}},
		{"g", {"\u011F"}},
		{"I", {"\u0130"}},
		{"i", {"\u00EE", "\u0131"}},
		{"O", {"\u00D6"}},
		{"o", {"\u00F6"}},
		{"S", {"\u015E"}},
		{"s", {"\u015F"}},
		{"U", {"\u00DC"}},
		{"u", {"\u00FB", "\u00FC"}}
	}, {})},

	// https://en.wikipedia.org/wiki/Vietnamese_alphabet
	// It's beyond my ability to handle multiple diacritics

	// https://en.wikipedia.org/wiki/Walloon_alphabet
	{"wa", substitution_table({
		{"A", {"\u00C0", "\u00C2", "\u00C5"}},
		{"a", {"\u00E0", "\u00E2", "\u00E5"}},
		{"C", {"\u00C7"}},
		{"c", {"\u00E7"}},
		{"E", {"\u00C8", "\u00C9", "\u00CA", "\u00CB"}},
		{"e", {"\u00E8", "\u00E9", "\u00EA", "\u00EB"}},
		{"I", {"\u00CC", "\u00CE"}},
		{"i", {"\u00EC", "\u00EE"}},
		{"O", {"\u00D4", "\u00D6"}},
		{"o", {"\u00F4", "\u00F6"}},
		{"U", {"\u00D9", "\u00DB"}},
		{"u", {"\u00F9", "\u00FB"}}
	}, {})}
});

bool substitution_table::is_substitutable(const std::string &s)
{
	// If the input goes out of the range of printable ASCII, then it means the user knows what they're doing.
	return std::all_of(s.cbegin(), s.cend(), [](unsigned char c)
					   { return std::isprint(c); }) &&
	// Strings with spaces can't be checked anyway
		   std::none_of(s.cbegin(), s.cend(), [](unsigned char c)
						{ return std::isspace(c); });
}

void substitution_table::generate_substitutions(const std::unordered_map<std::string, std::vector<std::string>> &map, const std::string &input, std::vector<std::string> &result, std::string::size_type index, std::string current, std::string::size_type len_of_key)
{
	if (index == input.size())
	{
		result.push_back(current);
	}
	else
	{
		std::string current_chars(input.substr(index, len_of_key));
		generate_substitutions(map, input, result, index + 1, current + input[index], len_of_key);
		if (map.find(current_chars) != map.end())
		{
			for (const std::string &s : map.at(current_chars))
			{
				generate_substitutions(map, input, result, index + len_of_key, current + s, len_of_key);
			}
		}
	}
}

std::vector<std::string> substitution_table::substitute(const std::string &original) const
{
	// std::string lower(original);
	// std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c)
	// 			   { return std::tolower(c); });

	std::vector<std::string> ligs;
	if (table_ligatures.empty())
	{
		ligs.push_back(std::move(original));
	}
	else
	{
		generate_substitutions(table_ligatures, original, ligs, 0, "", 2);
	}

	if (table_letters.empty())
	{
		return ligs;
	}
	else
	{
		std::vector<std::string> result;
		for (const std::string &s : ligs)
		{
			generate_substitutions(table_letters, s, result, 0, "", 1);
		}
		return result;
	}
}
