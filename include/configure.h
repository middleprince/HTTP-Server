#ifndef MINISERVER_CONFIGURE_H
#define MINISERVER_CONFIGURE_H

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <sstream>
#include <stdexcept>

namespace miniserver {

class ConfigureValueType {
public:
   ConfigureValueType();
   ConfigureValueType(const std::string &conf_value);
   ~ConfigureValueType();

  ConfigureValueType(const ConfigureValueType &other); 
  
  int32_t toInt32() const;
  int64_t toInt64() const;
  uint32_t toUint32() const;
  uint64_t toUint64() const;
  double toDouble() const;
  std::string toString() const;
  char toChar() const; 

private:
    template <typename T>
    T toType() const;

    std::string _conf_value;

    ConfigureValueType& operator=(const ConfigureValueType &rhs);
    
};

class ConfigureParser {
public:
    ConfigureParser();
    ~ConfigureParser();

    //@bref: parse configure file for server configurating
    //@file_path: Configure file path
    //@conf_pairs: poiter map, store parsed pair
    bool parse(const std::string &file_path,
            std::map<std::string, std::string> *conf_pairs);
private:
    
    //@bref: foremat str, exclude space, strs bgins with # 
    //@str: the string to be format
    //@return: success true, failed false.
    bool _strFormat(std::string *str);

    ConfigureParser& operator=(const ConfigureParser &rhs);
};

class Configure {
public:
    Configure();
    ~Configure();

    bool initConf(const std::string &file_path);
    ConfigureValueType operator[](const std::string &conf_key);
    
private:
    ConfigureParser _conf_parser;
    std::map<std::string, std::string> _conf_pairs;
    
    Configure(const Configure &other);
    Configure& operator=(const Configure &other);
};

}

#endif
