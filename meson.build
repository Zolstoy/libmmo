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
spdlog = dependency('spdlog', default_options: ['default_library=static'])

mmo = library('mmo', [
    'src/persistence/sqlite_database.cpp',
    'src/tranport/secure_websocket.cpp',
    'src/email.cpp',
    'src/hash.cpp',
    'src/http.cpp',
    'src/tracing.cpp'],
  dependencies: [
    boost_headers,
    cereal,
    dependency('libcurl'),
    openssl,
    dependency('perlin_noise'),
    spdlog,
    dependency('sqlitecpp', default_options: ['default_library=static']),],
  include_directories: include_directories('include/mmo', 'src'),
  cpp_args: common_cpp_args + '-DMMO_BUILDING_THE_LIB',
  override_options: override_args,
  install: true)

gtest = dependency('gtest')
gtest_main = dependency('gtest_main')

test('game', executable('test-game', [
    'tests/common.cpp',
    'tests/game.cpp'],
  dependencies: [gtest, gtest_main],
  include_directories: include_directories('include'),
  cpp_args: common_cpp_args,
  override_options: override_args,
  link_with: mmo),
args : ['--gtest_color=yes'])
