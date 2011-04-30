
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

#include <fstream>
#include <list>
#include <string>

#include "stringoper.h"

using namespace std;

class IO_operations
{
	list<string> transitions;
	list<string> states;
	list<string> events;
	string outputFilename;
	string name_of_machine;
	string name_of_first_state;
	string *table;
	int nState;
	int cols, rows;
	int find_place(string model, int type)
	{
		if(type == 1)
		{
			for(unsigned i = 3;i<events.size()+3;i++)
			{
				if(model.compare(0,model.size(),table[i])==0) return i;
			}
		}
		else 
		{
			for(unsigned i = 1;i<states.size()+1;i++)
			if(model.compare(0,model.size(),table[i*cols+2])==0) return i;
		}
		return -1;
	}
	public:
	IO_operations() {}
	
	IO_operations( const string outputFile, const string FSM_name, const string firstState, const list<string> trans, const list<string> state, const list<string> ev )
	{
		outputFilename = outputFile;
		name_of_machine = FSM_name;
		name_of_first_state = firstState;
		transitions = trans;
		states = state;
		events = ev;
	}
	~IO_operations()
	{
		delete table;
	}
	// set methods
	void setEvents(list<string> events)
	{
		this->events = events;
	}
	
	void setTransitions(list<string> transitions)
	{
		this->transitions = transitions;
	}

	void setStates(list<string> states)
	{
		this->states = states;
	}

	void setNameOfStateMachine(string name_of_FSM)
	{
		name_of_machine = name_of_FSM;
	}

	void setNameOfFirstState(string first_state)
	{
		name_of_first_state = first_state;
	}

	void setOutputFilename(string outputFilename)
	{
		this->outputFilename = outputFilename;
	}
	
	// outputfile writing methods
	void write_states(ofstream& filestr) // write states
	{
		int pos1, pos2, cnt, subs;
		nState = 1;
		string context, state, ctx, sState, str;
		list<string> nstates = states;
		context = name_of_machine;
		table[0] = "S";
		table[1] = "Context";
		table[2] = "State";
		for(list<string>::iterator i = nstates.begin();i!=nstates.end();i++) // write all states in the context of the automaton
		{
			state = *i;
			cnt = count(state,',');
			if(cnt==1)
			{
				pos1 = state.find(",");
				ctx = cut_namespaces(state.substr(pos1+1));
				if(ctx.compare(0,context.length(),context)==0)
				{
					str = cut_namespaces(state.substr(0,pos1));
					filestr<<str<<";\n";
					table[cols*nState+2] = str;
					table[cols*nState+1] = context;
					nState+=1;
					nstates.erase(i);
					i--;
					if(str.compare(0,str.length(),name_of_first_state)==0) table[cols*(nState-1)] = "*";
				}
			}
			if(cnt==2)
			{
				pos1 = state.find(",");
				pos2 = state.rfind(",");
				ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
				if(ctx.compare(0,context.length(),context)==0)
				{				
					str = cut_namespaces(state.substr(0,pos1));
					filestr<<str<<";\n";
					table[cols*nState+2] = str;
					table[cols*nState+1] = context;			
					nState+=1;
					if(str.compare(0,str.length(),name_of_first_state)==0) table[cols*(nState-1)] = "*";
				}
			}
		}
		filestr<<name_of_first_state<<" [peripheries=2] ;\n";
		subs = 0;
		while(!nstates.empty()) // substates ?
		{
			state = nstates.front();
			filestr<<"subgraph cluster"<<subs<<" {\n";			
			pos1 = state.find(",");
			pos2 = state.rfind(",");
			context = cut_namespaces(state.substr(0,pos1));
			filestr<<"label=\""<<context<<"\";\n";
			sState = cut_namespaces(state.substr(pos2+1));
			filestr<<sState<<" [peripheries=2] ;\n";	
			nstates.pop_front();	
			for(list<string>::iterator i = nstates.begin();i!=nstates.end();i++)
			{
				state = *i;
				cnt = count(state,',');
				if(cnt==1)
				{
					pos1 = state.find(",");
					ctx = cut_namespaces(state.substr(pos1+1));
					if(ctx.compare(0,context.length(),context)==0)
					{
						str = cut_namespaces(state.substr(0,pos1));
						filestr<<str<<";\n";
						table[cols*nState+2] = str;
						table[cols*nState+1] = context;
						nState+=1;
						nstates.erase(i);
						i--;
						if(str.compare(0,str.length(),sState)==0) table[cols*(nState-1)] = "*";
					}
				}
				if(cnt==2)
				{
					pos1 = state.find(",");
					pos2 = state.rfind(",");
					ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
					if(ctx.compare(0,context.length(),context)==0) 
					{
						str = cut_namespaces(state.substr(0,pos1));
						filestr<<str<<";\n";
						table[cols*nState+2] = str;
						table[cols*nState+1] = context;
						nState+=1;
						if(str.compare(0,str.length(),sState)==0) table[cols*(nState-1)] = "*";
					}
				}
			}
			filestr<<"}\n";
			subs+=1;	
		}
		return;
	}

