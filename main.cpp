#include "src/server/Server.hpp"
#include "src/stdlib.hpp"
#include "src/parsing/Conf.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        std::cerr << "Usage: " << argv[0] << " <config_file_name>" << std::endl;
        return 1;
    }
    
    Server serv;

    serv.start();
    // Conf confParser;

    // if (!confParser.parseConfigFile(argv[1]))
    // {
    //     std::cerr << "Error parsing the configuration file." << std::endl;
    //     return 1;
    // }

    // confParser.printGlobalConfig();
    // std::cout << "-----------------zzzzz--------------" << std::endl;
    // confParser.printServerConf();



    return 0;
}
