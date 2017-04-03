# SBLib
This is some work-in-progress library oriented on mathematical-physics which I do as a hobby.
At the moment, development is focused on developping template Clifford algebra library, including linear/multilinear algebra,
wedge(outer) product and hodge conjugation. For now, only Euclidian space (with +1 signature) is implemented. Support for other signatures
(including null space required for affine/conformal spaces) should come one day...

Please note that multi-platform is currently not a priority so it now requires Visual Studio 2017 :
newly introduced c++17 features in VC++ are being used. Moreover, future optimizations
(that should make use of SSE/AVX) and UI/graphical stuff may become platform specific and chances are that they will be focused
on Windows 10/DirectX12 API.

## SBLib
In the future, this should be compiled as a library (either .lib or .dll) for use of common non-inline stuff but it is currently
used as runtime testing of the development : These are not unit tests! Unit tests (c.f., next section) are stable tests verifying
integrety of the library. These tests are only there to do experiment / samples. At some time, all remaining tests should go
into samples.

## SBLib_unit_tests
This project is where unit tests (including statically checked data) are being tested to make sure everything is working as
intended upon modifications. For now, only binomial_coefficients template values and clifford space template values are being tested. 
