#._cv_part guppy.etc.PrettyPrintHtml.py

# Code copied (thanks!) from:
# http://www.bazza.com/~eaganj/weblog/2006/04/04/printing-html-as-text-in-python-with-unicode/


import codecs
import cStringIO
import formatter
import htmlentitydefs
import htmllib

class UnicodeHTMLParser(htmllib.HTMLParser):
    " HTMLParser that can handle unicode charrefs "
    
    entitydefs = dict([ (k, unichr(v)) for k, v in htmlentitydefs.name2codepoint.items() ])

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
            
def prettyPrintHTML(html):
    " Strip HTML formatting to produce plain text suitable for printing. "
    sio = cStringIO.StringIO()
    # cStringIO doesn’t like Unicode, so wrap with a utf8 encoder/decoder.
    encoder, decoder, reader, writer = codecs.lookup('utf8')
    utf8io = codecs.StreamReaderWriter(sio, reader, writer, 'replace')
    writer = formatter.DumbWriter(utf8io)
    prettifier = formatter.AbstractFormatter(writer)
    parser = UnicodeHTMLParser(prettifier)
    parser.feed(html)
    parser.close()
    utf8io.seek(0)
    result = utf8io.read()
    sio.close()
    utf8io.close()
    return result

