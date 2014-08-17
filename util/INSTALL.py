#!/usr/bin/python
# @file    INSTALL.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
import argparse
import os
from os.path import expanduser
import re
import sys
import tempfile
import shutil
import subprocess
import grp

try:
    # noinspection PyUnresolvedReferences
    from pip.backwardcompat import PermissionError
except ImportError:
    pass

import propwareUtils
from propwareUtils import OperatingSystem, Windows, Linux, Mac


class Installer(object):
    _PROPGCC_DIR_NAME = "propgcc"
    _PROPWARE_ROOT = ""

    __DEFAULT_PROPGCC_PATH = expanduser("~")
    __DEFAULT_CMAKE_PATH = expanduser("~")

    @staticmethod
    def create(operating_system):
        assert (isinstance(operating_system, OperatingSystem))
        operating_system = propwareUtils.get_os()
        if isinstance(operating_system, Linux):
            return DebInstaller()
        elif isinstance(operating_system, Mac):
            return MacInstaller()
        elif isinstance(operating_system, Windows):
            return WinInstaller()
        else:
            raise Exception("Your operating system (%s) is not supported. Please report this error to "
                            "david@zemon.name" % str(operating_system))

    def __init__(self):
        super(Installer, self).__init__()
        self._cmake_download_url = ""
        self._propgcc_download_url = ""
        self._cmake_parent = ""
        self._propgcc_parent = ""
        self._cmake_zip_name = None
        self._propgcc_zip_name = None
        self._make_installed = False
        self._cmake_installed = False
        self._cmake_root_dir_name = ""  # Differs between each OS - must be set in children
        self._cmake_path = ""
        self._propgcc_path = ""
        self._add_cmake_to_path = False
        self._add_propgcc_to_path = False

        propwareUtils.checkProperWorkingDirectory()
        Installer._PROPWARE_ROOT = os.path.abspath("..")

        # Parse arguments
        args = Installer._parse_args()
        if args.propgcc_path:
            self._propgcc_parent = args.propgcc_path
        else:
            self._propgcc_parent = Installer.__DEFAULT_PROPGCC_PATH
        if args.cmake_path:
            self._cmake_parent = args.cmake_path
        else:
            self._cmake_parent = Installer.__DEFAULT_CMAKE_PATH

    def install(self):
        self._confirm_dependencies()

        self._copy_cmake_files()  # Copy necessary language files (*COG* and eventually Spin)

        self._set_env_variables()

        self._import_propware()  # Download Simple and libpropeller

        if self._make_installed and self._cmake_installed:
            self._build_binaries()

    @staticmethod
    def _parse_args():
        parser = argparse.ArgumentParser(description="")
        parser.add_argument("-p", "--propgcc_path", dest="propgcc_path", action="store", type=str,
                            help="Path to install PropGCC contents", required=False)
        parser.add_argument("-c", "--cmake_path", dest="cmake_path", action='store', type=str,
                            help="Path to install CMake contents", required=False)
        return parser.parse_args()

    @staticmethod
    def _import_propware():
        if not os.path.exists(Installer._PROPWARE_ROOT + str(os.sep) + propwareUtils.DOWNLOADS_DIRECTORY):
            from propwareImporter import importAll

            importAll()

    def _download_cmake(self):
        assert (self._cmake_download_url != "")
        return propwareUtils.downloadFile(self._cmake_download_url, tempfile.gettempdir())[0]

    def _download_propgcc(self):
        assert (self._cmake_download_url != "")
        return propwareUtils.downloadFile(self._propgcc_download_url, tempfile.gettempdir())[0]

    def _check_for_make(self):
        if None == propwareUtils.which("make"):
            self._warn_make_instructions()
            self._make_installed = False
        else:
            self._make_installed = True

    def _warn_make_instructions(self):
        pass

    def _copy_cmake_files(self):
        cmake_modules_src_path = Installer._PROPWARE_ROOT + str(os.sep) + "CMakeModules"
        cmake_modules_dst_path = propwareUtils.get_cmake_modules_path(self._cmake_path)

        try:
            for entry in os.listdir(cmake_modules_src_path):
                src_file_path = cmake_modules_src_path + str(os.sep) + entry
                if os.path.isfile(src_file_path):
                    shutil.copy(src_file_path, cmake_modules_dst_path)

            cmake_platform_src_path = cmake_modules_src_path + str(os.sep) + "Platform"
            cmake_platform_dst_path = cmake_modules_dst_path + str(os.sep) + "Platform"
            for entry in os.listdir(cmake_platform_src_path):
                src_file_path = cmake_platform_src_path + str(os.sep) + entry
                if os.path.isfile(src_file_path):
                    shutil.copy(src_file_path, cmake_platform_dst_path)
        except (PermissionError, IOError):
            self._cmake_installed = self._sudo_copy_cmake_files(os.path.abspath(cmake_modules_src_path),
                                                                os.path.abspath(cmake_modules_dst_path))

        self._cmake_installed = True

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        assert (isinstance(cmake_modules_src_path, str))
        assert (isinstance(cmake_modules_dst_path, str))

        return False

    def _set_env_variables(self):
        pass

    def _build_binaries(self):
        pass

    def _confirm_dependencies(self):
        self._check_for_make()

        existing_cmake_bin = propwareUtils.which("cmake")
        if existing_cmake_bin:
            cmake_bin_dir = os.path.split(existing_cmake_bin)[0]
            self._cmake_path = os.path.abspath(cmake_bin_dir + str(os.sep) + '..')
        else:
            self._cmake_parent = propwareUtils.get_user_input(
                "CMake will be installed to %s. Press enter to continue or type another existing path to download to "
                "a new directory.\n>>> ", os.path.isdir, "'%s' does not exist or is not a directory.",
                self._cmake_parent)
            self._add_cmake_to_path = True

        existing_propgcc_bin = propwareUtils.which("propeller-elf-gcc")
        if existing_propgcc_bin:
            propgcc_bin_dir = os.path.split(existing_propgcc_bin)[0]
            self._propgcc_path = os.path.abspath(propgcc_bin_dir + str(os.sep) + '..')
        else:
            self._propgcc_parent = propwareUtils.get_user_input(
                "PropGCC will be installed to %s. Press enter to continue or type another existing path to download "
                "to a new directory.\n>>> ", os.path.isdir, "'%s' does not exists or is not a directory.",
                self._propgcc_parent)
            self._add_propgcc_to_path = True

        # If downloads are required, perform them after inquiring about both CMake and PropGCC

        if None == existing_cmake_bin:
            try:
                os.makedirs(self._cmake_parent, 0o644)
            except OSError:
                pass
            finally:
                self._cmake_zip_name = self._download_cmake()
                propwareUtils.extract(self._cmake_zip_name, self._cmake_parent)
                self._cmake_path = self._cmake_parent + str(os.sep) + self._cmake_root_dir_name

        if None == existing_propgcc_bin:
            try:
                os.makedirs(self._propgcc_parent, 0o644)
            except OSError:
                pass
            finally:
                self._propgcc_zip_name = self._download_propgcc()
                propwareUtils.extract(self._propgcc_zip_name, self._propgcc_parent)
                self._propgcc_path = self._propgcc_parent + str(os.sep) + self._PROPGCC_DIR_NAME


