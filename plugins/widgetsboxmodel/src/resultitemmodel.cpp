// Copyright (c) 2022-2025 Manuel Schneider

#include "resultitemmodel.h"
#include <QIcon>
#include <QStringListModel>
#include <QTimer>
#include <albert/extension.h>
#include <albert/frontend.h>
#include <albert/iconutil.h>
#include <albert/item.h>
#include <albert/logging.h>
#include <albert/query.h>
#include <albert/rankitem.h>
using enum ItemRoles;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;


// -------------------------------------------------------------------------------------------------

ResultItemsModel::ResultItemsModel(Query &query):
    query_(query)
{
}

QVariant ResultItemsModel::getResultItemData(const ResultItem &result_item, int role) const
{
    const auto &[extension, item] = result_item;

    switch (role) {

    case IdentifierRole:
    {
        try {
            return u"%1.%2"_s.arg(extension.id(), item->id());
        } catch (const exception &e) {
            WARN << "Exception in Item::id:" << e.what();
        }
        return {};
    }

    case TextRole:
    {
        try {
            auto text = item->text();
            text.replace(u'\n', u' ');
            return text;
        } catch (const exception &e) {
            WARN << "Exception in Item::text:" << e.what();
        }
        return {};
    }

    case SubTextRole:
    {
        try {
            auto text = item->subtext();
            text.replace(u'\n', u' ');
            return text;
        } catch (const exception &e) {
            WARN << "Exception in Item::subtext:" << e.what();
        }
        return {};
    }

    case Qt::ToolTipRole:
    {
        try {
            const auto text = item->text();
            try {
                return u"%1\n%2"_s.arg(text, item->subtext());
            } catch (const exception &e) {
                WARN << "Exception in Item::subtext:" << e.what();
            }
        } catch (const exception &e) {
            WARN << "Exception in Item::text:" << e.what();
        }
        return {};
    }

    case InputActionRole:
    {
        try {
            return item->inputActionText();
        } catch (const exception &e) {
            WARN << "Exception in Item::inputActionText:" << e.what();
        }
        return {};
    }

    case IconRole:
    {
        try {
            return qIcon(item->icon());
        } catch (const exception &e) {
            WARN << "Exception in Item::makeIcon:" << e.what();
        }
        return {};
    }

    case ActionsListRole:
    {
        if (auto it = actions_cache_.find(&result_item);
            it != actions_cache_.end())
            return it->second;

        try {
            QStringList action_names;
            for (const auto &action : item->actions())
                action_names << action.text;
            // actions_cache_.emplace(make_pair(extension, item.get()), actions_cache_);
            return action_names;
        } catch (const exception &e) {
            WARN << "Exception in Item::actions:" << e.what();
        }
        return {};
    }
    }
    return {};
}


// -------------------------------------------------------------------------------------------------

MatchItemsModel::MatchItemsModel(Query &query):
    ResultItemsModel(query)
{
    connect(&query, &Query::matchesAboutToBeAdded, this, [&, this](uint count){
        Q_ASSERT(count > 0);
        beginInsertRows({}, (int)query.matches().size(), (int)(query.matches().size() + count - 1));
    });

    connect(&query, &Query::matchesAdded, this, [this] { endInsertRows(); });

    connect(&query, &Query::dataChanged, this, [this](uint i){
        emit dataChanged(index(i, 0), index(i, 0));
    });
}

int MatchItemsModel::rowCount(const QModelIndex &) const
{ return (int)query_.matches().size(); }

bool MatchItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == ActivateActionRole)
        return query_.activateMatch(index.row(), value.toUInt());
    else
        return false;
}

QVariant MatchItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
        return getResultItemData(query_.matches().at(index.row()), role);
    return {};
}


// -------------------------------------------------------------------------------------------------

int FallbackItemsModel::rowCount(const QModelIndex &) const
{ return (int)query_.fallbacks().size(); }

bool FallbackItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == ActivateActionRole)
        return query_.activateFallback(index.row(), value.toUInt());
    else
        return false;
}

QVariant FallbackItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
        return getResultItemData(query_.fallbacks().at(index.row()), role);
    return {};
}















// void ItemsModel::add(Extension *extension, vector<shared_ptr<Item>> &&itemvec)
// {
//     if (itemvec.empty())
//         return;

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
//     items.reserve(items.size()+itemvec.size());
//     for (auto &&item : itemvec)
//         items.emplace_back(extension, ::move(item));
//     endInsertRows();
// }

// void ItemsModel::add(vector<pair<Extension*, shared_ptr<Item>>>::iterator begin,
//                      vector<pair<Extension*, shared_ptr<Item>>>::iterator end)
// {
//     if (begin == end)
//         return;

//     items.reserve(items.size()+(size_t)(end-begin));

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

//     items.insert(items.end(), make_move_iterator(begin), make_move_iterator(end));

//     endInsertRows();

// }

// void ItemsModel::add(vector<pair<Extension*,RankItem>>::iterator begin,
//                      vector<pair<Extension*,RankItem>>::iterator end)
// {
//     if (begin == end)
//         return;

//     items.reserve(items.size()+(size_t)(end-begin));

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

//     for (auto it = begin; it != end; ++it)
//         items.emplace_back(it->first, ::move(it->second.item));

//     endInsertRows();
// }

// QAbstractListModel *ItemsModel::buildActionsModel(uint i) const
// {
//     QStringList l;
//     for (const auto &a : items[i].second->actions())
//         l << a.text;
//     return new QStringListModel(l);
// }
