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
	list<string> defferedEvents;
    public:
	const string name;
	explicit State(string name) : name(name) {}
	void setInitialInnerState(string name) { initialInnerState = name; }
	void addDeferredEvent(const string &name) { defferedEvents.push_back(name); }
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
	string label = s.name;
	for (list<string>::const_iterator i = s.defferedEvents.begin(), e = s.defferedEvents.end(); i != e; ++i)
	    label.append("<br />").append(*i).append(" / defer");
	os << indent << s.name << " [label=<" << label << ">]\n";
	if (s.size()) {
	    os << indent << s.name << " -> " << s.initialInnerState << " [style = dashed]\n";
	    os << indent << "subgraph cluster_" << s.name << " {\n" << indent_inc;
	    os << indent << "label = \"" << s.name << "\"\n";
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
	Context undefined;	// For forward-declared state classes
    public:
	list< Transition*> transitions;

	iterator add(const Machine &m)
	{
	    pair<iterator, bool> ret =  insert(value_type(m.name, m));
	    return ret.first;
	}

	void addUndefinedState(State *m)
	{
	    undefined[m->name] = m;
	}


	Context *findContext(const string &name)
	{
	    Context::iterator ci = undefined.find(name);
	    if (ci != undefined.end())
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

	State *findState(const string &name)
	{
	    for (iterator i = begin(), e = end(); i != e; ++i) {
		Context *c = i->second.findContext(name);
		if (c)
		    return static_cast<State*>(c);
	    }
	    return 0;
	}


	State *removeFromUndefinedContexts(const string &name)
	{
	    Context::iterator ci = undefined.find(name);
	    if (ci == undefined.end())
		return 0;
	    undefined.erase(ci);
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

class FindTransitVisitor : public RecursiveASTVisitor<FindTransitVisitor>
{
    Model::Model &model;
    const CXXRecordDecl *SrcState;
    const Type *EventType;
public:
    explicit FindTransitVisitor(Model::Model &model, const CXXRecordDecl *SrcState, const Type *EventType)
	: model(model), SrcState(SrcState), EventType(EventType) {}

    bool VisitMemberExpr(MemberExpr *E) {
	if (E->getMemberNameInfo().getAsString() != "transit")
	    return true;
	if (E->hasExplicitTemplateArgs()) {
	    const Type *DstStateType = E->getExplicitTemplateArgs()[0].getArgument().getAsType().getTypePtr();
	    CXXRecordDecl *DstState = DstStateType->getAsCXXRecordDecl();
	    CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();
	    Model::Transition *T = new Model::Transition(SrcState->getName(), DstState->getName(), Event->getName());
	    model.transitions.push_back(T);
	}
	return true;
    }
};

class Visitor : public RecursiveASTVisitor<Visitor>
{
    ASTContext *ASTCtx;
    Model::Model &model;
    DiagnosticsEngine &Diags;
    unsigned diag_unhandled_reaction_type, diag_unhandled_reaction_decl,
	diag_found_state, diag_found_statemachine, diag_no_history, diag_warning;

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
	diag_unhandled_reaction_decl =
	    Diags.getCustomDiagID(DiagnosticsEngine::Error, "History is not yet supported");
	diag_warning =
	    Diags.getCustomDiagID(DiagnosticsEngine::Warning, "'%0' %1");
    }

    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) { return Diags.Report(Loc, DiagID); }

    void HandleCustomReaction(const CXXRecordDecl *SrcState, const Type *EventType)
    {
	IdentifierInfo& II = ASTCtx->Idents.get("react");
	// TODO: Lookup for react even in base classes - probably by using Sema::LookupQualifiedName()
	for (DeclContext::lookup_const_result ReactRes = SrcState->lookup(DeclarationName(&II));
	     ReactRes.first != ReactRes.second; ++ReactRes.first) {
	    if (CXXMethodDecl *React = dyn_cast<CXXMethodDecl>(*ReactRes.first))
		if (const ParmVarDecl *p = React->getParamDecl(0)) {
		    const Type *ParmType = p->getType().getTypePtr();
		    if (ParmType->isLValueReferenceType())
			ParmType = dyn_cast<LValueReferenceType>(ParmType)->getPointeeType().getTypePtr();
		    if (ParmType == EventType)
			FindTransitVisitor(model, SrcState, EventType).TraverseStmt(React->getBody());
		}
	}
    }

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
	    } else if (name == "boost::statechart::custom_reaction") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		HandleCustomReaction(SrcState, EventType);
	    } else if (name == "boost::statechart::deferral") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();

		Model::State *s = model.findState(SrcState->getName());
		assert(s);
		s->addDeferredEvent(Event->getName());
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

    CXXRecordDecl *getTemplateArgDecl(const Type *T, unsigned ArgNum, const SourceLocation Loc)
    {
	if (const ElaboratedType *ET = dyn_cast<ElaboratedType>(T))
	    return getTemplateArgDecl(ET->getNamedType().getTypePtr(), ArgNum, Loc);
	else if (const TemplateSpecializationType *TST = dyn_cast<TemplateSpecializationType>(T)) {
	    if (TST->getNumArgs() >= ArgNum+1)
		return TST->getArg(ArgNum).getAsType()->getAsCXXRecordDecl();
	} else
	    Diag(Loc, diag_warning) << T->getTypeClassName() << "type as template argument is not supported";
	return 0;
    }

    CXXRecordDecl *getTemplateArgDeclOfBase(const CXXBaseSpecifier *Base, unsigned ArgNum) {
	return getTemplateArgDecl(Base->getType().getTypePtr(), 1,
				  Base->getTypeSourceInfo()->getTypeLoc().getLocStart());
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
	if (!Declaration->isCompleteDefinition())
	    return true;
	if (Declaration->getQualifiedNameAsString() == "boost::statechart::state")
	    return true; // This is an "abstract class" not a real state

	MyCXXRecordDecl *RecordDecl = static_cast<MyCXXRecordDecl*>(Declaration);
	const CXXBaseSpecifier *Base;

	if (RecordDecl->isDerivedFrom("boost::statechart::simple_state", &Base))
	{
	    string name(RecordDecl->getName()); //getQualifiedNameAsString());
	    Diag(RecordDecl->getLocStart(), diag_found_state) << name;

	    Model::State *state;
	    // Either we saw a reference to forward declared state
	    // before, or we create a new state.
	    if (!(state = model.removeFromUndefinedContexts(name)))
		state = new Model::State(name);

	    CXXRecordDecl *Context = getTemplateArgDeclOfBase(Base, 1);
	    Model::Context *c = model.findContext(Context->getName());
	    if (!c) {
		Model::State *s = new Model::State(Context->getName());
		model.addUndefinedState(s);
		c = s;
	    }
	    c->add(state);

	    if (MyCXXRecordDecl *InnerInitialState =
		static_cast<MyCXXRecordDecl*>(getTemplateArgDeclOfBase(Base, 2))) {
		if (InnerInitialState->isDerivedFrom("boost::statechart::simple_state") ||
		    InnerInitialState->isDerivedFrom("boost::statechart::state_machine"))
		    state->setInitialInnerState(InnerInitialState->getName());
		else
		    Diag(Base->getTypeSourceInfo()->getTypeLoc().getLocStart(), diag_warning)
			<< InnerInitialState->getQualifiedNameAsString() << " as inner initial state is not supported";
	    }

// 	    if (CXXRecordDecl *History = getTemplateArgDecl(Base->getType().getTypePtr(), 3))
// 		Diag(History->getLocStart(), diag_no_history);

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

	    if (MyCXXRecordDecl *InitialState =
		static_cast<MyCXXRecordDecl*>(getTemplateArgDeclOfBase(Base, 1)))
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
