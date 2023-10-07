Lab0
==============================

1、将Lab的基础代码部署至虚拟机。

2、在终端中使用telnet打开一个可靠的字节流并运行http服务访问cs144.keithw.org。

3、在apps/Webget.cc中借助TCPSocket和Address发送http消息实现对外部网页的访问。

4、在src/byte_stream.cc和src/byte_steam.hh中实现一个内存中的可靠字节流，完成对数据的写入和读取。

Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`
