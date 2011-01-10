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


//using namespace clang;
class FindStates : public clang::ASTConsumer
{
	
	public:
	virtual void HandleTopLevelDecl(clang::DeclGroupRef DGR)
	{
	for (clang::DeclGroupRef::iterator i = DGR.begin(), e = DGR.end(); i != e; ++i) 
	{
		const clang::Decl *D = *i;
		clang::Stmt *x = D->getBody();
		const clang::NamedDecl *ND = dyn_cast<clang::NamedDecl>(D);
		//SourceLocation sl = D->getLocation();
		if (const clang::TagDecl *TD = dyn_cast<clang::TagDecl>(D))
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
       clang::DiagnosticOptions diagnosticOptions;
       clang::TextDiagnosticPrinter *tdp = new clang::TextDiagnosticPrinter(llvm::nulls(), diagnosticOptions);
        llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> dis(new clang::DiagnosticIDs());
       clang::Diagnostic diag(dis,tdp);
       clang::LangOptions lang;
       lang.BCPLComment=1;
       lang.CPlusPlus=1; 
       clang::FileSystemOptions fileSysOpt;
       clang::FileManager fm (fileSysOpt);

       clang::SourceManager sm ( diag, fm);
       clang::HeaderSearch *headers = new clang::HeaderSearch(fm);
       clang::CompilerInvocation::setLangDefaults(lang, clang::IK_ObjCXX);

       clang::HeaderSearchOptions hsopts;
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
       clang::TargetOptions to;
       to.Triple = llvm::sys::getHostTriple();
       clang::TargetInfo *ti = clang::TargetInfo::CreateTargetInfo(diag, to);
       clang::ApplyHeaderSearchOptions(
        *headers,
        hsopts,
        lang,
        ti->getTriple());


       clang::Preprocessor pp(diag, lang, *ti, sm, *headers);
       clang::FrontendOptions f;
       clang::PreprocessorOptions ppio;
       clang::InitializePreprocessor(pp, ppio,hsopts,f);
       const clang::FileEntry *file = fm.getFile("test.cpp");
       sm.createMainFileID(file);
       //pp.EnterMainSourceFile();
       clang::IdentifierTable tab(lang);
       clang::SelectorTable sel;
       clang::Builtin::Context builtins(*ti);
       FindStates c;
       clang::ASTContext ctx(lang, sm, *ti, tab, sel, builtins,0);
       tdp->BeginSourceFile(lang, &pp);
       clang::ParseAST(pp, &c, ctx, false, false);
	tdp->EndSourceFile();

       return 0;


}
