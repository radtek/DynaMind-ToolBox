/**
 * @file
 * @author  Gregor Burger <burger.gregor@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Gregor Burger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "dmlayer.h"
#include "dmlogger.h"
#include "dmcomponent.h"
#include "dmnode.h"
#include "dmface.h"
#include "dmedge.h"
#include "dmattribute.h"
#include "dmsystemiterators.h"
#include "tbvectordata.h"

#include <QString>
#include <QStringList>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/partition_2.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/Partition_is_valid_traits_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Partition_traits_2<K>                         Traits;
typedef CGAL::Is_convex_2<Traits>                           Is_convex_2;
typedef Traits::Point_2                                     Point_2;
typedef Traits::Polygon_2                                   Polygon_2;
typedef std::vector<Polygon_2>                              Polygon_list;
typedef CGAL::Partition_is_valid_traits_2<Traits, Is_convex_2>
Validity_traits;

#include <QImage>
#include <QPainter>
#include <QProgressDialog>
#include <cassert>

namespace DM {

template<int SD_GL_PRIMITIVE>
struct SimpleDrawer {

    GLuint name_start;
    
    SimpleDrawer(const Layer &l) : name_start(l.getNameStart()) {
        
    }
    
    void operator()(DM::System *s, DM::View v, void *f_e, DM::Node *n, iterator_pos pos) {
        if (pos == before) {
            glPushName(name_start);
            glBegin(SD_GL_PRIMITIVE);
            return;
        }
        if (pos == after) {
            glEnd();
            glPopName();
            name_start++;
            return;
        }
        glColor3f(0, 0, 0);
        const double tmp[3] = {n->getX(), n->getY(), n->getZ()};
        glVertex3dv(tmp);
    }
};

struct TesselatedFaceDrawer {
    double attr_span;
    const Layer &l;
    double height_scale;
    Polygon_2 polygon;
    double current_height;
    double current_tex;
    int name_start;
    QProgressDialog *dialog;
    
    TesselatedFaceDrawer(const Layer &l, QWidget *parent)
        : l(l), height_scale(0.0), name_start(l.getNameStart()) {
        
        dialog = new QProgressDialog("Tesselating Polygons...", "cancel",
                                     0, l.getViewMetaData().number_of_primitives,
                                     parent);
        dialog->show();
        
        if (l.getAttribute() == "") {
            return;
        }
        
        const ViewMetaData &vmd = l.getViewMetaData();
        this->attr_span = vmd.attr_max - vmd.attr_min;
        
        if (l.getHeightInterpretation() > 0.0 && this->attr_span != 0.0) {
            this->height_scale = 1.0/this->attr_span*vmd.radius() * l.getHeightInterpretation();
        }
    }
    
    ~TesselatedFaceDrawer() {
        delete dialog;
    }
    
    void operator()(DM::System *s, DM::View v, DM::Face *f, DM::Node *n, iterator_pos pos) {
        if (pos == after) {
            render();
            polygon.clear();
            name_start++;
            current_height = 0.0;
            current_tex = 0.0;
            dialog->setValue(dialog->value()+1);
            return;
        }
        if (pos == before) {
            if (height_scale > 0) {
                Attribute *a = f->getAttribute(l.getAttribute());
                if (a->getType() == Attribute::DOUBLEVECTOR || a->getType() == Attribute::TIMESERIES) {
                    double attr_value = a->getDoubleVector()[l.getAttributeVectorName()];
                    current_height = attr_value * height_scale;
                } else {
                    current_height = f->getAttribute(l.getAttribute())->getDouble() * height_scale;
                }
            }
            if (glIsTexture(l.getColorInterpretation())) {
                const ViewMetaData &vmd = l.getViewMetaData();
                Attribute *a = f->getAttribute(l.getAttribute());
                if (a->getType() == Attribute::DOUBLEVECTOR || a->getType() == Attribute::TIMESERIES) {
                    current_tex = (a->getDoubleVector()[l.getAttributeVectorName()] - vmd.attr_min) / attr_span;
                } else {
                    current_tex = (a->getDouble() - vmd.attr_min) / attr_span;
                }
            } else {
                current_tex = 0.0;
            }
            return;
        }
        
        if (pos != in_between) return;
        
        Point_2 p(n->getX(), n->getY());
        polygon.push_back(p);
    }
    
    void render() {
        if (glIsTexture(l.getColorInterpretation())) {
            glEnable(GL_TEXTURE_1D);
            glBindTexture(GL_TEXTURE_1D, l.getColorInterpretation());
        }
        assert(glGetError() == GL_NO_ERROR);
        if(!polygon.is_simple()) {
            DM::Logger(DM::Error) << "Polygon is not simple can't perform tessilation";
            return;
        }
        if (polygon.is_clockwise_oriented())
            polygon.reverse_orientation();
        Polygon_list tesselated;
        Validity_traits validity_traits;

        CGAL::greene_approx_convex_partition_2(polygon.vertices_begin(), polygon.vertices_end(),
                                               std::back_inserter(tesselated), validity_traits);
        
        glPushName(name_start);
        foreach(Polygon_2 poly, tesselated) {
#if 1
            glBegin(GL_POLYGON);
            
            foreach(Point_2 p, poly.container()) {
                if (glIsTexture(l.getColorInterpretation())) {
                    glColor4f(1.0, 1.0, 1.0, 0.75);
                    glTexCoord1d(current_tex);
                } else {
                    glColor3f(0.0, 0.0, 0.0);
                }
                glVertex3d(p.x(), p.y(), current_height);
            }
#else
            glBegin(GL_LINE_STRIP);
            glColor3f(.0f, .0f, .0f);
            foreach(Point_2 p, poly.container()) {
                glVertex3d(p.x(), p.y(), 0);
            }
            Point_2 first = poly.container().front();
            glVertex3d(first.x(), first.y(), 0);
#endif
            
            glEnd();
        }
        glPopName();
        if (glIsTexture(l.getColorInterpretation())) glDisable(GL_TEXTURE_1D);
    }
};


Layer::Layer(System *s, View v, const std::string &a,  bool D3Ojbect, bool asMesh)
    : system(s), view(v),
      attribute(a), vmd(a),
      texture(-1),
      attribute_vector_name(0),
      scale_height(-1),
      as3DObject(D3Ojbect),
      asMesh(asMesh){

    QString attr = QString::fromStdString(a);
    QStringList view_attr = attr.split(":");

    if (view_attr.size() != 2)
        return;
    this->attributeView = *(system->getViewDefinition(view_attr[0].toStdString()));
    this->attribute = view_attr[1].toStdString();

    vmd = ViewMetaData(this->attribute);




}

struct GeomtryDrawer {

    GLuint name_start;

    GeomtryDrawer(const Layer &l) : name_start(l.getNameStart()) {

    }

    void operator()(DM::System *s, DM::View v, DM::Component *cmp, DM::Node *node,  iterator_pos pos) {
        if (pos == before) {
            glPushName(name_start);
            //glBegin(GL_LINE_STRIP);
            glBegin(GL_TRIANGLES);
            return;
        }
        if (pos == after) {
            glEnd();
            glPopName();
            name_start++;
            return;
        }
        DM::Node * n = (DM::Node*) node;
        glColor3f(node->getAttribute("r")->getDouble(), node->getAttribute("g")->getDouble(), node->getAttribute("b")->getDouble());
        const double tmp[3] = {n->getX(), n->getY(), n->getZ()};
        glVertex3dv(tmp);
    }
};

struct MeshDrawer {

    GLuint name_start;
    double current_tex;
    const Layer &l;
    double attr_span;

    MeshDrawer(const Layer &l)
        : l(l), name_start(l.getNameStart()) {


        if (l.getAttribute() == "") {
            return;
        }

        const ViewMetaData &vmd = l.getViewMetaData();
        this->attr_span = vmd.attr_max - vmd.attr_min;

    }

    void operator()(DM::System *s, DM::View v, DM::Component *cmp, DM::Node *node,  iterator_pos pos) {

        if (pos == before) {
            const ViewMetaData &vmd = l.getViewMetaData();
            this->attr_span = vmd.attr_max - vmd.attr_min;
            glPushName(name_start);
            glBegin(GL_TRIANGLES);
            return;
        }
        if (pos == after) {
            glEnd();
            glPopName();
            name_start++;
            return;
        }

        if (glIsTexture(l.getColorInterpretation())) {
            const ViewMetaData &vmd = l.getViewMetaData();
            Attribute *a = cmp->getAttribute(l.getAttribute());

            if (a->getType() == Attribute::DOUBLEVECTOR || a->getType() == Attribute::TIMESERIES) {
                current_tex = (a->getDoubleVector()[l.getAttributeVectorName()] - vmd.attr_min) / attr_span *255;
            } else {
                current_tex = (a->getDouble() - vmd.attr_min) / attr_span * 255;
            }
        } else {
            current_tex = 0.0;
        }

        DM::Node * n = (DM::Node*) node;


        const double tmp[3] = {n->getX(), n->getY(), n->getZ()};



        if (current_tex < 0) {
            glColor3f(0.0, 0.0, 0.0);
            glVertex3dv(tmp);
            return;
        }

        float r = l.LayerColor[(int)current_tex][0]/255.;
        float g = l.LayerColor[(int)current_tex][1]/255.;
        float b = l.LayerColor[(int)current_tex][2]/255.;
        float a = l.LayerColor[(int)current_tex][3]/255.;

        glColor3f(r, g, b);
        glVertex3dv(tmp);
    }
};


void Layer::draw(QWidget *parent) {
    if (lists.size() <= attribute_vector_name) {
        lists.resize(attribute_vector_name+1, -1);
    }
    if (!glIsList(lists[attribute_vector_name])) {
        lists[attribute_vector_name] = glGenLists(1);
        glNewList(lists[attribute_vector_name], GL_COMPILE);
        if (view.getType() == DM::COMPONENT || this->as3DObject == true) {
            GeomtryDrawer drawer(*this);
            iterate_components(system, view, drawer);
        }
        if (view.getType() == DM::FACE && this->asMesh) {
            MeshDrawer drawer(*this);
            iterate_mesh(system, view, drawer);
        }
        if (view.getType() == DM::FACE && !this->as3DObject && !this->asMesh) {
            TesselatedFaceDrawer drawer(*this, parent);
            iterate_faces(system, view, drawer);
        }
        if (view.getType() == DM::EDGE) {
            SimpleDrawer<GL_LINES> drawer(*this);
            iterate_edges(system, view, drawer);
        }
        if (view.getType() == DM::NODE) {
            SimpleDrawer<GL_POINTS> drawer(*this);
            iterate_nodes(system, view, drawer);
        }

        glEndList();
    }

    glPushMatrix();
    glTranslated(x_off, y_off, z_off);
    assert(glIsList(lists[attribute_vector_name]));
    glCallList(lists[attribute_vector_name]);
    glPopMatrix();
}

void Layer::drawWithNames(QWidget *parent) {
    draw(parent);
}

void Layer::systemChanged() {
    vmd = ViewMetaData(attribute);

    if (view.getType() == DM::COMPONENT || this->as3DObject) {
        iterate_components(system, view, vmd);
    }
    if (view.getType() == DM::FACE && !this->as3DObject && !this->asMesh) {
        iterate_faces(system, view, vmd);
    }
    if (view.getType() == DM::FACE || this->asMesh) {
        iterate_mesh(system, view, vmd);
    }
    if (view.getType() == DM::EDGE) {
        iterate_edges(system, view, vmd);
    }
    if (view.getType() == DM::NODE) {
        iterate_nodes(system, view, vmd);
    }

    foreach (GLuint list, lists) {
        if (glIsList(list)) {
            glDeleteLists(list, 1);
        }
    }
}


} // namespace DM
