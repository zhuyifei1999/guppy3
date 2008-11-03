#._cv_part guppy

"""\
Top level package of Guppy, a library and programming environment
currently providing in particular the Heapy subsystem, which supports
object and heap memory sizing, profiling and debugging.

What is exported is the following:

help	An object that provides interactive help,
	by printing it or evaluating at interactive prompt, it has
        also e.g. the attributes .help and .read.
hpy()	Create an object that provides a Heapy entry point.
Root()	Create an object that provides a top level entry point.

"""

__all__ = ('getdoc','hpy', 'Root')

import guppy.etc.Compat		# Do one-time compatibility adjustments
from guppy.etc.Glue import Root	# Get main Guppy entry point

from guppy import doc
help = doc.StaticHelp('guppy.html')

def hpy(ht = None):
    """\
Main entry point to the Heapy system.
Returns an object that provides a session context and will import
required modules on demand. Some commononly used methods are:

.heap() 		get a view of the current reachable heap
.iso(obj..) 	get information about specific objects 

The optional argument, useful for debugging heapy itself, is:

    ht     an alternative hiding tag

"""
    r = Root()
    if ht is not None:
	r.guppy.heapy.View._hiding_tag_ = ht
    return r.guppy.heapy.Use

def _get_dir(obj,*args,**kwds):
    """dir: GuppyDir
dir(options: str+)

A replacement for the builtin function dir(), providing a listing of
public attributes. It also has an attribute for each item in the
listing, for example:

>>> hpy().dir.heap

provides documentation for the heap method. The dir object is also
callable with an argument string specifying what to do. Currently the
following options are provided:

        'l'	Generate a listing of the synopsis lines.
	'L'	Generate a listing of the entire doc strings."""
    return Root().guppy.doc.getdir(obj,*args,**kwds)

def getdoc(obj,*args,**kwds):
    return Root().guppy.doc.getdoc(obj,*args,**kwds)

def getman(obj,*args,**kwds):
    return Root().guppy.doc.getman(obj,*args,**kwds)
    
