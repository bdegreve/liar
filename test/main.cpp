/** @file
 *  @author Bram de Greve (bramz@users.sourceforge.net)
 *
 *  LiAR isn't a raytracer
 *  Copyright (C) 2023  Bram de Greve (bramz@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  http://liar.bramz.net/
 */

#include <gtest/gtest.h>
#include <lass/python/python_api.h>
#include <locale>

using namespace lass;

namespace
{

template <typename T>
struct Wrap: T
{
	template <typename... Args>
	Wrap(Args&&... args): T(std::forward<Args>(args)...) {}
	~Wrap() override = default;
};

class PythonEnvironment : public ::testing::Environment
{
public:
	void SetUp() override
	{
		PyStatus status;
		PyPreConfig preconfig;
		PyPreConfig_InitIsolatedConfig(&preconfig);

		preconfig.utf8_mode = 1;

		status = Py_PreInitialize(&preconfig);
		if (PyStatus_Exception(status))
		{
			Py_ExitStatusException(status);
		}

		PyConfig config;
		PyConfig_InitIsolatedConfig(&config);

		status = PyConfig_Read(&config);
		if (PyStatus_Exception(status))
		{
			Py_ExitStatusException(status);
		}

		std::wstring_convert<Wrap<std::codecvt<wchar_t, char, std::mbstate_t>>, wchar_t> conv;
		const std::wstring prefix = conv.from_bytes(LASS_STRINGIFY(PYTHON_BASE_PREFIX));
		status = PyConfig_SetString(&config, &config.prefix, prefix.c_str());
		if (PyStatus_Exception(status)) {
			Py_ExitStatusException(status);
		}
		status = PyConfig_SetString(&config, &config.exec_prefix, prefix.c_str());
		if (PyStatus_Exception(status)) {
			Py_ExitStatusException(status);
		}

		status = Py_InitializeFromConfig(&config);
		if (PyStatus_Exception(status))
		{
			Py_ExitStatusException(status);
		}

		PyConfig_Clear(&config);

		const char* bootstrap =
			"import os\n"
			"import importlib.util\n"
			"spec = importlib.util.spec_from_file_location('path_hooks', os.path.join(\"" LASS_STRINGIFY(TEST_SOURCE_DIR) "\", 'path_hooks.py'))\n"
			"assert spec\n"
			"path_hooks = importlib.util.module_from_spec(spec)\n"
			"spec.loader.exec_module(path_hooks)\n"
			"path_hooks.setup_liar_hooks(root_dir=\"" LASS_STRINGIFY(ROOT_DIR) "\", bin_dir=\"" LASS_STRINGIFY(BIN_DIR) "\")\n"
			"del os, importlib, spec, path_hooks\n"
			"import liar\n"
			;
		int res = PyRun_SimpleString(bootstrap);
		if (res < 0)
		{
			exit(1);
		}
	}

	void TearDown() override
	{
		Py_Finalize();
	}
};

}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new PythonEnvironment);
	return RUN_ALL_TESTS();
}
