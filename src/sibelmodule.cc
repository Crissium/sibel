#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <filesystem>
#include <hunspell/hunspell.hxx>
#include <mutex>
#include <thread>

#include "sibel.h"

static PyObject * DictionaryLoadingError; // Note that Hunspell itself does not throw exceptions.

typedef struct
{
	PyObject_HEAD
	Hunspell * hunspell;
	const substitution_table * sub_table;
} Speller;

static PyObject * Speller_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	Speller * self;
	self = (Speller *)type->tp_alloc(type, 0);
	if (self != nullptr)
	{
		self->hunspell = nullptr;
		self->sub_table = nullptr;
	}
	return (PyObject *)self;
}

static int Speller_init(Speller * self, PyObject * args, PyObject * kwds)
{
	const char * buf_base_path;
	const char * buf_lang_code;

	if (!PyArg_ParseTuple(args, "ss", &buf_base_path, &buf_lang_code))
	{
		return -1;
	}

	std::string lang_code(buf_lang_code);
	std::string lang_code_no_country = lang_code.substr(0, 2);
	if (SUBSTITUTION_TABLES.find(lang_code_no_country) != SUBSTITUTION_TABLES.end())
	{
		self->sub_table = &(SUBSTITUTION_TABLES.at(lang_code_no_country));
	}
	else
	{
		self->sub_table = nullptr;
	}

	std::filesystem::path base_path = std::filesystem::u8path(buf_base_path);
	std::filesystem::path aff_path = base_path / (lang_code + ".aff");
	std::filesystem::path dic_path = base_path / (lang_code + ".dic");

	if (!std::filesystem::exists(aff_path))
	{
		PyErr_SetString(DictionaryLoadingError, "The .aff file does not exist");
		return -1;
	}
	if (!std::filesystem::exists(dic_path))
	{
		PyErr_SetString(DictionaryLoadingError, "The .dic file does not exist");
		return -1;
	}

	self->hunspell = new Hunspell(aff_path.c_str(), dic_path.c_str());

	return 0;
}

