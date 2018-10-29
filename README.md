# lightfs
It refers to a distributed file system, which aims to achieve high performance and fault-tolerance.

It is based on Octopus， which is developed by Storage Research Group @ Tsinghua Universty.(source code: https://github.com/thustorage/octopus)

=========

Installation:

- OS: ubuntu
- Dependencies: fuse-devel,g++（4.9 and
higher），libibverbs

Build lightfs:
- create a new folder "build"
- run "cmake .. & make -j"
 * lightfs: server

Configuration:
conf.xml: configuration of the cluster
