from guppy.heapy.test import support

PORTABLE_TEST = 1       # Relax tests to be more portable


class IdentityCase(support.TestCase):
    def test_1(self):
        import itertools
        import random
        import sys
        vs = list(range(100))
        random.shuffle(vs)
        vs = [float(i) for i in vs]
        x = self.iso(*vs).byid

        sz = sys.getsizeof(0.0)
        self.assertEqual(str(x)+'\n'+str(x.more)+'\n', """\
Set of 100 <float> objects. Total size = {sztotal:,} bytes.
 Index     Size     % Cumulative     % Value
     0      {sz:>3}   1.0     {szacc[0]:>6,}   1.0 0.0
     1      {sz:>3}   1.0     {szacc[1]:>6,}   2.0 1.0
     2      {sz:>3}   1.0     {szacc[2]:>6,}   3.0 2.0
     3      {sz:>3}   1.0     {szacc[3]:>6,}   4.0 3.0
     4      {sz:>3}   1.0     {szacc[4]:>6,}   5.0 4.0
     5      {sz:>3}   1.0     {szacc[5]:>6,}   6.0 5.0
     6      {sz:>3}   1.0     {szacc[6]:>6,}   7.0 6.0
     7      {sz:>3}   1.0     {szacc[7]:>6,}   8.0 7.0
     8      {sz:>3}   1.0     {szacc[8]:>6,}   9.0 8.0
     9      {sz:>3}   1.0     {szacc[9]:>6,}  10.0 9.0
<90 more rows. Type e.g. '_.more' to view.>
 Index     Size     % Cumulative     % Value
    10      {sz:>3}   1.0     {szacc[10]:>6,}  11.0 10.0
    11      {sz:>3}   1.0     {szacc[11]:>6,}  12.0 11.0
    12      {sz:>3}   1.0     {szacc[12]:>6,}  13.0 12.0
    13      {sz:>3}   1.0     {szacc[13]:>6,}  14.0 13.0
    14      {sz:>3}   1.0     {szacc[14]:>6,}  15.0 14.0
    15      {sz:>3}   1.0     {szacc[15]:>6,}  16.0 15.0
    16      {sz:>3}   1.0     {szacc[16]:>6,}  17.0 16.0
    17      {sz:>3}   1.0     {szacc[17]:>6,}  18.0 17.0
    18      {sz:>3}   1.0     {szacc[18]:>6,}  19.0 18.0
    19      {sz:>3}   1.0     {szacc[19]:>6,}  20.0 19.0
<80 more rows. Type e.g. '_.more' to view.>
""".format(sz=sz, sztotal=sz*100,
           szacc=list(itertools.accumulate([sz] * 20))))

        self.assertEqual(str(x.all)+'\n', """\
Set of 100 <float> objects. Total size = {sztotal:,} bytes.
 Index     Size     % Cumulative     % Value
     0      {sz:>3}   1.0     {szacc[0]:>6,}   1.0 0.0
     1      {sz:>3}   1.0     {szacc[1]:>6,}   2.0 1.0
     2      {sz:>3}   1.0     {szacc[2]:>6,}   3.0 2.0
     3      {sz:>3}   1.0     {szacc[3]:>6,}   4.0 3.0
     4      {sz:>3}   1.0     {szacc[4]:>6,}   5.0 4.0
     5      {sz:>3}   1.0     {szacc[5]:>6,}   6.0 5.0
     6      {sz:>3}   1.0     {szacc[6]:>6,}   7.0 6.0
     7      {sz:>3}   1.0     {szacc[7]:>6,}   8.0 7.0
     8      {sz:>3}   1.0     {szacc[8]:>6,}   9.0 8.0
     9      {sz:>3}   1.0     {szacc[9]:>6,}  10.0 9.0
    10      {sz:>3}   1.0     {szacc[10]:>6,}  11.0 10.0
    11      {sz:>3}   1.0     {szacc[11]:>6,}  12.0 11.0
    12      {sz:>3}   1.0     {szacc[12]:>6,}  13.0 12.0
    13      {sz:>3}   1.0     {szacc[13]:>6,}  14.0 13.0
    14      {sz:>3}   1.0     {szacc[14]:>6,}  15.0 14.0
    15      {sz:>3}   1.0     {szacc[15]:>6,}  16.0 15.0
    16      {sz:>3}   1.0     {szacc[16]:>6,}  17.0 16.0
    17      {sz:>3}   1.0     {szacc[17]:>6,}  18.0 17.0
    18      {sz:>3}   1.0     {szacc[18]:>6,}  19.0 18.0
    19      {sz:>3}   1.0     {szacc[19]:>6,}  20.0 19.0
    20      {sz:>3}   1.0     {szacc[20]:>6,}  21.0 20.0
    21      {sz:>3}   1.0     {szacc[21]:>6,}  22.0 21.0
    22      {sz:>3}   1.0     {szacc[22]:>6,}  23.0 22.0
    23      {sz:>3}   1.0     {szacc[23]:>6,}  24.0 23.0
    24      {sz:>3}   1.0     {szacc[24]:>6,}  25.0 24.0
    25      {sz:>3}   1.0     {szacc[25]:>6,}  26.0 25.0
    26      {sz:>3}   1.0     {szacc[26]:>6,}  27.0 26.0
    27      {sz:>3}   1.0     {szacc[27]:>6,}  28.0 27.0
    28      {sz:>3}   1.0     {szacc[28]:>6,}  29.0 28.0
    29      {sz:>3}   1.0     {szacc[29]:>6,}  30.0 29.0
    30      {sz:>3}   1.0     {szacc[30]:>6,}  31.0 30.0
    31      {sz:>3}   1.0     {szacc[31]:>6,}  32.0 31.0
    32      {sz:>3}   1.0     {szacc[32]:>6,}  33.0 32.0
    33      {sz:>3}   1.0     {szacc[33]:>6,}  34.0 33.0
    34      {sz:>3}   1.0     {szacc[34]:>6,}  35.0 34.0
    35      {sz:>3}   1.0     {szacc[35]:>6,}  36.0 35.0
    36      {sz:>3}   1.0     {szacc[36]:>6,}  37.0 36.0
    37      {sz:>3}   1.0     {szacc[37]:>6,}  38.0 37.0
    38      {sz:>3}   1.0     {szacc[38]:>6,}  39.0 38.0
    39      {sz:>3}   1.0     {szacc[39]:>6,}  40.0 39.0
    40      {sz:>3}   1.0     {szacc[40]:>6,}  41.0 40.0
    41      {sz:>3}   1.0     {szacc[41]:>6,}  42.0 41.0
    42      {sz:>3}   1.0     {szacc[42]:>6,}  43.0 42.0
    43      {sz:>3}   1.0     {szacc[43]:>6,}  44.0 43.0
    44      {sz:>3}   1.0     {szacc[44]:>6,}  45.0 44.0
    45      {sz:>3}   1.0     {szacc[45]:>6,}  46.0 45.0
    46      {sz:>3}   1.0     {szacc[46]:>6,}  47.0 46.0
    47      {sz:>3}   1.0     {szacc[47]:>6,}  48.0 47.0
    48      {sz:>3}   1.0     {szacc[48]:>6,}  49.0 48.0
    49      {sz:>3}   1.0     {szacc[49]:>6,}  50.0 49.0
    50      {sz:>3}   1.0     {szacc[50]:>6,}  51.0 50.0
    51      {sz:>3}   1.0     {szacc[51]:>6,}  52.0 51.0
    52      {sz:>3}   1.0     {szacc[52]:>6,}  53.0 52.0
    53      {sz:>3}   1.0     {szacc[53]:>6,}  54.0 53.0
    54      {sz:>3}   1.0     {szacc[54]:>6,}  55.0 54.0
    55      {sz:>3}   1.0     {szacc[55]:>6,}  56.0 55.0
    56      {sz:>3}   1.0     {szacc[56]:>6,}  57.0 56.0
    57      {sz:>3}   1.0     {szacc[57]:>6,}  58.0 57.0
    58      {sz:>3}   1.0     {szacc[58]:>6,}  59.0 58.0
    59      {sz:>3}   1.0     {szacc[59]:>6,}  60.0 59.0
    60      {sz:>3}   1.0     {szacc[60]:>6,}  61.0 60.0
    61      {sz:>3}   1.0     {szacc[61]:>6,}  62.0 61.0
    62      {sz:>3}   1.0     {szacc[62]:>6,}  63.0 62.0
    63      {sz:>3}   1.0     {szacc[63]:>6,}  64.0 63.0
    64      {sz:>3}   1.0     {szacc[64]:>6,}  65.0 64.0
    65      {sz:>3}   1.0     {szacc[65]:>6,}  66.0 65.0
    66      {sz:>3}   1.0     {szacc[66]:>6,}  67.0 66.0
    67      {sz:>3}   1.0     {szacc[67]:>6,}  68.0 67.0
    68      {sz:>3}   1.0     {szacc[68]:>6,}  69.0 68.0
    69      {sz:>3}   1.0     {szacc[69]:>6,}  70.0 69.0
    70      {sz:>3}   1.0     {szacc[70]:>6,}  71.0 70.0
    71      {sz:>3}   1.0     {szacc[71]:>6,}  72.0 71.0
    72      {sz:>3}   1.0     {szacc[72]:>6,}  73.0 72.0
    73      {sz:>3}   1.0     {szacc[73]:>6,}  74.0 73.0
    74      {sz:>3}   1.0     {szacc[74]:>6,}  75.0 74.0
    75      {sz:>3}   1.0     {szacc[75]:>6,}  76.0 75.0
    76      {sz:>3}   1.0     {szacc[76]:>6,}  77.0 76.0
    77      {sz:>3}   1.0     {szacc[77]:>6,}  78.0 77.0
    78      {sz:>3}   1.0     {szacc[78]:>6,}  79.0 78.0
    79      {sz:>3}   1.0     {szacc[79]:>6,}  80.0 79.0
    80      {sz:>3}   1.0     {szacc[80]:>6,}  81.0 80.0
    81      {sz:>3}   1.0     {szacc[81]:>6,}  82.0 81.0
    82      {sz:>3}   1.0     {szacc[82]:>6,}  83.0 82.0
    83      {sz:>3}   1.0     {szacc[83]:>6,}  84.0 83.0
    84      {sz:>3}   1.0     {szacc[84]:>6,}  85.0 84.0
    85      {sz:>3}   1.0     {szacc[85]:>6,}  86.0 85.0
    86      {sz:>3}   1.0     {szacc[86]:>6,}  87.0 86.0
    87      {sz:>3}   1.0     {szacc[87]:>6,}  88.0 87.0
    88      {sz:>3}   1.0     {szacc[88]:>6,}  89.0 88.0
    89      {sz:>3}   1.0     {szacc[89]:>6,}  90.0 89.0
    90      {sz:>3}   1.0     {szacc[90]:>6,}  91.0 90.0
    91      {sz:>3}   1.0     {szacc[91]:>6,}  92.0 91.0
    92      {sz:>3}   1.0     {szacc[92]:>6,}  93.0 92.0
    93      {sz:>3}   1.0     {szacc[93]:>6,}  94.0 93.0
    94      {sz:>3}   1.0     {szacc[94]:>6,}  95.0 94.0
    95      {sz:>3}   1.0     {szacc[95]:>6,}  96.0 95.0
    96      {sz:>3}   1.0     {szacc[96]:>6,}  97.0 96.0
    97      {sz:>3}   1.0     {szacc[97]:>6,}  98.0 97.0
    98      {sz:>3}   1.0     {szacc[98]:>6,}  99.0 98.0
    99      {sz:>3}   1.0     {szacc[99]:>6,} 100.0 99.0
""".format(sz=sz, sztotal=sz*100,
           szacc=list(itertools.accumulate([sz] * 100))))

    def test_2(self):
        # Slicing
        ss = []
        for i in range(100):
            for c in 'abc':
                ss.append(c*i)
        x = self.iso(*ss).byid

        def ae(x):
            lines = str(x).split('\n')
            datapos = lines[1].index('Representation')
            s = lines[2:]
            if s[-1].startswith('<'):
                s.pop()
            s = [line[datapos:] for line in s]
            return s

        def aeq(x, y):
            self.assertEqual(ae(x), ae(y))

        for i in range(0, 300, 60):
            b = x[i:]
            aeq(b, b.byid)

        # (B) in  Notes Aug 26 2005

        self.assertEqual(x.bysize[2].kind, x.bysize[2].bysize.kind)

    def test_3(self):
        # Some indexing cases.
        # Came up Sep 29 2005.
        # The kind of the result of indexing is to be
        # the result of the er of the partition.

        hp = self.Use

        x = hp.iso([], [], *list(range(20))).byid

        eq = [x[-10], x[-10:-9], x[12], x[12:13],
              x.parts[-10], x.parts[12]]
        k = x[-10].byid.kind
        for i in range(len(eq)):
            self.assertEqual(eq[i], eq[(i + 1) % len(eq)])
            self.assertEqual(eq[i].kind, eq[(i + 1) % len(eq)].kind)
            self.assertEqual(eq[i].kind, k)


