# ._cv_part guppy.sets

from .setsc import BitSet        # base bitset type
from .setsc import ImmBitSet     # immutable bitset type
from .setsc import immbit        # immutable bitset singleton constructor
from .setsc import immbitrange   # immutable bitset range constructor
from .setsc import immbitset     # immutable bitset constructor
from .setsc import MutBitSet     # mutable bitset
from .setsc import NodeSet       # base nodeset type
from .setsc import ImmNodeSet    # immmutable nodeset type
from .setsc import MutNodeSet    # mutable nodeset type

from .setsc import _bs
_bs.__module__ = 'guppy.sets'  # ..to be able to set it.


# Define some constructors.
# Constructor names are lower case.
# Some constructors are equal to types.
# But this connection depends on the implementation.
# So one may wish the user to not depend on this.

mutbitset = MutBitSet
immnodeset = ImmNodeSet
mutnodeset = MutNodeSet


def mutnodeset_union(iterable):
    "Return a mutable nodeset which is the union of all nodesets in iterable."
    set = mutnodeset()
    for it in iterable:
        set |= it
    return set


def immnodeset_union(iterable, *args):
    "Return an immmutable nodeset which is the union of all nodesets in iterable."
    set = mutnodeset_union(iterable)
    return immnodeset(set, *args)


def laxnodeset(v):
    """\
Return a nodeset with elements from the argument.  If the argument is
already a nodeset, it self will be returned. Otherwise it will be
converted to a nodeset, that can be mutable or immutable depending on
what happens to be most effectively implemented."""

    if not isinstance(v, NodeSet):
        v = immnodeset(v)
    return v

# Make attributes assignable by reading one;
# this is getting around a bug in Python 2.3.3
# and should be harmless in any version.


try:
    mutnodeset()._hiding_tag_
except AttributeError:
    pass