static void Speller_dealloc(Speller * self)
{
	delete self->hunspell;
	Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject * Speller_spell(Speller * self, PyObject * args)
{
	const char * buf_word;
	if (!PyArg_ParseTuple(args, "s", &buf_word))
	{
		return nullptr;
	}

	const std::string word(buf_word);
	bool ok;
	
	Py_BEGIN_ALLOW_THREADS
	ok = self->hunspell->spell(word);
	Py_END_ALLOW_THREADS

	if (ok)
	{
		Py_RETURN_TRUE;
	}
	else
	{
		Py_RETURN_FALSE;
	}
}

static PyObject * Speller_suggest(Speller * self, PyObject * args)
{
	const char * buf_word;
	if (!PyArg_ParseTuple(args, "s", &buf_word))
	{
		return nullptr;
	}

	const std::string word(buf_word);
	std::vector<std::string> suggestions;

	Py_BEGIN_ALLOW_THREADS
	suggestions = self->hunspell->suggest(word);
	Py_END_ALLOW_THREADS

	PyObject * suggestions_list = PyList_New(suggestions.size());
	for (std::size_t i = 0; i < suggestions.size(); ++i)
	{
		PyList_SetItem(suggestions_list, i, PyUnicode_FromString(suggestions[i].c_str()));
	}

	return suggestions_list;
}

static PyObject * Speller_analyse(Speller * self, PyObject * args)
{
	const char * buf_word;
	if (!PyArg_ParseTuple(args, "s", &buf_word))
	{
		return nullptr;
	}

	const std::string word(buf_word);
	std::vector<std::string> analyses;

	Py_BEGIN_ALLOW_THREADS
	analyses = self->hunspell->analyze(word);
	Py_END_ALLOW_THREADS

	PyObject * analyses_list = PyList_New(analyses.size());
	for (std::size_t i = 0; i < analyses.size(); ++i)
	{
		PyList_SetItem(analyses_list, i, PyUnicode_FromString(analyses[i].c_str()));
	}

	return analyses_list;
}

static PyObject * Speller_stem(Speller * self, PyObject * args)
{
	const char * buf_word;
	if (!PyArg_ParseTuple(args, "s", &buf_word))
	{
		return nullptr;
	}

	const std::string word(buf_word);
	std::vector<std::string> stems;

	Py_BEGIN_ALLOW_THREADS
	stems = self->hunspell->stem(word);
	Py_END_ALLOW_THREADS

	PyObject * stems_list = PyList_New(stems.size());
	for (std::size_t i = 0; i < stems.size(); ++i)
	{
		PyList_SetItem(stems_list, i, PyUnicode_FromString(stems[i].c_str()));
	}

	return stems_list;
}

static PyObject * Speller_orthographic_forms(Speller * self, PyObject * args)
{
	const char * buf_word;
	if (!PyArg_ParseTuple(args, "s", &buf_word))
	{
		return nullptr;
	}

	const std::string word(buf_word);
	std::vector<std::string> forms;

	Py_BEGIN_ALLOW_THREADS
	std::vector<std::thread> threads;
	std::mutex mtx;

	if (substitution_table::is_substitutable(word))
	{
		if (self->sub_table && word.size() <= substitution_table::SUBSTITUTION_MAX_LENGTH)
		{
			for (const std::string & possible_form : self->sub_table->substitute(word))
			{
				threads.push_back(std::thread([&](const std::string & form)
				{
					if (self->hunspell->spell(form))
					{
						std::lock_guard<std::mutex> lock(mtx);
						forms.push_back(form);
					}
				}, possible_form));
			}

			for (std::thread & t : threads)
			{
				t.join();
			}
		}
		else
		{
			std::string word_simplified(simplify(word));
			
			for (const std::string & suggestion : self->hunspell->suggest(word))
			{
				threads.push_back(std::thread([&](const std::string & s)
				{
					if (is_without_banned_chars(s) && simplify(s) == word_simplified)
					{
						std::lock_guard<std::mutex> lock(mtx);
						forms.push_back(s);
					}
				}, suggestion));
			}

			for (std::thread & t : threads)
			{
				t.join();
			}
		}
	}
	else
	{
		if (self->hunspell->spell(word))
		{
			forms.push_back(word);
		}
	}
	Py_END_ALLOW_THREADS

	PyObject * forms_list = PyList_New(forms.size());
	for (std::size_t i = 0; i < forms.size(); ++i)
	{
		PyList_SetItem(forms_list, i, PyUnicode_FromString(forms[i].c_str()));
	}

	return forms_list;
}

static PyMethodDef Speller_methods[] = {
	{ "spell", (PyCFunction)Speller_spell, METH_VARARGS, "Check if a word is spelt correctly" },
	{ "suggest", (PyCFunction)Speller_suggest, METH_VARARGS, "Get spelling suggestions for a word" },
	{ "analyse", (PyCFunction)Speller_analyse, METH_VARARGS, "Get morphological analysis of a word" },
	{ "stem", (PyCFunction)Speller_stem, METH_VARARGS, "Get stems of a word" },
	{ "orthographic_forms", (PyCFunction)Speller_orthographic_forms, METH_VARARGS, "Get orthographic forms of a word in ASCII form" },
	{ nullptr, nullptr, 0, nullptr }
};

static PyTypeObject SpellerType = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	.tp_name = "sibel.Speller",
	.tp_basicsize = sizeof(Speller),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)Speller_dealloc,
	.tp_vectorcall_offset = 0,
	.tp_getattr = nullptr,
	.tp_setattr = nullptr,
	.tp_as_async = nullptr,
	.tp_repr = nullptr,
	.tp_as_number = nullptr,
	.tp_as_sequence = nullptr,
	.tp_as_mapping = nullptr,
	.tp_hash = nullptr,
	.tp_call = nullptr,
	.tp_str = nullptr,
	.tp_getattro = nullptr,
	.tp_setattro = nullptr,
	.tp_as_buffer = nullptr,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Python spellchecker with Hunspell as backend",
	.tp_traverse = nullptr,
	.tp_clear = nullptr,
	.tp_richcompare = nullptr,
	.tp_weaklistoffset = 0,
	.tp_iter = nullptr,
	.tp_iternext = nullptr,
	.tp_methods = Speller_methods,
	.tp_members = nullptr,
	.tp_getset = nullptr,
	.tp_base = nullptr,
	.tp_dict = nullptr,
	.tp_descr_get = nullptr,
	.tp_descr_set = nullptr,
	.tp_dictoffset = 0,
	.tp_init = (initproc)Speller_init,
	.tp_alloc = nullptr,
	.tp_new = Speller_new,
	.tp_free = nullptr,
	.tp_is_gc = nullptr,
	.tp_bases = nullptr,
	.tp_mro = nullptr,
	.tp_cache = nullptr,
	.tp_subclasses = nullptr,
	.tp_weaklist = nullptr,
	.tp_del = nullptr,
	.tp_version_tag = 0,
	.tp_finalize = nullptr,
	.tp_vectorcall = nullptr
};

static struct PyModuleDef sibelmodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "sibel",
	.m_doc = "Python spellchecker with Hunspell as backend",
	.m_size = -1
};

PyMODINIT_FUNC PyInit_sibel(void)
{
	PyObject * m;

	if (PyType_Ready(&SpellerType) < 0)
	{
		return nullptr;
	}

	m = PyModule_Create(&sibelmodule);
	if (m == nullptr)
	{
		return nullptr;
	}

	Py_INCREF(&SpellerType);
	if (PyModule_AddObject(m, "Speller", (PyObject *)&SpellerType) < 0)
	{
		Py_DECREF(&SpellerType);
		Py_DECREF(m);
		return nullptr;
	}

	DictionaryLoadingError = PyErr_NewException("sibel.DictionaryLoadingError", nullptr, nullptr);
	Py_INCREF(DictionaryLoadingError);
	if (PyModule_AddObject(m, "DictionaryLoadingError", DictionaryLoadingError) < 0)
	{
		Py_DECREF(DictionaryLoadingError);
		Py_DECREF(&SpellerType);
		Py_DECREF(m);
		return nullptr;
	}

	return m;
}
