import os, sys, commands

env = Environment()

env.Append(TOOLSET   = ['g++'])
env.Append(CXXFLAGS  = ['-std=c++11', '-g', '-O2', '-Wall', '-Werror', '-Wcast-align', '-Wshadow', '-Wunused-parameter'])
env.Append(LINKFLAGS = ['-static-libstdc++'])

build_dir     = 'build'
platform_libs = ['pthread', 'rt', 'dl']
boost_path    = '/usr/lib/x86_64-linux-gnu'

Export('env')
Export('platform_libs')
Export('boost_path')

common = SConscript("#/src/common/SConscript", variant_dir=build_dir+'/common', duplicate=0)

server1 = SConscript("#/src/server1/SConscript", variant_dir=build_dir+'/server1', duplicate=0)
server2 = SConscript("#/src/server2/SConscript", variant_dir=build_dir+'/server2', duplicate=0)
server3 = SConscript("#/src/server3/SConscript", variant_dir=build_dir+'/server3', duplicate=0)

client_stdin     = SConscript("#/src/client_stdin/SConscript", variant_dir=build_dir+'/client_stdin', duplicate=0)
client_benchmark = SConscript("#/src/client_benchmark/SConscript", variant_dir=build_dir+'/client_benchmark', duplicate=0)

