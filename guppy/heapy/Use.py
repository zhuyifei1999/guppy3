#._cv_part guppy.heapy.Use

class _GLUECLAMP_:
    _preload_ = '_hiding_tag_',
    _chgable_ = ('reprefix', 'default_reprefix', 'gcobjs',
                 'relheap', 'relheapg', 'relheapu')


    default_reprefix = 'hpy().'

    def _get_gcobjs(self):
	return self.Nothing

    def _get_help(self):
        return self.Help(filename='heapy_Use.html')

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
        self.warnings.warn(
"Method Use.heapg is depreciated, it doesn't work well. Use heapu instead.")
	h = self.View.heapg(rma)
	h -= self.relheapg
	return h
	
    def heapu(self, rma=1, abs=0, stat=1):
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

    def test(self):
        self._parent.test.test_all.test_main()

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
	'_root.guppy.gsl.Help:doc',
	'_root.guppy.ihelp:Help',
        '_root.time:ctime',
        '_root:warnings',
	)

