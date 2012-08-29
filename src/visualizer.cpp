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
#include <iomanip>
#include <fstream>
#include <map>

//LLVM Header files
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"

//clang header files
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;
using namespace std;

namespace Model
{

    inline int getIndentLevelIdx() {
	static int i = ios_base::xalloc();
	return i;
    }

    ostream& indent(ostream& os) { os << setw(2*os.iword(getIndentLevelIdx())) << ""; return os; }
    ostream& indent_inc(ostream& os) { os.iword(getIndentLevelIdx())++; return os; }
    ostream& indent_dec(ostream& os) { os.iword(getIndentLevelIdx())--; return os; }

    class State;

    class Context : public map<string, State*> {
    public:
	iterator add(State *state);
	Context *findContext(const string &name);
    };

    class State : public Context
    {
	string initialInnerState;
    public:
	const string name;
	explicit State(string name) : name(name) {}
	void setInitialInnerState(string name) { initialInnerState = name; }
	friend ostream& operator<<(ostream& os, const State& s);
    };


    Context::iterator Context::add(State *state)
    {
	pair<iterator, bool> ret =  insert(value_type(state->name, state));
	return ret.first;
    }

    Context *Context::findContext(const string &name)
    {
	iterator i = find(name), e;
	if (i != end())
	    return i->second;
	for (i = begin(), e = end(); i != e; ++i) {
	    Context *c = i->second->findContext(name);
	    if (c)
		return c;
	}
	return 0;
    }


    ostream& operator<<(ostream& os, const Context& c);

    ostream& operator<<(ostream& os, const State& s)
    {
	if (s.size()) {
	    os << indent << "subgraph cluster_" << s.name << " {\n" << indent_inc;
	    os << indent << "label = \"" << s.name << "\"\n";
	}
	os << indent << "" << s.name << "\n";
	if (s.size()) {
	    os << indent << s.initialInnerState << " [peripheries=2]\n";
	    os << static_cast<Context>(s);
	    os << indent_dec << indent << "}\n";
	}
	return os;
    }


    ostream& operator<<(ostream& os, const Context& c)
    {
	for (Context::const_iterator i = c.begin(), e = c.end(); i != e; i++) {
	    os << *i->second;
	}
	return os;
    }


    class Transition
    {
    public:
	const string src, dst, event;
	Transition(string src, string dst, string event) : src(src), dst(dst), event(event) {}
    };

    ostream& operator<<(ostream& os, const Transition& t)
    {
	os << indent << t.src << " -> " << t.dst << " [label = \"" << t.event << "\"]\n";
	return os;
    }


    class Machine : public Context
    {
    protected:
	string initial_state;
    public:
	const string name;
	explicit Machine(string name) : name(name) {}

	void setInitialState(string name) { initial_state = name; }

	friend ostream& operator<<(ostream& os, const Machine& m);
    };

    ostream& operator<<(ostream& os, const Machine& m)
    {
	os << indent << "subgraph " << m.name << " {\n" << indent_inc;
	os << indent << m.initial_state << " [peripheries=2]\n";
	os << static_cast<Context>(m);
	os << indent_dec << indent << "}\n";
	return os;
    }


    class Model : public map<string, Machine>
    {
	Context unknown;	// For forward-declared state classes
    public:
	list< Transition*> transitions;

	iterator add(const Machine &m)
	{
	    pair<iterator, bool> ret =  insert(value_type(m.name, m));
	    return ret.first;
	}

	void addUnknownState(State *m)
	{
	    unknown[m->name] = m;
	}


	Context *findContext(const string &name)
	{
	    Context::iterator ci = unknown.find(name);
	    if (ci != unknown.end())
		return ci->second;
	    iterator i = find(name), e;
	    if (i != end())
		return &i->second;
	    for (i = begin(), e = end(); i != e; ++i) {
		Context *c = i->second.findContext(name);
		if (c)
		    return c;
	    }
	    return 0;
	}

	State *removeFromUnknownContexts(const string &name)
	{
	    Context::iterator ci = unknown.find(name);
	    if (ci == unknown.end())
		return 0;
	    unknown.erase(ci);
	    return ci->second;
	}

	void write_as_dot_file(string fn)
	{
	    ofstream f(fn.c_str());
	    f << "digraph statecharts {\n" << indent_inc;
	    for (iterator i = begin(), e = end(); i != e; i++)
		f << i->second;
	    for (list<Transition*>::iterator t = transitions.begin(), e = transitions.end(); t != e; ++t)
		f << **t;
	    f << indent_dec << "}\n";
	}
    };
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
    bool isDerivedFrom(const char *baseStr, CXXBaseSpecifier const **Base = 0) const {
	CXXBasePaths Paths(/*FindAmbiguities=*/false, /*RecordPaths=*/!!Base, /*DetectVirtual=*/false);
	Paths.setOrigin(const_cast<MyCXXRecordDecl*>(this));
	if (!lookupInBases(&FindBaseClassString, const_cast<char*>(baseStr), Paths))
	    return false;
	if (Base)
	    *Base = Paths.front().back().Base;
	return true;
    }
};


class Visitor : public RecursiveASTVisitor<Visitor>
{
    ASTContext *ASTCtx;
    Model::Model &model;
    DiagnosticsEngine &Diags;
    unsigned diag_unhandled_reaction_type, diag_unhandled_reaction_decl,
	diag_found_state, diag_found_statemachine;

public:
    bool shouldVisitTemplateInstantiations() const { return true; }

    explicit Visitor(ASTContext *Context, Model::Model &model, DiagnosticsEngine &Diags)
	: ASTCtx(Context), model(model), Diags(Diags)
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
	// TODO: Improve Loc tracking
	if (const ElaboratedType *ET = dyn_cast<ElaboratedType>(T))
	    HandleReaction(ET->getNamedType().getTypePtr(), Loc, SrcState);
	else if (const TemplateSpecializationType *TST = dyn_cast<TemplateSpecializationType>(T)) {
	    string name = TST->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString();
	    if (name == "boost::statechart::transition") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		const Type *DstStateType = TST->getArg(1).getAsType().getTypePtr();
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();
		CXXRecordDecl *DstState = DstStateType->getAsCXXRecordDecl();

		Model::Transition *T = new Model::Transition(SrcState->getName(), DstState->getName(), Event->getName());
		model.transitions.push_back(T);
	    } else if (name == "boost::mpl::list") {
		for (TemplateSpecializationType::iterator Arg = TST->begin(), End = TST->end(); Arg != End; ++Arg)
		    HandleReaction(Arg->getAsType().getTypePtr(), Loc, SrcState);
	    } else
		Diag(Loc, diag_unhandled_reaction_type) << name;
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

    CXXRecordDecl *getTemplateArgDecl(const Type *T, unsigned ArgNum)
    {
	if (const ElaboratedType *ET = dyn_cast<ElaboratedType>(T))
	    return getTemplateArgDecl(ET->getNamedType().getTypePtr(), ArgNum);
	else if (const TemplateSpecializationType *TST = dyn_cast<TemplateSpecializationType>(T)) {
	    if (TST->getNumArgs() >= ArgNum+1)
		return TST->getArg(ArgNum).getAsType()->getAsCXXRecordDecl();
	}
	return 0;
    }


    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
	if (!Declaration->isCompleteDefinition())
	    return true;

	MyCXXRecordDecl *RecordDecl = static_cast<MyCXXRecordDecl*>(Declaration);
	const CXXBaseSpecifier *Base;

	if (RecordDecl->isDerivedFrom("boost::statechart::simple_state", &Base))
	{
	    string name(RecordDecl->getName()); //getQualifiedNameAsString());
	    Diag(RecordDecl->getLocStart(), diag_found_state) << name;

	    Model::State *state;
	    // Either we saw a reference to forward declared state
	    // before, or we create a new state.
	    if (!(state = model.removeFromUnknownContexts(name)))
		state = new Model::State(name);

	    CXXRecordDecl *Context = getTemplateArgDecl(Base->getType().getTypePtr(), 1);
	    Model::Context *c = model.findContext(Context->getName());
	    if (!c) {
		Model::State *s = new Model::State(Context->getName());
		model.addUnknownState(s);
		c = s;
	    }
	    c->add(state);

	    if (CXXRecordDecl *InnerInitialState = getTemplateArgDecl(Base->getType().getTypePtr(), 2))
		state->setInitialInnerState(InnerInitialState->getName());

	    IdentifierInfo& II = ASTCtx->Idents.get("reactions");
	    // TODO: Lookup for reactions even in base classes - probably by using Sema::LookupQualifiedName()
	    for (DeclContext::lookup_result Reactions = RecordDecl->lookup(DeclarationName(&II));
		 Reactions.first != Reactions.second; ++Reactions.first)
		HandleReaction(*Reactions.first, RecordDecl);
	}
	else if (RecordDecl->isDerivedFrom("boost::statechart::state_machine", &Base))
	{
	    Model::Machine m(RecordDecl->getName());
	    Diag(RecordDecl->getLocStart(), diag_found_statemachine) << m.name;

	    if (CXXRecordDecl *InitialState = getTemplateArgDecl(Base->getType().getTypePtr(), 1))
		m.setInitialState(InitialState->getName());
	    model.add(m);
	}
	else if (RecordDecl->isDerivedFrom("boost::statechart::event"))
	{
	    //sc.events.push_back(RecordDecl->getNameAsString());
	}
	return true;
    }
};


class VisualizeStatechartConsumer : public clang::ASTConsumer
{
    Model::Model model;
    Visitor visitor;
    string destFileName;
public:
    explicit VisualizeStatechartConsumer(ASTContext *Context, std::string destFileName,
					 DiagnosticsEngine &D)
	: visitor(Context, model, D), destFileName(destFileName) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
	visitor.TraverseDecl(Context.getTranslationUnitDecl());
	model.write_as_dot_file(destFileName);
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
