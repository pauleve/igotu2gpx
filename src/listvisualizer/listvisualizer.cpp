/******************************************************************************
 * Copyright (C) 2009 Michael Hofmann <mh21@piware.de>                        *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
 ******************************************************************************/

#include "igotu/exception.h"
#include "igotu/igotupoints.h"

#include "trackvisualizer.h"

#include <QAction>
#include <QHeaderView>
#include <QLayout>
#include <QTreeWidget>

using namespace igotu;

class ListVisualizer: public TrackVisualizer
{
    Q_OBJECT
public:
    ListVisualizer(QWidget *parent = NULL);

    virtual void setTracks(const igotu::IgotuPoints &points, int utcOffset);
    virtual QString tabTitle() const;
    virtual void highlightTrack(const QList<IgotuPoint> &track);

Q_SIGNALS:
    void saveTracksRequested(const QList<QList<igotu::IgotuPoint> > &tracks);
    void trackActivated(const QList<igotu::IgotuPoint> &tracks);

private Q_SLOTS:
    void on_trackList_activated(const QModelIndex &index);
    void on_saveTracksAction_activated();

private:
    QTreeWidget *trackList;
};

class ListVisualizerCreator :
    public QObject,
    public TrackVisualizerCreator
{
    Q_OBJECT
    Q_INTERFACES(TrackVisualizerCreator)
public:
    virtual QString trackVisualizer() const;
    virtual int visualizerPriority() const;
    virtual TrackVisualizer *createTrackVisualizer(QWidget *parent = NULL) const;
};

Q_EXPORT_PLUGIN2(listVisualizer, ListVisualizerCreator)

static QString formatCoordinates(const IgotuPoint &point)
{
    const double longitude = point.longitude();
    const int longitudeSeconds = qRound(qAbs(longitude * 3600));
    const double latitude = point.latitude();
    const int latitudeSeconds = qRound(qAbs(latitude * 3600));
    return QString::fromUtf8("%1\xc2\xb0%2'%3\"%4, %5\xc2\xb0%6'%7\"%8")
        .arg(longitudeSeconds / 3600)
        .arg((longitudeSeconds / 60) % 60)
        .arg(longitudeSeconds % 60)
        .arg(longitude < 0 ? ListVisualizer::tr("W") : ListVisualizer::tr("E"))
        .arg(latitudeSeconds / 3600)
        .arg((latitudeSeconds / 60) % 60)
        .arg(latitudeSeconds % 60)
        .arg(latitude < 0 ? ListVisualizer::tr("S") : ListVisualizer::tr("N"));

}

// ListVisualizer ==============================================================

ListVisualizer::ListVisualizer(QWidget *parent) :
    TrackVisualizer(parent)
{
    QLayout * const verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(0);

    trackList = new QTreeWidget(this);
    trackList->setObjectName(QLatin1String("trackList"));
    trackList->setContextMenuPolicy(Qt::ActionsContextMenu);
    trackList->setAllColumnsShowFocus(true);
    trackList->setRootIsDecorated(false);
    trackList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    trackList->setColumnCount(3);
    trackList->setHeaderLabels(QStringList()
            << tr("Date")
            << tr("Position")
            << tr("Number of trackpoints"));

    verticalLayout->addWidget(trackList);

    QAction * const saveTracksAction = new QAction(tr("Save selected tracks..."), this);
    saveTracksAction->setObjectName(QLatin1String("saveTracksAction"));
    trackList->addAction(saveTracksAction);

    QMetaObject::connectSlotsByName(this);
}

void ListVisualizer::setTracks(const igotu::IgotuPoints &points, int utcOffset)
{
    QList<QTreeWidgetItem*> items;
    Q_FOREACH (const QList<IgotuPoint> &track, points.tracks()) {
        QString date = track.at(0).humanDateTimeString(utcOffset);
        QTreeWidgetItem * const item = new QTreeWidgetItem((QTreeWidget*)NULL,
                QStringList()
                    << date
                    << formatCoordinates(track.at(0))
                    << tr("%n point(s)", NULL, track.count()));
        item->setData(0, Qt::UserRole, QVariant::fromValue(track));
        items.append(item);
    }

    trackList->clear();
    trackList->insertTopLevelItems(0, items);
    for (unsigned i = 0; i < 3; ++i)
        trackList->resizeColumnToContents(i);
}

QString ListVisualizer::tabTitle() const
{
    return tr("List");
}

void ListVisualizer::highlightTrack(const QList<igotu::IgotuPoint> &track)
{
    Q_UNUSED(track);
    // ignored
}

void ListVisualizer::on_trackList_activated(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const QList<IgotuPoint> track = index.sibling(index.row(),
            0).data(Qt::UserRole).value<QList<IgotuPoint> >();
    emit trackActivated(track);
}

void ListVisualizer::on_saveTracksAction_activated()
{
    QList<QList<IgotuPoint> > tracks;
    Q_FOREACH (const QModelIndex &index, trackList->selectionModel()->selectedRows())
        tracks.append(index.data(Qt::UserRole).value<QList<IgotuPoint> >());
    emit saveTracksRequested(tracks);
}

// ListVisualizerCreator =======================================================

QString ListVisualizerCreator::trackVisualizer() const
{
    return QLatin1String("list");
}

int ListVisualizerCreator::visualizerPriority() const
{
    return 100;
}

TrackVisualizer *ListVisualizerCreator::createTrackVisualizer(QWidget *parent) const
{
    return new ListVisualizer(parent);
}

#include "listvisualizer.moc"
