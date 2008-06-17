# -*- coding: utf-8 -*-
#._cv_part guppy.doc

# The prettyPrintHTML Code is adapted from:
# http://www.bazza.com/~eaganj/weblog/2006/04/04/printing-html-as-text-in-python-with-unicode/

import guppy
import os

import codecs
import cStringIO
import formatter
import htmlentitydefs
import htmllib
import urllib2 as urllib

from htmllib import HTMLParser

THISDIR = os.path.dirname(__file__)

def utf8StringIO():
    sio = cStringIO.StringIO()
    # cStringIO doesn't like Unicode, so wrap with a utf8 encoder/decoder.
    encoder, decoder, reader, writer = codecs.lookup('utf8')
    return codecs.StreamReaderWriter(sio, reader, writer, 'replace')

class HelpTextWriter(formatter.DumbWriter):
    def reset(self):
        formatter.DumbWriter.reset(self)
        self.margin = 0

    def new_margin(self, tag, level):
        #print >>self.file, "new_margin(%s, %d)" % (`margin`, level)
        self.margin = level*4

    def indento_margin(self):
        if self.col < self.margin:
            self.file.write(' '*(self.margin-self.col))
            self.col = self.margin

    def _send_literal_data(self, data):
        rows = data.split('\n')
        self.file.write(data)
        i = data.rfind('\n')
        if i >= 0:
            self.col = 0
            data = data[i+1:]
        data = data.expandtabs()
        self.col = self.col + len(data)
        self.atbreak = 0

    def send_flowing_data(self, data):
        if not data: return
        self.indento_margin()
        atbreak = self.atbreak or data[0].isspace()
        col = self.col
        maxcol = self.maxcol
        write = self.file.write
        for word in data.split():
            if atbreak:
                if col + len(word) >= maxcol:
                    self.file.write('\n'+' '*self.margin)
                    col = self.margin
                else:
                    write(' ')
                    col = col + 1
            write(word)
            col = col + len(word)
            atbreak = 1
        assert col >= self.margin
        self.col = col
        self.atbreak = data[-1].isspace()

class HelpTextHTMLParser(HTMLParser):
    " HTMLParser tailored to handle help text and can handle unicode charrefs"

    entitydefs = dict([ (k, unichr(v)) for k, v in htmlentitydefs.name2codepoint.items() ])

    def reset(self):
        HTMLParser.reset(self)
        self.index2href = []
        self.href2index = {}
        self.data2hrefs = {}
        self.name2textpos = {}

    def handle_charref(self, name):
        "Override builtin version to return unicode instead of binary strings for 8-bit chars."
        try:
            n = int(name)
        except ValueError:
            self.unknown_charref(name)
            return
        if not 0 <= n <= 255:
            self.unknown_charref(name)
            return
        if 0 <= n <= 127:
            self.handle_data(chr(n))
        else:
            self.handle_data(unichr(n))
            
    # --- Hooks for anchors

    def anchor_bgn(self, href, name, type):
        """This method is called at the start of an anchor region.

        The arguments correspond to the attributes of the <A> tag with
        the same names.  The implementation maintains a dictionary of
        hyperlinks (defined by the HREF attribute for <A> tags) within
        the document.  The dict of hyperlinks is available as the data
        attribute anchordict. It also keeps a list of unique hyperlinks.

        """
        self.anchor = (href, name, type)

        if name:
            self.name2textpos[name] = self.formatter.writer.file.tell()

        if href:
            self.save_bgn()

    def anchor_end(self):
        """This method is called at the end of an anchor region.

        The implementation adds a textual footnote marker using an
        index into the list of hyperlinks created by the anchor_bgn()method.

        """
        href, name, type = self.anchor
        if href:
            data = self.save_end()
            self.handle_data(data)
            data = data.strip()
            if href in self.href2index:
                index = self.href2index[href]
            else:
                index = len(self.index2href)
                self.href2index[href] = index
                self.index2href.append(href)
            self.handle_data("[%d]" % index)
            
            self.data2hrefs.setdefault(data,{})[index]=href

        self.anchor = None

    # --- Headings

    def start_hx(self, level, attrs):
        if not (self.list_stack and self.list_stack[-1][0] == 'dd'):
            self.formatter.end_paragraph(1)
        else:
            self.formatter.end_paragraph(0)
        self.formatter.push_font(('h%d'%level, 0, 1, 0))

    def start_h1(self, attrs):
        self.start_hx(1, attrs)

    def start_h2(self, attrs):
        self.start_hx(2, attrs)

    def start_h3(self, attrs):
        self.start_hx(3, attrs)

    def start_h4(self, attrs):
        self.start_hx(4, attrs)

    def start_h5(self, attrs):
        self.start_hx(5, attrs)

    def start_h6(self, attrs):
        self.start_hx(6, attrs)

    def end_hx(self):
        self.formatter.end_paragraph(0)
        self.formatter.pop_font()

    end_h1 = end_h2 = end_h3 = end_h4 = end_h5 = end_h6 = end_hx

    # --- List Elements

    def start_dl(self, attrs):
        self.formatter.end_paragraph(0)
        self.list_stack.append(['dl', '', 0])