class MixedCase(support.TestCase):
    def test_1(self):
        x = self.iso(1, 2, 1.0, 2.0, '1', '2')
        if not PORTABLE_TEST:
            self.assertEqual(str(x), """\
Partition of a set of 6 objects. Total size = 204 bytes.
 Index  Count   %     Size   % Cumulative  % Kind (class / dict of class)
     0      2  33      100  49       100  49 str
     1      2  33       56  27       156  76 int
     2      2  33       48  24       204 100 float""")

        for row in x.partition.get_rows():
            self.assertTrue(row.set <= row.kind)


class StatCase(support.TestCase):
    def test_1(self):
        hp = self.Use

        class C:
            pass
        c0 = C()

        class C:
            pass
        c1 = C()
        x = hp.iso(c0, c1)
        y = hp.iso(c1)

        d = x.diff(y)
        self.assertEqual(d.count, 1)
        self.assertEqual(d[0].count, 1)

        d = y.diff(x)
        self.assertEqual(d.count, -1)
        self.assertEqual(d[0].count, -1)

        d = x.diff(hp.iso())
        self.assertEqual(d.count, 2)
        self.assertEqual(d[0].count, 2)

        d = hp.iso().diff(x)
        self.assertEqual(d.count, -2)
        self.assertEqual(d[0].count, -2)


