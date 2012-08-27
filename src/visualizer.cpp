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

//standard header files
#include <fstream>

//LLVM Header files
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"

//clang header files
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;
using namespace std;

class Statechart
{
public:
    class Transition
    {
    public:
	string src, dst, event;
	Transition(string src, string dst, string event) : src(src), dst(dst), event(event) {}
    };
    string name;
    string name_of_start;
    list<Transition> transitions;
    list<string> cReactions; /** list of custom reactions. After all files are traversed this list should be empty. */
    list<string> events;
    list<string> states;

    void write_dot_file(string fn)
    {
	ofstream f(fn.c_str());
	f << "digraph " << name << " {\n";
	for (string& s : states) {
	    f << "  " << s << "\n";
	}

	for (Transition &t : transitions) {
	    f << t.src << " -> " << t.dst << " [label = \"" << t.event << "\"]\n";
	}

	f << "}";
    }
};


class MyCXXRecordDecl : public CXXRecordDecl
{
    static bool FindBaseClassString(const CXXBaseSpecifier *Specifier,
				    CXXBasePath &Path,
				    void *qualName)
    {
	string qn(static_cast<const char*>(qualName));
	const RecordType *rt = Specifier->getType()->getAs<RecordType>();
	assert(rt);
	TagDecl *canon = rt->getDecl()->getCanonicalDecl();
	return canon->getQualifiedNameAsString() == qn;
    }

public:
    bool isDerivedFrom(const char *baseStr) const {
	CXXBasePaths Paths(/*FindAmbiguities=*/false, /*RecordPaths=*/false, /*DetectVirtual=*/false);
	Paths.setOrigin(const_cast<MyCXXRecordDecl*>(this));
	return lookupInBases(&FindBaseClassString, const_cast<char*>(baseStr), Paths);
    }
};


class Visitor : public RecursiveASTVisitor<Visitor>
{
    ASTContext *Context;
    Statechart &sc;
    DiagnosticsEngine &Diags;
    unsigned diag_unhandled_reaction_type, diag_unhandled_reaction_decl,
	diag_found_state, diag_found_statemachine;

public:
    bool shouldVisitTemplateInstantiations() const { return true; }

