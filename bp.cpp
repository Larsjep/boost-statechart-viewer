#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "llvm/System/Host.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/FileSystemOptions.h"

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


using namespace clang;

class FindStates : public ASTConsumer
{
	public:
	virtual void HandleTopLevelDecl(DeclGroupRef DGR)
	{
	for (DeclGroupRef::iterator i = DGR.begin(), e = DGR.end(); i != e; ++i) 
	{
	      	const Decl *D = *i;
		const NamedDecl *ND = dyn_cast<NamedDecl>(D);
		if (const TagDecl *TD = dyn_cast<TagDecl>(D))
          	if(TD->isStruct()) //je to struktura?
                        llvm::errs() << "Novy stav: " << ND->getNameAsString() <<"\n";
	}
	/* TODO
	Zjistit, zda je dedena?
	Pokud ANO, tak projit strom dolu 
	*/
        }
};

int main(int argc, char *argv[])
{
       DiagnosticOptions diagnosticOptions;
       TextDiagnosticPrinter *tdp = new TextDiagnosticPrinter(llvm::outs(), diagnosticOptions);
       DiagnosticClient *dc;
       llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> dis;
       Diagnostic diag(dis,dc);
       LangOptions lang;

       FileSystemOptions fileSysOpt;
       FileManager fm (fileSysOpt);

       SourceManager sm ( diag, fm);
       HeaderSearch *headers = new HeaderSearch(fm);
       
       HeaderSearchOptions hsopts;
       TargetOptions to;

       //init.AddDefaultSystemIncludePaths(lang);
       //init.Realize();
       TargetInfo *ti = TargetInfo::CreateTargetInfo(diag, to);
       Preprocessor *pp = new Preprocessor(diag, lang, *ti, sm, *headers);
       FrontendOptions f;
       PreprocessorOptions ppio;
       InitializePreprocessor(*pp, ppio,hsopts,f);

       const FileEntry *file = fm.getFile("test.cpp");
       sm.createMainFileID(file);

       IdentifierTable tab(lang);
       SelectorTable sel;
       Builtin::Context builtins(*ti);
       FindStates c;
       ASTContext ctx(lang, sm, *ti, tab, sel, builtins,0);
       ParseAST(*pp, &c, ctx, false, true);

       return 0;


}
