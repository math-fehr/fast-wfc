# fast-wfc

An implementation of [Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse) with a focus on performance.
At the time of writing, only the overlapping method has been implemented.

# Requirements

You need a C++-17 compatible compiler, and CMake installed.

# Getting started

```
git clone https://github.com/math-fehr/fast-wfc && cd fast-wfc/
cmake .
make install
```

This will install the library `fastwfc` using CMake, and also compile a example code in `example/`.

```
cd example/
./wfc_demo
```

will execute WFC on the examples defined in `example/samples.xml`, and will put the results in `example/results`.

# Performance

If fast-wfc is an order of magnitude faster than the original WFC algorithm, it is thanks to three main optimizations that have been made to the original algorithm, described below.

The first one of these changes the algorithm to propagate information. Instead of storing for each position which patterns are allowed, it stores for each position and each possible direction the number of compatible patterns allowed in the corresponding neighbor. If that number reaches zero for any neighbor, that pattern is no longer allowed in that position and this information must be propagated. Thus, the propagation no longer recurses on the position only, but instead on a pair consisting in a position and a pattern that is no longer allowed in that position. Thanks to this, propagation is only done when necessary.

The second change is related to the entropy: instead of being recomputed each time we need to find the position with the lowest entropy, it is only recomputed when a pattern is removed in a location, in O(1) time thanks to memoization of intermediate results.

The third and last major change is specific to the overlapping model: when information is propagated, it is only propagated to the four neighbours (when in 2D) instead of all other positions that share part of a tile. Indeed, the information will be propagated to these other locations by the recursive propagation algorithm with the immediate neighbors being intermediate steps. This has two important consequences: first and foremost, the overlapping model can now be seen as an instance of the tiling model, with tiles being the subregions of the original image. Second, it greatly reduces the memory footprint of the first optimization, which would otherwise probably actually slow the code instead.

Besides, care has been taken for the code to be cache-friendly and to leave enough room to the compiler to optimize the code as it sees fit. 

# Third-parties library

The files in `example/src/lib/` come from:
* RapidXML [https://github.com/dwd/rapidxml](https://github.com/dwd/rapidxml)
* stb Library [https://github.com/nothings/stb](https://github.com/nothings/stb)

# Image samples

The image samples come from [https://github.com/mxgmn/WaveFunctionCollapse](https://github.com/mxgmn/WaveFunctionCollapse)

# Licence 

Copyright (c) 2018-2019 Mathieu Fehr and Nathanaël Courant.

MIT License, see `LICENSE` for further details.
