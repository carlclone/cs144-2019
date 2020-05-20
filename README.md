
- [x] Lab0 webget,ByteStream
- [x] Lab1 StreamReassembler
- [ ] Lab2 TCP Receiver
- [ ] Lab3 TCP Sender
- [ ] Lab4 TCP Connection
- [ ] Lab5 NetWork Interface
- [ ] Lab6 Router


# Lab0总结

1.实现了一个`in-memory reliable ordered bystream`,做这个的目的是因为`tcp`实际上就是这样的一个可靠有序的字节流,不过是基于网络的,
实现这一个小`lab`可以更好地理解了`tc`p两端的一些交互,
比如说`http`的请求中,客户端发送完请求后会向服务端发送一个`SHUTDOWN_WRITE`的 信号,
就对应`byteSteam`类的`input_ended()`,
这时候服务端就知道,在这个`sequence number`之后没有新数据了,接收完前面的字节就可以开始处理请求,
这里的`seq number`就可以用作`eof`的判断

2.`webget`这个`task`用课程提供的虚拟镜像运行不了,折腾了好久,放到自己的`ubuntu16.04`里成功运行,越底层的语言开发环境越折磨人

# Lab1总结

1.lab1用了5，6个小时的时间才做完，测试用例有点难度

2.实现的是`TCP`的一个组件，重新组装segment成为连续的字节流，提供给上层应用

3.本来想使用递归让代码看起来清楚一点，结果`segfault`了，还是改成循环了

4.在虚拟机里写的代码，没有输入法，写的英文注释看起来挺美观，蹩脚的英文

5.没有人肉走完`test case`就开始写代码，结果是想的模型不对，不能满足后面的`case`，基本是推翻重写 （第一次是用`segment`，字符串的角度进行重组，但遇到了增量更新，未重组字节级别的覆盖，重组字节级别覆盖的`case`，最后把思考的模型改成字节流，代码也清晰许多）,这点让我想到业务开发里也是一样，没有往后思考可能的需求（更完善的`test case` ） ，导致代码只能东拼西凑满足功能，完全不堪入目。




# Lab2

.....





# .

For build prereqs, see [the CS144 VM setup instructions](https://web.stanford.edu/class/cs144/vm_howto).

## Sponge quickstart

To set up your build directory:

	$ mkdir -p <path/to/sponge>/build
	$ cd <path/to/sponge>/build
	$ cmake ..

**Note:** all further commands listed below should be run from the `build` dir.

To build:

    $ make

You can use the `-j` switch to build in parallel, e.g.,

    $ make -j$(nproc)

To test (after building; make sure you've got the [build prereqs](https://web.stanford.edu/class/cs144/vm_howto) installed!)

    $ make check

The first time you run `make check`, it will run `sudo` to configure two
[TUN](https://www.kernel.org/doc/Documentation/networking/tuntap.txt) devices for use during
testing.

### build options

You can specify a different compiler when you run cmake:

    $ CC=clang CXX=clang++ cmake ..

You can also specify `CLANG_TIDY=` or `CLANG_FORMAT=` (see "other useful targets", below).

Sponge's build system supports several different build targets. By default, cmake chooses the `Release`
target, which enables the usual optimizations. The `Debug` target enables debugging and reduces the
level of optimization. To choose the `Debug` target:

    $ cmake .. -DCMAKE_BUILD_TYPE=Debug

The following targets are supported:

- `Release` - optimizations
- `Debug` - debug symbols and `-Og`
- `RelASan` - release build with [ASan](https://en.wikipedia.org/wiki/AddressSanitizer) and
  [UBSan](https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan/)
- `RelTSan` - release build with
  [ThreadSan](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/Thread_Sanitizer)
- `DebugASan` - debug build with ASan and UBSan
- `DebugTSan` - debug build with ThreadSan

Of course, you can combine all of the above, e.g.,

    $ CLANG_TIDY=clang-tidy-6.0 CXX=clang++-6.0 .. -DCMAKE_BUILD_TYPE=Debug

**Note:** if you want to change `CC`, `CXX`, `CLANG_TIDY`, or `CLANG_FORMAT`, you need to remove
`build/CMakeCache.txt` and re-run cmake. (This isn't necessary for `CMAKE_BUILD_TYPE`.)

### other useful targets

To generate documentation (you'll need `doxygen`; output will be in `build/doc/`):

    $ make doc

To lint (you'll need `clang-tidy`):

    $ make -j$(nproc) tidy

To run cppcheck (you'll need `cppcheck`):

    $ make cppcheck

To format (you'll need `clang-format`):

    $ make format

To see all available targets,

    $ make help
