## Building on Ubuntu

You need the following packages:

```
sudo apt install build-essential clang llvm libelf-dev ethtool
```

You also need to enable the `libpbf` submodule:

```
cd libbpf
git submodule update --init
```

You can now enter the `src` directory and build with `make`.
