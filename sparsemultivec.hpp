/*******************************************************************************
 * SparseMultiVec - A weird container.
 * Version: 0.0.2
 * https://github.com/dbralir/sparse-multi-vec
 *
 * Copyright (c) 2013 Jeramy Harrison <dbralir@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SPARSE_MULTI_VEC_INL
#define SPARSE_MULTI_VEC_INL

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <vector>
#include <iterator>

template <template <typename> class Container, typename T>
class SparseContainer
{
    class Item
    {
    public:
        Item()
            : toNext{0}
            , value{}
        {}
        
        template <typename A>
        Item(A&& in)
            : toNext{0}
            , value{std::forward<A>(in)}
        {}
        
        int toNext;
        T value;
    };
    
    using Data     = Container<Item>;
    using DataIter = typename Data::iterator;
    
public:
    class iterator
    {
        friend SparseContainer;
    public:
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using const_reference = const value_type&;
        using const_pointer = const value_type*;

        iterator() = default;
        
        template <typename A>
        iterator(int p, A&& i)
            : pos{p}
            , iter{std::forward<A>(i)}
        {}
        
        pointer operator*()
        {
            return (pos == 0)? &(iter->value) : nullptr;
        }
        
        iterator& operator++()
        {
            if (++pos > iter->toNext)
            {
                pos = 0;
                ++iter;
            }
            return *this;
        }
        
        bool operator==(const iterator& in) const
        {
            return (
                   std::tie(   pos,    iter)
                == std::tie(in.pos, in.iter)
            );
        }
        
        bool operator!=(const iterator& in) const
        {
            return (
                   std::tie(   pos,    iter)
                != std::tie(in.pos, in.iter)
            );
        }
        
    private:
        int      pos;
        DataIter iter;
    };
    
    SparseContainer()
        : toFirst{0}
        , data{}
    {}
    
    template <typename A>
    void push_back(A&& in)
    {
        data.emplace_back(in);
        ++sz;
    }
    
    void push_back(decltype(nullptr))
    {
        if (data.size() == 0) ++toFirst;
        else ++data.back().toNext;
        ++sz;
    }
    
    iterator erase(iterator it)
    {
        if (it == begin(*this)) return erase_front();
        
        if (it.pos == 0)
        {
            if (it.iter->toNext == 0)
            {
                return {0, data.erase(it.iter)};
            }
            else
            {
                auto prev = it.iter;
                --prev;
                auto newPos = prev->toNext;
                prev->toNext += it.iter->toNext;
                data.erase(it.iter);
                return {newPos, prev};
            }
        }
        else if (it.pos < 0)
        {
            --toFirst;
            ++it.pos;
            return it;
        }
        else
        {
            --it.pos;
            --it.iter->toNext;
            return ++it;
        }
    }
    
    iterator erase_front()
    {
        auto it = begin(data);
        toFirst += it->toNext;
        data.erase(it);
        return begin(*this);
    }
    
    int size() const
    {
        return sz;
    }
    
    friend iterator begin(SparseContainer& in)
    {
        return {-in.toFirst, begin(in.data)};
    }

    friend iterator end(SparseContainer& in)
    {
        return {0, end(in.data)};
    }

    friend iterator begin(SparseContainer&& in)
    {
        return {-in.toFirst, begin(in.data)};
    }

    friend iterator end(SparseContainer&& in)
    {
        return {0, end(in.data)};
    }

private:
    int toFirst;
    Data data;
    int sz;
};

namespace detailMultiContainer
{
    template <typename T>
    struct DereferenceType
    {
        using type = decltype(*T{});
    };

    template <typename T>
    struct iteratorType
    {
        using type = decltype(begin(std::declval<T>()));
    };

    template <typename T>
    struct TupleSize
    {
        using RawType = typename std::decay<T>::type;
        static constexpr auto value = std::tuple_size<RawType>::value;
    };
    
    template <int N>
    struct MakeBeginRecurse
    {
        template <typename I, typename T>
        static void makeBegin(I&& i, T&& t)
        {
            std::get<N>(i) = begin(std::get<N>(t));
            MakeBeginRecurse<N-1>::makeBegin(i, t);
        }
    };
    
    template <>
    struct MakeBeginRecurse<0>
    {
        template <typename I, typename T>
        static void makeBegin(I&& i, T&& t)
        {
            std::get<0>(i) = begin(std::get<0>(t));
        }
    };
    
    template <typename I, typename T>
    void makeBegin(I&& i, T&& t)
    {
        MakeBeginRecurse<TupleSize<I>::value-1>::makeBegin(i, t);
    }
    
    template <int N>
    struct MakeEndRecurse
    {
        template <typename I, typename T>
        static void makeEnd(I&& i, T&& t)
        {
            std::get<N>(i) = end(std::get<N>(t));
            MakeEndRecurse<N-1>::makeEnd(i, t);
        }
    };
    
    template <>
    struct MakeEndRecurse<0>
    {
        template <typename I, typename T>
        static void makeEnd(I&& i, T&& t)
        {
            std::get<0>(i) = end(std::get<0>(t));
        }
    };
    
    template <typename I, typename T>
    void makeEnd(I&& i, T&& t)
    {
        MakeEndRecurse<TupleSize<I>::value-1>::makeEnd(i, t);
    }
    
    template <int N>
    struct IndirectionRecurse
    {
        template <typename T, typename I>
        static void indirection(T&& p, I&& i)
        {
            std::get<N>(p) = *std::get<N>(i);
            IndirectionRecurse<N-1>::indirection(p, i);
        }
    };

    template <>
    struct IndirectionRecurse<0>
    {
        template <typename T, typename I>
        static void indirection(T&& p, I&& i)
        {
            std::get<0>(p) = *std::get<0>(i);
        }
    };

    template <typename T, typename I>
    void indirection(T&& p, I&& i)
    {
        IndirectionRecurse<TupleSize<T>::value-1>::indirection(p,i);
    }
    
    template <int N>
    struct IncrementRecurse
    {
        template <typename T>
        static void increment(T&& i)
        {
            ++std::get<N>(i);
            IncrementRecurse<N-1>::increment(i);
        }
    };
    
    template <>
    struct IncrementRecurse<0>
    {
        template <typename T>
        static void increment(T&& i)
        {
            ++std::get<0>(i);
        }
    };
    
    template <typename T>
    void increment(T&& t)
    {
        IncrementRecurse<TupleSize<T>::value-1>::increment(t);
    }
    
    template <int N>
    struct EraseRecurse
    {
        template <typename C, typename T>
        static void erase(C& c, const T& t, T& u)
        {
            std::get<N>(u) = std::get<N>(c).erase(std::get<N>(t));
            EraseRecurse<N-1>::erase(c, t, u);
        }
    };
    
    template <>
    struct EraseRecurse<0>
    {
        template <typename C, typename T>
        static void erase(C& c, const T& t, T& u)
        {
            std::get<0>(u) = std::get<0>(c).erase(std::get<0>(t));
        }
    };
    
    template <typename C, typename T>
    void erase(C& c, const T& t, T& u)
    {
        EraseRecurse<TupleSize<T>::value-1>::erase(c, t, u);
    }

} // namespace detailMultiContainer

template <template <typename> class Container, typename... Types>
class MultiContainer;

template <template <typename> class Container, typename... Types>
typename MultiContainer<Container, Types...>::iterator begin(MultiContainer<Container, Types...>& in);

template <template <typename> class Container, typename... Types>
typename MultiContainer<Container, Types...>::iterator end(MultiContainer<Container, Types...>& in);

template <template <typename> class Container, typename... Types>
class MultiContainer
{
    static_assert(sizeof...(Types) > 0, "Must have at least one type!");
    
    using Tuple = std::tuple<Container<Types>...>;
    
    template <typename A>
    using Iter = typename detailMultiContainer::iteratorType<Container<A>>::type;
    
    template <typename A>
    using Deref = typename detailMultiContainer::DereferenceType<Iter<A>>::type;
    
    using TupleIter = std::tuple<Iter<Types>...>;
    
    using TupleRef = std::tuple<Deref<Types>...>;
    
public:
    class iterator
    {
        friend MultiContainer;
        
        friend iterator begin<>(MultiContainer& in);
        friend iterator end<>(MultiContainer& in);
        
    public:
        TupleRef operator*()
        {
            TupleRef rval;
            detailMultiContainer::indirection(rval, iter);
            return rval;
        }
        
        iterator& operator++()
        {
            detailMultiContainer::increment(iter);
            return *this;
        }
        
        bool operator==(const iterator& in)
        {
            return (iter == in.iter);
        }
        
        bool operator!=(const iterator& in) const
        {
            return (iter != in.iter);
        }
        
    private:
        TupleIter iter;
    };
    
    MultiContainer()
        : data{}
    {}
    
    template <typename... A>
    void push_back(A&&... in)
    {
        static_assert(sizeof...(A) == std::tuple_size<Tuple>::value, "Argument count must match container count!");
        push_backer(in...);
    }
    
    iterator erase(iterator it)
    {
        iterator rval;
        detailMultiContainer::erase(data, it.iter, rval.iter);
        return rval;
    }
    
    friend iterator begin<>(MultiContainer& in);
    friend iterator end<>(MultiContainer& in);

private:
    template <int N = 0, typename A, typename... B>
    void push_backer(A&& a, B&&... o)
    {
        std::get<N>(data).push_back(a);
        push_backer<N+1>(o...);
    }
    
    template <int>
    void push_backer()
    {}

    Tuple data;
};

template <template <typename> class Container, typename... Types>
typename MultiContainer<Container, Types...>::iterator begin(MultiContainer<Container, Types...>& in)
{
    typename MultiContainer<Container, Types...>::iterator rval;
    detailMultiContainer::makeBegin(rval.iter, in.data);
    return rval;
}

template <template <typename> class Container, typename... Types>
typename MultiContainer<Container, Types...>::iterator end(MultiContainer<Container, Types...>& in)
{
    typename MultiContainer<Container, Types...>::iterator rval;
    detailMultiContainer::makeEnd(rval.iter, in.data);
    return rval;
}

struct detailSparseMultiVec
{
    template <typename T>
    using Decay = typename std::decay<T>::type;
    
    template <typename T>
    using Vec = std::vector<T>;
    
    template <typename T>
    using SparseVec = SparseContainer<Vec, T>;
    
    template <typename... P>
    using SparseMultiVec = MultiContainer<SparseVec, Decay<P>...>;
};

template <typename... P>
using SparseMultiVec = detailSparseMultiVec::SparseMultiVec<P...>;

#endif //SPARSE_MULTI_VEC_INL
