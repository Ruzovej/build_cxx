# `build_cxx`

## License

![LGPLv3 image](doc/lgplv3-with-text-154x68.png)

[`LGPLv3`](https://www.gnu.org/licenses/lgpl-3.0.html) -> [COPYING](COPYING) & [COPYING.lesser](COPYING.LESSER)

## TODO

- Rework (or delete) `tests/integration`
- Keep or delete `tests/system_bats`?
- Rework `tests/unit/cases/...`
- License
  - GPL for executables & test cases (this is missing currently), LGPL for libraries (currently for everything)
  - Or MIT license for everything?

## Progress

So far, it can't compile "build source code" itself, but can execute it:

```bash
$ scripts/compile.bash
...
# same one 3+ times:
$ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib03.so --target CCC::c1
... # it was created, etc.
$ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib03.so --target CCC::c1
... # no need to update (most probably, but there are some timestamp mismatches so it gets rebuild even when not necessary ...)
$ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib03.so --target CCC::c1
... # should definitely have everything properly up-to date now
# another one:
$ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib01.so build/tests/integration/lib02.so --target BBB::BBB -t AAA::a_phony_1
...
```
