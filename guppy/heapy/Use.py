#._cv_part guppy.heapy.Use

import guppy

class _GLUECLAMP_:
    _preload_ = '_hiding_tag_',
    _chgable_ = ('reprefix', 'default_reprefix', 'gcobjs',
                 'relheap', 'relheapg', 'relheapu')

    default_reprefix = 'hpy().'

    def _get_dir(self):
        return guppy.getdir(self)

    def _get_gcobjs(self):
	return self.Nothing

    def _get_help(self):
        return self.Help('heapy_Use.html')

    def _get_man(self):
        return guppy.getman(self,kind='tgt.heapykinds.Use',spec="guppy/specs/heapy_Use.gsl"
                            )

    def _get_relheap(self):
	return self.Nothing

    def _get_relheapg(self):
	return self.Nothing

    def _get_relheapu(self):
	return self.Nothing

    def _get_reprefix(self):
	# The name that this instance (or one with the same ._share)
	# has in the __main__ module, if any, or self.default_reprname otherwise.
	# Used for prefixing the result of repr() of various objects
	# so it becomes possible to evaluate it in a typical environment.
	import __main__
	for k, v in __main__.__dict__.items():
	    if (isinstance(v, self.__class__) and
		getattr(v, '_share', None) is self._share):
		return '%s.'%k
	return self.default_reprefix

    def _get_Root(self):
        return self.View.heapyc.RootState

    def __repr__(self):
        return """Top level interface to Heapy.
Look at '%shelp' for more info."""%self.reprefix
        

    __str__=__repr__

    def heapg(self, rma=1):
        """ DEPRECATED """
        self.warnings.warn(
"Method Use.heapg is depreciated, it doesn't work well. Use heapu instead.")
	h = self.View.heapg(rma)
	h -= self.relheapg
	return h
	
    def heapu(self, rma=1, abs=0, stat=1):
        """x.heapu() -> Stat 

Finds the objects in the heap that remain after garbage collection but
are _not_ reachable from the root.  This can be used to find objects
in extension modules that remain in memory even though they are
gc-collectable and not reachable.

Returns an object containing a statistical summary of the objects
found - not the objects themselves. This is to avoid making the
objects reachable.

See also: setref"""


	h = self.View.heapu(rma)
        rel = 0
        if not abs and self.relheapu and isinstance(self.relheapu, type(h)):
            h -= self.relheapu
            rel = 1
        if stat:
            h = h.stat
            if not abs and self.relheapu and isinstance(self.relheapu, type(h)):
                h -= self.relheapu
                rel = 1

            h.firstheader = 'Data from unreachable objects'

            if rel:
                h.firstheader += ' relative to: %s'%\
                                 self.ctime(self.relheapu.timemade)
            h.firstheader += '.\n'
            

	return h
	
    def heap(self):
        """x.heap() -> IdentitySet

Traverse the heap from a root to find all reachable and visible
objects. The objects that belong to a heapy instance are normally not
included. Return an IdentitySet with the objects found, which is
presented as a table partitioned according to a default equivalence
relation (Clodo).  """

	h = self.View.heap()
	h |= self.gcobjs
	h -= self.relheap
	return h

    def load(self, fn, usereadline=0):
	if isinstance(fn, basestring):
	    # We got a filename.
	    # I want to read only what is being requested
	    # so I can look quickly at some lines of a long table.
	    # (There are seemingly easier ways to do this
	    #  but this takes care of some tricky details.
	    #  Keeping f open avoids it to be overwritten
	    #  (at least by Stat.dump() and if OS=Linux)
	    #  if data are written to a new file with the same name.)
	    f = open(fn)
	    def get_trows():
		pos = 0
		while 1:
		    f.seek(pos)
		    line = f.readline()
		    if not line:
			break
		    pos = f.tell()
		    yield line
	elif hasattr(fn, '__iter__') and not hasattr(fn, 'next'):
	    # We got a sequence, that is not an iterator. Use it directly.
	    def get_trows():
		return fn
	elif hasattr(fn, 'next'):
	    # We got an iterator or file object.
	    # We 'have' to read all lines (at once)-
	    # to update the read position -
	    # to mimic 'pickle' semantics if several
	    # objects are stored in the same file.
	    # We can't use .next always - (eg not on pipes)
	    # it makes a big readahead (regardless of buffering setting).
	    # But since .next() (typically) is much faster, we use it
	    # per default unless usereadline is set.
	    if usereadline:
		get_line = fn.readline
	    else:
		get_line = fn.next

	    trows = []
	    line = get_line()
	    if not line:
		raise StopIteration
	    endline = '.end: %s'%line
	    try:
		while line:
		    trows.append(line)
		    if line == endline:
			break
		    line = get_line()
		else:
		    raise StopIteration
	    except StopIteration:
		trows.append(endline)

	    def get_trows():
		return trows
	else:
	    raise TypeError, 'Argument should be a string, file or an iterable yielding strings.'

	a = iter(get_trows()).next()
	if not a.startswith('.loader:'):
	    raise ValueError, 'Format error in %r: no initial .loader directive.'%fn
	loader = a[a.index(':')+1:].strip()
	try:
	    loader = getattr(self, loader)
	except AttributeError:
	    raise ValueError, 'Format error in %r: no such loader: %r.'%(fn, loader)
	return loader(get_trows)
	
    def loadc(self, fn):
	f = open(fn, 'r', 1)
	while 1:
	    print self.load(f, usereadline=1)
	    
    def dumph(self, fn):
	f = open(fn, 'w')
	import gc
	while 1:
	    x = self.heap()
	    x.stat.dump(f)
	    f.flush()
	    print len(gc.get_objects())

    def setref(self, reachable=None, unreachable=None):
        if reachable is None and unreachable is None:
            self.setrelheap()
            self.setrelheapu()
        else:
            if reachable is not None:
                self.setrelheap(reachable)
            if unreachable is not None:
                self.setrelheapu(unreachable)

    def setrelheap(self, reference=None):
	if reference is None:
	    reference = self.View.heap()
	self.relheap = reference

    def setrelheapg(self, reference=None):
        self.warnings.warn(
"Method Use.setrelheapg is depreciated, use setref instead.")
	if reference is None:
            self.relheapg = None
	    reference = self.View.heapg()
	self.relheapg = reference

    def setrelheapu(self, reference=None,stat=1):
	if reference is None:
            self.relheapu = None
	    reference = self.heapu(abs=True, stat=stat)
        if stat and not isinstance(reference, self.Stat):
            reference = reference.stat
        self.relheapu = reference

    def test(self, debug=False):
        self._parent.test.test_all.test_main(debug)

    _imports_ = (
	'_parent.Classifiers:Class',
	'_parent.Classifiers:Clodo',
	'_parent.Classifiers:Id',
	'_parent.Classifiers:Idset',
	'_parent.Classifiers:Module',
	'_parent.Classifiers:Rcs',
	'_parent.Classifiers:Size',
	'_parent.Classifiers:Type',
	'_parent.Classifiers:Unity',
	'_parent.Classifiers:Via',
	'_parent.Classifiers:findex',
	'_parent.Classifiers:sokind',
	'_parent.Classifiers:sonokind',
	'_parent.Classifiers:tc_adapt',
	'_parent.Classifiers:tc_repr',
	'_parent.Monitor:monitor',
	'_parent.Part:_load_stat',
	'_parent.Part:Stat',
	'_parent.Prof:pb',
	'_parent.UniSet:Anything',
	'_parent.UniSet:idset',
	'_parent.UniSet:iso',
	'_parent.UniSet:Nothing',
	'_parent.UniSet:union',
	'_parent.UniSet:uniset_from_setcastable',
	'_parent:View',
	'_parent.View:_hiding_tag_',
	'_root.guppy.doc:Help',
        '_root.time:ctime',
        '_root:warnings',
	)

    _doc_Class ="""x.Class:EquivalenceRelation
x.Class(tc:typeorclass+) -> Kind

Equivalence relation by class. It defines objects to be equivalent
when their builtin __class__ attributes are identical. When called it
returns the equivalenc class defined by the argument:

    tc: A type or class that the returned kind should represent."""

    _doc_Clodo ="""x.Clodo:EquivalenceRelation
x.Clodo(alt:[tc: typeorclassexceptdict+ or dictof =
        typeorclassoremptytuple+]) -> Kind

Equivalence relation by class or dict owner. It distinguishes between
objects based on their class just like the Class relation, and in
addition distinguishes between dicts depending on what class they are
'owned' by, i.e. occur in __dict__ attribute of.

When called it returns the equivalence class defined by the argument,

EITHER:
    tc: A positional argument, a type or class but not a dict, to
        create the corresponding equivalence class.
OR:
    dictof: A named argument, to create an equivalence class
        consisting of all dicts that are owned by objects of the type
        or class specified in the argument; or dicts with no owner if
        an empty tuple is given. XXX express this simpler&better..."""

    _doc_Id="""x.Id:EquivalenceRelation
x.Id(address: objectaddress+) -> Kind)

This equivalence relation defines objects to be equivalent only if
they are identical, i.e. have the same address. When called it returns
the equivalence class defined by the argument:

    address: The memory address of an object."""

    _doc_Idset = """x.Idset:EquivalenceRelation
XXX missing doc, deprecated?"""


    _doc_Module = """x.Module:EquivalenceRelation
x.Module( draw:[name = modulename+ , at = moduleaddress+]) -> Kind

This equivalence relation defines objects to be equivalent if they are
the same module, or if none of them is a module.  Partitioning a set
of objects using this equivalence relation will therefore result in
one singleton set for each module and one set containing all other
objects.

Calling the Module equivalence relation creates a Kind containing the
module given in the keyword argument(s). Either the name, address or
both may be specified. If no argument is specified the equivalence
class is that of non-module objects."""

    _doc_Nothing = """Nothing: IdentitySet

The empty set."""

    _doc_Rcs = """Rcs: EquivalenceRelation
callable: ( 0..*: alt:[kind: Kind+ or sok: SetOfKind+]) ->
    KindOfRetClaSetFamily

(Referrer classification set.)

In this equivalence relation, objects are classified by classifying
their referrers. The classification of the referrers is done using the
classifier of the Clodo equivalence relation. The classifications of
the referrers are collected in a set. This set represents the
classification of the object.

Calling Rcs creates an equivalence class from user-specified
classification arguments. The arguments specify a set of Kind objects,
each of which representing an equivalence class of Clodo.

    kind: Kind+
        This adds a single Kind to the set of Kinds of referrers.
    sok: SetOfKind+
        This adds each Kind in the sok argument to the total set of
        Kinds of referrers."""

