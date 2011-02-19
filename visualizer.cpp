#include <iostream>

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

//my own header files
#include "stringoper.h"

using namespace clang;


class FindStates : public ASTConsumer
{
	public:
	SourceLocation loc;
	FullSourceLoc *fSloc;
	
	virtual void Initialize(ASTContext &ctx)//run after the AST is constructed
	{	
		SourceManager &sman = ctx.getSourceManager();
		fSloc = new FullSourceLoc(loc, sman);
	}

	virtual void HandleTopLevelDecl(DeclGroupRef DGR)// traverse all top level declarations
	{
		SourceLocation loc;
		const SourceManager &sman = fSloc->getManager();
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
				if(!sman.isInSystemHeader(loc))
				{
					if (const TagDecl *tagDecl = dyn_cast<TagDecl>(decl))
					{
						if(tagDecl->isStruct() || tagDecl->isClass()) //is it a structure or class	
						{
							const CXXRecordDecl *cRecDecl = dyn_cast<CXXRecordDecl>(decl);
							line = cut_commentary(clean_spaces(get_line_of_code(sman.getCharacterData(loc))));
							if(is_derived(line))
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
					if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
					{
					
							DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
							//declCont->dumpDeclContext();				
							recursive_visit(declCont);
					
					}
				}
			}
		}
	}
	void recursive_visit(const DeclContext *declCont) //recursively visit all decls inside namespace
	{
		const SourceManager &sman = fSloc->getManager();
		std::string line;
		SourceLocation loc;
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
						line = cut_commentary(clean_spaces(get_line_of_code(sman.getCharacterData(loc))));
						if(is_derived(line))
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
				if(const NamespaceDecl *namespaceDecl = dyn_cast<NamespaceDecl>(decl))
				{
					DeclContext *declCont = namespaceDecl->castToDeclContext(namespaceDecl);
					//declCont->dumpDeclContext();				
					recursive_visit(declCont);
				}
			}
		} 
	}
	bool find_states(const CXXRecordDecl *cRecDecl, std::string line)
	{	
		std::string super_class = get_super_class(line), base;
		if(cRecDecl->getNumBases()>1)
		{
			for(int i = 0; i<cRecDecl->getNumBases();i++ )
			{
				if(i!=cRecDecl->getNumBases()-1) base = get_first_base(super_class);
				else base = super_class;
				if(is_state(base)) return true;
				else
				{
					super_class = get_next_base(super_class);
				}
			}
			return false;
		}
		else
		{ 
			if(is_state(super_class)) return true;
			else return false;
		}
	}

	void find_transitions (const std::string name_of_state,const DeclContext *declCont) // traverse all methods for finding declarations of transitions
	{	
		std::string output, line, event, dest, params;	
		llvm::raw_string_ostream x(output);
		int pos;		
		for (DeclContext::decl_iterator i = declCont->decls_begin(), e = declCont->decls_end(); i != e; ++i) 
		{
			const Decl *decl = *i;
			//decl->dump();
			
			if (const TypedefDecl *typeDecl = dyn_cast<TypedefDecl>(decl)) 
			{
				const NamedDecl *namedDecl = dyn_cast<NamedDecl>(decl);
				decl->print(x);
				output = x.str();
				if(count(output)==1)
				{
					line = clean_spaces(cut_typedef(output));
					params = get_transition_params(line);
					pos = params.find(",");
					event = params.substr(0,pos);
					dest = params.substr(pos+1);
					if(is_transition(line))	std::cout << "New transition: " << name_of_state<<" -> "<<event<<" -> "<< dest<<"\n";
				}
			}
		}	
	}	
};

int main(int argc, char *argv[])
{
	DiagnosticOptions diagnosticOptions;
 	TextDiagnosticPrinter *tdp = new TextDiagnosticPrinter(llvm::nulls(), diagnosticOptions);
	llvm::IntrusiveRefCntPtr<DiagnosticIDs> dis(new DiagnosticIDs());
	Diagnostic diag(dis,tdp);
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
	hsopts.AddPath("/home/petr/Dokumenty/BOOST/boost_1_44_0",
			clang::frontend::Angled,
			false,
			false,
			true);
	TargetOptions to;
	to.Triple = llvm::sys::getHostTriple();
	TargetInfo *ti = TargetInfo::CreateTargetInfo(diag, to);
	clang::ApplyHeaderSearchOptions(
	*headers,
	hsopts,
	lang,
	ti->getTriple());
	Preprocessor pp(diag, lang, *ti, sm, *headers);
	FrontendOptions f;
	PreprocessorOptions ppio;
	InitializePreprocessor(pp, ppio,hsopts,f);
	const FileEntry *file = fm.getFile("test.cpp");
	sm.createMainFileID(file);
	IdentifierTable tab(lang);
	SelectorTable sel;
	Builtin::Context builtins(*ti);
	FindStates c;
	ASTContext ctx(lang, sm, *ti, tab, sel, builtins,0);
	tdp->BeginSourceFile(lang, &pp);
	ParseAST(pp, &c, ctx, false, false);
	tdp->EndSourceFile();
	return 0;

}
