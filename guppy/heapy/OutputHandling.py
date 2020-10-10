import types
import weakref


# To restore the old-style class behavior that __getattr__ also affects special
# methods.
class _AttrProxy:
    _oh_proxied_classes = weakref.WeakSet({type, object})

    # Don'r rely on _oh_add_proxy_attr to prime us, some clients use
    # setup_printing to setup __repr__ function.
    def __repr__(self):
        return self.__getattr__('__repr__')()

    @classmethod
    def _oh_add_proxy_attr(cls, attr):
        if not attr.startswith('__') or not attr.endswith('__'):
            return
        for scls in cls.__mro__:
            if scls is object:
                break
            if attr in scls.__dict__:
                return
        if attr in ('__new__', '__init__', '__getattr__',
                    '__getattribute__', '__setattr__', '__delattr__',
                    '__str__',
                    ):
            return

        def closure(attr):
            def generated_function(self, *args, **kwds):
                func = self.__getattr__(attr)
                return func(*args, **kwds)

            return generated_function

        setattr(cls, attr, closure(attr))

    @classmethod
    def _oh_add_proxy_class(cls, base):
        if base in cls._oh_proxied_classes:
            return

        for scls in base.__mro__:
            cls._oh_proxied_classes.add(scls)
            for attr, val in scls.__dict__.items():
                if not isinstance(val, types.FunctionType):
                    continue

                cls._oh_add_proxy_attr(attr)


class OutputHandler:
    def __init__(self, mod, output_file):
        self.mod = mod
        self.output_file = output_file


class OutputBuffer:
    def __init__(self, mod, opts=None):
        self.mod = mod
        self.strio = mod._root.io.StringIO()

        if opts is None:
            opts = {}
        self.opts = opts

        self.lines = ['']
        self.line_no = 0

    def getopt(self, opt):
        return self.opts.get(opt)

    def getvalue(self):
        return '\n'.join(self.lines)

    def new_line(self):
        self.line_no += 1
        self.lines.append('')

    def write(self, s):
        lines = s.split('\n')
        for line in lines[:-1]:
            self.write_seg(line)
            self.new_line()
        self.write_seg(lines[-1])

    def write_seg(self, s):
        self.lines[self.line_no] += s


class AllPrinter(_AttrProxy):
    _oh_next_lineno = None

    def __init__(self, printer):
        self._oh_printer = printer
        self._hiding_tag_ = printer._hiding_tag_

    def __getattr__(self, attr):
        return self._oh_printer.getattr(self, attr)

    def __repr__(self):
        return self._oh_printer.get_str(self, True)

    def _oh_get_next_lineno(self):
        return 0

    def _oh_get_start_lineno(self):
        return 0

    def _oh_get_max_lines(self, max_lines):
        return None


class MorePrinter(_AttrProxy):
    _oh_next_lineno = None

    def __init__(self, printer, previous):
        self._oh_printer = printer
        self._oh_previous = previous
        self._hiding_tag_ = printer._hiding_tag_

    def __getattr__(self, attr):
        return self._oh_printer.getattr(self, attr)

    def _oh_get_next_lineno(self):
        next_lineno = self._oh_next_lineno
        if next_lineno is None:
            repr(self)
            next_lineno = self._oh_next_lineno
        return next_lineno

    def _oh_get_start_lineno(self):
        return self._oh_previous._oh_get_next_lineno()

    def _oh_get_max_lines(self, max_lines):
        return max_lines


