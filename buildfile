./: {*/ -build/ -docs/ -resources/} doc{README.md LICENSE} manifest

# Don't install tests.
#
tests/: install = false
