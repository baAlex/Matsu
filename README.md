
(Hama)Matsu! [浜松]
==================

Synthesized TR-606 samples. Done so far:

- [x] Kick
- [x] Snare
- [x] Closed hat
- [x] Open hat
- [x] Low tom
- [x] High tom
- [ ] Cymbal
- [ ] Accentuated versions


Clone and compile
-----------------
With `git`, `cmake` and a C++14 compiler, cloning and compilation should be:

```
git clone --recurse-submodules https://github.com/baAlex/Matsu
cd Matsu
mkdir build
cd build
cmake ..
make
```

Dependencies [dr_libs](https://github.com/mackron/dr_libs), [pffft](https://bitbucket.org/jpommier/pffft),
[cargs](https://github.com/likle/cargs) and [lodepng](https://github.com/lvandeve/lodepng),
cloned and statically compiled as part of above process.


License
-------
Under MPL-2.0 license. Every file includes its respective notice.

Files created from code/programs here under no license, those are yours.
