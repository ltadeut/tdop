# Top-down operator precedence parsing

The code is meant to accompany some notes I wrote on how Top-down operator
precedence parsing (Pratt) works.

You can find the article on https://lucastadeu.com/notes/top-down-operator-precedence-parsing.

## Building

### Linux or macOS

You'll need GCC with C++ 17 support installed. If you're running Ubuntu, you
can get it from `apt`.

```shell
sudo apt install gcc
```

If you're on macOS, you can get GCC through [homebrew](https://brew.sh).

```shell
brew install gcc
```

Once you have GCC installed on your machine, run the `./build.sh` script.
A directory called `build` will be created at the root of this repository. The
executable will be placed there under the name`PrecedenceParser`.

## License

All content in this repository is provided under the public domain license.

See the LICENSE file at the root of this repository for more information.
