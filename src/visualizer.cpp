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
#include <vector>

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
	list<string> inStateEvents;
	bool noTypedef;
    public:
	const string name;
	explicit State(string name) : noTypedef(false), name(name) {}
	void setInitialInnerState(string name) { initialInnerState = name; }
	void addDeferredEvent(const string &name) { defferedEvents.push_back(name); }
	void addInStateEvent(const string &name) { inStateEvents.push_back(name); }
	void setNoTypedef() { noTypedef = true;}
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
	for (list<string>::const_iterator i = s.inStateEvents.begin(), e = s.inStateEvents.end(); i != e; ++i)
	    label.append("<br />").append(*i).append(" / in state");
	if (s.noTypedef) os << indent << s.name << " [label=<" << label << ">, color=\"red\"]\n";
	else os << indent << s.name << " [label=<" << label << ">]\n";
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
	if (E->getMemberNameInfo().getAsString() == "defer_event") {
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();

		Model::State *s = model.findState(SrcState->getName());
		assert(s);
		s->addDeferredEvent(Event->getName());
	} else if (E->getMemberNameInfo().getAsString() != "transit")
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
    struct eventModel {
	string name;
	SourceLocation loc;
	eventModel(string ev, SourceLocation sourceLoc) : name(ev), loc(sourceLoc){}
    };

    struct eventHasName {
	string eventName;
	eventHasName(string name) : eventName(name){}
	bool operator() (const eventModel& model) { return (eventName.compare(model.name) == 0); }
    };
    ASTContext *ASTCtx;
    Model::Model &model;
    DiagnosticsEngine &Diags;
    unsigned diag_unhandled_reaction_type, diag_unhandled_reaction_decl,
	diag_found_state, diag_found_statemachine, diag_no_history, diag_missing_reaction, diag_warning;
    std::vector<bool> reactMethodInReactions; // Indicates whether i-th react method is referenced from typedef reactions.
    std::list<eventModel> unusedEvents;

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
	diag_no_history =
	    Diags.getCustomDiagID(DiagnosticsEngine::Error, "History is not yet supported");
	diag_missing_reaction =
	    Diags.getCustomDiagID(DiagnosticsEngine::Error, "Missing react method for event '%0'");
	diag_warning =
	    Diags.getCustomDiagID(DiagnosticsEngine::Warning, "'%0' %1");
    }

    DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) { return Diags.Report(Loc, DiagID); }

    void checkAllReactMethods(const CXXRecordDecl *SrcState) 
    {
	unsigned i = 0;
	IdentifierInfo& II = ASTCtx->Idents.get("react");
	for (DeclContext::lookup_const_result ReactRes = SrcState->lookup(DeclarationName(&II));
	     ReactRes.first != ReactRes.second; ++ReactRes.first, ++i) {
	    if (i >= reactMethodInReactions.size() || reactMethodInReactions[i] == false) {
		CXXMethodDecl *React = dyn_cast<CXXMethodDecl>(*ReactRes.first);
		Diag(React->getParamDecl(0)->getLocStart(), diag_warning)
		    << React->getParamDecl(0)->getType().getAsString() << " missing in typedef reactions";
	    }
	}
    }
    
    bool HandleCustomReaction(const CXXRecordDecl *SrcState, const Type *EventType)
    {
	unsigned i = 0;
	IdentifierInfo& II = ASTCtx->Idents.get("react");
	// TODO: Lookup for react even in base classes - probably by using Sema::LookupQualifiedName()
	for (DeclContext::lookup_const_result ReactRes = SrcState->lookup(DeclarationName(&II));
	     ReactRes.first != ReactRes.second; ++ReactRes.first) {
	    if (CXXMethodDecl *React = dyn_cast<CXXMethodDecl>(*ReactRes.first)) {
		if (React->getNumParams() >= 1) {
		    const ParmVarDecl *p = React->getParamDecl(0);
		    const Type *ParmType = p->getType().getTypePtr();
		    if (i == reactMethodInReactions.size()) reactMethodInReactions.push_back(false);
		    if (ParmType->isLValueReferenceType())
			ParmType = dyn_cast<LValueReferenceType>(ParmType)->getPointeeType().getTypePtr();
		    if (ParmType == EventType) {
			FindTransitVisitor(model, SrcState, EventType).TraverseStmt(React->getBody());
			reactMethodInReactions[i] = true;
			return true;
		    }
		} else
		    Diag(React->getLocStart(), diag_warning)
			<< React << "has not a parameter";
	    } else
		Diag((*ReactRes.first)->getSourceRange().getBegin(), diag_warning)
		    << (*ReactRes.first)->getDeclKindName() << "is not supported as react method";
	    i++;
	}
	return false;
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
		unusedEvents.remove_if(eventHasName(Event->getNameAsString()));

		Model::Transition *T = new Model::Transition(SrcState->getName(), DstState->getName(), Event->getName());
		model.transitions.push_back(T);
	    } else if (name == "boost::statechart::custom_reaction") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		if (!HandleCustomReaction(SrcState, EventType)) {
		    Diag(SrcState->getLocation(), diag_missing_reaction) << EventType->getAsCXXRecordDecl()->getName();
		}
		unusedEvents.remove_if(eventHasName(EventType->getAsCXXRecordDecl()->getNameAsString()));
	    } else if (name == "boost::statechart::deferral") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();
		unusedEvents.remove_if(eventHasName(Event->getNameAsString()));

		Model::State *s = model.findState(SrcState->getName());
		assert(s);
		s->addDeferredEvent(Event->getName());
	    } else if (name == "boost::mpl::list") {
		for (TemplateSpecializationType::iterator Arg = TST->begin(), End = TST->end(); Arg != End; ++Arg)
		    HandleReaction(Arg->getAsType().getTypePtr(), Loc, SrcState);
	    } else if (name == "boost::statechart::in_state_reaction") {
		const Type *EventType = TST->getArg(0).getAsType().getTypePtr();
		CXXRecordDecl *Event = EventType->getAsCXXRecordDecl();
		unusedEvents.remove_if(eventHasName(Event->getNameAsString()));

		Model::State *s = model.findState(SrcState->getName());
		assert(s);
		s->addInStateEvent(Event->getName());
	      
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
	checkAllReactMethods(SrcState);
    }

    TemplateArgumentLoc getTemplateArgLoc(const TypeLoc &T, unsigned ArgNum, bool ignore)
    {
	if (const ElaboratedTypeLoc *ET = dyn_cast<ElaboratedTypeLoc>(&T))
	    return getTemplateArgLoc(ET->getNamedTypeLoc(), ArgNum, ignore);
	else if (const TemplateSpecializationTypeLoc *TST = dyn_cast<TemplateSpecializationTypeLoc>(&T)) {
	    if (TST->getNumArgs() >= ArgNum+1) {
		return TST->getArgLoc(ArgNum);
	    } else
		if (!ignore)
		    Diag(TST->getBeginLoc(), diag_warning) << TST->getType()->getTypeClassName() << "has not enough arguments" << TST->getSourceRange();
	} else
	    Diag(T.getBeginLoc(), diag_warning) << T.getType()->getTypeClassName() << "type as template argument is not supported" << T.getSourceRange();
	return TemplateArgumentLoc();
    }

    TemplateArgumentLoc getTemplateArgLocOfBase(const CXXBaseSpecifier *Base, unsigned ArgNum, bool ignore) {
	return getTemplateArgLoc(Base->getTypeSourceInfo()->getTypeLoc(), ArgNum, ignore);
    }

    CXXRecordDecl *getTemplateArgDeclOfBase(const CXXBaseSpecifier *Base, unsigned ArgNum, TemplateArgumentLoc &Loc, bool ignore = false) {
	Loc = getTemplateArgLocOfBase(Base, ArgNum, ignore);
	switch (Loc.getArgument().getKind()) {
	case TemplateArgument::Type:
	    return Loc.getTypeSourceInfo()->getType()->getAsCXXRecordDecl();
	case TemplateArgument::Null:
	    // Diag() was already called
	    break;
	default:
	    Diag(Loc.getSourceRange().getBegin(), diag_warning) << Loc.getArgument().getKind() << "unsupported kind" << Loc.getSourceRange();
	}
	return 0;
    }

    CXXRecordDecl *getTemplateArgDeclOfBase(const CXXBaseSpecifier *Base, unsigned ArgNum, bool ignore = false) {
	TemplateArgumentLoc Loc;
	return getTemplateArgDeclOfBase(Base, ArgNum, Loc, ignore);
    }

    void handleSimpleState(CXXRecordDecl *RecordDecl, const CXXBaseSpecifier *Base)
    {
	int typedef_num = 0;
	string name(RecordDecl->getName()); //getQualifiedNameAsString());
	Diag(RecordDecl->getLocStart(), diag_found_state) << name;
	reactMethodInReactions.clear();

	Model::State *state;
	// Either we saw a reference to forward declared state
	// before, or we create a new state.
	if (!(state = model.removeFromUndefinedContexts(name)))
	    state = new Model::State(name);

	CXXRecordDecl *Context = getTemplateArgDeclOfBase(Base, 1);
	if (Context) {
	    Model::Context *c = model.findContext(Context->getName());
	    if (!c) {
		Model::State *s = new Model::State(Context->getName());
		model.addUndefinedState(s);
		c = s;
	    }
	    c->add(state);
	}
	//TODO support more innitial states
	TemplateArgumentLoc Loc;
	if (MyCXXRecordDecl *InnerInitialState =
	    static_cast<MyCXXRecordDecl*>(getTemplateArgDeclOfBase(Base, 2, Loc, true))) {
	      if (InnerInitialState->isDerivedFrom("boost::statechart::simple_state") ||
		InnerInitialState->isDerivedFrom("boost::statechart::state_machine")) {
		  state->setInitialInnerState(InnerInitialState->getName());
	    }
	    else if (!InnerInitialState->getNameAsString().compare("boost::mpl::list<>"))
	      Diag(Loc.getTypeSourceInfo()->getTypeLoc().getBeginLoc(), diag_warning)
		    << InnerInitialState->getName() << " as inner initial state is not supported" << Loc.getSourceRange();
	}

// 	    if (CXXRecordDecl *History = getTemplateArgDecl(Base->getType().getTypePtr(), 3))
// 		Diag(History->getLocStart(), diag_no_history);

	IdentifierInfo& II = ASTCtx->Idents.get("reactions");
	// TODO: Lookup for reactions even in base classes - probably by using Sema::LookupQualifiedName()
	for (DeclContext::lookup_result Reactions = RecordDecl->lookup(DeclarationName(&II));
	     Reactions.first != Reactions.second; ++Reactions.first, typedef_num++)
	    HandleReaction(*Reactions.first, RecordDecl);
	if(typedef_num == 0) {
	    Diag(RecordDecl->getLocStart(), diag_warning)
		<< RecordDecl->getName() << "state has no typedef for reactions";
	    state->setNoTypedef();
	}
    }

    void handleStateMachine(CXXRecordDecl *RecordDecl, const CXXBaseSpecifier *Base)
    {
	Model::Machine m(RecordDecl->getName());
	Diag(RecordDecl->getLocStart(), diag_found_statemachine) << m.name;

	if (MyCXXRecordDecl *InitialState =
	    static_cast<MyCXXRecordDecl*>(getTemplateArgDeclOfBase(Base, 1)))
	    m.setInitialState(InitialState->getName());
	model.add(m);
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
	if (!Declaration->isCompleteDefinition())
	    return true;
	if (Declaration->getQualifiedNameAsString() == "boost::statechart::state" ||
	    Declaration->getQualifiedNameAsString() == "TimedState" ||
	    Declaration->getQualifiedNameAsString() == "TimedSimpleState" ||
	    Declaration->getQualifiedNameAsString() == "boost::statechart::assynchronous_state_machine")
	    return true; // This is an "abstract class" not a real state or real state machine

	MyCXXRecordDecl *RecordDecl = static_cast<MyCXXRecordDecl*>(Declaration);
	const CXXBaseSpecifier *Base;

	if (RecordDecl->isDerivedFrom("boost::statechart::simple_state", &Base))
	    handleSimpleState(RecordDecl, Base);
	else if (RecordDecl->isDerivedFrom("boost::statechart::state_machine", &Base))
	    handleStateMachine(RecordDecl, Base);
	else if (RecordDecl->isDerivedFrom("boost::statechart::event")) {
	    // Mark the event as unused until we found that somebody uses it
	    unusedEvents.push_back(eventModel(RecordDecl->getNameAsString(), RecordDecl->getLocation()));
	}
	return true;
    }
    void printUnusedEventDefinitions() {
	for(list<eventModel>::iterator it = unusedEvents.begin(); it!=unusedEvents.end(); it++)
	    Diag((*it).loc, diag_warning)
		<< (*it).name << "event defined but not used in any state";
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
	visitor.printUnusedEventDefinitions();
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
