# About

Sibel is a Python spellchecker with the powerful Hunspell as the back-end. Specifically, the following methods are available:
- `spell()`: check if a word is spelled correctly.
- `suggest()`: get a list of suggestions for a word.
- `stem()`: get the stems of a word.
- `analyse()`: get the morphological analysis of a word.

Apart from these methods, Sibel also provides an additional one, `orthographic_forms()`, which, given an input in ASCII, returns a list of all possible orthographic forms of the input in Unicode, that is, with diacritics added, and constituent letters combined into proper ligatures.

# Comparison with [PyHunspell](https://github.com/pyhunspell/pyhunspell/)

- Sibel is not wedded to Hunspell. It might switch to other back-ends.
- PyHunspell is no longer actively maintained, and uses deprecated APIs.
- Sibel's methods return the natural types, instead of raw bytes.
- Sibel releases the GIL when appropriate.
- Sibel has an additional method, which requires libicu.
- Sibel does not expose all APIs of Hunspell.

# Installation

```bash
pip install sibel
```

On devices with limited memory, the compilation might fail due to the very large map defined in [substitutions.cc](/src/substitutions.cc). If this happens, the only way to use Sibel is manual compilation. First comment out the languages you don't use (by prepending `//` to irrelevant lines), then run the following:
```bash
python setup.py build
python setup.py install
```

# Usage

```python
>>> import sibel
>>> speller = sibel.Speller('/usr/share/hunspell', 'de_DE')
>>> speller.orthographic_forms('Massen')
['Massen', 'Maßen']
>>> speller = sibel.Speller('/usr/share/hunspell', 'en_GB')
>>> speller.spell('analyze')
False
>>> speller.suggest('analyze')
['analyse', 'analyte']
>>> speller.stem('archer')
['arch']
>>> speller.orthographic_forms('fiance')
['fiancé', 'fiance']
```
