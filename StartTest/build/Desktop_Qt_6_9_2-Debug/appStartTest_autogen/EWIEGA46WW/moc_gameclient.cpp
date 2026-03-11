/****************************************************************************
** Meta object code from reading C++ file 'gameclient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../gameclient.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gameclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10GameClientE_t {};
} // unnamed namespace

template <> constexpr inline auto GameClient::qt_create_metaobjectdata<qt_meta_tag_ZN10GameClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GameClient",
        "info",
        "",
        "msg",
        "error",
        "connectedChanged",
        "gameCreated",
        "code",
        "joinOk",
        "gameStateChanged",
        "connectToServer",
        "host",
        "port",
        "disconnectFromServer",
        "createGame",
        "joinGame",
        "startGame",
        "drawCards",
        "count",
        "playCard",
        "card",
        "chosenColor",
        "declareUno",
        "saveGameLog",
        "fileUrl",
        "connected",
        "hasGameInit",
        "gameCode",
        "hand",
        "discardTop",
        "drawCount",
        "players",
        "yourIndex",
        "currentPlayerIndex",
        "handCounts",
        "QVariantList",
        "currentColor",
        "finished",
        "winnerIndex",
        "gameLog",
        "hasGameLog"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'info'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'error'
        QtMocHelpers::SignalData<void(QString)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'connectedChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'gameCreated'
        QtMocHelpers::SignalData<void(QString)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'joinOk'
        QtMocHelpers::SignalData<void(QString)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'gameStateChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'connectToServer'
        QtMocHelpers::MethodData<void(const QString &, int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::Int, 12 },
        }}),
        // Method 'disconnectFromServer'
        QtMocHelpers::MethodData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'createGame'
        QtMocHelpers::MethodData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'joinGame'
        QtMocHelpers::MethodData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'startGame'
        QtMocHelpers::MethodData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'drawCards'
        QtMocHelpers::MethodData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Method 'drawCards'
        QtMocHelpers::MethodData<void()>(17, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Method 'playCard'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 20 }, { QMetaType::QString, 21 },
        }}),
        // Method 'playCard'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
        // Method 'declareUno'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'saveGameLog'
        QtMocHelpers::MethodData<bool(const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 24 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'connected'
        QtMocHelpers::PropertyData<bool>(25, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'hasGameInit'
        QtMocHelpers::PropertyData<bool>(26, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'gameCode'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'hand'
        QtMocHelpers::PropertyData<QStringList>(28, QMetaType::QStringList, QMC::DefaultPropertyFlags, 5),
        // property 'discardTop'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'drawCount'
        QtMocHelpers::PropertyData<int>(30, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'players'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'yourIndex'
        QtMocHelpers::PropertyData<int>(32, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'currentPlayerIndex'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'handCounts'
        QtMocHelpers::PropertyData<QVariantList>(34, 0x80000000 | 35, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 5),
        // property 'currentColor'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'finished'
        QtMocHelpers::PropertyData<bool>(37, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'winnerIndex'
        QtMocHelpers::PropertyData<int>(38, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'gameLog'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'hasGameLog'
        QtMocHelpers::PropertyData<bool>(40, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GameClient, qt_meta_tag_ZN10GameClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GameClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10GameClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10GameClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10GameClientE_t>.metaTypes,
    nullptr
} };

void GameClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->info((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->connectedChanged(); break;
        case 3: _t->gameCreated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->joinOk((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->gameStateChanged(); break;
        case 6: _t->connectToServer((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->disconnectFromServer(); break;
        case 8: _t->createGame(); break;
        case 9: _t->joinGame((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->startGame((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->drawCards((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->drawCards(); break;
        case 13: _t->playCard((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->playCard((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->declareUno(); break;
        case 16: { bool _r = _t->saveGameLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)(QString )>(_a, &GameClient::info, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)(QString )>(_a, &GameClient::error, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)()>(_a, &GameClient::connectedChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)(QString )>(_a, &GameClient::gameCreated, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)(QString )>(_a, &GameClient::joinOk, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameClient::*)()>(_a, &GameClient::gameStateChanged, 5))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->connected(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->hasGameInit(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->gameCode(); break;
        case 3: *reinterpret_cast<QStringList*>(_v) = _t->hand(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->discardTop(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->drawCount(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->players(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->yourIndex(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->currentPlayerIndex(); break;
        case 9: *reinterpret_cast<QVariantList*>(_v) = _t->handCounts(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->currentColor(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->finished(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->winnerIndex(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->gameLog(); break;
        case 14: *reinterpret_cast<bool*>(_v) = _t->hasGameLog(); break;
        default: break;
        }
    }
}

const QMetaObject *GameClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10GameClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GameClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void GameClient::info(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GameClient::error(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GameClient::connectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GameClient::gameCreated(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void GameClient::joinOk(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void GameClient::gameStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
