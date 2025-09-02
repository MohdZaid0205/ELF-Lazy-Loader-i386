# ELF-Loader-i386

![](https://capsule-render.vercel.app/api?type=venom&height=300&color=gradient&text=ELF-Loader-i386&desc=OS%20Assignment%20Implementation&descAlignY=64&descAlign=65)

![Linux](https://img.shields.io/badge/Ubuntu-E95420?style=for-the-badge&logo=ubuntu&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)

Executable and Linkable Format Loader for `i386(x86)` architecture made in `c` for Assignment 1 provided in `OS` course.

## Usage

In order to compile this project you may need to install i386(x86) development kit for c compiler.

```bash
sudo apt-get install gcc-multilib libc6-dev-i386
```

After installing required comipler and tools, use GNU Make to make this project

```bash
make prepare compile
```

Or just run make default, now you have executable and shared library in `bin` folder.
Use executable generated in bin named `launch` as follows.

```bash
launch <filename>
```

## Contributors
- [Mohd Zaid](https://github.com/MohdZaid0205)
