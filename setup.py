#!/usr/bin/env python3

import subprocess
import sys
from setuptools import Extension, setup

if sys.platform != 'linux':
	raise Exception('Only building on Linux is supported at the moment.')

# Get include paths and library paths from pkg-config
# We need hunspell and libicu
include_dirs = []
libraries = []
library_dirs = []

def pkg_config(name: str) -> None:
	output = subprocess.getoutput(f'pkg-config --cflags {name}')
	include_dirs.extend((path[2:] for path in output.split() if path.startswith('-I')))
	output = subprocess.getoutput(f'pkg-config --libs {name}')
	libraries.extend((path[2:] for path in output.split() if path.startswith('-l')))
	library_dirs.extend((path[2:] for path in output.split() if path.startswith('-L')))

for pkg in ('hunspell', 'icu-uc'):
	pkg_config(pkg)

setup(
	name='sibel',
	ext_modules=[
		Extension(
			'sibel',
			['src/substitutions.cc', 'src/simplification.cc', 'src/sibelmodule.cc'],
			include_dirs=include_dirs,
			library_dirs=library_dirs,
			libraries=libraries,
			extra_compile_args=['-std=c++17']
		)
	],
	package_data={'sibel': ['py.typed', 'sibel.pyi']}
)
