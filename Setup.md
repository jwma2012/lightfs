1. 下载源码
`git clone https://github.com/thustorage/octopus.git`
2. 安装一些必要依赖，如GCC（4.9及以上）、cmake（现在用的3.3.1）
由于ubuntu14.04的gcc版本是4.8.4,需要安装高版本的。下载了gcc 4.9.1之后，配置的时候发现
`configure: error: Building GCC requires GMP 4.2+, MPFR 2.4.0+ and MPC 0.8.0+.`
按照网上提示下载对应的文件
[ftp://gnu.mirror.iweb.com/gmp/gmp-4.3.2.tar.gz](ftp://gnu.mirror.iweb.com/gmp/gmp-4.3.2.tar.gz)
http://ftp.gnu.org/gnu/mpfr/
选择了3.1.0版本
[ftp://gnu.mirror.iweb.com/mpc/mpc-1.0.1.tar.gz](ftp://gnu.mirror.iweb.com/mpc/mpc-1.0.1.tar.gz ) 
下了之后按一般步骤安装
安装mpfr时
```
configure: WARNING: ==========================================================
configure: WARNING: 'gmp.h' and 'libgmp' seem to have different versions or
configure: WARNING: we cannot run a program linked with GMP (if you cannot
configure: WARNING: see the version numbers above). A cause may be different
configure: WARNING: GMP versions with different ABI's or the use of --with-gmp
configure: WARNING: or --with-gmp-include with a system include directory
configure: WARNING: (such as /usr/include or /usr/local/include).
configure: WARNING: However since we can't use 'libtool' inside the configure,
configure: WARNING: we can't be sure. See 'config.log' for details.
configure: WARNING: ==========================================================
```
原因多数是因为没有添加 `--with-gmp-include` 而造成的。 只要添加 
`--with-gmp-include=/usr/local/include --with-gmp-lib=/usr/local/lib`
即可，否则 很可能在 make check 的时候会出现以下错误`__gmp_get_memory_functions`
但是mpc1.0.1要求 `configure: error: GMP version >= 4.3.2 required`
重新安装gmp4.3.2
安装mpfr和mpc时，`./configure --with-gmp-include=/usr/local/include --wtth-gmp-lib=/usr/local/lib`
使用```./configure --enable-checking=release --enable-languages=c,c++ --disable-multilib```
此外，也可以使用`./contrib/download_prerequisites` 
直接下载所需的依赖。
3. 其他必要依赖，如fuse，cryptopp(用于加密),mpicxx(安装mpich或openmpi，我选的mpich)
https://www.cryptopp.com/release565.html 
安装cryptopp
`unzip cryptopp565.zip`
(注意：它会将解压后的文件放到当前文件夹，需要提前新建一个解压目录）
解压之后就有`GNUmakefile`,直接`make`，然后`sudo make install`

4. 在编译/链接阶段出现
gcc编译时对'xxxx'未定义的引用问题
```
libnrfsc.so：对‘ibv_dereg_mr’未定义的引用
libnrfsc.so：对‘ibv_free_device_list’未定义的引用
```
这个主要的原因是gcc编译的时候，各个文件依赖顺序的问题。
在gcc编译的时候，如果文件a依赖于文件b，那么编译的时候必须把a放前面，b放后面。
例如:在main.c中使用了temp，那么编译的时候必须是main.c在前，temp在后。
上面出现问题的原因就是引入库的顺序在前面了，将其放置在后面即可了。
在本次中也有遇到这种问题，通过修改link.txt来改链接顺序。
（猜测是因为编译器版本的原因，就是在链接的时候顺序不对，高版本更加智能，所以没报错）经过在同一个机器上验证，确实是这样。


