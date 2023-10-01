ifdef ComSpec
	MKDIRP=powershell md -Force
	RMRF=powershell function rmrf ($$path) { if (Test-Path $$path) { Remove-Item -Recurse -Force $$path } }; rmrf
	CP_P=powershell cp -Force -Path
	TOUCH=powershell echo $$null >
	SYS_PYTHON=py
	PYTHON=env\Scripts\python.exe
	CONAN=env\Scripts\conan.exe
	SITE_PACKAGES=env\Lib\site-packages
else
	MKDIRP=mkdir -p
	RMRF=rm -rf
	CP_P=cp -p
	TOUCH=touch
	SYS_PYTHON=python3
	PYTHON=env/bin/python
	CONAN=env/bin/conan
	SITE_PACKAGES=env/lib/python$(shell $(SYS_PYTHON) -c 'import sysconfig; print(sysconfig.get_python_version())')/site-packages
endif

# rwildcard doesn't work well with spaces, beware!
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES=$(call rwildcard, src, *.h *.cpp *.inl *.txt)
DATA=$(call rwildcard, src, *.py) $(call rwildcard, data, *.tsv *.json)

all: release

release: build-Release
debug: build-Debug
relwithdebinfo: build-RelWithDebInfo

build-%: requirements
	$(CONAN) install . --settings build_type=$* --options syspython/*:python_executable=$(PYTHON) --build missing --build=editable --conf=lass/*:tools.build:skip_test=True
	$(CONAN) build . --settings build_type=$*

requirements: env/.dev-requirements.stamp

env/.dev-requirements.stamp: .dev-requirements.txt .dev-constraints.txt env
	$(PYTHON) -m pip install --upgrade pip setuptools wheel --constraint .dev-constraints.txt
	$(PYTHON) -m pip install --requirement .dev-requirements.txt --constraint .dev-constraints.txt
	$(TOUCH) env/.dev-requirements.stamp

env:
	$(SYS_PYTHON) -m venv env

clean:
	$(RMRF) build
	$(RMRF) liar
	$(RMRF) $(SITE_PACKAGES)/liar.pth

distclean: clean
	$(RMRF) env
	$(RMRF) __pycache__
	$(RMRF) CMakeUserPresets.json


.PHONY: all release debug build-% requirements clean distclean
