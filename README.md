# Guppy 3
[![Build Status](https://travis-ci.com/zhuyifei1999/guppy3.svg?branch=master)](https://travis-ci.com/zhuyifei1999/guppy3) [![PyPI version](https://badge.fury.io/py/guppy3.svg)](https://pypi.org/project/guppy3/) ![PyPI - Implementation](https://img.shields.io/pypi/implementation/guppy3) ![PyPI - Python Version](https://img.shields.io/pypi/pyversions/guppy3) ![PyPI - License](https://img.shields.io/pypi/l/guppy3)

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

You should have Python 3.5, 3.6, or 3.7. This package is CPython only;
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

## Usage

The following example shows

1. How to create the session context: `h=hpy()`
2. How to show the reachable objects in the heap: `h.heap()`
4. How to show the shortest paths from the root to the single largest object: `h.heap().byid[0].sp`
3. How to create and show a set of objects: `h.iso(1,[],{})`

```python
>>> from guppy import hpy; h=hpy()
>>> h.heap()
Partition of a set of 30940 objects. Total size = 3562865 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0   8233  27   737593  21    737593  21 str
     1   7827  25   625128  18   1362721  38 tuple
     2    400   1   353952  10   1716673  48 type
     3   2077   7   299088   8   2015761  57 types.CodeType
     4   4164  13   296152   8   2311913  65 bytes
     5   1867   6   268848   8   2580761  72 function
     6    400   1   230376   6   2811137  79 dict of type
     7     79   0   139704   4   2950841  83 dict of module
     8   1080   3    95040   3   3045881  85 types.WrapperDescriptorType
     9    169   1    76504   2   3122385  88 dict (no owner)
<87 more rows. Type e.g. '_.more' to view.>
>>> h.heap().byid[0].sp
 0: h.Root.i0_modules['os'].__dict__
>>> h.iso(1,[],{})
Partition of a set of 3 objects. Total size = 352 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0      1  33      248  70       248  70 dict (no owner)
     1      1  33       72  20       320  91 list
     2      1  33       32   9       352 100 int
>>>
```

People have written awesome posts on how to use this toolset, including:
* [How to use guppy/heapy for tracking down memory usage](https://pkgcore.readthedocs.io/en/latest/dev-notes/heapy.html)
* [Debugging Django memory leak with TrackRefs and Guppy](https://opensourcehacker.com/2008/03/07/debugging-django-memory-leak-with-trackrefs-and-guppy/)
* [Diagnosing Memory “Leaks” in Python](https://chase-seibert.github.io/blog/2013/08/03/diagnosing-memory-leaks-python.html)

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
Copyright (C) 2019 YiFei Zhu

The right is granted to copy, use, modify and redistribute this code
according to the rules in what is commonly referred to as an MIT
license.

*** USE AT YOUR OWN RISK AND BE AWARE THAT THIS IS AN EARLY RELEASE ***
