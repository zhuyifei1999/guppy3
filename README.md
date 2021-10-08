# Guppy 3
[![Build Status](https://img.shields.io/travis/com/zhuyifei1999/guppy3?label=tests)](https://app.travis-ci.com/github/zhuyifei1999/guppy3) [![Codecov](https://img.shields.io/codecov/c/github/zhuyifei1999/guppy3)](https://codecov.io/gh/zhuyifei1999/guppy3) [![PyPI version](https://img.shields.io/pypi/v/guppy3)](https://pypi.org/project/guppy3/) [![Repology - Repositories](https://img.shields.io/repology/repositories/python:guppy3)](https://repology.org/project/python:guppy3/versions)  
[![PyPI - Implementation](https://img.shields.io/pypi/implementation/guppy3)](https://pypi.org/project/guppy3/) [![PyPI - Python Version](https://img.shields.io/pypi/pyversions/guppy3)](https://pypi.org/project/guppy3/) [![PyPI - Downloads](https://img.shields.io/pypi/dm/guppy3)](https://pypistats.org/packages/guppy3) [![PyPI - License](https://img.shields.io/pypi/l/guppy3)](https://github.com/zhuyifei1999/guppy3/blob/master/LICENSE)

A Python Programming Environment & Heap analysis toolset.

This package contains the following subpackages:
* etc - Support modules. Contains especially the Glue protocol module.
* gsl - The Guppy Specification Language implementation. This can be used
  to create documents and tests from a common source.
* heapy - The heap analysis toolset. It can be used to find information about
  the objects in the heap and display the information in various ways.
* sets - Bitsets and 'nodesets' implemented in C.

Guppy 3 is a fork of Guppy-PE, created by Sverker Nilsson for Python 2.

## Requirements

You should have Python 3.6, 3.7, 3.8, or 3.9. This package is CPython only;
PyPy and other Python implementations are not supported. Python 2 support
can be obtained from [guppy-pe](http://guppy-pe.sourceforge.net/) by
Sverker Nilsson, from which this package is forked.

To use the graphical browser, Tkinter is needed.
To use the remote monitor, threading must be available.

## Installation

Install with pip by:

```
pip install guppy3
```

Install with conda by:
```
conda install -c conda-forge guppy3
```

## Usage

The following example shows

1. How to create the session context: `h=hpy()`
2. How to show the reachable objects in the heap: `h.heap()`
4. How to show the shortest paths from the root to the single largest object: `h.heap().byid[0].sp`
3. How to create and show a set of objects: `h.iso(1,[],{})`

```python
>>> from guppy import hpy; h=hpy()
>>> h.heap()
Partition of a set of 30976 objects. Total size = 3544220 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0   8292  27   739022  21    739022  21 str
     1   7834  25   625624  18   1364646  39 tuple
     2   2079   7   300624   8   1665270  47 types.CodeType
     3    400   1   297088   8   1962358  55 type
     4   4168  13   279278   8   2241636  63 bytes
     5   1869   6   269136   8   2510772  71 function
     6    400   1   228464   6   2739236  77 dict of type
     7     79   0   139704   4   2878940  81 dict of module
     8   1061   3    93368   3   2972308  84 types.WrapperDescriptorType
     9    172   1    81712   2   3054020  86 dict (no owner)
<89 more rows. Type e.g. '_.more' to view.>
>>> h.heap().byid[0].sp
 0: h.Root.i0_modules['os'].__dict__
>>> h.iso(1,[],{})
Partition of a set of 3 objects. Total size = 348 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0      1  33      248  71       248  71 dict (no owner)
     1      1  33       72  21       320  92 list
     2      1  33       28   8       348 100 int
>>>
```

People have written awesome posts on how to use this toolset, including:
* [How to use guppy/heapy for tracking down memory usage](https://smira.ru/wp-content/uploads/2011/08/heapy.html)
* [Debugging Django memory leak with TrackRefs and Guppy](https://opensourcehacker.com/2008/03/07/debugging-django-memory-leak-with-trackrefs-and-guppy/)
* [Diagnosing Memory "Leaks" in Python](https://chase-seibert.github.io/blog/2013/08/03/diagnosing-memory-leaks-python.html)
* [Digging into python memory issues in ckan with heapy](https://leastsignificant.blogspot.com/2015/06/digging-into-python-memory-issues-in.html)

Formal and API documentation are [also available](https://zhuyifei1999.github.io/guppy3/).

## Contributing

Issues and pull requests are welcome. You may also ask for help on using this
toolset; however, in such cases, we will only provide guidance, and not profile
your code for you.

Please make sure to update tests as appropriate.

### Testing

To test if the heapy build and installation was ok, you can do:

```python
>>> from guppy import hpy
>>> hpy().test()
Testing sets
Test #0
Test #1
Test #2
...
```

There will be several more tests. Some tests may take a while.

## License

Copyright (C) 2005-2013 Sverker Nilsson, S. Nilsson Computer System AB  
Copyright (C) 2019-2021 YiFei Zhu

The right is granted to copy, use, modify and redistribute this code
according to the rules in what is commonly referred to as an MIT
license.

*** USE AT YOUR OWN RISK AND BE AWARE THAT THIS IS AN EARLY RELEASE ***
