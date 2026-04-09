<div align="center">
    <h1>zFetch</h1>
    <img src="imgs/preview.gif">
    <h6><i>Lightweight system information fetcher for Linux.</i></h6>

![](https://git.marcocetica.com/marco/zfetch/actions/workflows/build.yml/badge.svg)
![Static Badge](https://img.shields.io/badge/license-MIT-blue)
</div>

**zFetch** is a lightweight system information fetcher for Linux written in C99.
Unlike other system fetchers, this tool focuses on providing a simple, easy-to-remember
interface and configuration syntax, extremely fast startup and a reasonable amount
of core features.

## Installation
zFetch consists on a single source file which can be compiled with any modern C
compiler (Clang/GCC) on a Linux-based system. In order to build it, you can just run
`gcc zfetch.c -o zfetch` or use the following Makefile rule:

```sh
$ make clean all
rm -f *.o *.a zfetch
clang -Wall -Wextra -Werror -pedantic-errors -Wwrite-strings -std=c99 -O3 zfetch.c -o zfetch
```

In both cases, you will get a binary file named `zfetch` that you can move wherever you
want.

## Usage
In its simplest form, you can just invoke the program without any additional parameter and it will
run with all options enabled, that is:

![](imgs/ex1.png)

If you instead would like to granularly enable or disable some features,
you can do so by creating a configuration file in one of the following paths:

- `$HOME/.zfetch.conf`;  
- `$HOME/.config/zfetch/conf` (**this takes precedence**).

Inside it, you can specify which option to enable and which one to disable:

```conf
HOST     = 1
OS       = 0
UPTIME   = 1
CPU      = 1
MEMORY   = 1
DISK     = 1
IP       = 1
LOGO     = 1
BARS     = 0
SHELL    = 1
TERMINAL = 0
DESKTOP  = 1
INIT     = 1
BATTERY  = 1
```

Any other line will be considered invalid and silently skipped by the builtin parser.
To retrieve which options are currently enabled, you can run the program with the `-s` flag, that is:

```sh
$ ./zfetch  --list-opts # or -s
HOST     : ON
OS       : OFF
UPTIME   : ON
CPU      : ON
MEMORY   : ON
DISK     : ON
IP       : ON
LOGO     : ON
BARS     : OFF
SHELL    : ON
TERMINAL : OFF
DESKTOP  : ON
INIT     : ON
BATTERY  : ON
```

You can also dynamically specify a different path by using the `-c` CLI argument:

```sh
$ ./zfetch  -c $PWD/config -s
Using custom config file: '/home/marco/Projects/zfetch/config'
HOST     : ON
OS       : OFF
UPTIME   : OFF
CPU      : OFF
MEMORY   : ON
DISK     : ON
IP       : ON
LOGO     : ON
BARS     : OFF
SHELL    : ON
TERMINAL : OFF
DESKTOP  : ON
INIT     : ON
BATTERY  : ON
```

Finally, you can list all supported distribution, using the `--list-logos/-a` flag:

```sh
$ /zfetch -a
Available logos: alpine, arch, debian, fedora, mint, gentoo, artix, linux, nixos, redhat, slackware, suse, ubuntu.
```


## License & acknowledgement
This tool is distributed under the [MIT license](https://choosealicense.com/licenses/mit/), the logos were taken from
[pfetch](https://github.com/dylanaraps/pfetch), [neofetch](https://github.com/dylanaraps/neofetch) 
and [fastfetch](https://github.com/fastfetch-cli/fastfetch).
