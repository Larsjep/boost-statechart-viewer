#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/mpl/list.hpp>

#include <iostream>
#include <string>

using namespace std;


namespace sc = boost::statechart;
namespace mpl = boost::mpl;

struct state_1; // definice stavu
struct state_8; // definice stavu
struct state_9; // definice stavu
struct state_10; // definice stavu
struct state_11; // definice stavu
struct state_12; // definice stavu
struct state_13; // definice stavu
struct state_2; // definice stavu

struct state_3; // definice stavu
struct state_4; // definice stavu
struct state_5; // definice stavu
struct state_6; // definice stavu
struct state_7; // definice stavu

struct Ev1 : sc::event< Ev1 > {};//stisteno A
struct Ev2 : sc::event< Ev2 > {};//stisteno N
struct Ev3 : sc::event< Ev3 > {};//stisteno N
struct Ev4 : sc::event< Ev4 > {};//stisteno N
struct MyMachine : sc::state_machine< MyMachine, state_1 > {};//Nastaveni inicializacniho stavu

struct state_1 : sc::simple_state< state_1, MyMachine> // stav
{
    state_1() {}//FSM_Entry
    ~state_1() {}//FSM_Exit
    typedef mpl::list< // reakce na udalosti
		  sc::transition< Ev2, state_1 >,
        sc::transition< Ev1, state_2 >,
        sc::transition< Ev3, state_8 > > reactions;
};
struct state_2 : sc::simple_state< state_2, MyMachine, state_3 >
{
    state_2() { }
    ~state_2() { }
    typedef sc::transition< Ev2, state_2 > reactions;
};
struct state_8 : sc::simple_state< state_8, MyMachine> // stav
{
    state_8() {}//FSM_Entry
    ~state_8() {}//FSM_Exit
    typedef mpl::list< // reakce na udalosti
		  sc::transition< Ev2, state_10 >,
        sc::transition< Ev1, state_9 >,
        sc::transition< Ev3, state_11 >,
		  sc::transition< Ev4, state_12 > > reactions;
};

struct state_9 : sc::simple_state< state_9, MyMachine >
{
    state_9() { }
    ~state_9() { }
    typedef sc::transition< Ev1, state_13 > reactions;
};

struct state_10 : sc::simple_state< state_10, MyMachine >
{
    state_10() { }
    ~state_10() { }
    typedef sc::transition< Ev2, state_13 > reactions;
};

struct state_11 : sc::simple_state< state_11, MyMachine >
{
    state_11() { }
    ~state_11() { }
    typedef sc::transition< Ev3, state_13 > reactions;
};

struct state_12 : sc::simple_state< state_12, MyMachine >
{
    state_12() { }
    ~state_12() { }
    typedef sc::transition< Ev4, state_13 > reactions;
};

struct state_13 : sc::simple_state< state_13, MyMachine >
{
    state_13() { }
    ~state_13() { }
    typedef sc::transition< Ev2, state_8 > reactions;
};

struct state_3 : sc::simple_state< state_3, state_2 >
{
    state_3() { }
    ~state_3() { }
    typedef sc::transition< Ev1, state_4 > reactions;
};
struct state_4 : sc::simple_state< state_4, state_2 >
{
    state_4() { }
    ~state_4() { }
    typedef mpl::list<
			sc::transition< Ev3, state_3 >,
			sc::transition< Ev2, state_5 > > reactions;
};
struct state_5 : sc::simple_state< state_5, state_2, state_6 >
{
    state_5() { }
    ~state_5() { }
    typedef sc::transition< Ev3, state_1 > reactions;
};
struct state_6 : sc::simple_state< state_6, state_5 >
{
    state_6() { }
    ~state_6() { }
    typedef sc::transition< Ev4, state_7 > reactions;
};
struct state_7 : sc::simple_state< state_7, state_5 >
{
    state_7() { }
    ~state_7() { }
    typedef sc::transition< Ev2, state_6 > reactions;
};




