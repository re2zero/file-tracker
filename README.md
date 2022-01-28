Something like everything, but nothing is really like anything...

本包灵感来源 deepin-anything，加上有时需要跟踪文件的操作，抓出文件异常丢失的真凶。



# 编译

进入根目录，直接`make`即可编译生成对应的内核模块与用户态程序。编译所有程序需要安装`make`与`gcc`，编译内核模块需要安装内核头文件，编译用户态程序没有额外的依赖。编译生成的内核模块名为`vfs_tracker.ko`(使用了`-O3`参数优化)，在`kernelmod`目录下。

其中内核模块位于`kernelmod`目录，用户态服务程序位于`server`目录。当前，用户态程序支持所有平台，内核模块暂时仅支持x86与龙芯平台。

在运行环境上，内核模块需要依赖内核的CONFIG\_KPROBES选项，用户态程序仅依赖于glibc库。

# 日志说明

由于系统层程序会产生很多日志，用户只关心用户目录下的文件操作日志，因此只记录普遍的用户目录：

```shell
/home
/data
/media
```

文件操作日志采用循环回滚方式，依赖于`Dtk::Core::RollingFileAppender`，因此用户态程序需要依赖 DTK 编译。

日志记录在 `/var/cache/re2zero/file-tracker/footmark.log` 

# License 变更

file-tracker licensed under [GPLv3+](LICENSE)