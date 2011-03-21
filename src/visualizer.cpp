#include <iostream>
#include <string>
#include <fstream>
#include <list>


#include "llvm/Support/raw_ostream.h"
#include "llvm/System/Host.h"
#include "llvm/Config/config.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/FileSystemOptions.h"

#include "clang/Index/TranslationUnit.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Basic/FileManager.h"

#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/Utils.h"

#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/FrontendOptions.h"

#include "clang/Frontend/CompilerInvocation.h"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/Builtins.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Type.h"
#include "clang/AST/Decl.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Ownership.h"
#include "clang/AST/DeclGroup.h"

#include "clang/Parse/Parser.h"

#include "clang/Parse/ParseAST.h"
#include "clang/Basic/Version.h"

#include "llvm/Support/CommandLine.h"

//my own header files
#include "stringoper.h"
#include "commandlineopt.h"

using namespace clang;


class MyDiagnosticClient : public TextDiagnosticPrinter
{
	public:
	MyDiagnosticClient(llvm::raw_ostream &os, const DiagnosticOptions &diags, bool OwnsOutputStream = false):TextDiagnosticPrinter(os, diags, OwnsOutputStream = false){}
	virtual void HandleDiagnostic(Diagnostic::Level DiagLevel, const DiagnosticInfo &Info)
	{
		TextDiagnosticPrinter::HandleDiagnostic(DiagLevel, Info);
		if(DiagLevel == 3) exit(1);
	}
};

class FindStates : public ASTConsumer
{
	std::list<string> transitions;
	std::list<string> states;
	std::string name_of_machine;
	std::string name_of_start;
	FullSourceLoc *fSloc;
	public:

	virtual void Initialize(ASTContext &ctx)//run after the AST is constructed
	{	
		SourceLocation loc;
		name_of_start = "";
		name_of_machine = "";
		SourceManager &sman = ctx.getSourceManager();
		fSloc = new FullSourceLoc(loc, sman);
	}

	virtual void HandleTopLevelDecl(DeclGroupRef DGR)// traverse all top level declarations
	{
		const SourceManager &sman = fSloc->getManager();
		SourceLocation loc;
		std::string line;
		std::string super_class, output;
		llvm::raw_string_ostream x(output);
		for (DeclGroupRef::iterator i = DGR.begin(), e = DGR.end(); i != e; ++i) 
		{
			const Decl *decl = *i;
			loc = decl->getLocation();
			if(loc.isValid())
			{
				const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);
				//std::cout<<decl->getDeclKindName()<<"\n";
				if (const TagDecl *tagDecl = dyn_cast<TagDecl>(decl))
				{
					if(tagDecl->isStruct() || tagDecl->isClass()) //is it a structure or class	
					{
						const CXXRecordDecl *cRecDecl = dyn_cast<CXXRecordDecl>(decl);
						decl->print(x);
						//decl->dump();							
						line = cut_commentary(clean_spaces(get_line_of_code(x.str())));
						output = "";
						if(is_derived(line))
						{
							if(name_of_machine == "")
							{
								find_name_of_machine(cRecDecl, line);
							}
							else
							{
								if(find_states(cRecDecl, line))
								{				
									const DeclContext *declCont = tagDecl->castToDeclContext(tagDecl);					
									std::cout << "New state: " << namedDecl->getNameAsString() << "\n";
									find_transitions(namedDecl->getNameAsString(), declCont);
								}
							}
						}
					}
				}	
				if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
				{
					DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
					//declCont->dumpDeclContext();				
					recursive_visit(declCont);
				
				}
			}
		}
	}
	void recursive_visit(const DeclContext *declCont) //recursively visit all decls hidden inside namespaces
	{
		const SourceManager &sman = fSloc->getManager();
		std::string line, output;
		SourceLocation loc;
		llvm::raw_string_ostream x(output);
		for (DeclContext::decl_iterator i = declCont->decls_begin(), e = declCont->decls_end(); i != e; ++i)
		{
			const Decl *decl = *i;
			const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);
			
			//std::cout<<"a "<<decl->getDeclKindName()<<"\n";
			loc = decl->getLocation();
			if(loc.isValid())
			{			
				if (const TagDecl *tagDecl = dyn_cast<TagDecl>(decl))
				{
					if(tagDecl->isStruct() || tagDecl->isClass()) //is it a structure or class	
					{
						const CXXRecordDecl *cRecDecl = dyn_cast<CXXRecordDecl>(decl);
						decl->print(x);
						line = cut_commentary(clean_spaces(get_line_of_code(x.str())));
						output = "";
						if(is_derived(line))
						{
							if(name_of_machine == "")
							{
								find_name_of_machine(cRecDecl, line);
							}
							else
							{
								if(find_states(cRecDecl, line))
								{				
									const DeclContext *declCont = tagDecl->castToDeclContext(tagDecl);		
									//states.push_back(namedDecl->getNameAsString());
									std::cout << "New state: " << namedDecl->getNameAsString() << "\n";
									find_transitions(namedDecl->getNameAsString(), declCont);
								}
							}
						}
					}	
				}
				if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
				{
					DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
					//declCont->dumpDeclContext();				
					recursive_visit(declCont);
				}
			}
		} 
	}
	bool find_states(const CXXRecordDecl *cRecDecl, std::string line) // test if the struct/class is the state (must be derived from simple_state)
	{	
		std::string super_class = get_super_class(line), base;		
		if(cRecDecl->getNumBases()>1)
		{
			for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
			{
				if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
				else base = super_class;
				if(is_state(super_class)) 
				{
					//std::cout<<get_params(super_class);
					states.push_back(get_params(super_class));
					return true;
				}
				else
				{
					super_class = get_next_base(super_class);
				}
			}
			return false;
		}
		else
		{ 
			if(is_state(super_class)) 
			{
				//std::cout<<get_params(super_class);
				states.push_back(get_params(super_class));
				return true;
			}
			else return false;
		}
	}
		
	void find_name_of_machine(const CXXRecordDecl *cRecDecl, std::string line) // find name of the state machine and the start state
	{	
		std::string super_class = get_super_class(line), base, params;
		
		int pos = 0;
		if(cRecDecl->getNumBases()>1)
		{
			for(unsigned i = 0; i<cRecDecl->getNumBases();i++ )
			{
				if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
				else base = super_class;
				if(is_machine(base))
				{
					params = get_params(base);
					pos = params.find(",");
					name_of_machine = params.substr(0,pos);
					name_of_start = params.substr(pos);
					std::cout<<"Name of the state machine: "<<name_of_machine<<"\n";
					std::cout<<"Name of the first state: "<<name_of_start<<"\n";
				}
				else
				{
					super_class = get_next_base(super_class);
				}
			}
		}
		else
		{ 
			if(is_machine(super_class))
			{
				//std::cout<<super_class;
				params = get_params(super_class);
				//std::cout<<params;
				pos = params.find(",");
				name_of_machine = cut_namespaces(params.substr(0,pos));
				name_of_start = cut_namespaces(params.substr(pos+1));
				std::cout<<"Name of the state machine:"<<name_of_machine<<"\n";
				std::cout<<"Name of the first state:"<<name_of_start<<"\n";
			}
		}
	}

	void find_transitions (const std::string name_of_state,const DeclContext *declCont) // traverse all methods for finding declarations of transitions
	{	
		std::string output, line, dest, params, base;	
		llvm::raw_string_ostream x(output);
		int num;		
		for (DeclContext::decl_iterator i = declCont->decls_begin(), e = declCont->decls_end(); i != e; ++i) 
		{
			const Decl *decl = *i;
			if (const TypedefDecl *typedDecl = dyn_cast<TypedefDecl>(decl)) 
			{
					decl->print(x);
					output = x.str();
					line = clean_spaces(cut_typedef(output));
					num = count(output,'<');
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
						if(is_transition(base))
						{
							dest = name_of_state;
							params = get_params(base);
							dest.append(",");							
							dest.append(params);
							transitions.push_back(dest);
							line = get_next_base(line);
						}
						else
						{
							line = get_next_base(line);
						}
					}
					output = "";
			}
		}	
	}
	
	void save_to_file(std::string output)
	{
		std::string state, str, context, ctx;
		int pos1, pos2, cnt, subs;
		std::ofstream filestr(output.c_str());
		std::cout<<output<<"\n";
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
					
					//std::cout<<ctx<<" "<<context<<"\n";
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
					if(ctx.compare(0,context.length(),context)==0)
					{				
						filestr<<cut_namespaces(state.substr(0,pos1))<<";\n";
						//std::cout<<ctx<<"\n";
					}
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
};

