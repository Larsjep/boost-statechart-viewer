//standard header files
#include <iostream>
#include <string>
#include <fstream>
#include <list>

//LLVM Header files
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/Config/config.h"

//clang header files
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/Utils.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Sema/Lookup.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Basic/Version.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"

//my own header files
#include "stringoper.h"

using namespace clang;
using namespace clang::driver;
using namespace std;

class MyDiagnosticClient : public TextDiagnosticPrinter // My diagnostic Client
{
	public:
	MyDiagnosticClient(llvm::raw_ostream &os, const DiagnosticOptions &diags, bool OwnsOutputStream = false):TextDiagnosticPrinter(os, diags, OwnsOutputStream = false){}
	virtual void HandleDiagnostic(Diagnostic::Level DiagLevel, const DiagnosticInfo &Info)
	{
		TextDiagnosticPrinter::HandleDiagnostic(DiagLevel, Info); // print diagnostic information
		if(DiagLevel > 2) // if error/fatal error stop the program
		{		
			exit(1);
		}	
	}
};

class FindStates : public ASTConsumer
{
	list<string> transitions;
	list<string> cReactions;
	list<string> events;
	string name_of_machine;
	string name_of_start;
	int nbrStates;
	FullSourceLoc *fsloc;
	public:
	list<string> states;
	
	virtual void Initialize(ASTContext &ctx)//run after the AST is constructed before the consumer starts to work
	{	
		fsloc = new FullSourceLoc(* new SourceLocation(), ctx.getSourceManager());
		name_of_start = "";
		name_of_machine = "";
		nbrStates = 0;
	}

	virtual void HandleTopLevelDecl(DeclGroupRef DGR)// traverse all top level declarations
	{
		SourceLocation loc;
      string line, output, event;
		llvm::raw_string_ostream x(output);
		for (DeclGroupRef::iterator i = DGR.begin(), e = DGR.end(); i != e; ++i) 
		{
			const Decl *decl = *i;
			loc = decl->getLocation();
			if(loc.isValid())
			{
				//cout<<decl->getKind()<<"ss\n";
				if(decl->getKind()==35)
				{					
					method_decl(decl);
				}
				if (const TagDecl *tagDecl = dyn_cast<TagDecl>(decl))
				{
					if(tagDecl->isStruct() || tagDecl->isClass()) //is it a struct or class	
					{
						struct_class(decl);
					}
				}	
				if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
				{
					
					DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
					//cout<<namedDecl->getNameAsString()<<"   sss\n";
					recursive_visit(declCont);
				
				}
			}
			output = "";
		}
	}
	void recursive_visit(const DeclContext *declCont) //recursively visit all decls hidden inside namespaces
	{
      string line, output, event;
		llvm::raw_string_ostream x(output);
		SourceLocation loc;
		for (DeclContext::decl_iterator i = declCont->decls_begin(), e = declCont->decls_end(); i != e; ++i)
		{
			const Decl *decl = *i;
			//std::cout<<"a "<<decl->getDeclKindName()<<"\n";
			loc = decl->getLocation();
			if(loc.isValid())
			{	
				if(decl->getKind()==35)
				{
					method_decl(decl);
				}
				else if (const TagDecl *tagDecl = dyn_cast<TagDecl>(decl))
				{
					if(tagDecl->isStruct() || tagDecl->isClass()) //is it a structure or class	
					{
						struct_class(decl);
					}	
				}
				else if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
				{
					DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
					//cout<<namedDecl->getNameAsString()<<"  sss\n";			
					recursive_visit(declCont);
				}
			}
			output = "";
		} 
	}
		
