/*!
 * Tree view
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) GUI.
 *
 * Copyright (C) 2011
 *     Kai Christian Bader <mail@kaibader.de>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tree.h"

#include <cassis/tree.h>
#include <cassis/namemap.h>

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QStyleOptionGraphicsItem>

#include <cmath>

Node::Node(Tree *tree, Node *parent) :
QGraphicsItem(parent), m_tree(tree), m_parent(parent), m_left(NULL), m_right(
        NULL), m_name(NULL), m_color(NULL), m_collapsed(false), m_branch_len(
                0.0), m_subtree_species(0), m_subtree_nodes(0), m_subtree_nodedepth(
                        0), m_subtree_depth(0.0), m_nodedepth(0) {
    setFlags(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

Node::~Node() {

}

QRectF Node::boundingRect() const {
    double y_dist = m_tree->m_y_distance;
    double x_corr = m_tree->m_len_correction;

    if (isLeaf()) {
        if (m_name && (m_name->length() > 0)) {
            QFontMetrics qfm(QFont("Helvetica", 8, -1, true));
            QSize size = qfm.size(0, *m_name);
            return QRectF(0, 0, size.width(), 10 + y_dist);
        }
        return QRectF(0, 0, 10, 10 + y_dist);
    } else if (m_collapsed) {
        double height = 20 * log((double) m_subtree_species);
        if (height < 20)
            height = 20;
        return QRectF(0, 0, (m_subtree_depth * x_corr), height + y_dist);
    }
    QRectF leftrect = m_left->boundingRect();
    QRectF rightrect = m_right->boundingRect();

    double width =
            (m_left->m_branch_len > m_right->m_branch_len) ?
                    m_left->m_branch_len * x_corr :
                    m_right->m_branch_len * x_corr;

    return QRectF(0, 0, width, leftrect.height() + rightrect.height() + y_dist);
}

QPainterPath Node::shape() const {
    double y_dist = m_tree->m_y_distance;
    double x_corr = m_tree->m_len_correction;

    QPainterPath path;

    if (isLeaf()) {
        if (m_name && (m_name->length() > 0)) {
            QFontMetrics qfm(QFont("Helvetica", 8, -1, true));
            QSize size = qfm.size(0, *m_name);
            path.addRect(0, 0, size.width(), 10 + y_dist);
        }
        path.addRect(0, 0, 10, 10 + y_dist);
    } else if (m_collapsed) {
        double height = 20 * log((double) m_subtree_species);
        if (height < 20)
            height = 20;
        path.addRect(0, 0, (m_subtree_depth * x_corr), height + y_dist);
    } else {
        QRectF leftrect = m_left->boundingRect();
        QRectF rightrect = m_right->boundingRect();

        double width =
                (m_left->m_branch_len > m_right->m_branch_len) ?
                        m_left->m_branch_len * x_corr :
                        m_right->m_branch_len * x_corr;

        path.addRect(0, 0, width,
                leftrect.height() + rightrect.height() + y_dist);
    }
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *item,
        QWidget*) {
    double y_dist = m_tree->m_y_distance;
    double x_corr = m_tree->m_len_correction;

    if (isLeaf()) {
        painter->save();

        QFont font("Helvetica", 8, -1, true);
        QFontMetrics qfm(font);
        QSize size = qfm.size(0, *m_name);
        int width = size.width();
        if (width < 10)
            width = 10;

        if ((item->state & QStyle::State_MouseOver)
                || (item->state & QStyle::State_Selected)) {
            QColor fillcolor(255, 127, 127);
            painter->fillRect(QRectF(0, (y_dist / 2), width, 10), fillcolor);
        }

        if (m_name->length() > 0) {
            painter->setFont(font);
            painter->setPen(QPen(Qt::black));
            painter->drawText(0, (y_dist / 2) + 9, *m_name);
        }

        painter->restore();
    } else if (m_collapsed) {
        painter->save();
        QColor fillcolor;
        if (m_color)
            fillcolor = *m_color;
        else
            fillcolor.setHsv(180, 50, 250);

        //if ((item->state & QStyle::State_MouseOver)
        //        || (item->state & QStyle::State_Selected))
        if (item->state & QStyle::State_MouseOver)
            fillcolor = fillcolor.darker(125);

        QPen borderPen(Qt::black);
        if (item->state & QStyle::State_Selected)
            borderPen.setColor(QColor(63, 63, 255));

        double height = 20 * log((double) m_subtree_species);
        if (height < 20)
            height = 20;
        painter->fillRect(
                QRectF(0, (y_dist / 2), (m_subtree_depth * x_corr), height),
                fillcolor);
        painter->setPen(borderPen);
        painter->drawRect(
                QRectF(0, (y_dist / 2), (m_subtree_depth * x_corr), height));

        // Group information
        QString str = QString("(%1)").arg(m_subtree_species);
        if (m_name && (m_name->length() > 0)) {
            str.prepend(" ");
            str.prepend(*m_name);
        }
        painter->setFont(QFont("Helvetica", 8));
        painter->drawText(1, 10 + (y_dist / 2), str);
        painter->restore();
    } else {
        painter->save();

        QRectF leftrect = m_left->boundingRect();
        QRectF rightrect = m_right->boundingRect();

        if ((item->state & QStyle::State_MouseOver)
                || (item->state & QStyle::State_Selected)) {
            QColor fillcolor = QColor(127, 127, 255);

            double width =
                    (m_left->m_branch_len > m_right->m_branch_len) ?
                            m_left->m_branch_len * x_corr :
                            m_right->m_branch_len * x_corr;

            painter->fillRect(
                    QRectF(0, (y_dist / 2), width,
                            leftrect.height() + rightrect.height()), fillcolor);
        }

        painter->setPen(QPen(Qt::black));

        // Draw branches...
        painter->drawLine(0, leftrect.height() / 2 + (y_dist / 2), 0,
                leftrect.height() + rightrect.height() / 2 + (y_dist / 2));
        painter->drawLine(0, leftrect.height() / 2 + (y_dist / 2),
                m_left->m_branch_len * x_corr,
                leftrect.height() / 2 + (y_dist / 2));
        painter->drawLine(0,
                leftrect.height() + rightrect.height() / 2 + (y_dist / 2),
                m_right->m_branch_len * x_corr,
                leftrect.height() + rightrect.height() / 2 + (y_dist / 2));

        //if (m_name->length() > 0) {
        //    painter->save();
        //    painter->translate(0, 0);
        //    painter->rotate(90);
        //    painter->setFont(QFont("Helvetica", 8));
        //    painter->setPen(QPen(Qt::black));
        //    painter->drawText(0, 0, *m_name);
        //    painter->restore();
        //}

        painter->restore();
    }
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if ((event->button() == Qt::RightButton) && !isLeaf()) {
        // Left button was pressed: collapse/expand group node.
        prepareGeometryChange();
        if (m_parent)
            m_parent->prepareGeometryChange();

        // Toggle: collapsed <--> expanded
        m_collapsed = !m_collapsed;

        // make children (in)visible depending on the parents collapse state.
        if (m_left)
            m_left->setSubtreeVisibility(!m_collapsed);
        if (m_right)
            m_right->setSubtreeVisibility(!m_collapsed);

        // TODO: Fix scenery - disabled!
        ((Tree*) scene())->m_tree_has_changed = true;
    } else if ((event->button() == Qt::LeftButton)
            && (flags() & ItemIsSelectable)) {
        // Right button was pressed: select the node.
        // QGraphicsItem::mousePressEvent(event);

        bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
        if (!multiSelect && !this->isSelected()) {
            if (QGraphicsScene *my_scene = this->scene()) {
                my_scene->clearSelection();
            }
            setSelected(true);
        }
    }
    // QGraphicsItem::mousePressEvent(event);
    // update();
    event->accept();
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // TODO: Do we need to catch this event?
    // QGraphicsItem::mouseMoveEvent(event);
    event->accept();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // TODO: Do we need to catch this event?
    // QGraphicsItem::mouseReleaseEvent(event);
    // update();
    event->accept();
}

bool Node::isLeaf() const {
    return (!m_left && !m_right);
}

void Node::setSubtreeVisibility(bool state) {
    if (state != isVisible())
        setVisible(state);
    if (m_left)
        m_left->setSubtreeVisibility(state);
    if (m_right)
        m_right->setSubtreeVisibility(state);
}

Tree::Tree(const CaSSiSTree *cassis_tree, NameMap &map) :
                m_node_counter(0), m_species_counter(0), m_tree_depth(0.0), m_tree_nodedepth(
                        0), m_root(NULL), m_tree_has_changed(false), m_autoexpand_unnamed_nodes(
                                true), m_len_correction(1.0), m_y_distance(10.0) {

    // Do not build anything if no tree is given.
    if (!cassis_tree || !cassis_tree->tree_root)
        return;

    // Parse the BGRT tree and rebuild the tree in this format.
    m_root = copy_subtree(NULL, cassis_tree->tree_root, map, 0);

    // Make the root node visible and display the resulting tree...
    m_root->setVisible(true);
    m_root->m_collapsed = false;
    m_len_correction = 1000 / m_tree_depth;
    if (m_len_correction == 0.0)
        m_len_correction = 1.0;
    display_subtree(m_root, 0.0, 0.0);

    connect(this, SIGNAL(changed(const QList<QRectF> &)), this,
            SLOT(treeUpdate()));
}

Tree::~Tree() {

}

Node *Tree::copy_subtree(Node *parent_node, CaSSiSTreeNode *cassis_node,
        NameMap &map, unsigned int depth) {
    // Return NULL, if we have reached an empty (child) node pointer.
    if (!cassis_node)
        return NULL;

    // Create a new node and increase the overall node counter.
    Node *node = new Node(this, parent_node);
    ++m_node_counter;

    // Initialize the node and the overall tree depth.
    node->m_nodedepth = depth;
    if (m_tree_nodedepth < depth)
        m_tree_nodedepth = depth;

    // Set the z-value. Nodes with a higher z-value (deeper in the tree)
    // will be displayed on top of the others.
    node->setZValue((double) depth);

    // Copy the node name and the identifier (if available).
    node->m_name = new QString(map.name(cassis_node->this_id).c_str());

    // Copy the branch length.
    node->m_branch_len = cassis_node->length;

    // Nodes with a depth > 5 are collapsed by default.
    if (depth > 5)
        node->m_collapsed = true;

    // Dive into the subtrees...
    node->m_left = copy_subtree(node, cassis_node->left, map, depth + 1);
    node->m_right = copy_subtree(node, cassis_node->right, map, depth + 1);

    // Update the node statistics...
    node->m_subtree_nodes = 1;
    node->m_subtree_depth = 0.0;
    node->m_subtree_nodedepth = 0;

    if (node->isLeaf()) {
        // Leaf...
        node->m_subtree_species = 1;
        ++m_species_counter;
    } else {
        // Inner node...
        if (node->m_left) {
            node->m_subtree_species += node->m_left->m_subtree_species;
            node->m_subtree_nodes += node->m_left->m_subtree_nodes;
            node->m_subtree_depth = node->m_left->m_subtree_depth
                    + node->m_left->m_branch_len;
            node->m_subtree_nodedepth = node->m_left->m_subtree_nodedepth + 1;
        }
        if (node->m_right) {
            node->m_subtree_species += node->m_right->m_subtree_species;
            node->m_subtree_nodes += node->m_right->m_subtree_nodes;

            double right_len = node->m_right->m_subtree_depth
                    + node->m_right->m_branch_len;
            if (node->m_subtree_depth < right_len)
                node->m_subtree_depth = right_len;

            unsigned int right_depth = node->m_right->m_subtree_nodedepth + 1;
            if (node->m_subtree_nodedepth < right_depth)
                node->m_subtree_nodedepth = right_depth;
        }
    }

    // Fetch tree depth (in changes).
    if (m_tree_depth < node->m_subtree_depth)
        m_tree_depth = node->m_subtree_depth;

    // Add the node item to the tree.
    node->setPos(0, 0);
    this->addItem(node);
    node->setVisible(false);

    // Return current node.
    return node;
}

double Tree::display_subtree(Node *node, double at_x, double at_y) {
    if (!node)
        return 0.0;

    // Define a group node color
    // TODO: The color should be on a log scale...
    if (!node->isLeaf() && !node->m_color) {
        node->m_color = new QColor();
        node->m_color->setHsv(
                ((double) node->m_subtree_species * 359
                        / (double) m_species_counter), 50, 250);
    }

    if (node->m_collapsed) {
        // Also hide child nodes, if the current node is collapsed...
        if (node->m_left)
            node->m_left->setSubtreeVisibility(false);
        if (node->m_right)
            node->m_right->setSubtreeVisibility(false);
        node->setPos(at_x, at_y);
        node->setVisible(true);
    } else {
        // Node is not collapsed...
        if (!node->isLeaf()) {

            // We are a group node...
            display_subtree(node->m_left,
                    at_x + (node->m_left->m_branch_len * m_len_correction),
                    at_y);

            QRectF left_rect = node->m_left->boundingRect();
            display_subtree(node->m_right,
                    at_x + (node->m_right->m_branch_len * m_len_correction),
                    at_y + left_rect.height());
        }
        node->setPos(at_x, at_y);
        node->setVisible(true);
    }
    return 0.0;
}

unsigned int Tree::numNodes() {
    return m_node_counter;
}

unsigned int Tree::numSpecies() {
    return m_species_counter;
}

unsigned int Tree::nodeDepth() {
    return m_tree_nodedepth;
}

QStringList Tree::selectedIdentifiers() {

    QStringList list;
    addSubtreeIdentifiers(m_root, list);
    return list;
}

void Tree::addSubtreeIdentifiers(Node *node, QStringList &list,
        bool group_selected) {
    if (node->isLeaf() && (group_selected || node->isSelected())) {
        list.append(*(node->m_name));
    } else {
        if (node->m_left)
            addSubtreeIdentifiers(node->m_left, list,
                    group_selected | node->isSelected());
        if (node->m_right)
            addSubtreeIdentifiers(node->m_right, list,
                    group_selected | node->isSelected());
    }
}

void Tree::treeUpdate() {
    if (m_tree_has_changed) {
        display_subtree(m_root, 0.0, 0.0);
        update();
        m_tree_has_changed = false;
    }
}
