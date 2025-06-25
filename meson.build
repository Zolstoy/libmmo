project('libmmo', 'cpp', version : '0.2.0', meson_version : '>=1.0.0')

if host_machine.system() == 'windows'
 override_args = ['cpp_std=vc++latest']
 common_cpp_args =  ['-D_WIN32_WINNT=0x0601']
else
 override_args = ['cpp_std=c++23']
 common_cpp_args = []
endif

if get_option('default_library') == 'static'
  common_cpp_args += '-DMMO_STATIC'
else
  common_cpp_args += '-DBOOST_ALL_DYN_LINK'
endif

boost_headers = dependency('boost')
openssl = dependency('openssl')

mmo = library('mmo', [
      'src/mmo.cpp',
    ],
  dependencies: [
    boost_headers,
    openssl,
],
  include_directories: include_directories('include/mmo', 'src'),
  cpp_args: common_cpp_args + '-DMMO_BUILDING_THE_LIB',
  override_options: override_args,
  install: true)

mmo_dep = declare_dependency(
  include_directories: include_directories('include/'),
  link_with: mmo)

gtest = dependency('gtest')
gtest_main = dependency('gtest_main')

test('all', executable('tests', [
    'tests/common.cpp',
    'tests/mmo.cpp'],
  dependencies: [mmo_dep, gtest, gtest_main],
  cpp_args: common_cpp_args,
  override_options: override_args,
  ))
