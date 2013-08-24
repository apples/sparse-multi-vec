/*******************************************************************************
 * SparseMultiVec - A weird container.
 * Version: 0.0.1
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

#include <iostream>
#include <random>
#include <string>
#include <typeinfo>

#include "sparsemultivec.inl"

using namespace std;

template <typename T>
class Counter
{
public:
    Counter(T j = 1) : j{j} {}
    operator T() { auto tmp = i; i+=j; return tmp; }
    T i = 0;
    T j;
};

string randomName()
{
    static mt19937 rng(random_device{}());
    static uniform_int_distribution<int> alphaDist(0,25);
    static uniform_int_distribution<int> numDist(0,9);
    static string alpha = "qwertyuiopasdfghjklzxcvbnm";
    static string num = "1234567890";

    string rval;
    rval += alpha[alphaDist(rng)] + ('A'-'a');
    int len = uniform_int_distribution<int>{4,9}(rng);
    for (int i=0; i<len; ++i)
    {
        rval += alpha[alphaDist(rng)];
    }
    rval += num[numDist(rng)];
    return rval;
}

template <typename T>
void wreckIt(T& in)
{
    auto it = begin(in);
    
    for (int i=0; i<10; ++i) ++it;
    
    in.erase(it);
}

int main()
{
    SparseMultiVec<int, float, string> test;

    Counter<int>   c;
    Counter<float> f(0.5);

    mt19937 rng{random_device{}()};
    uniform_int_distribution<int> roll{0,7};
    
    for (int i=0; i<20; ++i)
    {
        switch (roll(rng))
        {
            case 0:
            { test.push_back(nullptr, nullptr,      nullptr);
            break;}
            
            case 1:
            { test.push_back(      c, nullptr,      nullptr);
            break;}
            
            case 2:
            { test.push_back(nullptr,       f,      nullptr);
            break;}
            
            case 3:
            { test.push_back(      c,       f,      nullptr);
            break;}
            
            case 4:
            { test.push_back(nullptr, nullptr, randomName());
            break;}
            
            case 5:
            { test.push_back(      c, nullptr, randomName());
            break;}
            
            case 6:
            { test.push_back(nullptr,       f, randomName());
            break;}
            
            case 7:
            { test.push_back(      c,       f, randomName());
            break;}
        }
    }
    
    wreckIt(test);
    
    Counter<int> id;
    
    cout << "\tint\tfloat\tstring" << endl;
    
    for (auto&& i : test)
    {
        if (std::get<0>(i)) *std::get<0>(i) += 5;
        
        cout << int(id) << "\t";
        
        if (std::get<0>(i)) cout << *std::get<0>(i) << "\t";
        else cout << "---" << "\t";
        
        if (std::get<1>(i)) cout << *std::get<1>(i) << "\t";
        else cout << "---" << "\t";
        
        if (std::get<2>(i)) cout << *std::get<2>(i) << endl;
        else cout << "---" << endl;
    }
    
    return 0;
}