    explicit Visitor(ASTContext *Context, Statechart &sc, DiagnosticsEngine &Diags)
	: Context(Context), sc(sc), Diags(Diags)
    {
	diag_found_statemachine =
	    Diags.getCustomDiagID(DiagnosticsEngine::Note, "Found statemachine '%0'");
	diag_found_state =
	    Diags.getCustomDiagID(DiagnosticsEngine::Note, "Found state '%0'");
	diag_unhandled_reaction_type =
	    Diags.getCustomDiagID(DiagnosticsEngine::Error, "Unhandled reaction type '%0'");
	diag_unhandled_reaction_decl =
	    Diags.getCustomDiagID(DiagnosticsEngine::Error, "Unhandled reaction decl '%0'");
    }

    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) { return Diags.Report(Loc, DiagID); }

    void HandleReaction(const Type *T, const SourceLocation Loc, CXXRecordDecl *SrcState)
    {
	if (const ElaboratedType *ET = dyn_cast<ElaboratedType>(T))
	    HandleReaction(ET->getNamedType().getTypePtr(), Loc, SrcState);
	else if (const TemplateSpecializationType *TST = dyn_cast<TemplateSpecializationType>(T)) {
	    string name = TST->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString();
	    if (name == "boost::statechart::transition") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		const Type *DstStateType = TST->getArg(1).getAsType().getTypePtr();
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();
		CXXRecordDecl *DstState = DstStateType->getAsCXXRecordDecl();

		sc.transitions.push_back(Statechart::Transition(SrcState->getName(), DstState->getName(),
								Event->getName()));
	    } else if (name == "boost::mpl::list") {
		for (TemplateSpecializationType::iterator Arg = TST->begin(), End = TST->end(); Arg != End; ++Arg)
		    HandleReaction(Arg->getAsType().getTypePtr(), Loc, SrcState);
	    }
	    //->getDecl()->getQualifiedNameAsString();
	} else
	    Diag(Loc, diag_unhandled_reaction_type) << T->getTypeClassName();
    }

    void HandleReaction(const NamedDecl *Decl, CXXRecordDecl *SrcState)
    {
	if (const TypedefDecl *r = dyn_cast<TypedefDecl>(Decl))
	    HandleReaction(r->getCanonicalDecl()->getUnderlyingType().getTypePtr(),
			   r->getLocStart(), SrcState);
	else
	    Diag(Decl->getLocation(), diag_unhandled_reaction_decl) << Decl->getDeclKindName();
    }


    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
	if (!Declaration->isCompleteDefinition())
	    return true;

	MyCXXRecordDecl *RecordDecl = static_cast<MyCXXRecordDecl*>(Declaration);

	if (RecordDecl->isDerivedFrom("boost::statechart::simple_state"))
	{
	    string state(RecordDecl->getName()); //getQualifiedNameAsString());
	    Diag(RecordDecl->getLocStart(), diag_found_state) << state;
	    sc.states.push_back(state);

	    IdentifierInfo& II = Context->Idents.get("reactions");
	    for (DeclContext::lookup_result Reactions = RecordDecl->lookup(DeclarationName(&II));
		 Reactions.first != Reactions.second; ++Reactions.first)
		HandleReaction(*Reactions.first, RecordDecl);
	}
	else if (RecordDecl->isDerivedFrom("boost::statechart::state_machine"))
	{
	    sc.name = RecordDecl->getQualifiedNameAsString();
	    sc.name_of_start = "tmp"; //RecordDecl->getStateMachineInitialStateAsString()
	    Diag(RecordDecl->getLocStart(), diag_found_statemachine) << sc.name;
	}
	else if (RecordDecl->isDerivedFrom("boost::statechart::event"))
	{
	    sc.events.push_back(RecordDecl->getNameAsString());
	}
	return true;
    }
};


class VisualizeStatechartConsumer : public clang::ASTConsumer
{
    Statechart statechart;
    Visitor visitor;
    string destFileName;
public:
    explicit VisualizeStatechartConsumer(ASTContext *Context, std::string destFileName,
					 DiagnosticsEngine &D)
	: visitor(Context, statechart, D), destFileName(destFileName) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
	visitor.TraverseDecl(Context.getTranslationUnitDecl());
	statechart.write_dot_file(destFileName);
    }
};

class VisualizeStatechartAction : public PluginASTAction
{
protected:
  ASTConsumer *CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) {
    size_t dot = getCurrentFile().find_last_of('.');
    std::string dest = getCurrentFile().substr(0, dot);
    dest.append(".dot");
    return new VisualizeStatechartConsumer(&CI.getASTContext(), dest, CI.getDiagnostics());
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string>& args) {
    for (unsigned i = 0, e = args.size(); i != e; ++i) {
      llvm::errs() << "Visualizer arg = " << args[i] << "\n";

      // Example error handling.
      if (args[i] == "-an-error") {
        DiagnosticsEngine &D = CI.getDiagnostics();
        unsigned DiagID = D.getCustomDiagID(
          DiagnosticsEngine::Error, "invalid argument '" + args[i] + "'");
        D.Report(DiagID);
        return false;
      }
    }
    if (args.size() && args[0] == "help")
      PrintHelp(llvm::errs());

    return true;
  }
  void PrintHelp(llvm::raw_ostream& ros) {
    ros << "Help for Visualize Statechart plugin goes here\n";
  }

};

static FrontendPluginRegistry::Add<VisualizeStatechartAction> X("visualize-statechart", "visualize statechart");

// Local Variables:
// c-basic-offset: 4
// End:
