project('libmmo', 'cpp', version : '0.1.0', meson_version : '>=1.0.0')

if host_machine.system() == 'windows'
 override_args = ['cpp_std=vc++latest']
 common_cpp_args =  ['-D_WIN32_WINNT=0x0601', '/bigobj']
else
 override_args = ['cpp_std=c++23']
 common_cpp_args = []
endif

if get_option('default_library') == 'static'
  common_cpp_args += ['-DMMO_STATIC']
else
  common_cpp_args += '-DBOOST_ALL_DYN_LINK'
endif

boost_headers = dependency('boost')

lib = library('mmo',
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
    dependency('cereal'),
    dependency('libcurl'),
    dependency('openssl'),
    dependency('openssl'),
    dependency('perlin_noise'),
    dependency('spdlog'),
    dependency('sqlitecpp'),
],
  include_directories: include_directories('src/include', 'src/include/private'),
  cpp_args: common_cpp_args + '-DMMO_BUILDING_THE_LIB',
  override_options: override_args,
  install: true
)

lib_asio = library('mmo-asio',
  src,
  dependencies: dependencies,
  include_directories: include_directories('src', 'src/include'),
  cpp_args: common_cpp_args + '-DMMO_BUILDING_THE_LIB' + '-DMMO_WITH_ASIO',
  override_options: override_args,
)

gtest = dependency('gtest')

test_database = executable('test-database',
  ['tests/database.cpp', 'src/common/common.cpp'],
  dependencies: gtest,
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: lib
)
test('database', test_database, args : ['--gtest_color=yes'])

test_instance_with_asio = executable('test-instance-with-asio',
  ['tests/instance_with_asio.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, boost_headers],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DMMO_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('instance-with-asio', test_instance_with_asio, args : ['--gtest_color=yes'])

test_network = executable('test-network',
  ['tests/network.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, boost_headers],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DMMO_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('network', test_network, args : ['--gtest_color=yes'])

test_siege = executable('test-siege',
  'tests/siege.cpp',
  dependencies: [gtest, boost_headers],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DMMO_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('siege', test_siege, args : ['--gtest_color=yes'])