class Document:
    ''' Abstracts a document, derived from one HTML file '''
    def __init__(self, mod, url):
        self.mod = mod
        self.url = url
        self.reset()

    def getdoc(self, hrefdata=None, url=None, idx=None):
        try:
            hrefs = self.doc.hrefsbydata(name)
        except KeyError:
            print "Help text has no link %r"%name
            self.print_available_links()
        
        return hrefs

    def hrefsbydata(self, data):
        return self.parser.data2hrefs[data]

    def hrefbyindex(self, idx):
        return self.parser.index2href[idx]

    def urlbyindex(self, idx):
        return self.urlbyhref(self.hrefbyindex(idx))

    def urlsbydata(self, data):
        return dict([(key, self.urlbyhref(href)) for (key, href) in
                     self.hrefsbydata(data).items()])

    def urlbyhref(self, href):
        if href.startswith('#'):
            href = self.url+href
        return href
        
    def reset(self):
        html = self.readhtml()
        self.text = self.prettyPrintHTML(html)

    def readhtml(self):
        return urllib.urlopen(self.url).read()

    def prettyPrintHTML(self, html):
        " Strip HTML formatting to produce plain text suitable for printing. "
        utf8io = utf8StringIO()
        writer = HelpTextWriter(utf8io)
        prettifier = formatter.AbstractFormatter(writer)
        self.parser = parser = HelpTextHTMLParser(prettifier)
        parser.feed(html)
        parser.close()


        utf8io.seek(0)
        result = utf8io.read()
        utf8io.close()
        return result #.lstrip()

class Subject:
    def __init__(self, mod, doc, fragment):
        self.mod = mod
        self.doc = doc
        self.fragment = fragment
        text = doc.text
        if fragment:
            text = text[doc.parser.name2textpos[fragment]:]
            fragment = '#'+fragment
        text = '--- url = %s%s\n'%(os.path.basename(doc.url),fragment)+text

        self.text = text
                        

class HelpHandler:
    ''' Handles a particular help strategy
    '''

    def __init__(self, mod, top):
        self.mod = mod
        self.top = top
        self.output_buffer = self.mod._parent.etc.OutputHandling.output_buffer()

    def get_more_index(self, firstindex):
        return firstindex+self.mod.pagerows

    def ppob(self, ob, index):
        text = self.get_text()
        splitext = text.split('\n')
        nextindex = index+self.mod.pagerows
        pst = splitext [index:nextindex]
        print >>ob, '\n'.join(pst)
        if nextindex < len(splitext):
            numore = len(splitext) - nextindex
            if numore == 1:
                pst.extend(splitext[-1:])
            else:
                print >>ob, \
"<%d more rows. Type e.g. '_.more' for more or '_.help' for help on help.>" \
		%numore
        self.nextindex = nextindex
    
    def get_text(self):
        text = self.top.subject.text
        return text

class Help:
    """\
Help class
"""
    _gslspec_ = """\


"""

    def __init__(self, mod, url):

        self.mod = mod
        self.url = url		# Universal Subject Locator
        self.subject = mod.getsubject(url)
        self.handler = HelpHandler(self.mod, self)

    def __getattr__(self, attr):
        while 1:
            if attr.startswith('go'):
                return self.go(attr[2:])
            else:
                print 'attr', attr
                raise AttributeError, attr

    def __getitem__(self, idx):
        return self.go(str(idx))

    def __repr__(self):
        hh = self.handler
        mp = self.mod._root.guppy.etc.OutputHandling.basic_more_printer(
            self, hh, 0)
        r = repr(mp)
        self.nextindex = hh.nextindex
        return r

    def _get_help(self):
        return Help(text=self.__doc__)

    def _get_more(self):
        repr(self)	# Just to calculate nextindex ...
        mp = self.mod._root.guppy.etc.OutputHandling.basic_more_printer(
            self, self.handler, self.nextindex)
        return mp
    
    def urlbydata(self, name):
        while 1:
            try:
                urls = self.subject.doc.urlsbydata(name)
            except KeyError:
                print "Help text has no link %r"%name
                self.print_available_links()
            else:
                if len(urls) == 1:
                    return urls.values()[0]
                print "Ambiguos name: ", urls

    def urlbyindex(self, index):
        try:
            url = self.subject.doc.urlbyindex(index)
        except KeyError:
            print "Help text has no link %r"%index
        return url

    def urlby(self, name):
        if name.isdigit():
            url = self.urlbyindex(int(name))
        else:
            url = self.urlbydata(name)
        return url

    def go(self, name):
        return self.gourl(self.urlby(name))

    def gourl(self, url):
        return self.mod.Help(url)
    
        
        

    help = property(fget=_get_help)
    more = property(fget=_get_more)


class StaticHelp:
    ''' For use in modules and classes to minimize memory impact.
        Does not cache anything but creates a (dynamic) Help instance
        each time it is used, which it acts as a proxy to.
    '''

    def __init__(self, *args, **kwds):
        self._args = args
        self._kwds = kwds

    def __getattr__(self, attr):
        mod = guppy.Root().guppy.doc
        dynhelp = Help(mod, *self._args, **self._kwds)
        return getattr(dynhelp, attr)

class X:
    def __repr__(self):
        return 'å'

class _GLUECLAMP_:
    #'public'

    pagerows=14		# Default number of rows to show at a time

    def Help(self, *args, **kwds):
        return Help(self, *args, **kwds)
    
    #'private'
    
    _chgable_=('pagerows',)
    _imports_ = (
        '_root.urlparse:urldefrag',
        )
    
    def _get_url2doc(self):
        return {}
    
    def fixurl(self, url):
        if '://' not in url:
            if not os.path.isabs(url):
                url = os.path.join(THISDIR,url)
            url = 'file://'+url
        return url

    def getdoc(self, url):
        if url in self.url2doc:
            return self.url2doc[url]
        doc = Document(self, url)
        self.url2doc[url] = doc
        return doc

    def getsubject(self, url):
        url = self.fixurl(url)
        url, frag = self.urldefrag(url)
        doc = self.getdoc(url)
        return self.Subject(self, doc, frag)

    def help_instance(self, inst):
        url = inst._help_url_
        return self.Help(url)
