#include "../include/config/config.h"

using namespace std;

Config::Config(const string& filename,const string& delimiter,
               const string& comment)
               :m_Delimiter(delimiter),m_Comment(comment)
{
    ifstream in(filename.c_str());
    if(!in) throw File_not_found(filename);
    in >> (*this);
}

Config::Config()
        :m_Delimiter(string(1,'=')),m_Comment(string(1,'#'))
{
    //Construct a Config without a file;empty
}


bool Config::keyExists(const string& key) const
{
    mapci p = m_Contents.find(key);
    return (p != m_Contents.end());
}

void Config::trim(string& inout_s)
{
    //Notice there is a " " in whitespace
    static const char whitespace[] = " \n\t\v\r\f";
    inout_s.erase(0,inout_s.find_first_not_of(whitespace));
    inout_s.erase(inout_s.find_last_not_of(whitespace) + 1U);
}

void Config::remove(const string& key)
{
    m_Contents.erase(m_Contents.find(key));
    return;
}

bool Config::fileExist(const string& filename)
{
    bool exists = false;
    ifstream in(filename.c_str());
    if(!in) exists = true;
    return exists;
}

void Config::readFile(const string& filename,const string& delimiter,
                      const string& comment)
{
    m_Delimiter = delimiter;
    m_Comment   = comment;
    ifstream in(filename.c_str());

    if(!in) throw File_not_found(filename);
    in >> (*this);
}

ostream& operator<<(ostream& os,const Config& cf)
{
    //Save a Config to os
    for(Config::mapci p = cf.m_Contents.begin();
            p != cf.m_Contents.end(); ++p){
        os<<p->first <<" " << cf.m_Delimiter<<" ";
        os<<p->second<<endl;
    }
    return os;
}

istream& operator>>(istream& is,Config& cf)
{
    typedef string::size_type pos;
    const string& delim = cf.m_Delimiter;
    const string& comm  = cf.m_Comment;
    const pos skip = delim.length();

    string nextline="";

    while(is || nextline.length() > 0){
        string line;
        if(nextline.length() > 0){
            line = nextline;
            nextline = "";
        }else{
            getline(is,line);
        }
        //Ignore comments
        line = line.substr(0,line.find(comm));

        //Parse the line if it contains a delimiter
        pos delimPos = line.find(delim);
        if(delimPos < string::npos){
            //Extract the key
            string key = line.substr(0,delimPos);
            line.replace(0,delimPos+skip,"");
            //See if value continues on the next line
            //Stop at blank line,next line with a key,end of stream,
            //or end of file sentry
            bool terminate = false;
            while(!terminate && is){
                getline(is,nextline);
                terminate = true;

                string nlcopy = nextline;
                Config::trim(nlcopy);
                if(nlcopy == "") continue;

                nextline = nextline.substr(0,nextline.find(comm));
                if(nextline.find(delim) != string::npos)
                    continue;
                nlcopy = nextline;
                Config::trim(nlcopy);
                if(nlcopy != "") line += "\n";
                line += nextline;
                terminate = false;
            }
            Config::trim(key);
            Config::trim(line);
            cf.m_Contents[key] = line;
        }
    }
    return is;
}
