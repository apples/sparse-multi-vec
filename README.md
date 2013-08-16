# Sparse Multi Vec

## Description

This is actually a set of two main classes.

### Sparse Container

This just takes any container you give it and makes it sparse.
A "sparse" container is such that all elements are optional.
In effect, it makes `nullptr` a valid element,
and consecutive null elements do not require space.
For example, if you have a sparse container of `int`
that has a solid `int = 3`
followed by three `nullptr`
and a final `int = 7`,
it would only store the two solid `int`
with minimum overhead,
like so:

```
{3, nullptr, nullptr, nullptr, 7}
```

The actual storage would look more like this:

```
{3, /* overhead */, 7}
```

This "overhead" is usually a RLE compression of the null data,
most likely an integer count of the null slots.

This overhead is basically guaranteed to be more efficient than solid storage.

### Multi Container

This container is essentially a tuple of containers.
For example, a `MultiVec<vector, int, float>`
is kind of like a `tuple<vector<int>, vector<float>>`,
but with proper iterators
that look like `tuple<vector<int>::iterator, vector<float>::iterator>`.

It will play nice with most containers,
even Sparse Containers.

### SparseMultiVec

This is a helper alias that defines a
Multi Container of Sparse Containers of Vectors.

It is guaranteed that each Sparse Container is independently contiguous,
which maximizes cache locality
while allowing for iteration over only the requested containers
(currently not implemented).

## Yep.

This was made for a personal project, so I thought I might share.
It's very good for maximizing cache locality while dealing with sparse values.
