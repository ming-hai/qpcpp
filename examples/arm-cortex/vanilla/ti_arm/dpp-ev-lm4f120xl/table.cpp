//////////////////////////////////////////////////////////////////////////////
// Model: dpp.qm
// File:  ./table.cpp
//
// This file has been generated automatically by QP Modeler (QM).
// DO NOT EDIT THIS FILE MANUALLY.
//
// Please visit www.state-machine.com/qm for more information.
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
// @(/2/1) ...................................................................
class Table : public QP::QActive {
private:
    uint8_t m_fork[N_PHILO];
    bool m_isHungry[N_PHILO];

public:
    Table();

protected:
    static QP::QState initial(Table * const me, QP::QEvt const * const e);
    static QP::QState active(Table * const me, QP::QEvt const * const e);
    static QP::QState serving(Table * const me, QP::QEvt const * const e);
    static QP::QState paused(Table * const me, QP::QEvt const * const e);
};


// helper function to provide the RIGHT neighbour of a Philo[n]
inline uint8_t RIGHT(uint8_t const n) {
    return static_cast<uint8_t>((n + (N_PHILO - 1U)) % N_PHILO);
}

// helper function to provide the LEFT neighbour of a Philo[n]
inline uint8_t LEFT(uint8_t const n) {
    return static_cast<uint8_t>((n + 1U) % N_PHILO);
}

static uint8_t const FREE = static_cast<uint8_t>(0);
static uint8_t const USED = static_cast<uint8_t>(1);

static char_t const * const THINKING = &"thinking"[0];
static char_t const * const HUNGRY   = &"hungry  "[0];
static char_t const * const EATING   = &"eating  "[0];

// Local objects -------------------------------------------------------------
static Table l_table; // the single instance of the Table active object

// Global-scope objects ------------------------------------------------------
QP::QActive * const AO_Table = &l_table; // "opaque" AO pointer

//............................................................................
// @(/2/1) ...................................................................
// @(/2/1/2) .................................................................
Table::Table() 
  : QActive(Q_STATE_CAST(&Table::initial))
{
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = false;
    }
}

// @(/2/1/3) .................................................................
// @(/2/1/3/0)
QP::QState Table::initial(Table * const me, QP::QEvt const * const e) {
    (void)e; // suppress the compiler warning about unused parameter

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QP::QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::active);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_FUN_DICTIONARY(&Table::paused);

    QS_SIG_DICTIONARY(DONE_SIG,      (void *)0); // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       (void *)0);
    QS_SIG_DICTIONARY(PAUSE_SIG,     (void *)0);
    QS_SIG_DICTIONARY(TERMINATE_SIG, (void *)0);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    me); // signal just for Table

    me->subscribe(DONE_SIG);
    me->subscribe(PAUSE_SIG);
    me->subscribe(TERMINATE_SIG);

    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        me->m_fork[n] = FREE;
        me->m_isHungry[n] = false;
        BSP_displayPhilStat(n, THINKING);
    }
    return Q_TRAN(&Table::serving);
}
// @(/2/1/3/1) ...............................................................
QP::QState Table::active(Table * const me, QP::QEvt const * const e) {
    QP::QState status;
    switch (e->sig) {
        // @(/2/1/3/1/0)
        case TERMINATE_SIG: {
            BSP_terminate(0);
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/1)
        case EAT_SIG: {
            Q_ERROR();
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}
// @(/2/1/3/1/2) .............................................................
QP::QState Table::serving(Table * const me, QP::QEvt const * const e) {
    QP::QState status;
    switch (e->sig) {
        // @(/2/1/3/1/2)
        case Q_ENTRY_SIG: {
            for (uint8_t n = 0U; n < N_PHILO; ++n) { // give permissions to eat...
                if (me->m_isHungry[n]
                    && (me->m_fork[LEFT(n)] == FREE)
                    && (me->m_fork[n] == FREE))
                {
                    me->m_fork[LEFT(n)] = USED;
                    me->m_fork[n] = USED;
                    TableEvt *te = Q_NEW(TableEvt, EAT_SIG);
                    te->philoNum = n;
                    QP::QF::PUBLISH(te, me);
                    me->m_isHungry[n] = false;
                    BSP_displayPhilStat(n, EATING);
                }
            }
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/2/0)
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, HUNGRY);
            uint8_t m = LEFT(n);
            // @(/2/1/3/1/2/0/0)
            if ((me->m_fork[m] == FREE) && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = USED;
                me->m_fork[n] = USED;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(n, EATING);
                status = Q_HANDLED();
            }
            // @(/2/1/3/1/2/0/1)
            else {
                me->m_isHungry[n] = true;
                status = Q_HANDLED();
            }
            break;
        }
        // @(/2/1/3/1/2/1)
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            // both forks of Phil[n] must be used
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = FREE;
            me->m_fork[n] = FREE;
            m = RIGHT(n); // check the right neighbor

            if (me->m_isHungry[m] && (me->m_fork[m] == FREE)) {
                me->m_fork[n] = USED;
                me->m_fork[m] = USED;
                me->m_isHungry[m] = false;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(m, EATING);
            }
            m = LEFT(n); // check the left neighbor
            n = LEFT(m); // left fork of the left neighbor
            if (me->m_isHungry[m] && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = USED;
                me->m_fork[n] = USED;
                me->m_isHungry[m] = false;
                TableEvt *pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displayPhilStat(m, EATING);
            }
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/2/2)
        case EAT_SIG: {
            Q_ERROR();
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/2/3)
        case PAUSE_SIG: {
            status = Q_TRAN(&Table::paused);
            break;
        }
        default: {
            status = Q_SUPER(&Table::active);
            break;
        }
    }
    return status;
}
// @(/2/1/3/1/3) .............................................................
QP::QState Table::paused(Table * const me, QP::QEvt const * const e) {
    QP::QState status;
    switch (e->sig) {
        // @(/2/1/3/1/3)
        case Q_ENTRY_SIG: {
            BSP_displayPaused(1U);
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/3)
        case Q_EXIT_SIG: {
            BSP_displayPaused(0U);
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/3/0)
        case PAUSE_SIG: {
            status = Q_TRAN(&Table::serving);
            break;
        }
        // @(/2/1/3/1/3/1)
        case HUNGRY_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // philo ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));
            me->m_isHungry[n] = true;
            BSP_displayPhilStat(n, HUNGRY);
            status = Q_HANDLED();
            break;
        }
        // @(/2/1/3/1/3/2)
        case DONE_SIG: {
            uint8_t n = Q_EVT_CAST(TableEvt)->philoNum;
            // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displayPhilStat(n, THINKING);
            uint8_t m = LEFT(n);
            /* both forks of Phil[n] must be used */
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = FREE;
            me->m_fork[n] = FREE;
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&Table::active);
            break;
        }
    }
    return status;
}


}                                                             // namespace DPP