int main(int argc, char *argv[])
{
	llvm::cl::ParseCommandLineOptions(argc, argv);	
	std::cout<<"Input file: "<<inputFilename<<"\n";	
	FILE* fileI = fopen(inputFilename.c_str(), "r");
	if (!fileI)  
	{
		perror(inputFilename.c_str());
    	exit(1);
	}
	fclose(fileI);
	DiagnosticOptions diagnosticOptions;
	llvm::IntrusiveRefCntPtr<DiagnosticIDs> dis(new DiagnosticIDs());
	MyDiagnosticClient *mdc = new MyDiagnosticClient(llvm::outs(), diagnosticOptions);
	Diagnostic diag(dis,mdc);
	diag.setIgnoreAllWarnings(true);
 	FileSystemOptions fileSysOpt;     
	LangOptions lang;
	lang.BCPLComment=1;
	lang.CPlusPlus=1; 
	FileManager fm (fileSysOpt);

	SourceManager sm ( diag, fm);
	HeaderSearch *headers = new HeaderSearch(fm);
	CompilerInvocation::setLangDefaults(lang, IK_ObjCXX);

	HeaderSearchOptions hsopts;
	hsopts.ResourceDir=LLVM_PREFIX "/lib/clang/" CLANG_VERSION_STRING;
	for(unsigned i = 0; i<includeFiles.size();i++)
	{
		hsopts.AddPath(includeFiles[i],
				clang::frontend::Angled,
				false,
				false,
				true);
	}
	TargetOptions to;
	to.Triple = llvm::sys::getHostTriple();
	TargetInfo *ti = TargetInfo::CreateTargetInfo(diag, to);
	clang::ApplyHeaderSearchOptions(
		*headers,
		hsopts,
		lang,
		ti->getTriple());
	Preprocessor pp(diag, lang, *ti, sm, *headers);
	pp.getBuiltinInfo().InitializeBuiltins(pp.getIdentifierTable(),
                                           pp.getLangOptions());
	FrontendOptions f;
	PreprocessorOptions ppio;
	InitializePreprocessor(pp, ppio,hsopts,f);
	const FileEntry *file = fm.getFile(inputFilename);
	sm.createMainFileID(file);
	IdentifierTable tab(lang);
	SelectorTable sel;
	Builtin::Context builtins(*ti);
	FindStates c;
	ASTContext ctx(lang, sm, *ti, tab, sel, builtins,0);
	mdc->BeginSourceFile(lang, &pp);
	ParseAST(pp, &c, ctx, false, false);
	mdc->EndSourceFile();
	c.save_to_file(outputFile);
	return 0;

}
