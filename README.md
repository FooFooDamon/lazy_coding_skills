# 懒编程秘笈 | Lazy Coding Skills

## 简介 | Introduction

一系列薄封装、少依赖、易集成、自描述、好维护的常用函数和脚本，以及一些实用编码提示，
旨在为现实项目中那些基础性、与具体业务无关的问题提供一种便捷的实现方式及一些解决思路，
以达到减少重复劳动、精简代码数量、提高代码质量的目的。

> A series of thin-wrapping, less-dependant, easily-integrated, self-described and maintainable
common functions and scripts along with some useful hints, which are aimed at helping implement
those fundamental and service-independent functionalities in real projects to achieve the goal of
reducing repetitions, refining code quantity and improving code quality.

我的观点：***贵精不贵多，码少生活好！***

> My opinions: ***Quality is more important than quantity. Less code, better world!***

**[温馨提示 | Warm Tips](WARM_TIPS.md)**

## 适用平台 | Supported Platforms

按支持程度从高到低依次为：`Linux` > 兼容**POSIX**接口的`类UNIX`系统 > `Windows`。

> Degree of support from high to low: `Linux` > `UNIX-like` systems compatible with **POSIX** > `Windows`.

## 用法 | Usage

* **按需**复制特定功能（后文以“模块”代称）的代码或脚本文件到你的项目即可，可以是一个或多个子目录下的一个/部分/全部文件。
不同于很多库，本项目里每个模块通常**彼此独立**，**可以单独使用**而无冗余之弊。

> Copy one/some/all of code/script file(s) for specific functionalities (named as modules in below)
in one or multiple sub-directories to your project **as you need**.
Unlike many other libraries, each module of this project is usually **independent of each other**
and hence **can be used alone** without redundancy.

* 模块及其细分接口的含义、用法可从文件名、函数名见名知意，或从头文件注释、函数注释（如果有）获知。

> The meanings and usages of a module and its interfaces can be fetched from file name(s), function names,
comments in header file(s) or comments for functions (if any).

* 对于带整型返回值的`C/C++`函数，返回值小于零表示出错，并可用`*_error()`获取出错信息；等于零表示执行成功；
大于零表示执行成功且执行结果为N个单位（结果及单位因函数而异，例如：对于\*_send()而言表示成功发送N个字节）。

> For a `C/C++` function with return value of integer type, return value is less than 0 on error,
and `*_error()` can be called to fetch the relevant message.
If return value is 0, it means success.
If return value is greater than 0, it means success and the execution result is N units (the result and unit
vary from function to function, e.g., it meaning sending N bytes successfully for \*_send()).

* 对于可解释执行的脚本，例如`Python`或`Shell`，既可使用`import`、`source`等指令导入到其他文件使用，
亦可在命令行直接执行，后者通过指定`-h`或`--help`选项可查看其用法。若目录内有`Makefile`，
可执行`make help`获取帮助。

> For an interpreted script, `Python` or `Shell` for example, it's okay to import it into another file
using `import`/`source`/... directive, or run it in command line. For the latter,
`-h` or `--help` option can be specified on execution to show usage of script.
If there is a `Makefile` in a directory, then it's okay to run `make help` for help.

## 测试 | Test

* `C/C++`：在`c_and_cpp/native`子目录，执行`make check`（需安装clang和cppcheck）可进行代码静态检查，
执行`make test-execs`（需安装g++）或`make <模块名>`（例如`make signal_handling`），
再运行相应的可执行文件即可进行功能性测试。

> `C/C++`: In `c_and_cpp/native`sub-directory, execute `make check` (clang and cppcheck required)
to do code static checking, execute `make test-execs` (g++ required) or `make <module>` (e.g., `make signal_handling`),
and run the executable of a certain module to do functional test.

* `Python`：正在完善，敬请期待……

> `Python`: Under construction, please wait and see...

## 联系方式及反馈 | Contact and Feedback

* `Author`: Man Hung-Coeng

* `Email`: <udc577@126.com>

* `GitHub`: https://github.com/FooFooDamon/

## 许可证 | License

Apache License Version 2.0

