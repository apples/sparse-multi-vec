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
 *     claim that you wrote the orig-inal software. If you use this software
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

namespace detailMetaContainers
{
    template <template <typename U> class Container, typename T>
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
        class Iterator
        {
        public:
            Iterator() = default;
            
            template <typename A>
            Iterator(int p, A&& i)
                : pos{p}
                , iter{std::forward<A>(i)}
            {}
            
            T* operator*()
            {
                return (pos == 0)? &(iter->value) : nullptr;
            }
            
            Iterator& operator++()
            {
                if (++pos > iter->toNext)
                {
                    pos = 0;
                    ++iter;
                }
                return *this;
            }
            
            bool operator==(const Iterator& in) const
            {
                return (
                       std::tie(   pos,    iter)
                    == std::tie(in.pos, in.iter)
                );
            }
            
            bool operator!=(const Iterator& in) const
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
        
        Iterator begin()
        {
            return Iterator{-toFirst, data.begin()};
        }
        
        Iterator end()
        {
            return Iterator{0, data.end()};
        }
        
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
        
        int size() const
        {
            return sz;
        }
        
    private:
        int toFirst;
        Data data;
        int sz;
    };

    template <typename T>
    struct DereferenceType
    {
        using type = decltype(*T{});
    };

    template <typename T>
    struct IteratorType
    {
        using type = decltype(T{}.begin());
    };

    namespace detailMultiContainer
    {
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
                std::get<N>(i) = std::get<N>(t).begin();
                MakeBeginRecurse<N-1>::makeBegin(i, t);
            }
        };
        
        template <>
        struct MakeBeginRecurse<0>
        {
            template <typename I, typename T>
            static void makeBegin(I&& i, T&& t)
            {
                std::get<0>(i) = std::get<0>(t).begin();
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
                std::get<N>(i) = std::get<N>(t).end();
                MakeEndRecurse<N-1>::makeEnd(i, t);
            }
        };
        
        template <>
        struct MakeEndRecurse<0>
        {
            template <typename I, typename T>
            static void makeEnd(I&& i, T&& t)
            {
                std::get<0>(i) = std::get<0>(t).end();
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
    }

    template <template <typename> class Container, typename... Types>
    class MultiContainer
    {
        static_assert(sizeof...(Types) > 0, "Must have at least one type!");
        
        using Tuple = std::tuple<Container<Types>...>;
        
        template <typename A>
        using Iter = typename IteratorType<Container<A>>::type;
        
        template <typename A>
        using Deref = typename DereferenceType<Iter<A>>::type;
        
        using TupleIter = std::tuple<Iter<Types>...>;
        
        using TupleRef = std::tuple<Deref<Types>...>;
        
    public:
        class Iterator
        {
            friend MultiContainer;
        public:
            Iterator()
                : iter{}
            {}
            
            TupleRef operator*()
            {
                TupleRef rval;
                detailMultiContainer::indirection(rval, iter);
                return rval;
            }
            
            Iterator& operator++()
            {
                detailMultiContainer::increment(iter);
                return *this;
            }
            
            bool operator==(const Iterator& in)
            {
                return (iter == in.iter);
            }
            
            bool operator!=(const Iterator& in) const
            {
                return (iter != in.iter);
            }
            
        private:
            TupleIter iter;
        };
        
        MultiContainer()
            : data{}
        {}
        
        Iterator begin()
        {
            Iterator rval;
            detailMultiContainer::makeBegin(rval.iter, data);
            return rval;
        }
        
        Iterator end()
        {
            Iterator rval;
            detailMultiContainer::makeEnd(rval.iter, data);
            return rval;
        }
        
        template <typename... A>
        void push_back(A&&... in)
        {
            static_assert(sizeof...(A) == std::tuple_size<Tuple>::value, "Argument count must match container count!");
            push_backer(in...);
        }
        
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
    
    struct GoodThings
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

} //namespace detailMetaContainers

template <typename... P>
using SparseMultiVec = detailMetaContainers::GoodThings::SparseMultiVec<P...>;

#endif //SPARSE_MULTI_VEC_INL
