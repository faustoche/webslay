#include "werbserv_config.hpp"

/*-----------------  CONSTRUCTOR -------------------*/

c_webserv_config::c_webserv_config(string const & file) : _config_files(file)
{
    
}

/*-----------------  DESTRUCTOR -------------------*/

c_webserv_config::~c_webserv_config()
{

}

/*-------------  MEMBERS FUNCTIONS ----------------*/

bool    c_webserv_config::load_configuration()
{
    try
    {
        c_parser  parser(_config_files);
        this->_servers = parser.parse();
        if(_servers.empty())
            throw invalid_argument("server empty");
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}


vector<c_server> &  c_webserv_config::get_servers()
{
    return this->_servers;
}


/*-----------------  utils ---------------------*/

void    c_webserv_config::print_configurations()
{
    vector<c_server>::iterator it;
    int nb_server = 1;

    for (it = _servers.begin(); it != _servers.end(); it++)
    {
        cout << nb_server << ") Server = " << endl; // remplacer par le nom du serveur
        it->print_config();
        nb_server++;
    }
}