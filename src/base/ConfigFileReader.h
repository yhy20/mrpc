#ifndef __CONFIG_FILE_READER_H__
#define __CONFIG_FILE_READER_H__

#include <map>
#include <string>

namespace mrpc
{

class CConfigFileReader {
public:
    CConfigFileReader();
    explicit CConfigFileReader(const char* filename);
    ~CConfigFileReader();

    char* getConfigName(const char* name);
    int setConfigValue(const char* name, const char* value);
    void loadFile(const char* filename);

private:
    int writeFile(const char* filename = NULL);
    void parseLine(char* line);
    char* trimSpace(char* name);

    bool        m_loadSuccess;
    std::string m_configFile;
    std::map<std::string, std::string> m_configMap;
    
};

}  // namespace mrpc

#endif 