class NumberFormatCase(support.TestCase):
    def make_format(self, cls, count=1234567, size=1234567890123,
                    numrows=1234568, **kwargs):
        from types import SimpleNamespace
        from guppy.heapy import Part
        impl = SimpleNamespace(mod=self.Part, count=count, size=size,
                               numrows=numrows, kindname='<str>', rows=[], **kwargs)
        return getattr(Part, cls)(impl)

    def assert_columns(self, fmt, rows):
        import re
        header = fmt.get_stat_header()
        starts = []
        for row in rows:
            text = fmt.get_stat_data(row)
            self.assertEqual(len(text), len(header))
            starts.append([match.end() for match in re.finditer(r'\S+', text)])
        self.assertTrue(all(value == starts[0] for value in starts))
        header_ends = [match.end() for match in re.finditer(r'\S+', header)]
        if '% of B' not in header:
            self.assertEqual(starts[0], header_ends)

    def test_set_columns_expand_and_group(self):
        from guppy.heapy.Part import StatRow
        fmt = self.make_format('SetFormat')
        rows = [StatRow(1234566, 1234567890000, 'large', 1234566, 1234567890000),
                StatRow(1, 123, 'small', 1234567, 1234567890123)]
        self.assert_columns(fmt, rows)
        text = fmt.get_stat_data(rows[0])
        self.assertIn('1,234,566', text)
        self.assertIn('1,234,567,890,000', text)
        self.assertIn('1,234,567 objects', fmt.get_label())
        self.assertIn('1,234,567,890,123 bytes', fmt.get_label())
        self.assertIn('<1,234,567 more rows.', fmt._oh_get_more_msg(0, 0))

    def test_identity_columns_expand_and_group(self):
        from guppy.heapy.Part import StatRow
        fmt = self.make_format('IdFormat')
        self.assert_columns(fmt, [StatRow(1, 1234567890000, 'large', 0, 1234567890000),
                                  StatRow(1, 123, 'small', 1234567, 1234567890123)])
        self.assertIn('1,234,567 <str> objects', fmt.get_label())

    def test_difference_columns_include_negative_sign(self):
        from guppy.heapy.Part import StatRow
        fmt = self.make_format('DiffFormat', count=-1, size=-1,
                               b_count=1234567, b_size=1234567890123)
        rows = [StatRow(-1234567, -1234567890123, 'removed', 0, -1234567890123),
                StatRow(1234566, 1234567890122, 'added', 1, -1)]
        self.assert_columns(fmt, rows)
        self.assertIn('-1,234,567', fmt.get_stat_data(rows[0]))
        self.assertIn('-1,234,567,890,123', fmt.get_stat_data(rows[0]))
        summary = fmt.get_label().splitlines()
        self.assertEqual(len(summary[1]), len(summary[2]))
        self.assertEqual(len(summary[2]), len(summary[3]))

    def test_zero_baseline_difference(self):
        from guppy.heapy.Part import StatRow
        fmt = self.make_format('DiffFormat', count=1, size=1234, b_count=0, b_size=0)
        self.assertIn('(n.a.)', fmt.get_stat_data(StatRow(1, 1234, 'added', 0, 1234)))

    def test_fast_partition_keeps_original_index_width(self):
        from guppy.heapy.Part import SetFormat, StatRow
        from types import SimpleNamespace
        row = StatRow(1, 12, 'value', 123456789, 1234567890123)
        fmt = SetFormat(SimpleNamespace(mod=self.Part, count=1, size=12, numrows=1, rows=[row]))
        self.assert_columns(fmt, [row])
        self.assertIn('123,456,789', fmt.get_stat_data(row))

    def test_format_initialization_does_not_iterate_identity_rows(self):
        from guppy.heapy.Part import IdFormat
        from types import SimpleNamespace
        def no_rows():
            raise AssertionError('identity rows must stay lazy')
        fmt = IdFormat(SimpleNamespace(mod=self.Part, count=10000000, size=1000000000,
                                       numrows=10000000, get_rows=no_rows))
        self.assertIn('Cumulative', fmt.get_stat_header())

    def test_wrapped_names_start_after_numeric_columns(self):
        from guppy.heapy.Part import StatRow
        fmt = self.make_format('SetFormat')
        row = StatRow(1234567, 1234567890123, 'a long name ' * 20 + '\nsecond line',
                      1234567, 1234567890123)
        lines = fmt.get_formatted_row(row).splitlines()
        width = len(fmt.get_stat_data(row))
        self.assertGreater(len(lines), 1)
        for line in lines[1:]:
            self.assertTrue(line.startswith(' ' * width))

    def test_serialization_keeps_plain_integers(self):
        from guppy.heapy.Part import StatRow
        row = StatRow(1234567, 1234567890123, 'str', 0, 1234567890123)
        for cls in ('SetFormat', 'DiffFormat'):
            kwargs = dict(b_count=1, b_size=1) if cls == 'DiffFormat' else {}
            fmt = self.make_format(cls, **kwargs)
            self.assertEqual(fmt.get_rowdata(row), '1234567 1234567890123 str')
        fmt = self.make_format('IdFormat')
        self.assertEqual(fmt.get_rowdata(row), '1234567890123 str')

    def test_loaded_statistics_pagination_and_roundtrip(self):
        trows = ['.loader: _load_stat', '.format: SetFormat', '.timemade: 0',
                 '.count: 1234567', '.size: 1234567890123', '.kindheader: Kind',
                 '.kindname:', '.numrows: 12']
        trows += ['.r: 1 1 small' for _ in range(11)]
        trows += ['.r: 1234556 1234567890112 large', '.end:']
        stat = self.Part._load_stat(lambda: iter(trows))
        top = str(stat)
        more = str(stat.more)
        self.assertIn('1,234,567 objects', top)
        self.assertIn('1,234,567,890,123 bytes', top)
        self.assertIn('1,234,567,890,112', more)
        self.assertEqual(top.splitlines()[1], more.splitlines()[0])
        raw = list(stat.get_trows())
        self.assertIn('.count: 1234567', raw)
        self.assertIn('.r: 1234556 1234567890112 large', raw)
        restored = self.Part._load_stat(lambda: iter(raw))
        self.assertEqual(str(restored.all), str(stat.all))

    def test_empty_and_singular_labels(self):
        self.assertEqual(str(self.iso()), '<Nothing>')
        fmt = self.make_format('SetFormat', count=0, size=0, numrows=0)
        self.assertEqual(fmt.get_label(), 'Partition of a set of 0 objects. Total size = 0 bytes.')
        fmt = self.make_format('SetFormat', count=1, size=1234, numrows=1)
        self.assertEqual(fmt.get_label(), 'Partition of a set of 1 object. Total size = 1,234 bytes.')



def test_main(debug=0):
    support.run_unittest(NumberFormatCase, debug)
    support.run_unittest(StatCase, debug)
    support.run_unittest(IdentityCase, debug)
    support.run_unittest(MixedCase, debug)


if __name__ == "__main__":
    test_main()