	void write_transitions(ofstream& filestr) // write transitions
	{
		int pos1, pos2;
		string params, state, event, dest;
		for(list<string>::iterator i = transitions.begin();i!=transitions.end();i++) // write all transitions
		{
			params = *i;
			pos1 = params.find(",");
			state = cut_namespaces(params.substr(0,pos1));
			filestr<<state<<"->";
			pos2 = params.rfind(",");
			dest = cut_namespaces(params.substr(pos2+1));
			filestr<<dest;
			event = cut_namespaces(params.substr(pos1+1,pos2-pos1-1));
			filestr<<"[label=\""<<event<<"\"];\n";
			table[find_place(state,2)*cols+find_place(event,1)]=dest;
		}		
		return;
	}

	void fill_table_with_events()
	{
		int j = 3;
		for(list<string>::iterator i = events.begin();i!=events.end();i++)
		{
			table[j] = *i;
			j++;
		}
	}

	void save_to_file() // save state automaton to file
	{
		if(!name_of_first_state.empty())	
		{	
			ofstream filestr(outputFilename.c_str());
			filestr<<"digraph "<< name_of_machine<< " {\n";
			cols = events.size()+3;
			rows = states.size()+1;
			table = new string [cols*rows];
			fill_table_with_events();			
			write_states(filestr);
			write_transitions(filestr);
			filestr<<"}";		
			filestr.close();
			print_table();
		}
		else cout<<"No state machine was found. So no output file was created.\n";
		return;
	}

	void print_table()
	{
		cout<<"\nTRANSITION TABLE\n";
		unsigned * len = new unsigned[cols];
		len[0] = 1;
		string line = "-|---|-";
		for(unsigned i = 1; i<cols; i++)
		{
			len[i] = 0;
			for(unsigned j = 0;j<rows;j++)
			{
				if(len[i]<table[j*cols+i].length()) len[i] = table[j*cols+i].length();
			}
			for(unsigned k = 0; k<len[i]; k++)
			{
				line.append("-");
			}
			line.append("-|-");
		}
		cout<<line<<"\n";
		for(unsigned i = 0; i<rows; i++)
		{
			cout<<" | ";		
			for(unsigned j = 0;j<cols;j++)
			{
				cout.width(len[j]);
				cout<<left<<table[i*cols+j]<<" | ";
			}
			cout<<"\n";
			cout<<line<<"\n";
		}
		delete len;
	}
	// method for printing statistics about state automaton
	void print_stats() // print statistics
	{
		cout<<"\n"<<"Statistics: \n";
		cout<<"Number of states: "<<states.size()<<"\n";
		cout<<"Number of events: "<<events.size()<<"\n";
		cout<<"Number of transitions: "<<transitions.size()<<"\n\n";
		return;
	}

};
