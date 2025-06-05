project('libmmo', 'cpp', version : '0.1.0', meson_version : '>=1.0.0')

if host_machine.system() == 'windows'
 override_args = ['cpp_std=vc++latest']
 common_cpp_args =  ['-D_WIN32_WINNT=0x0601', '/bigobj']
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
cereal = dependency('cereal')
openssl = dependency('openssl')

mmo = library('mmo',
  [
    'src/database.cpp',
    'src/event.cpp',
    'src/instance.cpp',
    'src/player.cpp',
    'src/email.cpp',
    'src/hash.cpp',
    'src/http.cpp',
    'src/tracing.cpp'
],
  dependencies: [
    boost_headers,
    cereal,
    dependency('libcurl'),
    openssl,
    dependency('perlin_noise'),
    dependency('spdlog', default_options: ['default_library=static']),
    dependency('sqlitecpp', default_options: ['default_library=static']),
],
  include_directories: include_directories('include/mmo'),
  cpp_args: common_cpp_args + '-DMMO_BUILDING_THE_LIB',
  override_options: override_args,
  install: true
)

gtest = dependency('gtest')
gtest_main = dependency('gtest_main')

test_database = executable('test-database',
  [
    'tests/common.cpp',
    'tests/database.cpp',
  ],
  dependencies: [gtest, gtest_main],
  include_directories: include_directories('include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: mmo
)
test('database', test_database, args : ['--gtest_color=yes'])

test_instance = executable('test-instance',
  [
    'tests/common.cpp',
    'tests/instance.cpp',
  ],
  dependencies: [gtest, gtest_main, boost_headers, openssl, cereal],
  include_directories: include_directories('include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: mmo
)
test('instance', test_instance, args : ['--gtest_color=yes'])

test_network = executable('test-network',
  [
    'tests/common.cpp',
    'tests/network.cpp',
  ],
  dependencies: [gtest, gtest_main, boost_headers, openssl, cereal],
  include_directories: include_directories('include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: mmo
)
test('network', test_network, args : ['--gtest_color=yes'])

test_siege = executable('test-siege',
  [
    'tests/common.cpp',
    'tests/siege.cpp',
  ],
  dependencies: [gtest, gtest_main, boost_headers, openssl, cereal],
  include_directories: include_directories('include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: mmo
)
test('siege', test_siege, args : ['--gtest_color=yes'])
