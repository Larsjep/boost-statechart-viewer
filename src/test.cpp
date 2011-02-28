
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/mpl/list.hpp>

#include <iostream>
#include <string>
#include <pthread.h>

using namespace std;


namespace sc = boost::statechart;
namespace mpl = boost::mpl;
struct state_1; // definice stavu
struct state_2; // definice stavu
struct state_3; // definice stavu
struct state_4; // definice stavu
struct EvA : sc::event< EvA > {};//stisteno A
struct EvN : sc::event< EvN > {};//stisteno N
struct EvTimer : sc::event< EvTimer > {};//vyprsel cas

struct DU : sc::state_machine< DU, state_1 > {};//Nastaveni inicializacniho stavu

DU Zm; // pro posilani udalosti

void* casovac(void* s)
{
    int es = int (s);
    sleep (es);
    Zm.process_event (EvTimer());
    return NULL;
}

struct state_1 : sc::simple_state< state_1, DU> // stav
{
    state_1() { cout<<"Chcete zformatovat vas disk (a/n) ? \n";}//FSM_Entry
    ~state_1() {}//FSM_Exit
    typedef mpl::list< // reakce na udalosti
        sc::transition< EvA, state_4 >,
        sc::transition< EvN, state_2 > > reactions;
};
struct state_2 : sc::simple_state< state_2, DU >
{
    state_2()
    {
         cout<<"Rozmysli si to! \n";
        pthread_t idthread;
        int *a = (int* )3;
        if ( pthread_create(&idthread, NULL, casovac, a) != 0)
        {
             cout << "Chyba pri vytvareni vlakna.";
            abort();
        }
        if ( pthread_detach(idthread ) )
        {
                 cout << "Error detaching ..\n";
                abort ();
        }
    }
    ~state_2() {}
    typedef sc::transition< EvTimer, state_3 > reactions;
};
struct state_3 : sc::simple_state< state_3, DU >
{
    pthread_t idthread;
    state_3() { cout<<"Rychle stiskni n nebo ti zformatuju disk. \n";
        int *a = (int* ) 1;
        if ( pthread_create(&idthread, NULL, casovac, a) != 0)
        {
             cout << "Chyba pri vytvareni vlakna.";
            abort();
        }
        if ( pthread_detach(idthread ) )
        {
                 cout << "Error detaching ..\n";
                abort ();
        }
    }
    ~state_3() {}
    typedef mpl::list<
        sc::transition< EvN, state_1 >,
        sc::transition< EvTimer, state_4 > > reactions;
};
struct state_4 : sc::simple_state< state_4, DU >
{
    state_4() {
         cout<<"Formatuji tvuj disk. \n";
        pthread_t idthread;
        int *a = (int* )2;
        if ( pthread_create(&idthread, NULL, casovac, a) != 0)
        {
             cout << "Chyba pri vytvareni vlakna.";
            abort();
        }
        if ( pthread_detach(idthread ) )
        {
                 cout << "Error detaching ..\n";
                abort ();
        }
    }
    ~state_4() { cout<<"HOTOVO.\n";}
    typedef sc::transition< EvTimer, state_1 > reactions;
};

void *nacitani (void * ptr)
{
     string s = "";
    while(1)
    {
         cin >> s;
        if (s == "n") Zm.process_event (EvN());
        if (s == "a") Zm.process_event (EvA());
        if (s == "exit") break;
    }
    return NULL;
}
int main()
{
    Zm.initiate();
    pthread_t idthread;
    if ( pthread_create(&idthread, NULL, nacitani, NULL) != 0)
    {
         cout << "Chyba pri vytvareni vlakna.";
        abort();
    }
    pthread_join(idthread, NULL);
    return 0;
}
