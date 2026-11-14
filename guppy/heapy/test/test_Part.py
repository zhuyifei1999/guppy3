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
    class Sized:
        def __init__(self, size):
            self.size = size

        def __sizeof__(self):
            return self.size

    def setUp(self):
        import sys

        support.TestCase.setUp(self)

        # We are creating 1000 objects in some tests. It's unrealistic to have
        # sum of object sizes to be larger than the address space, so constrain
        # the size on 32-bit arches to be about 1,234,567,000, the maximum
        # number of digits before overflow, while on 64-bit arches we stress
        # test it a bit.
        self.sizelimit = 1234567890123 if sys.maxsize > 2**32 else 1234567

    def make_objects(self, count=1, size=None, cls=None):
        if size is None:
            size = self.sizelimit
        cls = cls or self.Sized
        return [cls(size) for _ in range(count)]

    def assert_display_columns(self, text, columns):
        import re
        lines = text.splitlines()
        header = next(line for line in lines if line.lstrip().startswith('Index'))
        header = header.replace('% of B', '%_of_B')
        ends = [m.end() for m in re.finditer(r'\S+', header)][:columns]
        rows = [line for line in lines if re.match(r'^\s*\d[\d,]*\s', line)]
        self.assertTrue(rows)
        for row in rows:
            self.assertEqual([m.end() for m in re.finditer(r'\S+', row)][:columns], ends)

    def test_set_columns_expand_and_group(self):
        class Other(self.Sized):
            pass
        objects = self.make_objects(1000) + self.make_objects(size=123, cls=Other)
        partition = self.iso(*objects)
        text = str(partition)
        self.assertIn('1,001 objects', text)
        self.assertIn(f'{partition.size:,} bytes', text)
        self.assertIn('1,000', text)
        self.assert_display_columns(text, 7)

    def test_identity_columns_expand_and_group(self):
        objects = self.make_objects() + self.make_objects(size=123)
        partition = self.iso(*objects).byid
        text = str(partition)
        self.assertIn(f'{partition.size:,} bytes', text)
        self.assertIn(f'{self.iso(objects[0]).size:,}', text)
        self.assert_display_columns(text, 5)

    def test_difference_columns_include_negative_sign(self):
        class Other(self.Sized):
            pass
        removed = self.make_objects(1000)
        added = self.make_objects(999, cls=Other)
        difference = self.iso(*added).diff(self.iso(*removed))
        text = str(difference)
        self.assertIn('-1,000', text)
        self.assertIn(f'{-self.iso(*removed).size:,}', text)
        self.assertEqual(difference.count, -1)
        self.assert_display_columns(text, 5)
        summary = text.splitlines()
        self.assertEqual(len(summary[1]), len(summary[2]))
        self.assertEqual(len(summary[2]), len(summary[3]))

    def test_zero_baseline_difference(self):
        objects = self.make_objects()
        difference = self.iso(*objects).diff(self.iso())
        text = str(difference)
        self.assertIn('(n.a.)', text)
        self.assertIn(f'{difference.size:,}', text)
        self.assert_display_columns(text, 5)

    def test_one_row_stat_view_keeps_parent_cumulative_width(self):
        class Other(self.Sized):
            pass
        objects = self.make_objects() + self.make_objects(size=123, cls=Other)
        partition = self.iso(*objects)
        text = str(partition[1])
        self.assertIn(f'{partition.size:,}', text)
        self.assert_display_columns(text, 7)

    def test_identity_pagination_and_total_width(self):
        objects = self.make_objects(12)
        partition = self.iso(*objects).byid
        first, more = str(partition), str(partition.more)
        self.assertIn('<2 more rows.', first)
        self.assertEqual(first.splitlines()[1], more.splitlines()[0])
        self.assert_display_columns(first, 5)
        self.assert_display_columns(more, 5)

    def test_wrapped_kind_names_start_after_numeric_columns(self):
        long_class = type('LongName' * 25, (self.Sized,), {})
        objects = self.make_objects(cls=long_class)
        text = str(self.iso(*objects))
        lines = text.splitlines()
        header = next(line for line in lines if line.lstrip().startswith('Index'))
        prefix = header.index('Kind')
        continuation = [line for line in lines[3:] if line.strip()]
        self.assertTrue(continuation)
        for line in continuation:
            self.assertTrue(line.startswith(' ' * prefix))
        self.assert_display_columns(text, 7)

    def test_serialization_keeps_plain_integers(self):
        objects = self.make_objects(1001)
        partition = self.iso(*objects)
        for stat in (partition.stat, partition.byid.stat,
                     self.iso().diff(partition)):
            raw = list(stat.get_trows())
            self.assertIn(f'.count: {stat.count}', raw)
            self.assertIn(f'.size: {stat.size}', raw)
            for line in raw:
                if line.startswith('.r:'):
                    self.assertNotIn(',', line.split(' ', 3)[1])
            restored = self.Part._load_stat(lambda: iter(raw))
            self.assertEqual(str(restored), str(stat))

    def test_real_partition_pagination_and_roundtrip(self):
        classes = [type('Row' + str(i), (self.Sized,), {}) for i in range(12)]
        objects = [cls(self.sizelimit + i) for i, cls in enumerate(classes)]
        stat = self.iso(*objects).stat
        first, more = str(stat), str(stat.more)
        self.assertIn('<2 more rows.', first)
        self.assertEqual(first.splitlines()[1], more.splitlines()[0])
        self.assert_display_columns(first, 7)
        self.assert_display_columns(more, 7)
        raw = list(stat.get_trows())
        restored = self.Part._load_stat(lambda: iter(raw))
        self.assertEqual(str(restored.all), str(stat.all))

    def test_empty_and_singular_labels(self):
        self.assertEqual(str(self.iso()), '<Nothing>')
        objects = self.make_objects()
        partition = self.iso(*objects)
        self.assertIn(f'1 object. Total size = {partition.size:,} bytes.', str(partition))
        self.assert_display_columns(str(partition), 7)


def test_main(debug=0):
    support.run_unittest(NumberFormatCase, debug)
    support.run_unittest(StatCase, debug)
    support.run_unittest(IdentityCase, debug)
    support.run_unittest(MixedCase, debug)


if __name__ == "__main__":
    test_main()
