// Copyright (c) 2024-2025 Manuel Schneider

#include "statetransitions.h"
#include <QEventTransition>
#include <QKeyEvent>
#include <QKeyEventTransition>
#include <QStateMachine>
using namespace std;

struct EventTransition : public QAbstractTransition
{
    const int type;

    EventTransition(int t, QState *s) :
        QAbstractTransition(s),
        type(t)
    {}

    void onTransition(QEvent *) override {}

    bool eventTest(QEvent *e) override { return static_cast<QEvent::Type>(type) == e->type(); }

};

QAbstractTransition *addTransition(QState *source, QState *target,
                                   int event_type)
{
    auto *t = new EventTransition(event_type, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   int event_type,
                                   function<bool()> guard)
{
    auto *t = new GuardedTransition<EventTransition>(guard, event_type, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   QObject *object, QEvent::Type type, int key)
{
    auto *t = new QKeyEventTransition(object, type, key, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object,
                                   QEvent::Type type, int key,
                                   function<bool()> guard)
{
    auto *t = new GuardedTransition<QKeyEventTransition>(guard, object, type, key, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   QObject *object, QEvent::Type type)
{
    auto *t = new QEventTransition(object, type, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   QObject *object, QEvent::Type type,
                                   function<bool()> guard)
{
    auto *t = new GuardedTransition<QEventTransition>(guard, object, type, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   QObject *object, QEvent::Type type,
                                   function<bool(QEvent*)> guard)
{
    struct GuardedEventTransition : public QEventTransition
    {
        function<bool(QEvent*)> guard;

        GuardedEventTransition(QObject *o, QEvent::Type t, function<bool(QEvent*)> g, QState *s)
            : QEventTransition(o, t, s), guard(g) {}

        bool eventTest(QEvent *event) override
        {
            if (event->type() != QEvent::StateMachineWrapped)
                if (static_cast<QStateMachine::WrappedEvent*>(event)->event()->type() != eventType())
                    return guard(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            return false;
        }
    };

    auto *t = new GuardedEventTransition(object, type, guard, source);
    t->setTargetState(target);
    return t;
}

QAbstractTransition *addTransition(QState *source, QState *target,
                                   QObject *object, QEvent::Type type,
                                   function<bool(QKeyEvent *)> guard)
{
    struct GuardedEventTransition : public QEventTransition
    {
        function<bool(QKeyEvent*)> guard;

        GuardedEventTransition(QObject *o, QEvent::Type t, function<bool(QKeyEvent*)> g, QState *s)
            : QEventTransition(o, t, s), guard(g)
        {
            assert(t == QEvent::KeyPress || t == QEvent::KeyRelease);
        }

        bool eventTest(QEvent *event) override
        {
            if (event->type() == QEvent::StateMachineWrapped)
                if (static_cast<QStateMachine::WrappedEvent*>(event)->event()->type() != eventType())
                    return guard(
                        static_cast<QKeyEvent*>(
                            static_cast<QStateMachine::WrappedEvent*>(event)->event()));
            return false;
        }
    };

    auto *t = new GuardedEventTransition(object, type, guard, source);
    t->setTargetState(target);
    return t;
}
