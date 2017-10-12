project('gpgraph', 'cpp', default_options : ['cpp_std=c++11'])

gtest_sp = subproject('gtest')
gtest = gtest_sp.get_variable('gmock_main_dep')

e = executable('gpgraph_test', 'gpgraph_test.cc', dependencies : [gtest])
test('gpgraph test', e)