class Printer:
    def __init__(self, mod, client, handler,
                 get_line_iter=None,
                 max_more_lines=None,
                 get_num_lines=None,
                 get_label=None,
                 get_row_header=None,
                 get_more_msg=None,
                 get_more_state_msg=None,
                 get_empty_msg=None,
                 stop_only_when_told=None
                 ):
        try:
            handler = handler.printer.handler
        except AttributeError:
            pass

        if get_line_iter is None:
            get_line_iter = handler._oh_get_line_iter
        if max_more_lines is None:
            max_more_lines = mod.max_more_lines

        self.mod = mod
        self._hiding_tag_ = mod._hiding_tag_
        self.client = client
        self.handler = handler
        self.get_line_iter = get_line_iter
        self.max_more_lines = max_more_lines
        if get_num_lines is None:
            get_num_lines = getattr(
                handler, '_oh_get_num_lines', None)
        if get_num_lines is not None:
            self.get_num_lines = get_num_lines
        if get_label is None:
            get_label = getattr(
                handler, '_oh_get_label', None)
        if get_label is not None:
            self.get_label = get_label
        if get_row_header is None:
            get_row_header = getattr(
                handler, '_oh_get_row_header', None)
        if get_row_header is not None:
            self.get_row_header = get_row_header
        if get_more_msg is None:
            get_more_msg = getattr(
                handler, '_oh_get_more_msg', None)
        if get_more_msg is not None:
            self.get_more_msg = get_more_msg
        if get_more_state_msg is None:
            get_more_state_msg = getattr(
                handler, '_oh_get_more_state_msg', None)
        if get_more_state_msg is not None:
            self.get_more_state_msg = get_more_state_msg
        if get_empty_msg is None:
            get_empty_msg = getattr(
                handler, '_oh_get_empty_msg', None)
        if get_empty_msg is not None:
            self.get_empty_msg = get_empty_msg
        self.stop_only_when_told = stop_only_when_told
        self.reset()

    def getattr(self, mp, attr):
        try:
            g = getattr(self, '_get_'+attr)
        except AttributeError:
            return getattr(self.client, attr)
        else:
            return g(mp)

    def line_at(self, idx):
        while idx >= len(self.lines_seen):
            try:
                li = next(self.line_iter)
            except StopIteration:
                raise IndexError
            else:
                if isinstance(li, tuple):
                    cmd, line = li
                    if cmd == 'STOP_AFTER':
                        self.stop_linenos[len(self.lines_seen)] = 1
                else:
                    line = li
                self.lines_seen.append(line)

        return self.lines_seen[idx]

    def lines_from(self, idx=0):
        line_iter = self.line_iter
        if line_iter is None:
            line_iter = self.line_iter = self.get_line_iter()
        while 1:
            try:
                yield self.line_at(idx)
            except IndexError:
                return
            idx += 1

    def _get_more(self, mp):
        return MorePrinter(self, mp)

    def _get_all(self, mp):
        return AllPrinter(self)

    def _oh_get_next_lineno(self):
        next_lineno = getattr(self, '_oh_next_lineno', None)
        if next_lineno is None:
            self.get_str_of_top()
            next_lineno = self._oh_next_lineno
        return next_lineno

    def _get_prev(self, mp):
        return mp._oh_previous

    def _oh_get_start_lineno(self):
        return 0

    def _oh_get_max_lines(self, max_lines):
        return max_lines

    def _get_top(self, mp):
        return self.client

    def _get___repr__(self, mp):
        return lambda: self.get_str(mp, False)

    def get_label(self):
        return None

    def get_row_header(self):
        return None

    def get_str_of_top(self):
        return self.get_str(self, True)

    def get_more_state_msg(self, start_lineno, end_lineno):
        num_lines = self.get_num_lines()
        if num_lines is None:
            of_num_lines = ''
        else:
            of_num_lines = ' of %d' % num_lines
        return "Lines %d..%d%s. " % (start_lineno, end_lineno, of_num_lines)

    def get_more_msg(self, start_lineno, end_lineno):
        state_msg = self.get_more_state_msg(start_lineno, end_lineno)
        return "<%sType e.g. '_.more' for more.>" % (state_msg)

    def get_empty_msg(self):
        return None

    def get_num_lines(self):
        return None

    def get_str(self, printer, is_top):
        def f():
            _hiding_tag_ = printer._hiding_tag_
            start_lineno = printer._oh_get_start_lineno()
            m_max_lines = printer._oh_get_max_lines(self.max_more_lines)
            ob = self.mod.output_buffer()
            it = self.lines_from(start_lineno)

            if is_top:
                msg = self.get_label()
                if msg is not None:
                    print(msg, file=ob)
                printer._oh_next_lineno = start_lineno

            try:
                nxt = next(it)
            except StopIteration:
                msg = self.get_empty_msg()
                if msg is not None:
                    print(msg, file=ob)
                printer._oh_next_lineno = start_lineno
            else:
                msg = self.get_row_header()
                if msg is not None:
                    print(msg, file=ob)

                it = self.mod.itertools.chain((nxt,), it)
                numlines = 0
                lineno = start_lineno
                for line in it:
                    if (m_max_lines is not None and numlines >= m_max_lines and
                            ((not self.stop_only_when_told) or self.stop_linenos.get(lineno-1))):
                        try:
                            self.line_at(lineno+1)
                        except IndexError:
                            print(line, file=ob)
                            lineno += 1
                            break
                        else:
                            print(self.get_more_msg(start_lineno, lineno-1), file=ob)
                            break
                    numlines += 1
                    print(line, file=ob)
                    lineno += 1
                printer._oh_next_lineno = lineno
            return ob.getvalue().rstrip()

        return self.mod.View.enter(lambda: f())

    def reset(self):
        self.lines_seen = []
        self.stop_linenos = {}
        self.line_iter = None


class _GLUECLAMP_:
    _chgable_ = 'output_file', 'max_more_lines',
    _preload_ = ('_hiding_tag_',)

    _imports_ = (
        '_parent:View',
        '_root:itertools',
    )

    max_more_lines = 10

    def _get__hiding_tag_(self): return self._parent.View._hiding_tag_
    def _get_output_file(self): return self._root.sys.stdout

    def more_printer(self, client, handler=None, **kwds):
        if handler is None:
            handler = client
        printer = Printer(self, client, handler, **kwds)
        _AttrProxy._oh_add_proxy_class(client.__class__)
        return MorePrinter(printer, printer)

    def output_buffer(self):
        return OutputBuffer(self)

    def output_handler(self, output_file=None):
        if output_file is None:
            output_file = self.output_file
        return OutputHandler(self, output_file)

    def setup_printing(self, client, handler=None, **kwds):
        if handler is None:
            handler = client
        more = self.more_printer(client, handler, **kwds)
        printer = more._oh_printer
        client.more = more
        client.all = more.all
        client.printer = printer

        def reprfunc(self):
            return self.printer.get_str_of_top()

        # Don't recreate, messes with relheap, and this is not
        # under _hiding_tag_
        cls = client.__class__
        if '__repr__' not in cls.__dict__:
            cls.__repr__ = reprfunc

    def _get_stdout(self): return self._root.sys.stdout