	void struct_class(const Decl *decl) // works with struct or class decl
	{
		string output, line, ret, trans, event;	
		llvm::raw_string_ostream x(output);
		decl->print(x);
		line = get_line_of_code(x.str());
		output = "";
		int pos;
		const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);		
		if(is_derived(line))
		{
			const CXXRecordDecl *cRecDecl = dyn_cast<CXXRecordDecl>(decl);
					
			if(find_events(cRecDecl, line))
			{
				events.push_back(namedDecl->getNameAsString());
				cout<<"New event: "<<namedDecl->getNameAsString()<<"\n";
			}
			else if(name_of_machine == "")
			{
				ret = find_name_of_machine(cRecDecl, line);
				if(!ret.empty())
				{
					pos = ret.find(",");
					name_of_machine = ret.substr(0,pos);
					name_of_start = ret.substr(pos+1);
					cout<<"Name of the state machine: "<<name_of_machine<<"\n";
					cout<<"Name of the first state: "<<name_of_start<<"\n";
				}
			}
			else
			{
				ret = find_states(cRecDecl, line);	
				if(!ret.empty())
				{	
					cout << "New state: " << namedDecl->getNameAsString() << "\n";
					states.push_back(ret);			
					methods_in_class(decl,namedDecl->getNameAsString());
				}
			}
		}
	}

	void methods_in_class(const Decl *decl, const string state)
	{
		string output, line, ret, trans, event;	
		llvm::raw_string_ostream x(output);
		int pos, num;
		const TagDecl *tagDecl = dyn_cast<TagDecl>(decl);
		const DeclContext *declCont = tagDecl->castToDeclContext(tagDecl);		
		//states.push_back(namedDecl->getNameAsString());
		
		output="";
		for (DeclContext::decl_iterator i = declCont->decls_begin(), e = declCont->decls_end(); i != e; ++i) 
		{
			if (i->getKind()==26) 
			{
				i->print(x);
				output = x.str();
				line = clean_spaces(cut_type(output));		
				ret = find_transitions(state,line);
				if(!ret.empty()) 
				{
					num = count(ret,';')+1;
					for(int i = 0;i<num;i++)
					{
						pos = ret.find(";");
						if(pos == 0)
						{
							ret = ret.substr(1);
							pos = ret.find(";");
							if(pos==-1) cReactions.push_back(ret);
							else cReactions.push_back(ret.substr(0,pos));	
							num-=1;
						}
						else 
						{
							if(pos==-1) transitions.push_back(ret);
							else transitions.push_back(ret.substr(0,pos));
						}
						//cout<<ret<<"\n";
						if(i!=num-1) ret = ret.substr(pos+1);
					}
					output="";
				}
			}
			if(i->getKind()==35) method_decl(decl);
		}
	}

	void method_decl(const Decl *decl)
	{
		string output, line, event;	
		llvm::raw_string_ostream x(output);
		if(decl->hasBody())
		{
			decl->print(x);
			line = get_return(x.str());
			if(test_model(line,"result"))
			{
				const FunctionDecl *fDecl = dyn_cast<FunctionDecl>(decl);
				const ParmVarDecl *pvd = fDecl->getParamDecl(0);
				QualType qt = pvd->getOriginalType(); 				
				event = qt.getAsString();
				if(event[event.length()-1]=='&') event = event.substr(0,event.length()-2);
				event = event.substr(event.rfind(" ")+1);
				line = dyn_cast<NamedDecl>(decl)->getQualifiedNameAsString();
				line = cut_namespaces(line.substr(0,line.rfind("::")));
				line.append(",");
				line.append(event);
				find_return_stmt(decl->getBody(),line); 
				for(list<string>::iterator i = cReactions.begin();i!=cReactions.end();i++)
				{
					event = *i;
					if(line.compare(event)==0) 
					{
						cReactions.erase(i);
						break;
					}
				}
			}
		}
	}

	void find_return_stmt(Stmt *statemt,string event)
	{
		if(statemt->getStmtClass() == 99) test_stmt(dyn_cast<CaseStmt>(statemt)->getSubStmt(), event);
		else
		{
			for (Stmt::child_range range = statemt->children(); range; ++range)    
			{
				test_stmt(*range, event);
			}
		}
	}
	
	void test_stmt(Stmt *stmt, string event)
	{
		const SourceManager &sman = fsloc->getManager();
		int type;
		string line, param;
		type = stmt->getStmtClass();
		switch(type)
		{	
			case 8 :		find_return_stmt(dyn_cast<DoStmt>(stmt)->getBody(), event); // do
							break;
			case 86 :	find_return_stmt(dyn_cast<ForStmt>(stmt)->getBody(), event); // for
							break;
			case 88 :   find_return_stmt(dyn_cast<IfStmt>(stmt)->getThen(), event); //if then
							find_return_stmt(dyn_cast<IfStmt>(stmt)->getElse(), event); //if else
							break;
			case 90 :	find_return_stmt(dyn_cast<LabelStmt>(stmt)->getSubStmt(), event); //label
							break;
			case 98 :	line = sman.getCharacterData(dyn_cast<ReturnStmt>(stmt)->getReturnLoc()); 
							line = get_line_of_code(line).substr(6);
							line = line.substr(0,line.find("("));
							if(test_model(line,"transit"))
							{
								param = get_params(line);
								transitions.push_back(event.append(",").append(param));
							}
							break;
			case 99 :  	find_return_stmt(stmt, event);
							break;
			case 101 : 	find_return_stmt(dyn_cast<SwitchStmt>(stmt)->getBody(), event); // switch
							break;
			case 102 : 	find_return_stmt(dyn_cast<WhileStmt>(stmt)->getBody(), event); // while
							break;
			}
	}

	void save_to_file(string output) // save all to the output file
	{
		nbrStates = states.size();
		string state, str, context, ctx;
		int pos1, pos2, cnt, subs;
		ofstream filestr(output.c_str());
		//std::cout<<output<<"\n";
		filestr<<"digraph "<< name_of_machine<< " {\n";
		context = name_of_machine;
		for(list<string>::iterator i = states.begin();i!=states.end();i++) // write all states in the context of the automaton
		{
			state = *i;
			cnt = count(state,',');
			if(cnt==1)
			{
				pos1 = state.find(",");
				ctx = cut_namespaces(state.substr(pos1+1));
				//std::cout<<name_of_machine.length();				
				if(ctx.compare(0,context.length(),context)==0)
				{
					filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
					states.erase(i);
					i--;
				}
			}
			if(cnt==2)
			{
				pos1 = state.find(",");
				pos2 = state.rfind(",");
				ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
				//std::cout<<ctx<<" "<<context<<"\n";
				if(ctx.compare(0,context.length(),context)==0)
				{				
					filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
				}
			}
		}
		filestr<<name_of_start<<" [peripheries=2] ;\n";
		subs = 0;
		while(!states.empty()) // substates ?
		{
			state = states.front();
			filestr<<"subgraph cluster"<<subs<<" {\n";			
			pos1 = state.find(",");
			pos2 = state.rfind(",");
			context = cut_namespaces(state.substr(0,pos1));
			filestr<<"label=\""<<context<<"\";\n";
			filestr<<cut_namespaces(state.substr(pos2+1))<<" [peripheries=2] ;\n";	
			states.pop_front();	
			//std::cout<<states.size();	
			for(list<string>::iterator i = states.begin();i!=states.end();i++)
			{
				state = *i;
				cnt = count(state,',');
				//std::cout<<state<<" \n";
				if(cnt==1)
				{
					pos1 = state.find(",");
					ctx = cut_namespaces(state.substr(pos1+1));
					if(ctx.compare(0,context.length(),context)==0)
					{
						filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
						states.erase(i);
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
		for(list<string>::iterator i = transitions.begin();i!=transitions.end();i++) // write all transitions
		{
			state = *i;
			pos1 = state.find(",");
			filestr<<cut_namespaces(state.substr(0,pos1))<<"->";
			pos2 = state.rfind(",");
			filestr<<cut_namespaces(state.substr(pos2+1));
			filestr<<"[label=\""<<cut_namespaces(state.substr(pos1+1,pos2-pos1-1))<<"\"];\n";
		}		
		filestr<<"}";
		filestr.close();
	}
	void print_stats() // print statistics
	{
		cout<<"\n"<<"Statistics: \n";
		cout<<"Number of states: "<<nbrStates<<"\n";
		cout<<"Number of events: "<<events.size()<<"\n";
		cout<<"Number of transitions: "<<transitions.size()<<"\n";
		return;
	}

};

int main(int argc, char **argv)
{ 
	string inputFilename = "";
	string outputFilename = "graph.dot"; // initialize output Filename
	MyDiagnosticClient *mdc = new MyDiagnosticClient(llvm::errs(), * new DiagnosticOptions());
	llvm::IntrusiveRefCntPtr<DiagnosticIDs> dis(new DiagnosticIDs());	
	Diagnostic diag(dis,mdc);
	FileManager fm( * new FileSystemOptions());
	SourceManager sm (diag, fm);
	HeaderSearch *headers = new HeaderSearch(fm);
	
	Driver TheDriver(LLVM_PREFIX "/bin", llvm::sys::getHostTriple(), "", false, false, diag);
	TheDriver.setCheckInputsExist(true);
	TheDriver.CCCIsCXX = 1;	
	CompilerInvocation compInv;
	llvm::SmallVector<const char *, 16> Args(argv, argv + argc);
	llvm::OwningPtr<Compilation> C(TheDriver.BuildCompilation(Args.size(),
                                                            Args.data()));
	const driver::JobList &Jobs = C->getJobs();
	const driver::Command *Cmd = cast<driver::Command>(*Jobs.begin());
	const driver::ArgStringList &CCArgs = Cmd->getArguments();
	for(unsigned i = 0; i<Args.size();i++) // find -o in ArgStringList
	{	
		if(strncmp(Args[i],"-o",2)==0) 
		{
			if(strlen(Args[i])>2)
			{
				string str = Args[i];
				outputFilename = str.substr(2);
			}
			else outputFilename = Args[i+1];
			break;
		}
	}
		
	CompilerInvocation::CreateFromArgs(compInv,
	                                  const_cast<const char **>(CCArgs.data()),
	                                  const_cast<const char **>(CCArgs.data())+CCArgs.size(),
	                                  diag);

	HeaderSearchOptions hsopts = compInv.getHeaderSearchOpts();
	hsopts.ResourceDir = LLVM_PREFIX "/lib/clang/" CLANG_VERSION_STRING;
	LangOptions lang = compInv.getLangOpts();
	CompilerInvocation::setLangDefaults(lang, IK_ObjCXX);
	TargetInfo *ti = TargetInfo::CreateTargetInfo(diag, compInv.getTargetOpts());
	ApplyHeaderSearchOptions(*headers, hsopts, lang, ti->getTriple());
	FrontendOptions f = compInv.getFrontendOpts();
	inputFilename = f.Inputs[0].second;

	cout<<"Input filename: "<<inputFilename<<"\n"; // print Input filename
	cout<<"Output filename: "<<outputFilename<<"\n"; // print Output filename


	Preprocessor pp(diag, lang, *ti, sm, *headers);
	pp.getBuiltinInfo().InitializeBuiltins(pp.getIdentifierTable(), lang);
		
	InitializePreprocessor(pp, compInv.getPreprocessorOpts(),hsopts,f);
	
	const FileEntry *file = fm.getFile(inputFilename);
	sm.createMainFileID(file);
	IdentifierTable tab(lang);
	Builtin::Context builtins(*ti);
	FindStates c;
	ASTContext ctx(lang, sm, *ti, tab, * new SelectorTable(), builtins,0);
	mdc->BeginSourceFile(lang, &pp);//start using diagnostic
	ParseAST(pp, &c, ctx, false, false);
	mdc->EndSourceFile(); //end using diagnostic
	if(c.states.size()>0) c.save_to_file(outputFilename);
	else cout<<"No state machine was found\n";
	c.print_stats();
	return 0;
}
