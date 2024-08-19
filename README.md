
(Hama)Matsu
===========

Synthesized TR-606 samples. Done so far:

- [x] Kick
- [x] Snare
- [x] Closed hat
- [x] Open hat
- [x] Low tom
- [x] High tom
- [x] Cymbal
- [ ] Accentuated versions


Clone and compile code
----------------------
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
[argh](https://github.com/adishavit/argh) and [lodepng](https://github.com/lvandeve/lodepng),
cloned and statically compiled as part of above process.


License
-------
Code under MPL-2.0 license. Every file includes its respective notice.

Files created from code/programs here under no license, those are yours.
