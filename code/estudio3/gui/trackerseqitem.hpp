/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TRACKER_SEQ_ITEM_HPP
#define TRACKER_SEQ_ITEM_HPP

#include <QtGui/QGraphicsItem>
#include <QtXml/QDomElement>

#include "patterneditor.hpp"

class eTrackerSeqItem : public QGraphicsItem
{
public:
    eTrackerSeqItem(tfSong::PatternInstance *pi, class eTrackerSeqView *view);
    eTrackerSeqItem(eTrackerSeqItem &tsi, eBool clonePattern);
    virtual ~eTrackerSeqItem();

    void                            saveToXml(QDomElement &node);
    void                            loadFromXml(QDomElement &node);

    tfSong::PatternInstance *       getPatternInstance();
    const tfSong::PatternInstance * getPatternInstance() const;

    void                            init();

public:
    virtual QRectF                  boundingRect() const;

private:
    virtual void                    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant                itemChange(GraphicsItemChange change, const QVariant &value);

    QPixmap                         _createPixmap() const;
    const QString &                 _cacheKey() const;

private:
    tfSong::PatternInstance *       m_pi;
    QFontMetrics                    m_fontMetrics;
    class eTrackerSeqView *         m_trackerSeqView;
};

#endif // TRACKER_SEQ_ITEM_HPP