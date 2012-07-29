/** @file */
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

string clean_spaces(const string line) /** Remove gaps from string.*/
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

string cut_commentary(const string line) /** This function cuts commentaries at the end of line. */
{
	unsigned i = 0;
	for(i = 0;i<line.length()-1;i++)
	{
		if(line[i]=='/' && line[i+1]=='/') return line.substr(0,i);
		if(line[i]=='/' && line[i+1]=='*') return line.substr(0,i);
	}
	return line;
}

string get_line_of_code(const string code) /** Thie function return the line of code that belongs to a declaration. The output string line doesn't have gaps and commentaries.*/
{
	string ret;
	unsigned i;
	for(i = 0;i<code.length();i++)
	{
		if(code[i]=='\n'||code[i]=='{') break;
	}
	ret = code.substr(0,i);
	ret = clean_spaces(ret);
	return cut_commentary(ret);
}

string get_return(const string code) /** Return return statement. */
{
	string ret;
	unsigned i;
	for(i = 0;i<code.length();i++)
	{
		if(code[i]==' ') break;
	}
	return code.substr(0,i);
}

string cut_namespaces(string line) /** Cut namespaces from the declarations. */
{
	int i;
	int brackets = 0;
	for(i = line.length()-1;i>0;i--)
	{
		if(line[i]=='<') brackets+=1;
		if(line[i]=='>') brackets-=1;
		if(line[i]==':' && line[i-1]==':')
		{
			if(brackets == 0) break;
			else i--;
		}
	}
	if(i==0) return line;
	return line.substr(i+1);
}

string cut_namespaces(string line, int ortho) /** Cut namespaces ORTHOGONAL STATES. */
{
	int i;
	int brackets = 0;
	for(i = line.length()-1;i>0;i--)
	{
		if(line[i]=='<') brackets+=1;
		if(line[i]=='>') brackets-=1;
		if(line[i]==':' && line[i-1]==':')
		{
			if(brackets == 0) break;
			else i--;
		}
	}
	if(i==0) return line;
	return line.substr(0,i-1);
}

bool is_derived(const string line) /** Test whether this struct or class is derived */
{
	unsigned i;
	for(i = 0;i<line.length()-2;i++)
	{
		if(line[i]==':')
		{
			if(line[i+1]!=':') return true;
			else i+=1;
		}
	}
	return false;
}

string get_super_class(const string line) /** Get the super classes of this declarations. */
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

string get_next_base(const string line) /** Get next super classes of this declarations Do the first super class is ommitted. */
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

string get_first_base(const string line) /** Get the first super class of this declarations. */
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

bool is_state(const string line) /** Test if this declaration is a state. It is used to test the base classes. */
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

string cut_type(string line) /** This function cuts type of the declaration from the string. */
{
	unsigned i;
	for(i = 0;i<line.length();i++)
	{
		if(line[i]==' ') break;
	}
	return line.substr(i);
}

int count(const string line, const char c) /** Count all specified char in string. So it returns the number of specified characters. */
{
	int number = 0;
	for(unsigned i = 0;i<line.length();i++)
	{
		if(line[i]==c) number+=1;
	}
	return number;
}

bool is_list(const string line) /** Test if this decl is mpl::list. */
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

string get_inner_part(const string line) /** Get inner part of the list. */
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
	return str.substr(0,i);
}

int get_model(const string line) /** Test the string to has a specified model. */
{
	string str = cut_namespaces(line);
	if(str.find("<")<str.length()) str = str.substr(0,str.find("<"));
	switch(str.length())
	{
		case 5 : if(str.compare(0,5,"event")==0) return 1;
					break;
		case 6 : if(str.compare(0,6,"result")==0) return 5;
				   break;
		case 7 : if(str.compare(0,7,"transit")==0) return 6;
				   break;
		case 8 : if(str.compare(0,8,"deferral")==0) return 13;
				   break;
		case 10 : if(str.compare(0,10,"transition")==0) return 11;
					 break;
		case 11 : if(str.compare(0,11,"defer_event")==0) return 7;
					 break;
		case 13 : if(str.compare(0,13,"state_machine")==0) return 3;
					 break;
		case 15 : if(str.compare(0,15,"custom_reaction")==0) return 12;
					 break;
		default : return -1;
	}
	return -1;
}

string get_params(string line) /** Return parameters of the specified transition */
{
	int pos_front = line.find("<")+1;
	int pos_end = line.rfind(">");
	return line.substr(pos_front,pos_end-pos_front);
}

string find_states(const CXXRecordDecl *cRecDecl, string line) /** test if the struct/class is he state (must be derived from simple_state or state). */
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
			params = get_params(super_class);
		}
		else params = "";
	}
	return params;
}

string find_name_of_machine(const CXXRecordDecl *cRecDecl, string line) /** Find name of the state machine and the start state. */
{
	string super_class = get_super_class(line), base, params;
	if(cRecDecl->getNumBases()>1)
	{
		for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
		{
			if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
			else base = super_class;
			if(get_model(base)==3)
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
		if(get_model(super_class)==3)
		{
			params = get_params(super_class);
		}
	}
	return params;
}

string find_transitions (const string name_of_state, string line) /** Traverse all methods for finding declarations of transitions. */
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
		if(get_model(base)>10)
		{
			if(get_model(base)==12) dest = ";";
			else if (get_model(base) == 13) dest = ",";
			else dest = "";
			dest.append(name_of_state);
			params = get_params(base);
			dest.append(",");
			dest.append(params);
			trans.append(dest);
			if(j!=num-1) trans.append(";");
		}
		line = get_next_base(line);
	}
	if(trans[trans.length()-1]==';') return trans.substr(0,trans.length()-1);
	else return trans;
}

bool find_events(const CXXRecordDecl *cRecDecl, string line) /** This function provides testing if the decl is decl of event*/
{
	string super_class = get_super_class(line), base, params;
	if(cRecDecl->getNumBases()>1)
	{
		for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
		{
			if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
			else base = super_class;
			if(get_model(base)==1) return true;
			else
			{
				super_class = get_next_base(super_class);
			}
		}
	}
	else
	{
		if(get_model(super_class)==1)return true;
	}
	return false;
}
