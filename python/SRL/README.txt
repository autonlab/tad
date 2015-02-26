===========
SRL
===========

The Library (SRL) provides some convinient Python wrappers for
interfacing with an SRL interface layer.

To build a pip-installable distributable package, in SRL run

    python setup.py sdist

which will build a package in the SRL/dist directory. To install
the SRL package, use

    pip install dist/SRL.<version>.tar.gz

This can be done in a virtualenv to avoid installing as a root
package.
