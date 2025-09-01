#include "request.hpp"

/************ CHECK FUNCTIONS ************/

bool    c_request::is_valid_header_name(const string& key_name)
{
	const string allowed_special_chars = "!#$%&'*+-.^_`|~";

	if (key_name.empty())
		return (false);
	for (size_t i = 0; i < key_name.length(); i++)
	{
		if (!isalnum(key_name[i]) && allowed_special_chars.find(key_name[i]) == string::npos)
			return (false);
	}
	return (true);
}

bool    c_request::is_valid_header_value(string& key, const string& value)
{
	for (size_t i = 0; i < value.length(); i++)
	{
		if ((value[i] < 32 && value[i] != '\t') || value[i] == 127)
		{
			this->_status_code = 400;
			return (false);
		}
	}

	if (key == "Content-Length")
	{
		for (size_t i = 0; i < value.length(); i++)
		{
			if (!isdigit(value[i]))
			{
				this->_status_code = 400;
				return (false);
			}
		}
		char   *end;
		this->_content_length = strtoul(value.c_str(), &end, 10);
	}

	return (true);
}

void c_request::check_required_headers()
{
	bool has_content_length = this->_headers.count("Content-Length");
	bool has_transfer_encoding = this->_headers.count("Transfer-Encoding");

    if (this->_method == "POST" && (has_content_length || has_transfer_encoding))
	{
		cout << "Warning: Request has body!\n" << endl;
        this->_has_body = true;
	}

	if (this->_headers.count("Host") != 1)
	{
		cerr << "(Request) Error: Header host" << endl;
		this->_error = true;
		this->_status_code = 400;
	}

	if (this->_has_body)
	{
		if (!has_content_length && !has_transfer_encoding)
		{
			this->_error = true;
			this->_status_code = 400;
		}
		if (has_content_length && has_transfer_encoding)
		{
			this->_error = true;
			this->_status_code = 400;
		}
	}     
}

void    c_request::check_port()
{
    char *end;
    size_t pos = this->get_header_value("Host").find_first_of(':');

    if (pos == string::npos)
        this->_port = 80;
    else
    {
        string port = this->_headers["Host"].substr(pos + 1);
        this->_port = strtod(port.c_str(), &end);
    }
}

/************ UTILS ************/

void	c_request::print_full_request()
{
	if (this->_request_fully_parsed)
	{
		cout << "*********** Start-line ************" << endl;
		cout << "method: " << this->_method << endl;
		cout << "target: " << this->_target << endl;
		cout << "version: " << this->_version << endl << endl;

		cout << "*********** Headers map ***********" << endl;
		for (map<string, string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
			cout << it->first << " : " << it->second << endl;
		cout << endl;
		
		if (this->_has_body)
		{
			cout << "*********** Body ************" << endl;
			cout << this->_body << endl;
		}

		cout << "Status code: " << this->_status_code << endl;
		cout << endl;
	}
}

void	c_request::init_request()
{
	this->_method.clear();
	this->_target.clear();
	this->_version.clear();
	this->_body.clear();
	this->_chunk_accumulator.clear();
	this->_status_code = 200;
	this->_port = 8080;
	this->_has_body = false;
	this->_chunk_line_count = 0;
	this->_expected_chunk_size = -1;
	this->_request_fully_parsed = false;
	this->_error = false; 
	this->_content_length = 0;

	for (map<string, string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		it->second = "";
}

string  c_request::ft_trim(const string& str)
{
    size_t start = 0;
    size_t end = str.length();

    while (start < end && (str[start] == ' ' || str[start] == '\t'))
        start++;

    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'))
        end--;
    
    return (str.substr(start, end - start));
}

// void debugLine(const std::string &s)
// {
//     std::cout << "DEBUG: [";
//     for (size_t i = 0; i < s.size(); ++i)
//     {
//         unsigned char c = static_cast<unsigned char>(s[i]);
//         if (isprint(c))
//             std::cout << s[i];
//         else
//             std::cout << "\\x" << std::hex << (int)c << std::dec;
//     }
//     std::cout << "] (len=" << s.size() << ")" << std::endl;
// }

