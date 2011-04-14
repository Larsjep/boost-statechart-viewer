////////////////////////////////////////////////////////////////////////////////////////  
//    
//    This file is part of Boost Statechart Viewer.
//
//    Boost Statechart Viewer is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Boost Statechart Viewer is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Boost Statechart Viewer.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <string>

using namespace std;
using namespace clang;
	
string clean_spaces(const string line) // remove backspaces
{
	unsigned i;
	string new_line = "";
	for(i = 0;i<line.length();i++)
	{
		if(!isspace(line[i])) new_line+=line[i];
	}
	//cout<<new_line;
	return new_line;
}

string cut_commentary(const string line) //cut commentary at the end of line
{
	unsigned i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='/' && line[i+1]=='/') return line.substr(0,i);
		if(line[i]=='/' && line[i+1]=='*') return line.substr(0,i);
	}
	return line;
}

string get_line_of_code(const string code) //return the line of code
{
	string ret;	
	unsigned i;	
	//cout<<code<<"\n\n";
	for(i = 0;i<code.length();i++)
	{
		if(code[i]=='\n'||code[i]=='{') break;
	}
	ret = code.substr(0,i);
	ret = clean_spaces(ret);
	return cut_commentary(ret);
} 

string get_return(const string code)
{
	string ret;	
	unsigned i;
	for(i = 0;i<code.length();i++)
	{
		if(code[i]==' ') break;
	}
	return code.substr(0,i);
}

string cut_namespaces(string line) //cut namespaces from the name of the state
{
	int i = line.rfind("::");
	if(i==-1) return line;
	return line.substr(i+2);
}

bool is_derived(const string line) // return true if the struct or class is derived
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

string get_super_class(const string line) // get the super class of this declarations
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

string get_next_base(const string line) // get the super class of this declarations
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

string get_first_base(const string line) // get the super class of this declarations
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

bool is_state(const string line) // test if it is a state
{
	if(cut_namespaces(line).compare(0,12,"simple_state")==0)
	{
		return true;	
	}
	else
	{
		if(cut_namespaces(line).compare(0,5,"state")==0)
		{
			return true;	
		}
		return false;
	}
}

string cut_type(string line) // cut typedef from the beginning
{
	unsigned i;
	for(i = 0;i<line.length();i++)
	{
		if(line[i]==' ') break;
	}
	return line.substr(i);
}

int count(const string line, const char c) //count all specified char in string
{
	int number = 0;
	for(unsigned i = 0;i<line.length();i++)
	{
		if(line[i]==c) number+=1;
	}
	return number;
}

bool is_list(const string line) // is it a list?
{
	//cout<<line<<"\n";
	int pos = 0;
	for(unsigned i = 0; i<line.length();i++)
	{
		if(line[i]=='<') break;
		if(line[i]==':' && line[i+1]==':') pos = i+2;
	}	
	if(line.substr(pos).compare(0,4,"list")==0)
	{
		return true;	
	}
	else
	{
		return false;
	}
}

string get_inner_part(const string line) // get inner part of the list
{
	string str;
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
	//cout<<str.substr(0,i);
	return str.substr(0,i);
}

bool test_model(const string line, const string model) // is it a transition
{
	if(cut_namespaces(line).compare(0,model.length(),model)==0)
	{
		return true;	
	}
	else
	{
		return false;
	}
}

string get_params(string line) // get parameters of transition
{
	int pos_front = line.find("<")+1;
	int pos_end = line.find(">");
	return line.substr(pos_front,pos_end-pos_front);
}
	
string find_states(const CXXRecordDecl *cRecDecl, string line) // test if the struct/class is he state (must be derived from simple_state)
{	
	string super_class = get_super_class(line), base, params;		
	if(cRecDecl->getNumBases()>1)
	{
		for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
		{
			if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
			else base = super_class;
			if(is_state(super_class)) 
			{
				params = get_params(super_class);
			}
			else
			{
				super_class = get_next_base(super_class);
			}
		}
	}
	else
	{ 
		if(is_state(super_class)) 
		{
			//std::cout<<get_params(super_class);
			params = get_params(super_class);
		}
		else params = "";
	}
	return params;
}
		
string find_name_of_machine(const CXXRecordDecl *cRecDecl, string line) // find name of the state machine and the start state
{	
	string super_class = get_super_class(line), base, params;
	if(cRecDecl->getNumBases()>1)
	{
		for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
		{
			if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
			else base = super_class;
			if(test_model(base,"state_machine"))
			{
				params = get_params(base);
			}
			else
			{
				super_class = get_next_base(super_class);
			}
		}
	}
	else
	{ 
		if(test_model(super_class,"state_machine"))
		{
			//std::cout<<super_class;
			params = get_params(super_class);
			//std::cout<<params;
		}
	}
	return params;
}

string find_transitions (const string name_of_state, string line) // traverse all methods for finding declarations of transitions
{	
	string dest, params, base, trans;
	int num = count(line,'<');	
	if(num>1)
	{
		num-=1;
		if(is_list(line))
		{
			line = get_inner_part(line);
		}
	}
	for(int j = 0;j<num;j++)
	{
		if(j!=num-1) base = get_first_base(line);			
		else base = line;
		if(test_model(base,"transition"))
		{
			dest = name_of_state;
			params = get_params(base);
			//cout<<params<<"\n";
			dest.append(",");							
			dest.append(params);
			line = get_next_base(line);
			trans.append(dest);
			if(j+1!=num) trans.append(";");
		}
		else if(test_model(base,"custom_reaction"))
		{
			dest = ";";
			dest.append(name_of_state);
			params = get_params(base);
			//cout<<params<<"\n";
			dest.append(",");							
			dest.append(params);
			line = get_next_base(line);
			trans.append(dest);
			if(j+1!=num) trans.append(";");
		}
		else
		{
			line = get_next_base(line);
		}
	}
	if(trans[trans.length()-1]==';') return trans.substr(0,trans.length()-1);
	else return trans;	
}

bool find_events(const CXXRecordDecl *cRecDecl, string line) // test if the decl is decl of event
{	
	string super_class = get_super_class(line), base, params;
	if(cRecDecl->getNumBases()>1)
	{
		for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
		{
			if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
			else base = super_class;
			if(test_model(base,"event")) return true;
			else
			{
				super_class = get_next_base(super_class);
			}
		}
	}
	else
	{ 
		if(test_model(super_class,"event"))return true;
	}
	return false;
}

