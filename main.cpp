#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "llvm/System/Host.h"
#include "llvm/Config/config.h"

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


using namespace clang;
class FindStates : public ASTConsumer
{
	
	public:
	virtual void HandleTopLevelDecl(DeclGroupRef DGR)
	{
	for (DeclGroupRef::iterator i = DGR.begin(), e = DGR.end(); i != e; ++i) 
	{
		const Decl *D = *i;
		Stmt *x = D->getBody();
		const NamedDecl *ND = dyn_cast<NamedDecl>(D);
		//SourceLocation sl = D->getLocation();
		if (const TagDecl *TD = dyn_cast<TagDecl>(D))
		if(TD->isStruct() && D->hasAttrs()) //je to struktura?
		{
			ND->print(llvm::errs())	;		
			std::cout << "Novy stav: " << ND->getNameAsString() << "   xx \n";
		}
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
       TextDiagnosticPrinter *tdp = new TextDiagnosticPrinter(llvm::nulls(), diagnosticOptions);
        llvm::IntrusiveRefCntPtr<DiagnosticIDs> dis(new DiagnosticIDs());
       Diagnostic diag(dis,tdp);
       LangOptions lang;
       lang.BCPLComment=1;
       lang.CPlusPlus=1; 
       FileSystemOptions fileSysOpt;
       FileManager fm (fileSysOpt);

       SourceManager sm ( diag, fm);
       HeaderSearch *headers = new HeaderSearch(fm);
       CompilerInvocation::setLangDefaults(lang, IK_ObjCXX);

       HeaderSearchOptions hsopts;
       hsopts.ResourceDir="/usr/local/lib/clang/2.9/";
	/*hsopts.AddPath("/usr/include/linux",
			clang::frontend::Angled,
			false,
			false,
			false);	
	hsopts.AddPath("/usr/include/c++/4.4",
			clang::frontend::Angled,
			false,
			false,
			false);
	hsopts.AddPath("/usr/include/c++/4.4/tr1",
			clang::frontend::Angled,
			false,
			false,
			false);

	hsopts.AddPath("/home/petr/Dokumenty/BOOST/boost_1_44_0",
			clang::frontend::Angled,
			false,
			false,
			false);
	hsopts.AddPath("/usr/include/c++/4.4/i686-linux-gnu",
			clang::frontend::Angled,
			false,
			false,
			false);*/
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
       //pp.EnterMainSourceFile();
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
