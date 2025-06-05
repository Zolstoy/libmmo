project('hyper-block', 'cpp', version : '0.1.0', meson_version : '>=1.0.0')

if host_machine.system() == 'windows'
 override_args = ['cpp_std=vc++latest']
 common_cpp_args =  ['-D_WIN32_WINNT=0x0601', '/bigobj', '-DHYPERBLOCK_SYSTEM_WINDOWS']
else
 override_args = ['cpp_std=c++23']
 common_cpp_args = []
endif


if get_option('default_library') == 'static'
  common_cpp_args += ['-DHYPERBLOCK_STATIC']
else
  common_cpp_args += '-DBOOST_ALL_DYN_LINK'
endif

boost_headers = dependency('boost')
openssl = dependency('openssl')
cereal = dependency('cereal')
spdlog = dependency('spdlog', default_options: '--default-library=static')
sqlite = dependency('sqlitecpp')
perlin_noise = dependency('perlin_noise')
curl = dependency('libcurl')
argparse = dependency('argparse')

dependencies = [
  boost_headers,
  openssl,
  cereal,
  spdlog,
  sqlite,
  perlin_noise,
  curl
]

gtest = dependency('gtest', default_options: '--default-library=static')
gtest_main = dependency('gtest_main', default_options: '--default-library=static')

src = [
  'src/database.cpp',
  'src/event.cpp',
  'src/instance.cpp',
  'src/player.cpp',
  'src/utils/email.cpp',
  'src/utils/hash.cpp',
  'src/utils/http.cpp'
]

lib = library('hyperblock',
  src,
  dependencies: dependencies,
  include_directories: include_directories('src', 'src/include'),
  cpp_args: common_cpp_args + '-DHYPERBLOCK_BUILDING_THE_LIB',
  override_options: override_args,
  install: true
)

lib_asio = library('hyperblock-asio',
  src,
  dependencies: dependencies,
  include_directories: include_directories('src', 'src/include'),
  cpp_args: common_cpp_args + '-DHYPERBLOCK_BUILDING_THE_LIB' + '-DHYPERBLOCK_WITH_ASIO',
  override_options: override_args,
  install: true
)

server = executable('hyperblock-server',
  ['src/bin/server.cpp', 'src/common/common.cpp'],
  dependencies : [argparse, spdlog],
  include_directories : include_directories('src/bin', 'src/include'),
  cpp_args : common_cpp_args,
  override_options: override_args,
  install: true,
  link_with: lib
)

tools = executable('hyperblock-tools',
  ['src/bin/tools.cpp', 'src/common/common.cpp'],
  dependencies : [argparse, spdlog],
  include_directories : include_directories('src/bin', 'src/include'),
  cpp_args : common_cpp_args,
  override_options: override_args,
  install: true,
  link_with: lib
)

test_database = executable('test-database',
  ['src/tests/database.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, gtest_main, spdlog],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: lib
)
test('database', test_database, args : ['--gtest_color=yes'])

test_instance_with_asio = executable('test-instance-with-asio',
  ['src/tests/instance_with_asio.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, spdlog, boost_headers, openssl, cereal],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DHYPERBLOCK_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('instance-with-asio', test_instance_with_asio, args : ['--gtest_color=yes'])

test_network = executable('test-network',
  ['src/tests/network.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, spdlog, boost_headers, openssl, cereal],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DHYPERBLOCK_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('network', test_network, args : ['--gtest_color=yes'])

test_siege = executable('test-siege',
  ['src/tests/siege.cpp', 'src/common/common.cpp'],
  dependencies: [gtest, gtest_main, spdlog, boost_headers, openssl],
  include_directories: include_directories('src/include'),
  cpp_args: common_cpp_args + '-DHYPERBLOCK_WITH_ASIO',
  override_options: override_args,
  link_with: lib_asio
)
test('siege', test_siege, args : ['--gtest_color=yes'])
