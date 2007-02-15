from distutils.core import setup, Extension
import sys
from glob import glob

setsc = Extension("guppy.sets.setsc",
		  [
		      "src/sets/sets.c",
		      "src/sets/bitset.c",
		      "src/sets/nodeset.c"
		      ]
		  )

heapyc = Extension("guppy.heapy.heapyc",
		   [
		       'src/heapy/heapyc.c',
		       'src/heapy/stdtypes.c'
		       ]
		   )

def doit():
    setup(name="guppy",
      version="0.1.7",
      description="Guppy-PE -- A Python Programming Environment",
      long_description="""\
The guppy top-level package contains the following subpackages:

etc	Support modules. Contains especially the Glue protocol module.

gsl	The Guppy Specification Language implementation. This can
	be used to create documents and tests from a common source.

heapy	The heap analysis toolset. It can be used to find information
	about the objects in the heap and display the information
	in various ways.

sets	Bitsets and 'nodesets' implemented in C.


""",
      author="Sverker Nilsson",
      author_email="sn@sncs.se",
      url="http://guppy-pe.sourceforge.net",
      license='MIT',
      packages=[
      "guppy",
      "guppy.etc",
      "guppy.gsl",
      "guppy.heapy",
      "guppy.heapy.test",
      "guppy.sets",
      ],
      ext_modules=[setsc, heapyc])
doit()

