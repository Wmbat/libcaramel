./: {*/ -build/ -documentation/} doc{README.md} manifest

# Don't install tests.
#
tests/: install = false
