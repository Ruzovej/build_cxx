# `build_cxx`

## License

![LGPLv3 image](doc/lgplv3-with-text-154x68.png)

[`LGPLv3`](https://www.gnu.org/licenses/lgpl-3.0.html) -> [COPYING](COPYING) & [COPYING.lesser](COPYING.LESSER)

## TODO

- Rework (or delete) `tests/integration`
- Keep or delete `tests/system_bats`?
- Rework `tests/unit/cases/...`
- License ... GPL for executables & test cases (this is missing currently), LGPL for libraries (currently for everything)

## Progress

So far, it can't compile "build source code" itself, but can execute it:

```bash
$ scripts/compile.bash
...
$ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib01.so build/tests/integration/lib02.so build/tests/integration/lib02.so
Processing target 'Root target' defined at '/home/lukas/tmp/build_cxx/tests/integration/01/build.cxx:24' (index 0):
I'm happy :-) - inside 'Root target'
Processing target '   A target' defined at '/home/lukas/tmp/build_cxx/tests/integration/01/a/build.cxx:24' (index 1):
I'm happy :-) - inside '   A target'
Processing target '   B target' defined at '/home/lukas/tmp/build_cxx/tests/integration/01/b/build.cxx:24' (index 2):
I'm happy :-) - inside '   B target'
Processing target '   C target' defined at '/home/lukas/tmp/build_cxx/tests/integration/01/b/c/build.cxx:24' (index 3):
I'm happy :-) - inside '   C target'
Processing target 'Root target BBB 1' defined at '/home/lukas/tmp/build_cxx/tests/integration/02/build.cxx:24' (index 0):
I'm happy :-) - inside 'Root target BBB 1'
Processing target 'Root target BBB 2' defined at '/home/lukas/tmp/build_cxx/tests/integration/02/build.cxx:28' (index 1):
I'm happy :-) - inside 'Root target BBB 2'
```
