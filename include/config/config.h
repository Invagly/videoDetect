#pragma once

#include<string>
#include<map>
#include<iostream>
#include<fstream>
#include<sstream>

class Config
{
    friend std::ostream& operator<<(std::ostream& os,const Config& cf);
    friend std::istream& operator>>(std::istream& is,Config& cf);
protected:
    std::string m_Delimiter; //!< separator between key and value
    std::string m_Comment;   //!< separator between value and comments
    std::map<std::string,std::string> m_Contents; //!< extracted keys and values

    typedef std::map<std::string,std::string>::iterator mapi;
    typedef std::map<std::string,std::string>::const_iterator mapci;

public:
    Config(const std::string& filename,const std::string& delimiter = "=",
           const std::string& comment = "#");
    Config();
    //!< Search for key and read value or optional default value
    //call as read<T>
    template<class T> T read(const std::string& in_key) const;
    template<class T> T read(const std::string& in_key,
                             const T& in_value) const;
    //Get the value corresponding to key and store in var
    //Return true if key is found
    //Otherwise lear var untouched
    template<class T> bool readInto(T& out_val,const std::string& in_key) const;
    //Get the value corresponding to key and stor in var
    //Return true if key is found
    //Otherwise set var to given default
    template<class T>
    bool readInto(T& out_var,const std::string& in_key,const T& in_value) const;
    bool fileExist(const std::string& filename);
    void readFile(const std::string& filename,const std::string& delimiter = "=",
                  const std::string& comment = "#");
    // Check whether key exists in configuration
    bool keyExists(const std::string& in_key) const;
    //Modify keys and values
    template<class T> void add(const std::string& in_key,const T& in_value);
    void remove(const std::string& in_key);

    std::string getDelimiter() const {return m_Delimiter;}
    std::string getComment() const {return m_Comment;}
    std::string setDelimiter(const std::string& in_s)
    {std::string old = m_Delimiter;m_Delimiter = in_s; return old;}
    std::string setComment(const std::string& in_s)
    {std::string old = m_Comment;m_Comment = in_s; return old;}

protected:
    //Convert from a T to a string
    //Type T must support << operator
    template<class T> static std::string T_as_string(const T& t);
    //Convert from a string to a T
    //TYpe T must support >> operator
    template<class T> static T string_as_T(const std::string& s);
    static void trim(std::string& inout_s);

public:
    struct File_not_found
    {
        std::string filename;
        File_not_found(const std::string& filename_ = std::string())
            :filename(filename_){}
    };

    struct Key_not_found
    {
        std::string key;
        Key_not_found(const std::string& key_ = std::string())
            :key(key_){}
    };
};
template<class T>
std::string Config::T_as_string(const T& t)
{
    std::ostringstream ost;
    ost << t;
    return ost.str();
}

template<class T>
T Config::string_as_T(const std::string& s)
{
    T t;
    std::istringstream ist(s);
    ist >> t;
    return t;
}

template<>
inline std::string Config::string_as_T<std::string>(const std::string& s)
{
    //Convert from a string to a string
    //In other words,do nothing
    return s;
}

template<>
inline bool Config::string_as_T<bool>(const std::string& s)
{
    //Convert from a string to a bool
    //Interpret "false","F","no","n","0" as false
    //Interpret "true","T","yes","y","1","-1",or anthing else as true
    bool b = true;
    std::string sup = s;
    for(std::string::iterator p = sup.begin(); p != sup.end(); ++p)
        *p = toupper(*p);
    if(sup == std::string("FALSE") || sup == std::string("F")||
       sup == std::string("NO") || sup == std::string("N") ||
       sup == std::string("0") || sup == std::string("NONE"))
        b = false;
    return b;
}

template<class T>
T Config::read(const std::string& in_key) const
{
    mapci p = m_Contents.find(in_key);
    if(p ==  m_Contents.end()) throw Key_not_found(in_key);
    return string_as_T<T>(p->second);
}

template<class T>
T Config::read(const std::string& in_key,const T& value) const
{
    mapci p = m_Contents.find(in_key);
    if(p == m_Contents.end()) return value;
    return string_as_T<T>(p->second);
}

template<class T>
bool Config::readInto(T& var,const std::string& key) const
{
    mapci p = m_Contents.find(key);
    bool found = (p != m_Contents.end());
    if(found) var = string_as_T<T>(p->second);
    return found;
}

template<class T>
bool Config::readInto(T& var,const std::string& key,const T& value) const
{
    mapci p = m_Contents.find(key);
    bool found = (p != m_Contents.end());
    if(found)
        var = string_as_T<T>(p->second);
    else
        var = value;
    return found;
}

template<class T>
void Config::add(const std::string& in_key,const T& value)
{
    std::string v = T_as_string(value);
    std::string key = in_key;
    trim(key);
    trim(v);
    m_Contents[key] = v;
    return;
}

