
- [x] Lab0 webget,ByteStream
- [x] Lab1 StreamReassembler
- [ ] Lab2 TCP Receiver
- [ ] Lab3 TCP Sender
- [ ] Lab4 TCP Connection
- [ ] Lab5 NetWork Interface
- [ ] Lab6 Router


# Lab0总结

1.实现了一个in-memory reliable ordered bystream,做这个的目的是因为tcp实际上就是这样的一个可靠有序的字节流,不过是基于网络的,
实现这一个小lab可以更好地理解了tcp两端的一些交互,
比如说http的请求中,客户端发送完请求后会向服务端发送一个SHUTDOWN_WRITE的 信号,
就对应byteSteam类的input_ended(),
这时候服务端就知道,在这个sequence number之后没有新数据了,接收完前面的字节就可以开始处理请求,
这里的seq number就可以用作eof的判断

2.webget这个task用课程提供的虚拟镜像运行不了,折腾了好久,放到自己的ubuntu16.04里成功运行,越底层的语言开发环境越折磨人

# Lab1

1.有几个特殊的 case 要处理,增量更新...

2.pesudo code 在这里


# 其他

1.减少循环内判断, 放到外面减少循环次数

```
[0,.....size....len)
[0,.....size)
[i,.....minSize)
for i=0;i<len;i++ {
    if i>=size {
    break;
    }
    //....
}

//improve:
minSize = min(len,size);
for i=0;i<minSize;i++ {
    //...
}
```


2.eof是信号

3.作业提供的接口,先想想作为用户是如何使用这些接口

4.还是要先理清楚各种index,边界,开闭区间的含义

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
