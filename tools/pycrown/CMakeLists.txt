cmake_minimum_required(VERSION 2.8)

project(pycrown)

set (PYFILES
	__init__.py
	Repository.py
	Compiler.py
)

install (FILES ${PYFILES} DESTINATION tools/pycrown)

