#pragma once
#include<optional>
#include<string>
#include<utility>
#include<limits>
#include"parsing_checks.h"

using big_uint = unsigned long long;

int getnum(char a)
{
    switch (a)
    {
    case ('0'):
        return 0;
        break;
    case ('1'):
        return 1;
        break;
    case ('2'):
        return 2;
        break;
    case ('3'):
        return 3;
        break;
    case ('4'):
        return 4;
        break;
    case ('5'):
        return 5;
        break;
    case ('6'):
        return 6;
        break;
    case ('7'):
        return 7;
        break;
    case ('8'):
        return 8;
        break;
    case ('9'):
        return 9;
        break;
    case (' '):
        return -1;
        break;
    case ('/'):
        return -2;
        break;
    case ('.'):
        return -3;
        break;
    default:
        return -4;
        break;
    }
}

std::optional<std::pair<bool,std::pair<big_uint,big_uint>>> parse_range_to_number(std::string::const_iterator& start, std::string::const_iterator stop)
{
    std::pair<big_uint,big_uint> ret;
    bool signret = false;
    while(start!=stop && *start == ' ')
    {
        ++start;
    }
    if(start==stop)
    {
        return std::nullopt;
    }

    if(*start == '-')
    {
        signret = true;
        ++start;
    }

    if(start==stop)
    {
        return std::nullopt;
    }

    ret.first = 0;

    int curv;

    while(start!=stop)
    {
        curv = getnum(*start);
        if(curv < 0)
        {
            break;
        }
        else
        {
            if((std::numeric_limits<big_uint>::max() - curv)/10 <= ret.first)
            {
                return std::nullopt;
            }
            ret.first*=10;
            ret.first+=curv;
        }
        ++start;
    }

    if(start==stop || curv == -1)
    {
        ret.second = 1;
        return std::pair<bool,std::pair<big_uint,big_uint>>{signret,ret};
    }
    else if(curv==-2)
    {
        ++start;
        if(start==stop || *start == ' ')
        {
            return std::nullopt;
        }
        ret.second = 0;
        while(start!=stop)
        {
            curv = getnum(*start);
            if(curv < -1)
            {
                return std::nullopt;
            }
            else if(curv == -1)
            {
                break;
            }
            else
            {
                if((std::numeric_limits<big_uint>::max() - curv)/10 <= ret.second)
                {
                    return std::nullopt;
                }
                ret.second*=10;
                ret.second+=curv;
            }
            ++start;
        }
    }
    else if(curv==-3)
    {
        ++start;
        if(start==stop || *start == ' ')
        {
            return std::nullopt;
        }
        ret.second = 1;
        while(start!=stop)
        {
            curv = getnum(*start);
            if(curv < -1)
            {
                return std::nullopt;
            }
            else if(curv == -1)
            {
                break;
            }
            else
            {
                if((std::numeric_limits<big_uint>::max() - curv)/10 <= ret.first || std::numeric_limits<big_uint>::max()/10 <= ret.second)
                {
                    return std::nullopt;
                }
                ret.first*=10;
                ret.first+=curv;
                ret.second*=10;
            }
            ++start;
        }
    }
    else if(curv==-4)
    {
        return std::nullopt;
    }
    
    return std::pair<bool,std::pair<big_uint,big_uint>>{signret,ret};
}

std::optional<std::pair<bool,std::pair<big_uint,big_uint>>> parse_string_to_number(std::string const& a)
{
    auto b = a.cbegin();
    auto p = parse_range_to_number(b,a.cend());
    if(!p)
    {
        return std::nullopt;
    }
    while(b!=a.cend())
    {
        if(*b==' ')
        {
            return std::nullopt;
        }
        ++b;
    }
    return *p;
}


template<typename t>
std::optional<t> convert(std::string const& tar)
{
    if constexpr(std::is_arithmetic<t>::value)
    {
        auto p = parse_string_to_number(tar);
        if(!p)
        {
            return std::nullopt;
        }
        bool is_negative = p->first;
        big_uint numerator = std::move(p->second.first);
        big_uint denomenator = std::move(p->second.second);
        t ret;
        

        if constexpr(!std::is_signed<t>::value)
        {
            if(is_negative)
            {
                return std::nullopt;
            }
        }


        if constexpr(std::is_integral<t>::value)
        {
            big_uint tm = (numerator/denomenator);
            if(tm > std::numeric_limits<t>::max())
            {
                return std::nullopt;
            }
            ret = tm;
        }
        else
        {
            if(numerator > std::numeric_limits<t>::max())
            {
                return std::nullopt;
            }
            ret = t{numerator}/denomenator;
        }

        if(is_negative)
        {
            ret *= -1;
        }

        return std::optional<t>{std::move(ret)};
    }
    else if constexpr(std::is_same<t,std::string>::value)
    {
        return std::optional<std::string>{tar};
    }
    else
    {
        return std::nullopt;
    }
}

