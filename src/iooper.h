
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
		string context, state, ctx;
		list<string> nstates = states;
		context = name_of_machine;
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
					filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
					nstates.erase(i);
					i--;
				}
			}
			if(cnt==2)
			{
				pos1 = state.find(",");
				pos2 = state.rfind(",");
				ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
				if(ctx.compare(0,context.length(),context)==0)
				{				
					filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
				}
			}
		}
		subs = 0;
		while(!nstates.empty()) // substates ?
		{
			state = nstates.front();
			filestr<<"subgraph cluster"<<subs<<" {\n";			
			pos1 = state.find(",");
			pos2 = state.rfind(",");
			context = cut_namespaces(state.substr(0,pos1));
			filestr<<"label=\""<<context<<"\";\n";
			filestr<<cut_namespaces(state.substr(pos2+1))<<" [peripheries=2] ;\n";	
			nstates.pop_front();	
			//std::cout<<states.size();	
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
						filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
						nstates.erase(i);
						i--;
					}
				}
				if(cnt==2)
				{
					pos1 = state.find(",");
					pos2 = state.rfind(",");
					ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
					if(ctx.compare(0,context.length(),context)==0) filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
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
		string state;
		for(list<string>::iterator i = transitions.begin();i!=transitions.end();i++) // write all transitions
		{
			state = *i;
			pos1 = state.find(",");
			filestr<<cut_namespaces(state.substr(0,pos1))<<"->";
			pos2 = state.rfind(",");
			filestr<<cut_namespaces(state.substr(pos2+1));
			filestr<<"[label=\""<<cut_namespaces(state.substr(pos1+1,pos2-pos1-1))<<"\"];\n";
		}		
		return;
	}

	void save_to_file() // save state automaton to file
	{
		if(!name_of_first_state.empty())	
		{	
			ofstream filestr(outputFilename.c_str());
			filestr<<"digraph "<< name_of_machine<< " {\n";
			filestr<<name_of_first_state<<" [peripheries=2] ;\n";
			write_states(filestr);
			write_transitions(filestr);
			filestr<<"}";		
			filestr.close();
		}
		else cout<<"No state machine was found. So no output file was created.\n";
		return;
	}
		
	// method for printing statistics about state automaton
	void print_stats() // print statistics
	{
		cout<<"\n"<<"Statistics: \n";
		cout<<"Number of states: "<<states.size()<<"\n";
		cout<<"Number of events: "<<events.size()<<"\n";
		cout<<"Number of transitions: "<<transitions.size()<<"\n";
		return;
	}

};