class NixInstaller(Installer):
    def __init__(self):
        Installer.__init__(self)

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        Installer._sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path)

        cmd = "sudo cp -r " + cmake_modules_src_path + ' ' + cmake_modules_dst_path
        print("Your CMake installation directory is write-protected. Please provide root level permissions (normally, "
              "your standard system password) to copy necessary files into CMake.")
        print(' '.join(cmd))
        if 0 == subprocess.call(cmd.split()):
            return True
        else:
            return False

    def _build_binaries(self):
        Installer._build_binaries(self)

        set_propgcc_prefix = 'export PROPGCC_PREFIX=%s' % os.path.abspath(self._propgcc_path)
        set_propware_path = 'export PROPWARE_PATH=%s' % os.path.abspath(self._PROPWARE_ROOT)
        run_cmake = self._cmake_path + str(os.sep) + 'bin' + str(os.sep) + 'cmake -G "Unix Makefiles" .'

        cmd_list = '%s ; %s ; %s' % (set_propgcc_prefix, set_propware_path, run_cmake)
        cmd = ['sh', '-c', cmd_list]

        print(cmd_list)
        if 0 != subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT):
            return

        run_make = 'make'
        cmd_list = '%s ; %s ; %s' % (set_propgcc_prefix, set_propware_path, run_make)
        cmd = ['sh', '-c', cmd_list]
        print(cmd_list)
        subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT)

    def _set_env_variables(self):
        super(NixInstaller, self)._set_env_variables()

        cmake_bin = self._cmake_path + str(os.sep) + "bin"
        propgcc_bin = self._propgcc_path + str(os.sep) + "bin"

        if os.environ["SHELL"].endswith("/bash"):
            if self._add_cmake_to_path:
                cmd = ["sh", '-c',
                       'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (cmake_bin, os.path.expanduser("~"))]
                print(' '.join(cmd))
                subprocess.check_output(cmd)

            if self._add_propgcc_to_path:
                cmd = ["sh", '-c',
                       'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (propgcc_bin, os.path.expanduser("~"))]
                print(' '.join(cmd))
                subprocess.check_output(cmd)
        else:
            print("Unknown shell is used. It is recommended that you add '%s' and '%s' to the PATH variable for your "
                  "user's environment." % (cmake_bin, propgcc_bin))


class DebInstaller(NixInstaller):
    def __init__(self):
        NixInstaller.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Linux-i386.tar.gz"
        self._cmake_root_dir_name = "cmake-3.0.1-Linux-i386"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-linux_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by executing \"sudo apt-get install "
              "make\".", file=sys.stderr)

    def _confirm_dependencies(self):
        super(DebInstaller, self)._confirm_dependencies()

        # 64-bit versions of Debian also need the 32-bit C libraries. Install them.
        if propwareUtils.is_64_bit():
            if DebInstaller._needs_libc6():
                cmd = ['sudo', 'dpkg', '--add-architecture', 'i386']
                print(' '.join(cmd))
                subprocess.call(cmd)

                cmd = ['sudo', 'apt-get', 'install', 'libc6:i386']
                print(' '.join(cmd))
                subprocess.call(cmd)

        # Also, add user to "dialout" group if necessary
        if os.environ['USER'] not in grp.getgrnam('dialout')[3]:
            subprocess.call(['sudo', 'usermod', '-a', '-G', 'dialout', os.environ['USER']])

    def _set_env_variables(self):
        super(DebInstaller, self)._set_env_variables()

        if 'PROPGCC_PREFIX' not in os.environ or 'PROPWARE_PATH' not in os.environ:
            print('Environment variables will now be configured for the root environment.')
            prompt = 'Press "enter" to continue or "no" to configure them yourself.\n>>> '
            usr_input = propwareUtils.get_user_input(prompt, "no".__eq__, prompt, None)
            if None == usr_input:
                if 'PROPGCC_PREFIX' not in os.environ:
                    propgcc_env = 'echo "PROPGCC_PREFIX=%s" >> /etc/environment' % self._propgcc_path
                    cmd = ['sudo', 'sh', '-c', propgcc_env]
                    print(' '.join(cmd))
                    subprocess.call(cmd)

                if 'PROPWARE_PATH' not in os.environ:
                    propware_env = 'echo "PROPWARE_PATH=%s" >> /etc/environment' % Installer._PROPWARE_ROOT
                    cmd = ['sudo', 'sh', '-c', propware_env]
                    print(' '.join(cmd))
                    subprocess.call(cmd)
            else:
                print('You have selected to configure the following environment variables for yourself:')
                if 'PROPGCC_PREFIX' not in os.environ:
                    print('\tPlease set PROPGCC_PREFIX to "%s"' % self._propgcc_path)
                if 'PROPWARE_PATH' not in os.environ:
                    print('\tPlease set PROPWARE_PATH to "%s"' % Installer._PROPWARE_ROOT)

    def _check_for_make(self):
        if None == propwareUtils.which("make"):
            cmd = ["sudo", "apt-get", "install", "make"]
            print(' '.join(cmd))
            subprocess.call(cmd)

        # Run the parent AFTER we attempt to install Make
        super(DebInstaller, self)._check_for_make()

    @staticmethod
    def _needs_libc6():
        cmd = ['dpkg-query', '-l', 'libc6*']
        package_list = subprocess.check_output(cmd).split('\n')
        matcher = re.compile('.*libc6(:|-)i386.*')
        for package in package_list:
            if matcher.match(package):
                return False

        return True


class MacInstaller(NixInstaller):
    def __init__(self):
        NixInstaller.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Darwin-universal.tar.gz"
        self._cmake_root_dir_name = "cmake-3.0.1-Darwin64-universal"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by following these instructions:\n\t"
              "http://stackoverflow.com/a/6767528", file=sys.stderr)

    def _set_env_variables(self):
        super(MacInstaller, self)._set_env_variables()
        # TODO: Finish this


class WinInstaller(Installer):
    def __init__(self):
        Installer.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-win32-x86.zip"
        self._cmake_root_dir_name = "cmake-3.0.1-win32-x86"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-win_v1_0_0.zip"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. Make is packaged with PropGCC so be sure to add "
              "PropGCC's bin folder to your system's PATH environment variable.", file=sys.stderr)

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        Installer._sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path)
        print("Your CMake installation directory is in a write-protected directory. Please copy the contents of %s "
              "into %s before attempting to use CMake for the Propeller or use a version of CMake in a different "
              "directory." % (cmake_modules_src_path, cmake_modules_dst_path), file=sys.stderr)
        return False

    def _set_env_variables(self):
        super(WinInstaller, self)._set_env_variables()

        # TODO: Set windows environment variables


if "__main__" == __name__:
    installer = Installer.create(propwareUtils.get_os())
    installer.install()
