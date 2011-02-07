#include <string>

std::string get_line_of_code(const char* code) //return the line of code
{
	std::string ret = code;	
	int i;	
	for(i = 0;i<ret.length();i++)
	{
		if(ret[i]=='\n') break;
	}	
	return ret.substr(0,i);
} 

std::string cut_commentary(const std::string line) //cut commentary at the end of line
{
	int i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='/' && line[i+1]=='/') return line.substr(0,i);
		if(line[i]=='/' && line[i+1]=='*') return line.substr(0,i);
	}
	return line;

}

bool is_derived(const std::string line) // return true if the struct or class is derived
{
	int i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]==':')
		{
			if(line[i+1]!=':') return true;
			else i+=1;
		}
	}
	return false;
}

std::string get_super_class(const std::string line)
{
	int i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]==':')
		{
			if(line[i+1]!=':') break;
			else i+=1;
		}
	}
	return line.substr(i+1);
}

std::string clean_spaces(const std::string line)
{
	int i;
	int j = 0;
	std::string new_line = "";
	for(i = 0;i<line.length();i++)
	{
		if(!isspace(line[i])) new_line+=line[i];
	}
	//std::cout<<new_line;
	return new_line;
}

bool is_state(const std::string line)
{
	int pos = line.find("::");
	if(pos==5)
	{
		if(line.compare(0,31,"boost::statechart::simple_state")==0)
		{
			return true;	
		}
		else
		{
			std::string str = line.substr(pos+2);
			if(str.compare(0,12,"simple_state")==0)return true;
		}
	}
	else
	{
		std::string str = line.substr(pos+2);
		//std::cout<<str;
		if(str.compare(0,12,"simple_state")==0)return true;
	}
	return false;
}
/*TODO
boost::statechart::simple_state
Pokud to neni ono, tak odstranim vse pred:: a zkusim nastrcit na simple_state
*/
