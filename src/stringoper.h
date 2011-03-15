#include <string>

std::string get_line_of_code(const std::string code) //return the line of code
{
	std::string ret = code;	
	unsigned i;	
	//std::cout<<code<<"\n\n";
	for(i = 0;i<ret.length();i++)
	{
		if(ret[i]=='\n'||ret[i]=='{') break;
	}	
	return ret.substr(0,i);
} 

std::string cut_commentary(const std::string line) //cut commentary at the end of line
{
	unsigned i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='/' && line[i+1]=='/') return line.substr(0,i);
		if(line[i]=='/' && line[i+1]=='*') return line.substr(0,i);
	}
	return line;

}

std::string cut_namespaces(std::string line) //cut namespaces from the name of the state
{
	int i = line.rfind("::");
	if(i==-1) return line;
	return line.substr(i+2);
}

bool is_derived(const std::string line) // return true if the struct or class is derived
{
	unsigned i;
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

std::string get_super_class(const std::string line) // get the super class of this declarations
{
	unsigned i;
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

std::string get_next_base(const std::string line) // get the super class of this declarations
{
	unsigned i;
	int brackets = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='<') brackets+=1;
		if(line[i]=='>') brackets-=1;
		if(line[i]==',' && brackets == 0) break;
	}
	return line.substr(i+1);
}  

std::string get_first_base(const std::string line) // get the super class of this declarations
{
	unsigned i;
	int brackets = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='<') brackets+=1;
		if(line[i]=='>') brackets-=1;
		if(line[i]==',' && brackets == 0) break;
	}
	return line.substr(0,i);
}  


std::string clean_spaces(const std::string line)
{
	unsigned i;
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
	if(pos == 5)
	{
		if(line.compare(0,31,"boost::statechart::simple_state")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(line.compare(0,24,"statechart::simple_state")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
}
// Transitions
std::string cut_typedef(std::string line) // cut typedef from the beginning
{
	if(line.compare(0,8,"typedef ")==0)
	{
		return line.substr(8);
	}
	else return line;	
}

int count(const std::string line, const char c) //count all < in string
{
	int number = 0;
	for(unsigned i = 0;i<line.length();i++)
	{
		if(line[i]==c) number+=1;
	}
	return number;
}

bool is_list(const std::string line)
{
	int pos = line.find("::");
	if(pos == 5)
	{
		if(line.compare(0,16,"boost::mpl::list")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
	if(line.compare(0,9,"mpl::list")==0)
	{
		return true;	
	}
	else
	{
		return false;
	}
}

std::string get_inner_part(const std::string line)
{
	std::string str;
	unsigned i, pos = 0;
	for(i = 0;i<line.length();i++)
	{
		if(line[i]=='<') break;
	}
	str = line.substr(i+1);
	for(i = 0;i<str.length();i++)
	{
		if(str[i]=='<') pos+=1;
		if(str[i]=='>')
		{ 
			if(pos==0) break;
			else pos-=1;
		}
	}
	//std::cout<<str.substr(0,i);
	return str.substr(0,i);
}

bool is_transition(const std::string line)
{
	int pos = line.find("::");
	if(pos == 5)
	{
		if(line.compare(0,29,"boost::statechart::transition")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
	else
	{	
		if(line.compare(0,22,"statechart::transition")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
}

std::string get_params(std::string line)
{
	int pos_front = line.find("<")+1;
	int pos_end = line.find(">");
	std::string params;
	params = line.substr(pos_front,pos_end-pos_front);
	return params;
	
}

bool is_machine(const std::string line)
{
	int pos = line.find("::");
	if(pos == 5)
	{
		if(line.compare(0,32,"boost::statechart::state_machine")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
	else
	{	
		if(line.compare(0,25,"statechart::state_machine")==0)
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
}
