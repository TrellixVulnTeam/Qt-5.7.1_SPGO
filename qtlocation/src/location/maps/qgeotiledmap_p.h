/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QGEOTILEDMAP_P_H
#define QGEOTILEDMAP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QString>

#include "qgeomap_p.h"
#include "qgeocameradata_p.h"
#include "qgeomaptype_p.h"

#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

class QGeoTileSpec;
class QGeoTileTexture;
class QAbstractGeoTileCache;
class QGeoTiledMapPrivate;
class QGeoTiledMappingManagerEngine;
class QGeoTileRequestManager;

class QQuickWindow;
class QSGNode;

class QPointF;

class Q_LOCATION_EXPORT QGeoTiledMap : public QGeoMap
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoTiledMap)
public:
    enum PrefetchStyle { PrefetchNeighbourLayer, PrefetchTwoNeighbourLayers };
    QGeoTiledMap(QGeoTiledMappingManagerEngine *engine, QObject *parent);
    virtual ~QGeoTiledMap();

    QAbstractGeoTileCache *tileCache();
    QGeoTileRequestManager *requestManager();
    void updateTile(const QGeoTileSpec &spec);
    void setPrefetchStyle(PrefetchStyle style);

    QGeoCoordinate itemPositionToCoordinate(const QDoubleVector2D &pos, bool clipToViewport = true) const Q_DECL_OVERRIDE;
    QDoubleVector2D coordinateToItemPosition(const QGeoCoordinate &coordinate, bool clipToViewport = true) const Q_DECL_OVERRIDE;

    double minimumZoomForMapSize(int width, int height) const Q_DECL_OVERRIDE;
    double maximumLatitudeForZoom(double zoomLevel) const Q_DECL_OVERRIDE;

    QDoubleVector2D referenceCoordinateToItemPosition(const QGeoCoordinate &coordinate) const Q_DECL_OVERRIDE;
    QGeoCoordinate referenceItemPositionToCoordinate(const QDoubleVector2D &pos) const Q_DECL_OVERRIDE;

    void prefetchData() Q_DECL_OVERRIDE;
    void clearData() Q_DECL_OVERRIDE;

protected:
    QSGNode *updateSceneGraph(QSGNode *, QQuickWindow *window) Q_DECL_OVERRIDE;
    virtual void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles);

private Q_SLOTS:
    void handleTileVersionChanged();

private:
    Q_DISABLE_COPY(QGeoTiledMap)

};

QT_END_NAMESPACE

#endif // QGEOMAP_P_H
