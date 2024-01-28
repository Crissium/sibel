#include "sibel.h"

#include <algorithm>
#include <array>
#include <unicode/normalizer2.h>

UErrorCode err = U_ZERO_ERROR;
const icu::Normalizer2 * normaliser = icu::Normalizer2::getNFKDInstance(err);

// Banned: combining characters (M), punctuation (P), and space (Z)
const icu::UnicodeSet BANNED_SET("[[:M:][:P:][:Z:]]", err);

const icu::UnicodeString LIGATURE_SS(0x1E9E);
const icu::UnicodeString LIGATURE_ss(0x00DF);
const icu::UnicodeString LIGATURE_AE(0x00C6);
const icu::UnicodeString LIGATURE_ae(0x00E6);
const icu::UnicodeString LIGATURE_OE(0x0152);
const icu::UnicodeString LIGATURE_oe(0x0153);
const icu::UnicodeString PLAIN_SS("SS");
const icu::UnicodeString PLAIN_ss("ss");
const icu::UnicodeString PLAIN_AE("AE");
const icu::UnicodeString PLAIN_ae("ae");
const icu::UnicodeString PLAIN_OE("OE");
const icu::UnicodeString PLAIN_oe("oe");

std::string simplify(const std::string & s)
{
	icu::UnicodeString us = icu::UnicodeString::fromUTF8(s);

	UErrorCode err = U_ZERO_ERROR;
	icu::UnicodeString normalised = normaliser->normalize(us, err);

	icu::UnicodeString result;
	for (int32_t i = 0; i < normalised.length(); ++i)
	{
		UChar32 c = normalised.char32At(i);
		if (!BANNED_SET.contains(c))
		{
			result.append(c);
		}
	}

	// result.toLower();

	result.findAndReplace(LIGATURE_SS, PLAIN_SS);
	result.findAndReplace(LIGATURE_ss, PLAIN_ss);
	result.findAndReplace(LIGATURE_AE, PLAIN_AE);
	result.findAndReplace(LIGATURE_ae, PLAIN_ae);
	result.findAndReplace(LIGATURE_OE, PLAIN_OE);
	result.findAndReplace(LIGATURE_oe, PLAIN_oe);

	std::string result_utf8;
	result.toUTF8String(result_utf8);
	return result_utf8;
}

const std::string BANNED_CHARS[] = {" ", "-", "'", "\u2019"};

bool is_without_banned_chars(const std::string &s)
{
	for (const std::string & c : BANNED_CHARS)
	{
		if (s.find(c) != std::string::npos)
		{
			return false;
		}
	}
	return true;
}
